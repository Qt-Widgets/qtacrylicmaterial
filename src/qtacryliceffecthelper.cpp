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

#include "qtacryliceffecthelper.h"
#include "utilities.h"
#include <QtGui/qpainter.h>
#include <QtCore/qdebug.h>
#include <QtGui/qguiapplication.h>
#include <QtGui/qscreen.h>

QtAcrylicEffectHelper::QtAcrylicEffectHelper()
{
    Q_INIT_RESOURCE(qtacrylichelper);
    QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
#ifdef Q_OS_MACOS
    if (Utilities::shouldUseTraditionalBlur()) {
        m_tintOpacity = 0.6;
    }
#endif
}

QtAcrylicEffectHelper::~QtAcrylicEffectHelper() = default;

void QtAcrylicEffectHelper::showPerformanceWarning() const
{
    qDebug() << "The Acrylic blur effect has been enabled. Rendering acrylic material surfaces is highly GPU-intensive, which can slow down the application, increase the power consumption on the devices on which the application is running.";
}

void QtAcrylicEffectHelper::regenerateWallpaper()
{
    if (!m_bluredWallpaper.isNull()) {
        m_bluredWallpaper = {};
    }
    generateBluredWallpaper();
}

QBrush QtAcrylicEffectHelper::getAcrylicBrush() const
{
    return m_acrylicBrush;
}

QColor QtAcrylicEffectHelper::getTintColor() const
{
    return m_tintColor;
}

qreal QtAcrylicEffectHelper::getTintOpacity() const
{
    return m_tintOpacity;
}

qreal QtAcrylicEffectHelper::getNoiseOpacity() const
{
    return m_noiseOpacity;
}

QPixmap QtAcrylicEffectHelper::getBluredWallpaper() const
{
    return m_bluredWallpaper;
}

void QtAcrylicEffectHelper::setTintColor(const QColor &value)
{
    if (!value.isValid()) {
        qWarning() << value << "is not a valid color.";
        return;
    }
    if (m_tintColor != value) {
        m_tintColor = value;
    }
}

void QtAcrylicEffectHelper::setTintOpacity(const qreal value)
{
    if (m_tintOpacity != value) {
        m_tintOpacity = value;
    }
}

void QtAcrylicEffectHelper::setNoiseOpacity(const qreal value)
{
    if (m_noiseOpacity != value) {
        m_noiseOpacity = value;
    }
}

void QtAcrylicEffectHelper::paintBackground(QPainter *painter, const QRect &rect)
{
    Q_ASSERT(painter);
    Q_ASSERT(rect.isValid());
    if (!painter || !rect.isValid()) {
        return;
    }
    // TODO: should we limit it to Win32 only? Or should we do something about the
    // acrylic brush instead?
    if (Utilities::disableExtraProcessingForBlur()) {
        return;
    }
    painter->save();
    const QRect maskRect = {QPoint{0, 0}, rect.size()};
    if (Utilities::shouldUseTraditionalBlur()) {
        const QPainter::CompositionMode mode = painter->compositionMode();
        painter->setCompositionMode(QPainter::CompositionMode_Clear);
        painter->fillRect(maskRect, Qt::white);
        painter->setCompositionMode(mode);
    } else {
        // Emulate blur behind window by blurring the desktop wallpaper.
        if (m_bluredWallpaper.isNull()) {
            generateBluredWallpaper();
        }
        painter->drawPixmap(QPoint{0, 0}, m_bluredWallpaper, rect);
    }
    painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter->setOpacity(1);
    painter->fillRect(maskRect, m_acrylicBrush);
    painter->restore();
}

void QtAcrylicEffectHelper::updateAcrylicBrush(const QColor &alternativeTintColor)
{
    const auto getAppropriateTintColor = [&alternativeTintColor, this]() -> QColor {
        if (alternativeTintColor.isValid() && (alternativeTintColor != Qt::transparent)) {
            return alternativeTintColor;
        }
        if (m_tintColor.isValid() && (m_tintColor != Qt::transparent)) {
            return m_tintColor;
        }
        return Qt::white;
    };
    static const QImage noiseTexture(QStringLiteral(":/QtAcrylicHelper/Noise.png"));
    QImage acrylicTexture({64, 64}, QImage::Format_ARGB32_Premultiplied);
    QColor fillColor = Qt::transparent;
#ifdef Q_OS_WINDOWS
    if (!Utilities::isOfficialMSWin10AcrylicBlurAvailable()) {
        // Add a soft light layer for the background.
        fillColor = Qt::white;
        fillColor.setAlpha(150);
    }
#endif
    acrylicTexture.fill(fillColor);
    QPainter painter(&acrylicTexture);
    painter.setOpacity(m_tintOpacity);
    painter.fillRect(QRect{0, 0, acrylicTexture.width(), acrylicTexture.height()}, getAppropriateTintColor());
    painter.setOpacity(m_noiseOpacity);
    painter.fillRect(QRect{0, 0, acrylicTexture.width(), acrylicTexture.height()}, noiseTexture);
    m_acrylicBrush = acrylicTexture;
}

void QtAcrylicEffectHelper::generateBluredWallpaper()
{
    if (!m_bluredWallpaper.isNull()) {
        return;
    }
    const QSize size = QGuiApplication::primaryScreen()->size();
    m_bluredWallpaper = QPixmap(size);
    m_bluredWallpaper.fill(Qt::transparent);
    QImage image = Utilities::getDesktopWallpaperImage();
    // On some platforms we may not be able to get the desktop wallpaper, such as Linux and WebAssembly.
    if (image.isNull()) {
        return;
    }
    const Utilities::DesktopWallpaperAspectStyle aspectStyle = Utilities::getDesktopWallpaperAspectStyle();
    QImage buffer(size, QImage::Format_ARGB32_Premultiplied);
#ifdef Q_OS_WINDOWS
    if ((aspectStyle == Utilities::DesktopWallpaperAspectStyle::Central) ||
            (aspectStyle == Utilities::DesktopWallpaperAspectStyle::KeepRatioFit)) {
        buffer.fill(Utilities::getDesktopBackgroundColor());
    }
#endif
    if (aspectStyle == Utilities::DesktopWallpaperAspectStyle::IgnoreRatioFit ||
            aspectStyle == Utilities::DesktopWallpaperAspectStyle::KeepRatioFit ||
            aspectStyle == Utilities::DesktopWallpaperAspectStyle::KeepRatioByExpanding) {
        Qt::AspectRatioMode mode;
        if (aspectStyle == Utilities::DesktopWallpaperAspectStyle::IgnoreRatioFit) {
            mode = Qt::IgnoreAspectRatio;
        } else if (aspectStyle == Utilities::DesktopWallpaperAspectStyle::KeepRatioFit) {
            mode = Qt::KeepAspectRatio;
        } else {
            mode = Qt::KeepAspectRatioByExpanding;
        }
        QSize newSize = image.size();
        newSize.scale(size, mode);
        image = image.scaled(newSize, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    }
    if (aspectStyle == Utilities::DesktopWallpaperAspectStyle::Tiled) {
        QPainter painterBuffer(&buffer);
        painterBuffer.fillRect(QRect{{0, 0}, size}, image);
    } else {
        QPainter painterBuffer(&buffer);
        const QRect rect = Utilities::alignedRect(Qt::LeftToRight, Qt::AlignCenter, image.size(), {{0, 0}, size});
        painterBuffer.drawImage(rect.topLeft(), image);
    }
    QPainter painter(&m_bluredWallpaper);
#if 1
    Utilities::blurImage(&painter, buffer, 128, false, false);
#else
    painter.drawImage(QPoint{0, 0}, buffer);
#endif
}
