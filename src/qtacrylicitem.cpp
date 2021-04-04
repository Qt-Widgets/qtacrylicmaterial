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

#include "qtacrylicitem.h"
#include <QtQuick/qquickwindow.h>
#include <QtCore/qdebug.h>

QtAcrylicItem::QtAcrylicItem(QQuickItem *parent) : QQuickPaintedItem(parent)
{
    m_acrylicHelper.showPerformanceWarning();
    m_acrylicHelper.updateAcrylicBrush();
    connect(this, &QtAcrylicItem::xChanged, this, [this](){
        update();
    });
    connect(this, &QtAcrylicItem::yChanged, this, [this](){
        update();
    });
    connect(this, &QtAcrylicItem::windowChanged, this, [this](QQuickWindow *w){
        if (m_xConnection) {
            disconnect(m_xConnection);
        }
        if (m_yConnection) {
            disconnect(m_yConnection);
        }
        if (w) {
            m_xConnection = connect(w, &QQuickWindow::xChanged, this, [this](){
                update();
            });
            m_yConnection = connect(w, &QQuickWindow::yChanged, this, [this](){
                update();
            });
        }
    });
}

QtAcrylicItem::~QtAcrylicItem() = default;

void QtAcrylicItem::paint(QPainter *painter)
{
    const QRectF rectF = {mapToGlobal(QPointF{0.0, 0.0}), size()};
    m_acrylicHelper.paintBackground(painter, rectF.toRect());
}

QColor QtAcrylicItem::tintColor() const
{
    const QColor color = m_acrylicHelper.getTintColor();
    if (color.isValid() && (color != Qt::transparent)) {
        return color;
    } else {
        return /*palette().color(backgroundRole())*/Qt::white;
    }
}

void QtAcrylicItem::setTintColor(const QColor &value)
{
    if (!value.isValid()) {
        qWarning() << "Tint color not valid.";
        return;
    }
    if (m_acrylicHelper.getTintColor() != value) {
        m_acrylicHelper.setTintColor(value);
#if 0
        QPalette pal = palette();
        pal.setColor(backgroundRole(), m_acrylicHelper.getTintColor());
        setPalette(pal);
#endif
        m_acrylicHelper.updateAcrylicBrush();
        update();
        Q_EMIT tintColorChanged();
    }
}

qreal QtAcrylicItem::tintOpacity() const
{
    return m_acrylicHelper.getTintOpacity();
}

void QtAcrylicItem::setTintOpacity(const qreal value)
{
    if (m_acrylicHelper.getTintOpacity() != value) {
        m_acrylicHelper.setTintOpacity(value);
        m_acrylicHelper.updateAcrylicBrush();
        update();
        Q_EMIT tintOpacityChanged();
    }
}

qreal QtAcrylicItem::noiseOpacity() const
{
    return m_acrylicHelper.getNoiseOpacity();
}

void QtAcrylicItem::setNoiseOpacity(const qreal value)
{
    if (m_acrylicHelper.getNoiseOpacity() != value) {
        m_acrylicHelper.setNoiseOpacity(value);
        m_acrylicHelper.updateAcrylicBrush();
        update();
        Q_EMIT noiseOpacityChanged();
    }
}
