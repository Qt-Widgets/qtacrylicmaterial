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

#include "widget.h"
#include <QtWidgets/qlayout.h>
#include <QtWidgets/qlabel.h>
#include <QtCore/qdatetime.h>
#include "qtacrylicwidget.h"

Widget::Widget(QWidget *parent) : QWidget(parent)
{
    setupUi();
    startTimer(500);
}

Widget::~Widget() = default;

void Widget::timerEvent(QTimerEvent *event)
{
    QWidget::timerEvent(event);
    if (m_label) {
        m_label->setText(QTime::currentTime().toString(QStringLiteral("hh:mm:ss")));
    }
}

void Widget::moveEvent(QMoveEvent *event)
{
    QWidget::moveEvent(event);
    if (m_acrylicWidget) {
        // This line is necessary!
        m_acrylicWidget->update();
    }
}

void Widget::setupUi()
{
    m_acrylicWidget = new QtAcrylicWidget(this);
    m_label = new QLabel(this);
    QFont f = font();
    f.setBold(true);
    f.setPointSize(70);
    m_label->setFont(f);
    const auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(m_acrylicWidget);
    setLayout(mainLayout);
    const auto labelLayout2 = new QHBoxLayout;
    labelLayout2->setContentsMargins(0, 0, 0, 0);
    labelLayout2->setSpacing(0);
    labelLayout2->addStretch();
    labelLayout2->addWidget(m_label);
    labelLayout2->addStretch();
    const auto labelLayout1 = new QVBoxLayout(m_acrylicWidget);
    labelLayout1->setContentsMargins(0, 0, 0, 0);
    labelLayout1->setSpacing(0);
    labelLayout1->addStretch();
    labelLayout1->addLayout(labelLayout2);
    labelLayout1->addStretch();
    m_acrylicWidget->setLayout(labelLayout1);
}
