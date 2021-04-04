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

#pragma once

#include "qtacrylichelper_global.h"
#include <QtGui/qcolor.h>
#include <QtGui/qwindow.h>

namespace _qam::Utilities {

enum class DesktopWallpaperAspectStyle
{
    Central,
    Tiled,
    IgnoreRatioFit, // Stretch
    KeepRatioFit, // Fit
    KeepRatioByExpanding, // Fill
    Span
};

// Common
QTACRYLICHELPER_API bool shouldUseWallpaperBlur();
QTACRYLICHELPER_API bool shouldUseTraditionalBlur();
QTACRYLICHELPER_API bool setBlurEffectEnabled(const QWindow *window, const bool enabled, const QColor &gradientColor = {});

QTACRYLICHELPER_API bool isDarkThemeEnabled();

QTACRYLICHELPER_API QWindow *findWindow(const WId winId);

QTACRYLICHELPER_API QImage getDesktopWallpaperImage(const int screen = -1);
QTACRYLICHELPER_API QColor getDesktopBackgroundColor(const int screen = -1);
QTACRYLICHELPER_API DesktopWallpaperAspectStyle getDesktopWallpaperAspectStyle(const int screen = -1);

QTACRYLICHELPER_API QRect alignedRect(const Qt::LayoutDirection direction, const Qt::Alignment alignment, const QSize &size, const QRect &rectangle);

QTACRYLICHELPER_API void blurImage(QImage &blurImage, const qreal radius, const bool quality, const int transposed = 0);
QTACRYLICHELPER_API void blurImage(QPainter *painter, QImage &blurImage, const qreal radius, const bool quality, const bool alphaOnly, const int transposed = 0);

QTACRYLICHELPER_API bool disableExtraProcessingForBlur();
QTACRYLICHELPER_API bool forceEnableTraditionalBlur();
QTACRYLICHELPER_API bool forceDisableTraditionalBlur();
QTACRYLICHELPER_API bool forceEnableWallpaperBlur();
QTACRYLICHELPER_API bool forceDisableWallpaperBlur();

#ifdef Q_OS_WINDOWS
// Windows specific
QTACRYLICHELPER_API bool isWin8OrGreater();
QTACRYLICHELPER_API bool isWin10OrGreater();
QTACRYLICHELPER_API bool isWin10OrGreater(const int subVer);

QTACRYLICHELPER_API bool isOfficialMSWin10AcrylicBlurAvailable();

QTACRYLICHELPER_API QColor getColorizationColor();
#endif

}
