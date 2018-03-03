/****************************************************************************
**
** Copyright (C) 2012 Denis Shienkov <denis.shienkov@gmail.com>
** Copyright (C) 2012 Laszlo Papp <lpapp@kde.org>
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtSerialPort module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QScrollBar>
#include <QKeyEvent>
#include <QtSerialPort/QtSerialPort>
#include "console.h"

Console::Console(QWidget *parent, QSerialPort *port):
    QTextEdit(parent),
    m_port(port),
    localEchoEnabled(true)
{
    QPalette p = palette();
    p.setColor(QPalette::Base, Qt::black);
    p.setColor(QPalette::Text, Qt::green);
    setPalette(p);

    setAcceptRichText(false);

    m_inputTimer.setSingleShot(true);

    connect(&m_inputTimer, &QTimer::timeout, [&](){

        QTextCursor m_cursor = textCursor();
        m_cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);

        setTextCursor(m_cursor);
        insertPlainText("\n");

        QScrollBar *bar = verticalScrollBar();
        bar->setValue(bar->maximum());
    });
}

void Console::putData(const QString &data)
{
    if (QColor(Qt::green) != textColor())
        setTextColor(Qt::green);

    insertPlainText(data);

    QScrollBar *bar = verticalScrollBar();
    bar->setValue(bar->maximum());
}

void Console::setLocalEchoEnabled(bool set)
{
    localEchoEnabled = set;
}

void Console::keyPressEvent(QKeyEvent *e)
{
    Q_CHECK_PTR(m_port);

    if (m_port->isOpen()) {
        emit getData(e->text().toLocal8Bit());
        m_inputTimer.start(std::chrono::seconds(5));
    } else {
        localEchoEnabled = true;
    }

    if (localEchoEnabled) {
        if (QColor(Qt::red) != textColor())
            setTextColor(Qt::red);

        QTextEdit::keyPressEvent(e);
    }
}

bool Console::isBusy() const
{
    return m_inputTimer.isActive();
}

