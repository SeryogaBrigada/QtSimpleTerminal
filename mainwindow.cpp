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

#include <QMessageBox>
#include <QLabel>
#include <QtSerialPort/QSerialPort>
#include <QTextCodec>
#include <QTextStream>
#include <QFile>
#include <QDir>
#include <QDate>
#include <QTime>
#include <QFileDialog>
#include <QCoreApplication>

#include "ui_mainwindow.h"
#include "console.h"
#include "settingsdialog.h"
#include "mainwindow.h"

//! [0]
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //! [0]
    serial = new QSerialPort(this);

    //! [1]
    ui->setupUi(this);
    console = new Console(this, serial);
    setCentralWidget(console);
    console->setFocus();

    //! [1]
    settings = new SettingsDialog(this);

    ui->actionOpen->setEnabled(true);
    ui->actionSave->setEnabled(true);

    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setEnabled(false);
    ui->actionQuit->setEnabled(true);
    ui->actionConfigure->setEnabled(true);

    initActionsConnections();

    connect(serial, static_cast<void (QSerialPort::*)(QSerialPort::SerialPortError)>(&QSerialPort::error),
            this, &MainWindow::handleError);

    //! [2]
    connect(serial, &QSerialPort::readyRead, this, &MainWindow::readData);
    //! [2]
    connect(console, &Console::getData, this, &MainWindow::writeData);
    //! [3]

    m_readTimer.setSingleShot(true);
    connect(&m_readTimer, &QTimer::timeout, this, &MainWindow::flushBuffer);
}
//! [3]

MainWindow::~MainWindow()
{
    if (saveFileOnExit && !console->toPlainText().isEmpty())
        writeToFile(QCoreApplication::applicationDirPath() + QDir::separator() +
                    QDate::currentDate().toString("yyyy-MM-dd") + QLatin1Char(' ') +
                    QTime::currentTime().toString("hh-mm-ss") + QLatin1String(".log"));

    settings->deleteLater();
    delete ui;
}

//! [4]
void MainWindow::openSerialPort()
{
    SettingsDialog::Settings p = settings->settings();
    serial->setPortName(p.name);
    serial->setBaudRate(p.baudRate);
    serial->setDataBits(p.dataBits);
    serial->setParity(p.parity);
    serial->setStopBits(p.stopBits);
    serial->setFlowControl(p.flowControl);
    serial->setReadBufferSize(10000);

    codec = QTextCodec::codecForName(p.textEncoding.toLatin1());
    saveFileOnExit = p.saveOnExit;

    if (serial->open(QIODevice::ReadWrite)) {
        console->setFocus();
        console->setLocalEchoEnabled(p.localEchoEnabled);
        serial->setDataTerminalReady(p.dtrEnabled);
        serial->setRequestToSend(p.rtsEnabled);

        ui->actionConnect->setEnabled(false);
        ui->actionDisconnect->setEnabled(true);
        ui->actionConfigure->setEnabled(false);

        showStatusMessage(tr("Connected to %1 : %2, %3, %4, %5, %6")
                          .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                          .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl));
    } else {
        QMessageBox::critical(this, tr("Error"), serial->errorString());
        showStatusMessage(tr("Open error"));
    }
}
//! [4]

//! [5]
void MainWindow::closeSerialPort()
{
    if (serial->isOpen())
        serial->close();

    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setEnabled(false);
    ui->actionConfigure->setEnabled(true);

    showStatusMessage(tr("Disconnected"));
}
//! [5]

void MainWindow::about()
{
    QMessageBox::about(this, tr("About Simple Terminal"),
                       tr("<b>Simple Terminal</b> is an advanced tool\n"
                          " for working with serial interface devices.\n"
                          "<center>Powered by Sergey Kovalenko</center>\n"
                          "<center>seryoga.engineering@gmail.com</center>"));
}

//! [6]
void MainWindow::writeData(const QByteArray &data)
{
    if (serial->isOpen())
        serial->write(data);
}
//! [6]

//! [7]
void MainWindow::readData()
{
    flushBuffer();
}
//! [7]

void MainWindow::flushBuffer()
{
    if (console->isBusy()) {
        m_readTimer.start(100);
        return;
    }

    console->putData(codec->toUnicode(serial->readAll()));
}

//! [8]
void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    if (QSerialPort::ResourceError == error) {
        //    QMessageBox::critical(this, tr("Critical Error"), serial->errorString());
        closeSerialPort();
    }
}
//! [8]

void MainWindow::initActionsConnections()
{
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::fileOpen);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::fileSave);
    connect(ui->actionConnect, &QAction::triggered, this, &MainWindow::openSerialPort);
    connect(ui->actionDisconnect, &QAction::triggered, this, &MainWindow::closeSerialPort);
    connect(ui->actionQuit, &QAction::triggered, this, &MainWindow::close);
    connect(ui->actionConfigure, &QAction::triggered, settings, &SettingsDialog::show);
    connect(ui->actionClear, &QAction::triggered, console, &Console::clear);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::about);
    connect(ui->actionAboutQt, &QAction::triggered, qApp, &QApplication::aboutQt);
}

void MainWindow::showStatusMessage(const QString &message)
{
    ui->statusBar->showMessage(message);
}


void MainWindow::fileOpen()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open file"), QString(),
                                                    tr("Text files (*.txt);;"
                                                       "All Files (*)"));
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (!file.open(QFile::ReadOnly)) {
            QMessageBox::warning(this, tr("Codecs"),
                                 tr("Can't read file %1:\n%2")
                                 .arg(fileName)
                                 .arg(file.errorString()));
            return;
        }

        if (!serial->isOpen())
            openSerialPort();

        QByteArray data = file.readAll();
        file.close();
        writeData(data);

        console->putData(codec->toUnicode(data));
    }
}

void MainWindow::writeToFile(const QString &fileName)
{
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (!file.open(QFile::WriteOnly | QFile::Text)) {
            QMessageBox::warning(this, tr("Warning!"),
                                 tr("Could not write to file %1:\n%2")
                                 .arg(fileName)
                                 .arg(file.errorString()));
            return;
        }

        QTextStream out(&file);
        out.setCodec("UTF-8");
        out << console->toPlainText();
        file.close();
    }
}

void MainWindow::fileSave()
{
    closeSerialPort();

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save as..."), QString(),
                                                    tr("Log files (*.log);;"
                                                       "Text files (*.txt);;"
                                                       "All Files (*)"));
    writeToFile(fileName);
}

