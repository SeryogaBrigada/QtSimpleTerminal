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

#include <QtSerialPort/QSerialPortInfo>
#include <QIntValidator>
#include <QLineEdit>
#include <QDir>
#include <QCoreApplication>
#include <QFileInfo>

#include "settingsdialog.h"
#include "ui_settingsdialog.h"

QT_USE_NAMESPACE

static const char blankString[] = QT_TRANSLATE_NOOP("SettingsDialog", "N/A");

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    m_settings = new QSettings(QCoreApplication::applicationDirPath() + QDir::separator() +
                               QFileInfo(QCoreApplication::applicationFilePath()).fileName() +
                               QLatin1String(".ini"), QSettings::IniFormat);
    ui->setupUi(this);
    ui->baudRateBox->setInsertPolicy(QComboBox::NoInsert);

    connect(ui->applyButton, &QPushButton::clicked, this, &SettingsDialog::apply);
    connect(ui->serialPortInfoListBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &SettingsDialog::showPortInfo);

    connect(ui->baudRateBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &SettingsDialog::checkCustomBaudRatePolicy);

    connect(ui->serialPortInfoListBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &SettingsDialog::checkCustomBaudRatePolicy);

    intValidator = new QIntValidator(0, 4000000, this);

    fillPortsParameters();
    fillPortsInfo();

    updateSettings();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

SettingsDialog::Settings SettingsDialog::settings() const
{
    return currentSettings;
}

void SettingsDialog::showPortInfo(int idx)
{
    if (-1 == idx)
        return;

    QStringList list = ui->serialPortInfoListBox->itemData(idx).toStringList();

    ui->descriptionLabel->setText(tr("Description: %1")
                                  .arg(list.count() > 1 ? list.at(1) : tr(blankString)));

    ui->manufacturerLabel->setText(tr("Manufacturer: %1")
                                   .arg(list.count() > 2 ? list.at(2) : tr(blankString)));

    ui->serialNumberLabel->setText(tr("Serial number: %1")
                                   .arg(list.count() > 3 ? list.at(3) : tr(blankString)));

    ui->locationLabel->setText(tr("Location: %1")
                               .arg(list.count() > 4 ? list.at(4) : tr(blankString)));

    ui->vidLabel->setText(tr("Vendor Identifier: %1")
                          .arg(list.count() > 5 ? list.at(5) : tr(blankString)));

    ui->pidLabel->setText(tr("Product Identifier: %1")
                          .arg(list.count() > 6 ? list.at(6) : tr(blankString)));
}

void SettingsDialog::apply()
{
    updateSettings();

    m_settings->setValue(QLatin1String("Port"), ui->serialPortInfoListBox->currentText());
    m_settings->setValue(QLatin1String("Baudrate"), ui->baudRateBox->currentText());
    m_settings->setValue(QLatin1String("BaudrateBoxIndex"), ui->baudRateBox->currentIndex());
    m_settings->setValue(QLatin1String("DataBits"), ui->dataBitsBox->currentText());
    m_settings->setValue(QLatin1String("Parity"), ui->parityBox->currentText());
    m_settings->setValue(QLatin1String("StopBits"), ui->stopBitsBox->currentText());
    m_settings->setValue(QLatin1String("FlowControl"), ui->flowControlBox->currentText());
    m_settings->setValue(QLatin1String("Encoding"), ui->encodingBox->currentText());
    m_settings->setValue(QLatin1String("DTR"), ui->dtrCheckBox->isChecked());
    m_settings->setValue(QLatin1String("RTS"), ui->rtsCheckBox->isChecked());
    m_settings->setValue(QLatin1String("SaveDataOnExit"), ui->saveFileCheckBox->isChecked());

    hide();
}

void SettingsDialog::checkCustomBaudRatePolicy(int idx)
{
    bool isCustomBaudRate = !ui->baudRateBox->itemData(idx).isValid();
    ui->baudRateBox->setEditable(isCustomBaudRate);
    if (isCustomBaudRate) {
        ui->baudRateBox->clearEditText();
        QLineEdit *edit = ui->baudRateBox->lineEdit();
        edit->setValidator(intValidator);
    }
}

void SettingsDialog::checkCustomDevicePathPolicy(int idx)
{
    bool isCustomPath = !ui->serialPortInfoListBox->itemData(idx).isValid();
    ui->serialPortInfoListBox->setEditable(isCustomPath);
    if (isCustomPath)
        ui->serialPortInfoListBox->clearEditText();
}

void SettingsDialog::fillPortsParameters()
{
    ui->baudRateBox->addItem(QLatin1String("2400"), QSerialPort::Baud2400);
    ui->baudRateBox->addItem(QLatin1String("9600"), QSerialPort::Baud9600);
    ui->baudRateBox->addItem(QLatin1String("19200"), QSerialPort::Baud19200);
    ui->baudRateBox->addItem(QLatin1String("38400"), QSerialPort::Baud38400);
    ui->baudRateBox->addItem(QLatin1String("115200"), QSerialPort::Baud115200);

    ui->baudRateBox->addItem(m_settings->value(QLatin1String("Baudrate")).toString().isEmpty() ?
                                 QLatin1String("Custom") : m_settings->value(QLatin1String("Baudrate")).toString());

    ui->baudRateBox->setCurrentIndex(m_settings->value(QLatin1String("BaudrateBoxIndex"), 4).toInt());

    ui->dataBitsBox->addItem(QLatin1String("5"), QSerialPort::Data5);
    ui->dataBitsBox->addItem(QLatin1String("6"), QSerialPort::Data6);
    ui->dataBitsBox->addItem(QLatin1String("7"), QSerialPort::Data7);
    ui->dataBitsBox->addItem(QLatin1String("8"), QSerialPort::Data8);

    ui->dataBitsBox->setCurrentText(m_settings->value(QLatin1String("DataBits"),
                                                      QLatin1String("8")).toString());

    ui->parityBox->addItem(QLatin1String("None"), QSerialPort::NoParity);
    ui->parityBox->addItem(QLatin1String("Even"), QSerialPort::EvenParity);
    ui->parityBox->addItem(QLatin1String("Odd"), QSerialPort::OddParity);
    ui->parityBox->addItem(QLatin1String("Mark"), QSerialPort::MarkParity);
    ui->parityBox->addItem(QLatin1String("Space"), QSerialPort::SpaceParity);

    ui->parityBox->setCurrentText(m_settings->value(QLatin1String("Parity"),
                                                    QLatin1String("None")).toString());

    ui->stopBitsBox->addItem(QLatin1String("1"), QSerialPort::OneStop);
#ifdef Q_OS_WIN
    ui->stopBitsBox->addItem(QLatin1String("1.5"), QSerialPort::OneAndHalfStop);
#endif
    ui->stopBitsBox->addItem(QLatin1String("2"), QSerialPort::TwoStop);

    ui->stopBitsBox->setCurrentText(m_settings->value(QLatin1String("StopBits"),
                                                      QLatin1String("1")).toString());

    ui->flowControlBox->addItem(QLatin1String("None"), QSerialPort::NoFlowControl);
    ui->flowControlBox->addItem(QLatin1String("RTS/CTS"), QSerialPort::HardwareControl);
    ui->flowControlBox->addItem(QLatin1String("XON/XOFF"), QSerialPort::SoftwareControl);

    ui->flowControlBox->setCurrentText(m_settings->value(QLatin1String("FlowControl"),
                                                         QLatin1String("None")).toString());

    ui->encodingBox->addItem(QLatin1String("Windows 1251"));
    ui->encodingBox->addItem(QLatin1String("IBM 866"));
    ui->encodingBox->addItem(QLatin1String("UTF-8"));

    ui->encodingBox->setCurrentText(m_settings->value(QLatin1String("Encoding"),
                                                      QLatin1String("Windows 1251")).toString());

    ui->dtrCheckBox->setChecked(m_settings->value("DTR", true).toBool());
    ui->rtsCheckBox->setChecked(m_settings->value("RTS", false).toBool());

    ui->saveFileCheckBox->setChecked(m_settings->value(QLatin1String("SaveDataOnExit"),
                                                       false).toBool());
}

void SettingsDialog::fillPortsInfo()
{
    ui->serialPortInfoListBox->clear();
    QString description;
    QString manufacturer;
    QString serialNumber;
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        QStringList list;
        description = info.description();
        manufacturer = info.manufacturer();
        serialNumber = info.serialNumber();
        list << info.portName()
             << (!description.isEmpty() ? description : blankString)
             << (!manufacturer.isEmpty() ? manufacturer : blankString)
             << (!serialNumber.isEmpty() ? serialNumber : blankString)
             << info.systemLocation()
             << (info.vendorIdentifier() ? QString::number(info.vendorIdentifier(), 16) : blankString)
             << (info.productIdentifier() ? QString::number(info.productIdentifier(), 16) : blankString);

        ui->serialPortInfoListBox->addItem(list.first(), list);
    }

    ui->serialPortInfoListBox->
            setCurrentText(m_settings->value("Port", ui->serialPortInfoListBox->currentText()).toString());

//    ui->serialPortInfoListBox->addItem(tr("Custom"));
}

void SettingsDialog::updateSettings()
{
    currentSettings.name = ui->serialPortInfoListBox->currentText();

    if (5 == ui->baudRateBox->currentIndex()) {
        currentSettings.baudRate = ui->baudRateBox->currentText().toInt();
    } else {
        currentSettings.baudRate = static_cast<QSerialPort::BaudRate>(
                    ui->baudRateBox->itemData(ui->baudRateBox->currentIndex()).toInt());
    }

    currentSettings.stringBaudRate = QString::number(currentSettings.baudRate);

    currentSettings.dataBits = static_cast<QSerialPort::DataBits>(
                ui->dataBitsBox->itemData(ui->dataBitsBox->currentIndex()).toInt());

    currentSettings.stringDataBits = ui->dataBitsBox->currentText();

    currentSettings.parity = static_cast<QSerialPort::Parity>(
                ui->parityBox->itemData(ui->parityBox->currentIndex()).toInt());

    currentSettings.stringParity = ui->parityBox->currentText();

    currentSettings.stopBits = static_cast<QSerialPort::StopBits>(
                ui->stopBitsBox->itemData(ui->stopBitsBox->currentIndex()).toInt());

    currentSettings.stringStopBits = ui->stopBitsBox->currentText();

    currentSettings.flowControl = static_cast<QSerialPort::FlowControl>(
                ui->flowControlBox->itemData(ui->flowControlBox->currentIndex()).toInt());

    currentSettings.stringFlowControl = ui->flowControlBox->currentText();

    currentSettings.localEchoEnabled = ui->localEchoCheckBox->isChecked();

    currentSettings.dtrEnabled = ui->dtrCheckBox->isChecked();

    currentSettings.rtsEnabled = ui->rtsCheckBox->isChecked();

    currentSettings.textEncoding = ui->encodingBox->currentText();

    currentSettings.saveOnExit = ui->saveFileCheckBox->isChecked();
}
