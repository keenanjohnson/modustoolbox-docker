/*
 * Copyright 2018-2023 Cypress Semiconductor Corporation (an Infineon company)
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mainwindow.h"

#include <QDebug>
#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>
#include <QMetaType>
#include <QProgressBar>
#include <QRegExpValidator>
#include <QSettings>
#include <QTextStream>
#include <cmath>

#include "cychanneli2c.h"
#include "cychannelspi.h"
#include "cychanneluart.h"
#include "settings.h"

namespace cydfuht {
namespace gui {

/// A validator that allows both decimal and hexadecimal input, and also checks to see if the number is in range for an I2C address.
class I2cAddressValidator : public QValidator {
   public:
    /// The constructor.
    /// \param parent The parent of this Qt object.
    explicit I2cAddressValidator(QObject *parent)
        : QValidator(parent), regexp_validator(QRegExp(QLatin1String("(0x[0-9a-f]+|[0-9]+)"), Qt::CaseInsensitive), this) {}

    /// The virtual function that does the validating.
    /// \param input The string to validate.
    /// \param pos The cursor position (not used).
    /// \return Whether the string is valid, could become valid if more characters were added, or definitely invalid.
    QValidator::State validate(QString &input, int &pos) const override {
        QValidator::State re_valid = regexp_validator.validate(input, pos);
        if (re_valid != QValidator::Acceptable) {
            return re_valid;
        }
        bool ok;
        unsigned int num = input.toUInt(&ok, 0);
        if (!ok || (num > 120)) {
            return QValidator::Invalid;
        }
        if (num < 8) {
            return QValidator::Intermediate;
        }
        return QValidator::Acceptable;
    }

   private:
    QRegExpValidator regexp_validator;
};

static void qstrToCombobox(QComboBox *combo_p, const QString &str) {
    const int index = combo_p->findText(str);
    if (index != -1) {
        combo_p->setCurrentIndex(index);
    } else {
        combo_p->setCurrentIndex(0);
    }
}

const QLatin1String MainWindow::WindowState = QLatin1String("WindowState");

const QLatin1String MainWindow::UartEndString = QLatin1String("UART");
const QLatin1String MainWindow::I2CEndString = QLatin1String("I2C");
const QLatin1String MainWindow::SpiEndString = QLatin1String("SPI");

const QLatin1String MainWindow::Cyacd2Filename = QLatin1String("Cyacd2Filename");

const QLatin1String MainWindow::I2CIsDisplayed = QLatin1String("I2CIsDisplayed");
const QLatin1String MainWindow::I2CAddress = QLatin1String("I2CAddress");
const QLatin1String MainWindow::I2CSpeed = QLatin1String("I2CSpeed");

const QLatin1String MainWindow::SpiIsDisplayed = QLatin1String("SpiIsDisplayed");
const QLatin1String MainWindow::SpiMode = QLatin1String("SpiMode");
const QLatin1String MainWindow::SpiBitOrder = QLatin1String("SpiBitOrder");
const QLatin1String MainWindow::SpiFreq = QLatin1String("SpiFreq");

const QLatin1String MainWindow::UartIsDisplayed = QLatin1String("UartIsDisplayed");
const QLatin1String MainWindow::UartBaudRate = QLatin1String("UartBaudRate");
const QLatin1String MainWindow::UartDataBits = QLatin1String("UartDataBits");
const QLatin1String MainWindow::UartStopBits = QLatin1String("UartStopBits");
const QLatin1String MainWindow::UartParityBits = QLatin1String("UartParityBits");

const QLatin1String MainWindow::MtbdfuExt = QLatin1String("mtbdfu");
const QLatin1String MainWindow::HexExt = QLatin1String("hex");
const QLatin1String MainWindow::CyAcd2Ext = QLatin1String("cyacd2");

MainWindow::MainWindow() : m_ui(), m_filterdialog(this), m_fileerror_p(nullptr), m_i2caddresserror_p(nullptr), m_workerobject() {
    m_ui.setupUi(this);
    m_ui.m_file_pushbutton_p->setIcon(QIcon(QLatin1String(":/images/ellipsis")));
    m_filterdialog.adjustSize();
    m_ui.m_formwidget_p->hide();
    m_ui.m_stackedwidget_p->setCurrentWidget(m_ui.m_blankpage_p);
    m_ui.m_abort_action_p->setEnabled(false);
    auto *addrvalidator_p = new I2cAddressValidator(this);
    m_ui.m_i2c_address_lineedit_p->setValidator(addrvalidator_p);

    m_statusbar_label_p = new QLabel(tr("Ready"));
    m_statusbar_label_p->setMinimumWidth(100);
    m_ui.statusBar->addPermanentWidget(m_statusbar_label_p);
    m_statusbar_progressbar_p = new QProgressBar();
    m_statusbar_progressbar_p->setRange(0, ProgressBarWidth);
    m_statusbar_progressbar_p->setTextVisible(false);
    m_statusbar_progressbar_p->setMinimumWidth(ProgressBarWidth);
    m_ui.statusBar->addPermanentWidget(m_statusbar_progressbar_p, 100);

    QAction *toolbar_toggle_action = m_ui.m_toolbar_p->toggleViewAction();
    toolbar_toggle_action->setText(tr("&Toolbar"));
    toolbar_toggle_action->setStatusTip(tr("Show/hide the main application toolbar."));
    m_ui.menuView->addAction(toolbar_toggle_action);

    m_ui.m_open_action_p->setIcon(QIcon::fromTheme(QLatin1String("document-open"), QIcon(QLatin1String(":/images/Open-file"))));
    m_ui.m_program_action_p->setIcon(QIcon(QLatin1String(":/images/Program-flash-memory-of-the-target-device")));
    m_ui.m_verify_action_p->setIcon(QIcon(QLatin1String(":/images/Verify-programmed-flash-memory-against-the-given-file")));
    m_ui.m_eraseall_action_p->setIcon(QIcon(QLatin1String(":/images/Erase-flash-memory-of-the-target-device")));
    m_ui.m_abort_action_p->setIcon(QIcon(QLatin1String(":/images/Abort-programming-operation")));
    m_ui.m_about_action_p->setIcon(windowIcon());
    m_ui.m_spi_clockspeed_combobox_p->addItem(tr("8 MHz"), 8000000);
    for (int divisor = 2; divisor < 256; ++divisor) {
        int hertz = (int)std::lround(12000000.0 / divisor);
        if (hertz >= 1000000) {
            m_ui.m_spi_clockspeed_combobox_p->addItem(QString::number(hertz / 1000000.0, 'g', 7) + tr(" MHz"), hertz);
        } else if (hertz >= 1000) {
            m_ui.m_spi_clockspeed_combobox_p->addItem(QString::number(hertz / 1000.0) + tr(" kHz"), hertz);
        } else {
            m_ui.m_spi_clockspeed_combobox_p->addItem(QString::number(hertz) + tr(" Hz"), hertz);
        }
    }

    QSettings &settings = getSettings();
    cydfuht::core::cyInfo() << "settingsFileName=" << QDir::fromNativeSeparators(settings.fileName());
    QByteArray windowstate = settings.value(WindowState).toByteArray();
    if (!windowstate.isEmpty()) {
        restoreState(windowstate);
    }
    m_ui.m_file_lineedit_p->setText(settings.value(Cyacd2Filename).toString());
    m_ui.m_i2c_address_lineedit_p->setText(settings.value(I2CAddress, QLatin1String("8")).toString());
    const int i2cspeed = settings.value(I2CSpeed, 400000).toInt();
    switch (i2cspeed) {
        case ONE_MHZ_IN_HZ:
            m_ui.m_i2c_1megahertz_radiobutton_p->setChecked(true);
            break;
        case FOUR_HUNDRED_KHZ_IN_HZ:
        default:
            m_ui.m_i2c_400kilohertz_radiobutton_p->setChecked(true);
            break;
        case HUNDRED_KHZ_IN_HZ:
            m_ui.m_i2c_100kilohertz_radiobutton_p->setChecked(true);
            break;
        case FIFTY_KHZ_IN_HZ:
            m_ui.m_i2c_50kilohertz_radiobutton_p->setChecked(true);
            break;
    }
    qstrToCombobox(m_ui.m_spi_clockspeed_combobox_p, settings.value(SpiFreq, QLatin1String("1 MHz")).toString());
    const core::SpiBitOrder spibitorder = static_cast<core::SpiBitOrder>(settings.value(SpiBitOrder, 0).toInt());
    if (spibitorder == core::SpiBitOrder::LSB) {
        m_ui.m_spi_lsbfirst_radiobutton_p->setChecked(true);
    } else {
        m_ui.m_spi_msbfirst_radiobutton_p->setChecked(true);
    }
    const core::SpiMode spimode = static_cast<core::SpiMode>(settings.value(SpiMode, 0).toInt());
    if (spimode == core::SpiMode::MODE1) {
        m_ui.m_spi_mode01_radiobutton_p->setChecked(true);
    } else if (spimode == core::SpiMode::MODE2) {
        m_ui.m_spi_mode02_radiobutton_p->setChecked(true);
    } else if (spimode == core::SpiMode::MODE3) {
        m_ui.m_spi_mode03_radiobutton_p->setChecked(true);
    } else {
        m_ui.m_spi_mode00_radiobutton_p->setChecked(true);
    }
    qstrToCombobox(m_ui.m_uart_baud_combobox_p, settings.value(UartBaudRate, QLatin1String("115200")).toString());
    qstrToCombobox(m_ui.m_uart_databits_combobox_p, settings.value(UartDataBits, QLatin1String("8")).toString());
    qstrToCombobox(m_ui.m_uart_stopbits_combobox_p, settings.value(UartStopBits, QLatin1String("1")).toString());
    qstrToCombobox(m_ui.m_uart_parity_combobox_p, settings.value(UartParityBits, tr("None")).toString());

    m_workerobject.moveToThread(&m_workerthread);
    connect(m_ui.m_port_listwidget_p, &QListWidget::currentRowChanged, this, &MainWindow::portRowChanged);
    connect(m_ui.m_filter_pushbutton_p, &QPushButton::clicked, this, &MainWindow::filterButtonPressed);
    connect(m_ui.m_generate_pushbutton_p, &QPushButton::clicked, this, &MainWindow::generateMtbDfuPressed);
    connect(&m_filterdialog, &FilterDialog::filterChanged, this, &MainWindow::filterChanged);
    connect(m_ui.m_file_lineedit_p, &QLineEdit::textEdited, this, &MainWindow::validateData);
    connect(m_ui.m_file_pushbutton_p, &QPushButton::clicked, this, &MainWindow::findCyacd2File);
    connect(m_ui.m_open_action_p, &QAction::triggered, this, &MainWindow::findCyacd2File);
    connect(m_ui.m_savelogas_action_p, &QAction::triggered, this, &MainWindow::saveLogFile);
    connect(m_ui.m_clearlog_action_p, &QAction::triggered, m_ui.m_log_textedit_p, &QPlainTextEdit::clear);
    connect(m_ui.m_exit_action_p, &QAction::triggered, this, &QMainWindow::close);
    connect(m_ui.m_program_action_p, &QAction::triggered, this, &MainWindow::programDevice);
    connect(m_ui.m_verify_action_p, &QAction::triggered, this, &MainWindow::verifyDevice);
    connect(m_ui.m_eraseall_action_p, &QAction::triggered, this, &MainWindow::eraseDevice);
    connect(m_ui.m_abort_action_p, &QAction::triggered, this, &MainWindow::abortOperation);
    connect(m_ui.m_viewhelp_action_p, &QAction::triggered, this, &MainWindow::showDocumentation);
    connect(m_ui.m_about_action_p, &QAction::triggered, this, &MainWindow::showAboutBox);
    connect(m_ui.m_i2c_address_lineedit_p, &QLineEdit::textEdited, this, &MainWindow::validateData);

    qRegisterMetaType<QVector<cydfuht::core::CyChannelSettings>>();
    qRegisterMetaType<cydfuht::core::CyChannelSettings>();
    connect(this, &MainWindow::workerInitialize, &m_workerobject, &backend::WorkerObject::initialize);
    connect(this, &MainWindow::workerClose, &m_workerobject, &backend::WorkerObject::close, Qt::BlockingQueuedConnection);
    connect(this, &MainWindow::workerProgramDevice, &m_workerobject, &backend::WorkerObject::programDevice);
    connect(this, &MainWindow::workerVerifyDevice, &m_workerobject, &backend::WorkerObject::verifyDevice);
    connect(this, &MainWindow::workerEraseDevice, &m_workerobject, &backend::WorkerObject::eraseDevice);
    connect(this, &MainWindow::workerSendCommand, &m_workerobject, &backend::WorkerObject::sendCommand);
    connect(&m_workerobject, &backend::WorkerObject::deviceConnected, this, &MainWindow::workerDeviceConnected);
    connect(&m_workerobject, &backend::WorkerObject::deviceDisconnected, this, &MainWindow::workerDeviceDisconnected);
    connect(&m_workerobject, &backend::WorkerObject::progressReport, this, &MainWindow::workerProgressReport);
    connect(&m_workerobject, &backend::WorkerObject::returnValue, this, &MainWindow::workerReturnValue);
    m_workerthread.start();
    emit workerInitialize();

    setUnifiedTitleAndToolBarOnMac(true);  // This removes 'Show Tab Bar' command from the View menu on MacOS

    setContextMenuPolicy(Qt::NoContextMenu);
    adjustSize();

    validateData();
}

void MainWindow::portRowChanged(int row) {
    enableOrDisableActions();
    if (row == -1) {
        m_ui.m_stackedwidget_p->setCurrentWidget(m_ui.m_blankpage_p);
    } else {
        QString rowtext = m_ui.m_port_listwidget_p->currentItem()->text();
        if (rowtext.endsWith(UartEndString, Qt::CaseInsensitive)) {
            m_ui.m_stackedwidget_p->setCurrentWidget(m_ui.m_uartpage_p);
        } else if (rowtext.endsWith(I2CEndString, Qt::CaseInsensitive)) {
            m_ui.m_stackedwidget_p->setCurrentWidget(m_ui.m_i2cpage_p);
        } else if (rowtext.endsWith(SpiEndString, Qt::CaseInsensitive)) {
            m_ui.m_stackedwidget_p->setCurrentWidget(m_ui.m_spipage_p);
        }
    }
    validateData();
}

void MainWindow::filterButtonPressed() { m_filterdialog.show(); }

void MainWindow::generateMtbDfuPressed() { generateMtbDfu(core::ProgramAction); }

void MainWindow::generateMtbDfu(const QString &action) {
    bool ok;
    CyBtldr_CustomCommandHeaderData metaHeaderData;
    metaHeaderData.fileVersion = m_ui.m_fileversion_lineedit_p->text().toUInt(&ok, 0);
    metaHeaderData.productId = m_ui.m_productid_lineedit_p->text().toUInt(&ok, 16);
    // metadata is cannot be used from GUI, only passed as CLI parameters
    metaHeaderData.applicationId = 0x0;
    metaHeaderData.applicationStart = 0x0;
    metaHeaderData.applicationLength = 0x0;
    metaHeaderData.checksumType = m_ui.m_checksumtype_lineedit_p->text().toUInt(&ok, 0);
    core::generateMtbDfuJson(m_ui.m_file_lineedit_p->text(), action, m_mtbdfu_filename, metaHeaderData);
    QString genFileName = action + tr("Hex.mtbdfu");
    logMessage(QFileInfo(genFileName).absolutePath() + tr("/") + genFileName + tr(" was generated."));
}

void MainWindow::filterChanged() {
    const static QString i2c_name = QString::fromStdString(core::interfaceEnumToString(core::InterfaceEnum::I2C));
    const static QString spi_name = QString::fromStdString(core::interfaceEnumToString(core::InterfaceEnum::SPI));
    const static QString uart_name = QString::fromStdString(core::interfaceEnumToString(core::InterfaceEnum::UART));
    for (int i = 0; i < m_ui.m_port_listwidget_p->count(); ++i) {
        QListWidgetItem *item = m_ui.m_port_listwidget_p->item(i);
        bool should_be_hidden = true;
        if (item->text().endsWith(i2c_name)) {
            should_be_hidden = !m_filterdialog.shouldInterfaceBeDisplayed(core::InterfaceEnum::I2C);
        } else if (item->text().endsWith(spi_name)) {
            should_be_hidden = !m_filterdialog.shouldInterfaceBeDisplayed(core::InterfaceEnum::SPI);
        } else if (item->text().endsWith(uart_name)) {
            should_be_hidden = !m_filterdialog.shouldInterfaceBeDisplayed(core::InterfaceEnum::UART);
        }
        if (item->isSelected()) {
            if (should_be_hidden && !item->isHidden()) {
                portRowChanged(-1);
            } else if (!should_be_hidden && item->isHidden()) {
                portRowChanged(i);
            }
        }
        item->setHidden(should_be_hidden);
    }
}

void MainWindow::findCyacd2File() {
    QString filename =
        QFileDialog::getOpenFileName(this, tr("Open firmware file"), QFileInfo(m_ui.m_file_lineedit_p->text()).absolutePath(),
                                     tr("Firmware files (*.mtbdfu *.cyacd2 *.hex)"));
    if (filename.isEmpty()) {
        return;
    }
    m_ui.m_file_lineedit_p->setText(filename);
    validateData();
}

void MainWindow::saveLogFile() {
    QString filename = QFileDialog::getSaveFileName(
        this, tr("Save Log File"), QFileInfo(m_ui.m_file_lineedit_p->text()).absolutePath() + QLatin1String("/firmware_update.log"),
        tr("Log files (*.log);;Text files (*.txt)"));
    if (filename.isEmpty()) {
        return;
    }
    QFile logfile(filename);
    if (!logfile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, QApplication::applicationDisplayName(), tr("Cannot open file %1 for writing").arg(filename));
        return;
    }
    QTextStream logstream(&logfile);
    logstream << m_ui.m_log_textedit_p->toPlainText();
    logfile.close();
}

core::CyChannelSettings MainWindow::initializeCurrentChannel() {
    core::CyChannelSettings channelsettings;
    channelsettings.m_interface = core::InterfaceEnum::NONE;

    QListWidgetItem *portitem = m_ui.m_port_listwidget_p->currentItem();
    if (portitem == nullptr) {
        return channelsettings;
    }
    for (const auto &c : m_channellist) {
        const QString stringtotest = c.m_displayName;
        if (portitem->text() == stringtotest) {
            channelsettings = c;
            break;
        }
    }
    switch (channelsettings.m_interface) {
        case core::InterfaceEnum::I2C: {
            bool ok;
            channelsettings.m_i2c.m_addr = m_ui.m_i2c_address_lineedit_p->text().toUInt(&ok, 0);
            if (!ok) {
                logMessage(tr("I2C address is invalid."));
                return channelsettings;
            }
            channelsettings.m_i2c.m_freq_hz = getI2cSpeed();
            return channelsettings;
        }
        case core::InterfaceEnum::SPI: {
            channelsettings.m_spi.m_freq_hz = m_ui.m_spi_clockspeed_combobox_p->currentData().toUInt();
            channelsettings.m_spi.m_mode = getSpiMode();
            channelsettings.m_spi.m_bit_order = getSpiBitOrder();
            return channelsettings;
        }
        case core::InterfaceEnum::UART: {
            channelsettings.m_uart.m_baudrate = m_ui.m_uart_baud_combobox_p->currentText().toUInt();
            channelsettings.m_uart.m_databits = m_ui.m_uart_databits_combobox_p->currentText().toUInt();
            channelsettings.m_uart.m_stopbits = m_ui.m_uart_stopbits_combobox_p->currentText().toFloat();
            const QString parity = m_ui.m_uart_parity_combobox_p->currentText();
            if (parity == tr("Odd")) {
                channelsettings.m_uart.m_paritytype = core::ParityTypeEnum::ODD;
            } else if (parity == tr("Even")) {
                channelsettings.m_uart.m_paritytype = core::ParityTypeEnum::EVEN;
            } else {
                channelsettings.m_uart.m_paritytype = core::ParityTypeEnum::NONE;
            }
            return channelsettings;
        }
        default:
            logMessage(tr("Model and view do not match."));
            return channelsettings;
    }
}

void MainWindow::logMessage(const QString &message) {
    const QString prefix = QDateTime::currentDateTime().toString(QLatin1String("yyyy-MM-dd HH:mm:ss.zzz "));
    m_ui.m_log_textedit_p->appendPlainText(prefix + message);
}

void MainWindow::programDevice() {
    core::CyChannelSettings channel = initializeCurrentChannel();
    if (channel.m_interface == core::InterfaceEnum::NONE) {
        return;
    }
    if (m_ext == MtbdfuExt) {
        logMessage(tr("Execution started."));
    } else {
        logMessage(tr("Program started."));
    }
    m_statusbar_progressbar_p->reset();
    m_statusbar_progressbar_p->setStyleSheet(QLatin1String(""));
    m_startofoperation = QDateTime::currentDateTime();
    m_statusbar_label_p->setText(tr("Programming..."));
    if (m_ext == HexExt) {
        generateMtbDfu(core::ProgramAction);
        emit workerSendCommand(channel, m_mtbdfu_filename);
    } else if (m_ext == MtbdfuExt) {
        emit workerSendCommand(channel, m_ui.m_file_lineedit_p->text());
    } else if (m_ext == CyAcd2Ext) {
        emit workerProgramDevice(channel, m_ui.m_file_lineedit_p->text());
    }
    enableOrDisableActions();
}

void MainWindow::verifyDevice() {
    core::CyChannelSettings channel = initializeCurrentChannel();
    if (channel.m_interface == core::InterfaceEnum::NONE) {
        return;
    }
    logMessage(tr("Verify started."));
    m_statusbar_progressbar_p->reset();
    m_statusbar_progressbar_p->setStyleSheet(QLatin1String(""));
    m_startofoperation = QDateTime::currentDateTime();
    m_statusbar_label_p->setText(tr("Verifying..."));
    if (m_ext == HexExt) {
        generateMtbDfu(core::VerifyAction);
        emit workerSendCommand(channel, m_mtbdfu_filename);
    } else if (m_ext == CyAcd2Ext) {
        emit workerVerifyDevice(channel, m_ui.m_file_lineedit_p->text());
    }
    enableOrDisableActions();
}

void MainWindow::eraseDevice() {
    core::CyChannelSettings channel = initializeCurrentChannel();
    if (channel.m_interface == core::InterfaceEnum::NONE) {
        return;
    }
    logMessage(tr("Erase started."));
    m_statusbar_progressbar_p->reset();
    m_statusbar_progressbar_p->setStyleSheet(QLatin1String(""));
    m_startofoperation = QDateTime::currentDateTime();
    m_statusbar_label_p->setText(tr("Erasing..."));
    if (m_ext == HexExt) {
        generateMtbDfu(core::EraseAction);
        emit workerSendCommand(channel, m_mtbdfu_filename);
    } else if (m_ext == CyAcd2Ext) {
        emit workerEraseDevice(channel, m_ui.m_file_lineedit_p->text());
    }
    enableOrDisableActions();
}

void MainWindow::workerDeviceConnected(QVector<cydfuht::core::CyChannelSettings> newchannels) {
    m_channellist.insert(m_channellist.end(), newchannels.begin(), newchannels.end());
    for (const auto &channel : newchannels) {
        m_ui.m_port_listwidget_p->addItem(channel.m_displayName);
        if (!m_filterdialog.shouldInterfaceBeDisplayed(channel.m_interface)) {
            QListWidgetItem *newitem = m_ui.m_port_listwidget_p->item(m_ui.m_port_listwidget_p->count() - 1);
            newitem->setHidden(true);
        }
    }
}

void MainWindow::workerDeviceDisconnected(QString name) {
    m_statusbar_progressbar_p->reset();
    m_statusbar_progressbar_p->setStyleSheet(QLatin1String(""));
    for (auto itchannel = m_channellist.begin(); itchannel != m_channellist.end(); /*nothing*/) {
        if ((*itchannel).m_name == name) {
            // Delete from port list widget
            QList<QListWidgetItem *> matchingrows = m_ui.m_port_listwidget_p->findItems((*itchannel).m_displayName, Qt::MatchExactly);
            // There should only just be one item.
            for (auto row : matchingrows) {
                delete row;
            }
            // Erase from channel list
            itchannel = m_channellist.erase(itchannel);
        } else {
            ++itchannel;
        }
    }
}

void MainWindow::workerProgressReport(double percentage) { m_statusbar_progressbar_p->setValue(percentage / 100.0 * ProgressBarWidth); }

void MainWindow::workerReturnValue(int rval) {
    m_statusbar_progressbar_p->setValue(ProgressBarWidth);
    if (rval != CYRET_SUCCESS) {
        m_statusbar_progressbar_p->setStyleSheet(QLatin1String("QProgressBar::chunk {background: rgb(255, 0, 0, 80%);}"));
    }
    logMessage(tr("Operation took %1 milliseconds.").arg(m_startofoperation.msecsTo(QDateTime::currentDateTime())));
    m_startofoperation = QDateTime();
    m_statusbar_label_p->setText(tr("Ready"));
    logMessage(cydfuht::core::cyChannelErrToString(rval));
    if ((rval == CYRET_SUCCESS) || (rval == (CYRET_ERR_BTLDR_MASK | CYBTLDR_STAT_SUCCESS))) {
        m_statusbar_progressbar_p->setStyleSheet(QLatin1String(""));
    }
    enableOrDisableActions();
    if ((rval != CYRET_SUCCESS) && (rval != (CYRET_ERR_COMM_MASK | CYRET_ABORT)) && (rval != CYRET_ABORT)) {
        logMessage(tr("Operation FAILED."));
    }
}

void MainWindow::abortOperation() {
    logMessage(tr("Aborting operation."));
    m_workerobject.abort();
}

static QDir getContentsDir() {
    QDir dir(QApplication::applicationDirPath());

#ifdef Q_OS_WIN32
    return dir;
#elif defined(Q_OS_MAC)
    dir.cdUp();
    return QDir(dir.absolutePath());
#elif defined(Q_OS_LINUX)
    dir.cdUp();
    dir.cd(QLatin1String("share"));
    dir.cd(QFileInfo(QApplication::applicationFilePath()).baseName());
    return QDir(dir.absolutePath());
#else
#error "Unknown OS"
#endif
}

void MainWindow::showDocumentation() {
    QDir dir = getContentsDir();
    if (dir.cd(QLatin1String("docs"))) {
        const QString fileName = QFileInfo(QApplication::applicationFilePath()).baseName() + QLatin1String(".pdf");
        const QString file = dir.absoluteFilePath(fileName);
        if (dir.exists(fileName)) {
            const QUrl url = QUrl::fromLocalFile(file);
            QDesktopServices::openUrl(url);
        } else {
            QMessageBox::critical(this, QApplication::applicationDisplayName(), tr("Cannot find help file: %1").arg(file));
        }
    } else {
        QMessageBox::critical(this, QApplication::applicationDisplayName(), tr("Cannot find 'docs' folder in: %1").arg(dir.absolutePath()));
    }
}

void MainWindow::showAboutBox() {
    QString text = QApplication::tr(
                       "<h2>%1 %2</h2>"
                       "<p>Copyright &copy; 2018-%3, Cypress Semiconductor Corporation (an Infineon company) "
                       "or a subsidiary of Cypress Semiconductor Corporation. All rights reserved. "
                       "Other names and brands may be claimed as property of their respective owners."
                       "<p>Visit: <a href=\"http://www.infineon.com\">www.infineon.com</a>")
                       .arg(QApplication::applicationName())
                       .arg(QApplication::applicationVersion())
                       .arg(QString::number(QDate::currentDate().year()));
    if (!m_logfile.isEmpty()) {
        text = text + QApplication::tr("<p>Log(s): <a href=\"file:///%1\">%1</a>").arg(m_logfile);
    }

    QMessageBox::about(this, QApplication::tr("About"), text);
}

void MainWindow::enableOrDisableActions() {
    // get firmware file: .mtbdfu, .hex or .cyacd2
    QFile firmwareFile(m_ui.m_file_lineedit_p->text());
    bool filevalid = firmwareFile.exists();
    bool portvalid = ((m_ui.m_port_listwidget_p->currentRow() != -1) && ((m_ui.m_stackedwidget_p->currentWidget() != m_ui.m_i2cpage_p) ||
                                                                         m_ui.m_i2c_address_lineedit_p->hasAcceptableInput()));
    m_ext = QFileInfo(firmwareFile).suffix();
    if (filevalid) {
        if (m_ext == HexExt) {
            m_ui.m_fileversion_lineedit_p->setEnabled(true);
            m_ui.m_productid_lineedit_p->setEnabled(true);
            m_ui.m_checksumtype_lineedit_p->setEnabled(true);
            if (m_ui.m_fileversion_lineedit_p->text().isEmpty()) m_ui.m_fileversion_lineedit_p->setText(QLatin1String("0x1"));
            if (m_ui.m_productid_lineedit_p->text().isEmpty()) m_ui.m_productid_lineedit_p->setText(QLatin1String("01020304"));
            if (m_ui.m_checksumtype_lineedit_p->text().isEmpty()) m_ui.m_checksumtype_lineedit_p->setText(QLatin1String("0x0"));

            m_ui.m_generate_pushbutton_p->setVisible(true);
            m_ui.m_formwidget_p->setVisible(true);
        } else if (m_ext == MtbdfuExt) {
            CyBtldr_CustomCommandHeaderData metaHeaderData;
            core::parseJsonHeaderFromFile(m_ui.m_file_lineedit_p->text(), metaHeaderData);
            m_ui.m_fileversion_lineedit_p->setEnabled(false);
            m_ui.m_productid_lineedit_p->setEnabled(false);
            m_ui.m_checksumtype_lineedit_p->setEnabled(false);
            m_ui.m_fileversion_lineedit_p->setText(QStringLiteral("0x%1").arg(metaHeaderData.fileVersion, 0, 16));
            m_ui.m_productid_lineedit_p->setText(QStringLiteral("%1").arg(metaHeaderData.productId, 8, 16, QLatin1Char('0')));
            m_ui.m_checksumtype_lineedit_p->setText(QStringLiteral("0x%1").arg(metaHeaderData.checksumType, 0, 16));
            m_ui.m_generate_pushbutton_p->setVisible(false);
            m_ui.m_formwidget_p->setVisible(true);
        } else if (m_ext == CyAcd2Ext) {
            m_ui.m_formwidget_p->setVisible(false);
        }
    }

    if (m_ext == MtbdfuExt) {
        m_ui.m_program_action_p->setText(core::ExecuteAction);
        m_ui.m_program_action_p->setIconText(core::ExecuteAction);
        m_ui.m_program_action_p->setToolTip(core::ExecuteAction);
    } else {
        m_ui.m_program_action_p->setText(core::ProgramAction);
        m_ui.m_program_action_p->setIconText(core::ProgramAction);
        m_ui.m_program_action_p->setToolTip(core::ProgramAction);
    }
    m_ui.m_program_action_p->setEnabled(filevalid && portvalid && m_startofoperation.isNull());
    m_ui.m_verify_action_p->setEnabled(filevalid && portvalid && m_ext != MtbdfuExt && m_startofoperation.isNull());
    m_ui.m_eraseall_action_p->setEnabled(filevalid && portvalid && m_ext != MtbdfuExt && m_startofoperation.isNull());
    m_ui.m_abort_action_p->setEnabled(!m_startofoperation.isNull());
}
void MainWindow::displayFileError(QString errorStr) {
    m_ui.m_file_lineedit_p->setToolTip(errorStr);
    if (m_fileerror_p == nullptr) {
        m_fileerror_p = m_ui.m_file_lineedit_p->addAction(
            QIcon::fromTheme(QLatin1String("dialog-error"), QIcon(QLatin1String(":/images/error"))), QLineEdit::TrailingPosition);
    }
}

void MainWindow::validateData() {
    QFile firmwareFile(m_ui.m_file_lineedit_p->text());
    QString fileExt = QFileInfo(firmwareFile).suffix();
    if (!firmwareFile.exists()) {
        displayFileError(tr("File does not exist."));
    } else if (fileExt != MtbdfuExt && fileExt != HexExt && fileExt != CyAcd2Ext) {
        displayFileError(tr("Unsupported file format."));
    } else if (m_fileerror_p != nullptr) {
        QString emptyStr;
        m_ui.m_file_lineedit_p->setToolTip(emptyStr);
        m_ui.m_file_lineedit_p->removeAction(m_fileerror_p);
        delete m_fileerror_p;
        m_fileerror_p = nullptr;
    }
    if ((m_ui.m_stackedwidget_p->currentWidget() == m_ui.m_i2cpage_p) && !m_ui.m_i2c_address_lineedit_p->hasAcceptableInput()) {
        QString errorstr = tr("I2C address must be a decimal or hexadecimal number between 8 and 120 (0x78).");
        if (m_i2caddresserror_p == nullptr) {
            m_ui.m_i2c_address_lineedit_p->setToolTip(errorstr);
            m_i2caddresserror_p = m_ui.m_i2c_address_lineedit_p->addAction(
                QIcon::fromTheme(QLatin1String("dialog-error"), QIcon(QLatin1String(":/images/error"))), QLineEdit::TrailingPosition);
        }
    } else if (m_i2caddresserror_p != nullptr) {
        m_ui.m_i2c_address_lineedit_p->removeAction(m_i2caddresserror_p);
        delete m_i2caddresserror_p;
        m_i2caddresserror_p = nullptr;
    }
    enableOrDisableActions();
}

int MainWindow::getI2cSpeed() const {
    if (m_ui.m_i2c_1megahertz_radiobutton_p->isChecked()) {
        return ONE_MHZ_IN_HZ;
    } else if (m_ui.m_i2c_100kilohertz_radiobutton_p->isChecked()) {
        return HUNDRED_KHZ_IN_HZ;
    } else if (m_ui.m_i2c_50kilohertz_radiobutton_p->isChecked()) {
        return FIFTY_KHZ_IN_HZ;
    } else {
        return FOUR_HUNDRED_KHZ_IN_HZ;
    }
}

core::SpiBitOrder MainWindow::getSpiBitOrder() const {
    if (m_ui.m_spi_lsbfirst_radiobutton_p->isChecked()) {
        return core::SpiBitOrder::LSB;
    }
    return core::SpiBitOrder::MSB;
}

core::SpiMode MainWindow::getSpiMode() const {
    if (m_ui.m_spi_mode01_radiobutton_p->isChecked()) {
        return core::SpiMode::MODE1;
    } else if (m_ui.m_spi_mode02_radiobutton_p->isChecked()) {
        return core::SpiMode::MODE2;
    } else if (m_ui.m_spi_mode03_radiobutton_p->isChecked()) {
        return core::SpiMode::MODE3;
    }
    return core::SpiMode::MODE0;
}

void MainWindow::closeEvent(QCloseEvent *event) {
    m_workerobject.abort();
    emit workerClose();
    m_workerthread.exit(0);
    m_workerthread.wait();

    QSettings &settings = getSettings();
    settings.setValue(WindowState, saveState());
    settings.setValue(Cyacd2Filename, m_ui.m_file_lineedit_p->text());
    settings.setValue(I2CIsDisplayed, m_filterdialog.shouldInterfaceBeDisplayed(core::InterfaceEnum::I2C));
    settings.setValue(I2CAddress, m_ui.m_i2c_address_lineedit_p->text());
    settings.setValue(I2CSpeed, getI2cSpeed());
    settings.setValue(SpiIsDisplayed, m_filterdialog.shouldInterfaceBeDisplayed(core::InterfaceEnum::SPI));
    settings.setValue(SpiFreq, m_ui.m_spi_clockspeed_combobox_p->currentText());
    settings.setValue(SpiMode, static_cast<int>(getSpiMode()));
    settings.setValue(SpiBitOrder, static_cast<int>(getSpiBitOrder()));
    settings.setValue(UartIsDisplayed, m_filterdialog.shouldInterfaceBeDisplayed(core::InterfaceEnum::UART));
    settings.setValue(UartBaudRate, m_ui.m_uart_baud_combobox_p->currentText());
    settings.setValue(UartDataBits, m_ui.m_uart_databits_combobox_p->currentText());
    settings.setValue(UartStopBits, m_ui.m_uart_stopbits_combobox_p->currentText());
    settings.setValue(UartParityBits, m_ui.m_uart_parity_combobox_p->currentText());
}

}  // namespace gui
}  // namespace cydfuht
