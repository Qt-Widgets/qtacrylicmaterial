/*
 * MIT License
 *
 * Copyright (C) 2021 by wangwenx190 (Yuhang Zhao)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "utilities.h"
#include <QtCore/qsettings.h>
#include <QtCore/qlibrary.h>
#include <QtCore/qt_windows.h>
#include <QtCore/qdebug.h>
#include <QtCore/qfileinfo.h>
#include <dwmapi.h>
#include <shobjidl_core.h>
#include <wininet.h>
#include <shlobj_core.h>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
#include <QtCore/qoperatingsystemversion.h>
#else
#include <QtCore/qsysinfo.h>
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
#include <QtCore/qscopeguard.h>
#endif

using WINDOWCOMPOSITIONATTRIB = enum _WINDOWCOMPOSITIONATTRIB
{
    WCA_ACCENT_POLICY = 19
};

using WINDOWCOMPOSITIONATTRIBDATA = struct _WINDOWCOMPOSITIONATTRIBDATA
{
    WINDOWCOMPOSITIONATTRIB Attrib;
    PVOID pvData;
    SIZE_T cbData;
};

using ACCENT_STATE = enum _ACCENT_STATE
{
    ACCENT_DISABLED = 0,
    ACCENT_ENABLE_GRADIENT = 1,
    ACCENT_ENABLE_TRANSPARENTGRADIENT = 2,
    ACCENT_ENABLE_BLURBEHIND = 3,
    ACCENT_ENABLE_ACRYLICBLURBEHIND = 4, // RS4 1803
    ACCENT_ENABLE_HOSTBACKDROP = 5, // RS5 1809
    ACCENT_INVALID_STATE = 6
};

using ACCENT_POLICY = struct _ACCENT_POLICY
{
    ACCENT_STATE AccentState;
    DWORD AccentFlags;
    COLORREF GradientColor;
    DWORD AnimationId;
};

static const QString g_dwmRegistryKey = QStringLiteral(R"(HKEY_CURRENT_USER\Software\Microsoft\Windows\DWM)");
static const QString g_desktopRegistryKey = QStringLiteral(R"(HKEY_CURRENT_USER\Control Panel\Desktop)");

using SetWindowCompositionAttributePtr = BOOL(WINAPI *)(HWND, WINDOWCOMPOSITIONATTRIBDATA *);

using Win32Data = struct _QAH_UTILITIES_WIN32_DATA
{
    SetWindowCompositionAttributePtr SetWindowCompositionAttributePFN = nullptr;

    _QAH_UTILITIES_WIN32_DATA()
    {
        load();
    }

    void load()
    {
        QLibrary User32Dll(QStringLiteral("User32"));
        SetWindowCompositionAttributePFN = reinterpret_cast<SetWindowCompositionAttributePtr>(User32Dll.resolve("SetWindowCompositionAttribute"));
    }
};

Q_GLOBAL_STATIC(Win32Data, win32Data)

bool Utilities::isDwmBlurAvailable()
{
    if (isWin8OrGreater()) {
        return true;
    }
    BOOL enabled = FALSE;
    if (FAILED(DwmIsCompositionEnabled(&enabled))) {
        qWarning() << "DwmIsCompositionEnabled failed.";
        return false;
    }
    return isWin7OrGreater() && (enabled != FALSE);
}

bool Utilities::setBlurEffectEnabled(const QWindow *window, const bool enabled, const QColor &gradientColor)
{
    Q_ASSERT(window);
    if (!window) {
        return false;
    }
    const auto hwnd = reinterpret_cast<HWND>(window->winId());
    Q_ASSERT(hwnd);
    if (!hwnd) {
        return false;
    }
    bool result = false;
    // We prefer DwmEnableBlurBehindWindow on Windows 7.
    if (isWin8OrGreater() && win32Data()->SetWindowCompositionAttributePFN) {
        ACCENT_POLICY accentPolicy;
        SecureZeroMemory(&accentPolicy, sizeof(accentPolicy));
        WINDOWCOMPOSITIONATTRIBDATA wcaData;
        SecureZeroMemory(&wcaData, sizeof(wcaData));
        wcaData.Attrib = WCA_ACCENT_POLICY;
        wcaData.pvData = &accentPolicy;
        wcaData.cbData = sizeof(accentPolicy);
        if (enabled) {
            // The gradient color must be set otherwise it'll look like a classic blur.
            // Use semi-transparent gradient color to get better appearance.
            if (gradientColor.isValid()) {
                accentPolicy.GradientColor = qRgba(gradientColor.blue(), gradientColor.green(), gradientColor.red(), gradientColor.alpha());
            } else {
                const QColor colorizationColor = getColorizationColor();
                accentPolicy.GradientColor =
                    RGB(qRound(colorizationColor.red() * (colorizationColor.alpha() / 255.0) + 255 - colorizationColor.alpha()),
                        qRound(colorizationColor.green() * (colorizationColor.alpha() / 255.0) + 255 - colorizationColor.alpha()),
                        qRound(colorizationColor.blue() * (colorizationColor.alpha() / 255.0) + 255 - colorizationColor.alpha()));
            }
            if (isOfficialMSWin10AcrylicBlurAvailable()) {
                accentPolicy.AccentState = ACCENT_ENABLE_ACRYLICBLURBEHIND;
                if (!gradientColor.isValid()) {
                    accentPolicy.GradientColor = 0x01FFFFFF;
                }
            } else {
                accentPolicy.AccentState = ACCENT_ENABLE_BLURBEHIND;
            }
        } else {
            accentPolicy.AccentState = ACCENT_DISABLED;
        }
        result = (win32Data()->SetWindowCompositionAttributePFN(hwnd, &wcaData) != FALSE);
        if (!result) {
            qWarning() << "SetWindowCompositionAttribute failed.";
        }
    } else {
        DWM_BLURBEHIND dwmBB;
        SecureZeroMemory(&dwmBB, sizeof(dwmBB));
        dwmBB.dwFlags = DWM_BB_ENABLE;
        dwmBB.fEnable = enabled ? TRUE : FALSE;
        result = SUCCEEDED(DwmEnableBlurBehindWindow(hwnd, &dwmBB));
        if (!result) {
            qWarning() << "DwmEnableBlurBehindWindow failed.";
        }
    }
    if (result) {
        const auto win = const_cast<QWindow *>(window);
        win->setProperty(_qah_global::_qah_blurEnabled_flag, enabled);
        win->setProperty(_qah_global::_qah_gradientColor_flag, gradientColor);
    }
    return result;
}

QColor Utilities::getColorizationColor()
{
    DWORD color = 0;
    BOOL opaqueBlend = FALSE;
    if (SUCCEEDED(DwmGetColorizationColor(&color, &opaqueBlend))) {
        return QColor::fromRgba(color);
    }
    qWarning() << "DwmGetColorizationColor failed, reading from the registry instead.";
    bool ok = false;
    const QSettings settings(g_dwmRegistryKey, QSettings::NativeFormat);
    const DWORD value = settings.value(QStringLiteral("ColorizationColor"), 0).toULongLong(&ok);
    return ok ? QColor::fromRgba(value) : Qt::darkGray;
}

QImage Utilities::getDesktopWallpaperImage(const int screen)
{
    if (isWin8OrGreater()) {
        if (SUCCEEDED(CoInitialize(nullptr))) {
            IDesktopWallpaper* pDesktopWallpaper = nullptr;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
            const auto cleanup = qScopeGuard([pDesktopWallpaper](){
                if (pDesktopWallpaper) {
                    pDesktopWallpaper->Release();
                }
                CoUninitialize();
            });
#endif
            // TODO: Why CLSCTX_INPROC_SERVER failed?
            if (SUCCEEDED(CoCreateInstance(CLSID_DesktopWallpaper, nullptr, CLSCTX_LOCAL_SERVER, IID_IDesktopWallpaper, reinterpret_cast<void **>(&pDesktopWallpaper))) && pDesktopWallpaper) {
                UINT monitorCount = 0;
                if (SUCCEEDED(pDesktopWallpaper->GetMonitorDevicePathCount(&monitorCount))) {
                    if (screen > int(monitorCount - 1)) {
                        qWarning() << "Screen number above total screen count.";
                        return {};
                    }
                    const UINT monitorIndex = qMax(screen, 0);
                    LPWSTR monitorId = nullptr;
                    if (SUCCEEDED(pDesktopWallpaper->GetMonitorDevicePathAt(monitorIndex, &monitorId)) && monitorId) {
                        LPWSTR wallpaperPath = nullptr;
                        if (SUCCEEDED(pDesktopWallpaper->GetWallpaper(monitorId, &wallpaperPath)) && wallpaperPath) {
                            CoTaskMemFree(monitorId);
                            const QString _path = QString::fromWCharArray(wallpaperPath);
                            CoTaskMemFree(wallpaperPath);
                            return QImage(_path);
                        } else {
                            CoTaskMemFree(monitorId);
                            qWarning() << "IDesktopWallpaper::GetWallpaper() failed.";
                        }
                    } else {
                        qWarning() << "IDesktopWallpaper::GetMonitorDevicePathAt() failed";
                    }
                } else {
                    qWarning() << "IDesktopWallpaper::GetMonitorDevicePathCount() failed";
                }
            } else {
                qWarning() << "Failed to create COM instance - DesktopWallpaper.";
            }
        } else {
            qWarning() << "Failed to initialize COM.";
        }
        qDebug() << "The IDesktopWallpaper interface failed. Trying the IActiveDesktop interface instead.";
    }
    if (SUCCEEDED(CoInitialize(nullptr))) {
        IActiveDesktop *pActiveDesktop = nullptr;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
        const auto cleanup = qScopeGuard([pActiveDesktop](){
            if (pActiveDesktop) {
                pActiveDesktop->Release();
            }
            CoUninitialize();
        });
#endif
        if (SUCCEEDED(CoCreateInstance(CLSID_ActiveDesktop, nullptr, CLSCTX_INPROC_SERVER, IID_IActiveDesktop, reinterpret_cast<void **>(&pActiveDesktop))) && pActiveDesktop) {
            const auto wallpaperPath = new WCHAR[MAX_PATH];
            // TODO: AD_GETWP_BMP, AD_GETWP_IMAGE, AD_GETWP_LAST_APPLIED. What's the difference?
            if (SUCCEEDED(pActiveDesktop->GetWallpaper(wallpaperPath, MAX_PATH, AD_GETWP_LAST_APPLIED))) {
                const QString _path = QString::fromWCharArray(wallpaperPath);
                delete [] wallpaperPath;
                return QImage(_path);
            } else {
                qWarning() << "IActiveDesktop::GetWallpaper() failed.";
            }
        } else {
            qWarning() << "Failed to create COM instance - ActiveDesktop.";
        }
    } else {
        qWarning() << "Failed to initialize COM.";
    }
    qDebug() << "Shell API failed. Using SystemParametersInfoW instead.";
    const auto wallpaperPath = new WCHAR[MAX_PATH];
    if (SystemParametersInfoW(SPI_GETDESKWALLPAPER, MAX_PATH, wallpaperPath, 0) != FALSE) {
        const QString _path = QString::fromWCharArray(wallpaperPath);
        delete [] wallpaperPath;
        return QImage(_path);
    }
    qWarning() << "SystemParametersInfoW failed. Reading from the registry instead.";
    const QSettings settings(g_desktopRegistryKey, QSettings::NativeFormat);
    const QString path = settings.value(QStringLiteral("WallPaper")).toString();
    if (QFileInfo::exists(path)) {
        return QImage(path);
    }
    qWarning() << "Failed to read the registry.";
    return {};
}

QColor Utilities::getDesktopBackgroundColor(const int screen)
{
    Q_UNUSED(screen);
    if (isWin8OrGreater()) {
        if (SUCCEEDED(CoInitialize(nullptr))) {
            IDesktopWallpaper *pDesktopWallpaper = nullptr;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
            const auto cleanup = qScopeGuard([pDesktopWallpaper]() {
                if (pDesktopWallpaper) {
                    pDesktopWallpaper->Release();
                }
                CoUninitialize();
            });
#endif
            // TODO: Why CLSCTX_INPROC_SERVER failed?
            if (SUCCEEDED(CoCreateInstance(CLSID_DesktopWallpaper, nullptr, CLSCTX_LOCAL_SERVER, IID_IDesktopWallpaper, reinterpret_cast<void **>(&pDesktopWallpaper))) && pDesktopWallpaper) {
                COLORREF color = 0;
                if (SUCCEEDED(pDesktopWallpaper->GetBackgroundColor(&color))) {
                    return QColor::fromRgba(color);
                } else {
                    qWarning() << "IDesktopWallpaper::GetBackgroundColor() failed.";
                }
            } else {
                qWarning() << "Failed to create COM instance - DesktopWallpaper.";
            }
        } else {
            qWarning() << "Failed to initialize COM.";
        }
        qDebug() << "The IDesktopWallpaper interface failed.";
    }
    // TODO: Is there any other way to get the background color? Traditional Win32 API? Registry?
    // Is there a Shell API for Win7?
    return Qt::black;
}

Utilities::DesktopWallpaperAspectStyle Utilities::getDesktopWallpaperAspectStyle(const int screen)
{
    Q_UNUSED(screen);
    if (isWin8OrGreater()) {
        if (SUCCEEDED(CoInitialize(nullptr))) {
            IDesktopWallpaper *pDesktopWallpaper = nullptr;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
            const auto cleanup = qScopeGuard([pDesktopWallpaper](){
                if (pDesktopWallpaper) {
                    pDesktopWallpaper->Release();
                }
                CoUninitialize();
            });
#endif
            // TODO: Why CLSCTX_INPROC_SERVER failed?
            if (SUCCEEDED(CoCreateInstance(CLSID_DesktopWallpaper, nullptr, CLSCTX_LOCAL_SERVER, IID_IDesktopWallpaper, reinterpret_cast<void **>(&pDesktopWallpaper))) && pDesktopWallpaper) {
                DESKTOP_WALLPAPER_POSITION position = DWPOS_FILL;
                if (SUCCEEDED(pDesktopWallpaper->GetPosition(&position))) {
                    switch (position) {
                    case DWPOS_CENTER:
                        return DesktopWallpaperAspectStyle::Central;
                    case DWPOS_TILE:
                        return DesktopWallpaperAspectStyle::Tiled;
                    case DWPOS_STRETCH:
                        return DesktopWallpaperAspectStyle::IgnoreRatioFit;
                    case DWPOS_FIT:
                        return DesktopWallpaperAspectStyle::KeepRatioFit;
                    case DWPOS_FILL:
                        return DesktopWallpaperAspectStyle::KeepRatioByExpanding;
                    case DWPOS_SPAN:
                        return DesktopWallpaperAspectStyle::Span;
                    }
                } else {
                    qWarning() << "IDesktopWallpaper::GetPosition() failed.";
                }
            } else {
                qWarning() << "Failed to create COM instance - DesktopWallpaper.";
            }
        } else {
            qWarning() << "Failed to initialize COM.";
        }
        qDebug() << "The IDesktopWallpaper interface failed. Trying the IActiveDesktop interface instead.";
    }
    if (SUCCEEDED(CoInitialize(nullptr))) {
        IActiveDesktop *pActiveDesktop = nullptr;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
        const auto cleanup = qScopeGuard([pActiveDesktop](){
            if (pActiveDesktop) {
                pActiveDesktop->Release();
            }
            CoUninitialize();
        });
#endif
        if (SUCCEEDED(CoCreateInstance(CLSID_ActiveDesktop, nullptr, CLSCTX_INPROC_SERVER, IID_IActiveDesktop, reinterpret_cast<void **>(&pActiveDesktop))) && pActiveDesktop) {
            WALLPAPEROPT opt;
            SecureZeroMemory(&opt, sizeof(opt));
            opt.dwSize = sizeof(opt);
            if (SUCCEEDED(pActiveDesktop->GetWallpaperOptions(&opt, 0))) {
                switch (opt.dwStyle) {
                case WPSTYLE_CENTER:
                    return DesktopWallpaperAspectStyle::Central;
                case WPSTYLE_TILE:
                    return DesktopWallpaperAspectStyle::Tiled;
                case WPSTYLE_STRETCH:
                    return DesktopWallpaperAspectStyle::IgnoreRatioFit;
                case WPSTYLE_KEEPASPECT:
                    return DesktopWallpaperAspectStyle::KeepRatioFit;
                case WPSTYLE_CROPTOFIT:
                    return DesktopWallpaperAspectStyle::KeepRatioByExpanding;
                case WPSTYLE_SPAN:
                    return DesktopWallpaperAspectStyle::Span;
                }
            } else {
                qWarning() << "IActiveDesktop::GetWallpaperOptions() failed.";
            }
        } else {
            qWarning() << "Failed to create COM instance - ActiveDesktop.";
        }
    } else {
        qWarning() << "Failed to initialize COM.";
    }
    qDebug() << "Shell API failed. Reading from the registry instead.";
    const QSettings settings(g_desktopRegistryKey, QSettings::NativeFormat);
    bool ok = false;
    const DWORD style = settings.value(QStringLiteral("WallpaperStyle"), 0).toULongLong(&ok);
    if (!ok) {
        qWarning() << "Failed to read the registry.";
        return DesktopWallpaperAspectStyle::KeepRatioByExpanding; // Fill
    }
    switch (style) {
    case 0: {
        bool ok = false;
        if ((settings.value(QStringLiteral("TileWallpaper"), 0).toULongLong(&ok) != 0) && ok) {
            return DesktopWallpaperAspectStyle::Tiled;
        } else {
            return DesktopWallpaperAspectStyle::Central;
        }
    }
    case 2:
        return DesktopWallpaperAspectStyle::IgnoreRatioFit;
    case 6:
        return DesktopWallpaperAspectStyle::KeepRatioFit;
    case 10:
        return DesktopWallpaperAspectStyle::KeepRatioByExpanding;
    case 22:
        return DesktopWallpaperAspectStyle::Span;
    default:
        return DesktopWallpaperAspectStyle::KeepRatioByExpanding; // Fill
    }
}

bool Utilities::isWin7OrGreater()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
    return QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows7;
#else
    return QSysInfo::WindowsVersion >= QSysInfo::WV_WINDOWS7;
#endif
}

bool Utilities::isWin8OrGreater()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
    return QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows8;
#else
    return QSysInfo::WindowsVersion >= QSysInfo::WV_WINDOWS8;
#endif
}

bool Utilities::isWin8Point1OrGreater()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
    return QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows8_1;
#else
    return QSysInfo::WindowsVersion >= QSysInfo::WV_WINDOWS8_1;
#endif
}

bool Utilities::isWin10OrGreater()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
    return QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows10;
#else
    return QSysInfo::WindowsVersion >= QSysInfo::WV_WINDOWS10;
#endif
}

bool Utilities::isWin10OrGreater(const int subVer)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
    return QOperatingSystemVersion::current() >= QOperatingSystemVersion(QOperatingSystemVersion::Windows, 10, 0, subVer);
#else
    Q_UNUSED(ver);
    return QSysInfo::WindowsVersion >= QSysInfo::WV_WINDOWS10;
#endif
}

static inline bool forceEnableOfficialMSWin10AcrylicBlur()
{
    return qEnvironmentVariableIsSet(_qah_global::_qah_forceEnableOfficialMSWin10AcrylicBlur_flag);
}

static inline bool shouldUseOfficialMSWin10AcrylicBlur()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
    const QOperatingSystemVersion currentVersion = QOperatingSystemVersion::current();
    if (currentVersion > QOperatingSystemVersion::Windows10) {
        return true;
    }
    return ((currentVersion.microVersion() >= 16190) && (currentVersion.microVersion() < 18362));
#else
    // TODO
    return false;
#endif
}

bool Utilities::isOfficialMSWin10AcrylicBlurAvailable()
{
    if (!isWin10OrGreater()) {
        return false;
    }
    if (!forceEnableTraditionalBlur() && !forceDisableWallpaperBlur() && !disableExtraProcessingForBlur()) {
        // We can't enable the official Acrylic blur in wallpaper blur mode.
        return false;
    }
    if (forceEnableOfficialMSWin10AcrylicBlur()) {
        return true;
    }
    return shouldUseOfficialMSWin10AcrylicBlur();
}

static inline bool shouldUseOriginalDwmBlur()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
    return Utilities::isWin10OrGreater() || (QOperatingSystemVersion::current() >= QOperatingSystemVersion::OSXYosemite);
#else
    // TODO
    return false;
#endif
}

bool Utilities::shouldUseTraditionalBlur()
{
    if ((forceEnableTraditionalBlur() || forceDisableWallpaperBlur() || disableExtraProcessingForBlur()) && shouldUseOriginalDwmBlur()) {
        return true;
    }
    return false;
}
