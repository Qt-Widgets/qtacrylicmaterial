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
#include <QtGui/qbrush.h>

class QTACRYLICHELPER_API QtAcrylicEffectHelper
{
    Q_DISABLE_COPY_MOVE(QtAcrylicEffectHelper)

public:
    explicit QtAcrylicEffectHelper();
    ~QtAcrylicEffectHelper();

    void setTintColor(const QColor &value);
    QColor getTintColor() const;

    void setTintOpacity(const qreal value);
    qreal getTintOpacity() const;

    void setNoiseOpacity(const qreal value);
    qreal getNoiseOpacity() const;

    QBrush getAcrylicBrush() const;
    QPixmap getBluredWallpaper() const;
    void showPerformanceWarning() const;
    void regenerateWallpaper();

    void paintBackground(QPainter *painter, const QRect &rect);
    void updateAcrylicBrush(const QColor &alternativeTintColor = {});

private:
    void generateBluredWallpaper();

private:
    QBrush m_acrylicBrush = {};
    QColor m_tintColor = {};
    qreal m_tintOpacity = 0.7;
    qreal m_noiseOpacity = 0.04;
    QPixmap m_bluredWallpaper = {};
};
