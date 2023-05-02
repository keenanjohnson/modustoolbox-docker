/*
 * Copyright 2018-2022 Cypress Semiconductor Corporation (an Infineon company)
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

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>
#include <QProcess>
#include <cstdlib>
#include <iostream>
#include <memory>

#include "cliobject.h"
#include "cybridge.h"
#include "cychannelsettings.h"
#include "cychannelspi.h"
#include "cycommandlineparser.h"
#include "cydfuhtcore.h"
#include "cylog.h"
#include "cyversion.h"
#include "dfuh_tool.h"
#include "settings.h"

static const QLatin1String CYACD2_FILE_VALUENAME = QLatin1String("cyacd2_file");
static const QLatin1String INTEGER_VALUENAME = QLatin1String("int");
static const QLatin1String FLOAT_VALUENAME = QLatin1String("float");
static const QLatin1String STRING_VALUENAME = QLatin1String("string");

static const QLatin1String DISPLAY_HW_OPTION = QLatin1String("display-hw");
static const QLatin1String PROGRAM_OPTION = QLatin1String("program-device");
static const QLatin1String VERIFY_OPTION = QLatin1String("verify-device");
static const QLatin1String ERASE_OPTION = QLatin1String("erase-device");
static const QLatin1String HWID_OPTION = QLatin1String("hwid");
static const QLatin1String I2C_ADDRESS_OPTION = QLatin1String("i2c-address");
static const QLatin1String I2C_SPEED_OPTION = QLatin1String("i2c-speed");
static const QLatin1String SPI_CLOCKSPEED_OPTION = QLatin1String("spi-clockspeed");
static const QLatin1String SPI_MODE_OPTION = QLatin1String("spi-mode");
static const QLatin1String SPI_LSB_OPTION = QLatin1String("spi-lsb-first");
static const QLatin1String UART_BAUDRATE_OPTION = QLatin1String("uart-baudrate");
static const QLatin1String UART_DATABITS_OPTION = QLatin1String("uart-databits");
static const QLatin1String UART_PARITYTYPE_OPTION = QLatin1String("uart-paritytype");
static const QLatin1String UART_STOPBITS_OPTION = QLatin1String("uart-stopbits");

static void displayHardware() {
    cydfuht::core::CyDfuhtCore core(nullptr);
    auto channels = core.retrieveAllChannels();
    for (const auto &channel : channels) {
        std::cout << cydfuht::core::interfaceEnumToString(channel.m_interface) << ": " << channel.m_name.toStdString() << std::endl;
    }
}

static QString findFirstDevice(cydfuht::core::InterfaceEnum interface) {
    cydfuht::core::CyDfuhtCore core(nullptr);
    auto channels = core.retrieveAllChannels();
    for (const auto &channel : channels) {
        if (channel.m_interface == interface) {
            return channel.m_name;
        }
    }
    return QString();
}

int commandLineMode(const QCommandLineParser &parser) {
    bool program_mode = parser.isSet(PROGRAM_OPTION);
    bool verify_mode = parser.isSet(VERIFY_OPTION);
    bool erase_mode = parser.isSet(ERASE_OPTION);
    bool i2c_address_given = parser.isSet(I2C_ADDRESS_OPTION);
    bool i2c_speed_given = parser.isSet(I2C_SPEED_OPTION);
    bool spi_clockspeed_given = parser.isSet(SPI_CLOCKSPEED_OPTION);
    bool spi_mode_given = parser.isSet(SPI_MODE_OPTION);
    bool uart_baudrate_given = parser.isSet(UART_BAUDRATE_OPTION);
    bool uart_databits_given = parser.isSet(UART_DATABITS_OPTION);
    bool uart_paritytype_given = parser.isSet(UART_PARITYTYPE_OPTION);
    bool uart_stopbits_given = parser.isSet(UART_STOPBITS_OPTION);

    if ((int)program_mode + (int)verify_mode + (int)erase_mode > 1) {
        std::cout << "Please provide only one of --" << PROGRAM_OPTION.latin1() << ", --" << VERIFY_OPTION.latin1() << " and --"
                  << ERASE_OPTION.latin1() << "." << std::endl;
        return 1;
    }
    int i2c_options_given = (int)i2c_address_given + (int)i2c_speed_given;
    int spi_options_given = (int)spi_clockspeed_given + (int)spi_mode_given;
    int uart_options_given = (int)uart_baudrate_given + (int)uart_databits_given + (int)uart_paritytype_given + (int)uart_stopbits_given;
    if ((i2c_options_given == 0) && (spi_options_given == 0) && (uart_options_given == 0)) {
        std::cout << "Please provide configuration parameters for the communication protocol to use (I2C or SPI or UART)." << std::endl;
        return 19;
    }
    if (((i2c_options_given != 0) && (spi_options_given != 0)) || ((i2c_options_given != 0) && (uart_options_given != 0)) ||
        ((spi_options_given != 0) && (uart_options_given != 0))) {
        std::cout << "Please provide configuration parameters for only one protocol." << std::endl;
        return 2;
    }

    cydfuht::core::CyChannelSettings settings;
    bool ok;
    settings.m_name = parser.value(HWID_OPTION);

    if (i2c_options_given > 0) {
        if (i2c_options_given == 1) {
            std::cout << "Please provide both --" << I2C_ADDRESS_OPTION.latin1() << " and --" << I2C_SPEED_OPTION.latin1()
                      << " to use the I2C protocol." << std::endl;
            return 3;
        }
        if (settings.m_name.isEmpty()) {
            settings.m_name = findFirstDevice(cydfuht::core::InterfaceEnum::I2C);
        }
        settings.m_interface = cydfuht::core::InterfaceEnum::I2C;
        settings.m_i2c.m_freq_hz = parser.value(I2C_SPEED_OPTION).toUInt(&ok) * 1000;
        if (!ok) {
            std::cout << "The argument provided to --" << I2C_SPEED_OPTION.latin1() << " was not a decimal number." << std::endl;
            return 4;
        }
        uint addr = parser.value(I2C_ADDRESS_OPTION).toUInt(&ok, 0);
        if (!ok) {
            std::cout << "The argument provided to --" << I2C_ADDRESS_OPTION.latin1() << " was not a decimal, hexadecimal or octal number."
                      << std::endl;
            return 5;
        }
        if ((addr < 8) || (addr > 120)) {
            std::cout << "The argument provided to --" << I2C_ADDRESS_OPTION.latin1() << " was not a number between 8 and 120."
                      << std::endl;
            return 6;
        }
        settings.m_i2c.m_addr = static_cast<uint8_t>(addr);
    } else if (spi_options_given > 0) {
        if (spi_options_given == 1) {
            std::cout << "Please provide both --" << SPI_CLOCKSPEED_OPTION.latin1() << " and --" << SPI_MODE_OPTION.latin1()
                      << " to use the SPI protocol." << std::endl;
            return 7;
        }
        if (settings.m_name.isEmpty()) {
            settings.m_name = findFirstDevice(cydfuht::core::InterfaceEnum::SPI);
        }
        settings.m_interface = cydfuht::core::InterfaceEnum::SPI;
        settings.m_spi.m_freq_hz = static_cast<int>(parser.value(SPI_CLOCKSPEED_OPTION).toDouble(&ok) * 1000000.0);
        if (!ok) {
            std::cout << "The argument provided to --" << SPI_CLOCKSPEED_OPTION.latin1() << " was not a floating-point number."
                      << std::endl;
            return 8;
        }
        uint32_t spimode = parser.value(SPI_MODE_OPTION).toUInt(&ok, 2);
        if (!ok) {
            std::cout << "The argument provided to --" << SPI_MODE_OPTION.latin1() << " was not a binary number." << std::endl;
            return 9;
        }
        if (spimode > 3) {
            std::cout << "The argument provided to --" << SPI_MODE_OPTION.latin1() << " was not a binary number between 0 and 11."
                      << std::endl;
            return 10;
        }
        settings.m_spi.m_mode = static_cast<cydfuht::core::SpiMode>(spimode);
        if (parser.isSet(SPI_LSB_OPTION)) {
            settings.m_spi.m_bit_order = cydfuht::core::SpiBitOrder::LSB;
        } else {
            settings.m_spi.m_bit_order = cydfuht::core::SpiBitOrder::MSB;
        }
    } else {  // must be UART
        if (uart_options_given < 4) {
            std::cout << "Please provide the --" << UART_BAUDRATE_OPTION.latin1() << ", --" << UART_DATABITS_OPTION.latin1() << ", --"
                      << UART_PARITYTYPE_OPTION.latin1() << " and --" << UART_STOPBITS_OPTION.latin1()
                      << " options to use the UART protocol." << std::endl;
            return 11;
        }
        if (settings.m_name.isEmpty()) {
            settings.m_name = findFirstDevice(cydfuht::core::InterfaceEnum::UART);
        }
        settings.m_interface = cydfuht::core::InterfaceEnum::UART;
        settings.m_uart.m_baudrate = parser.value(UART_BAUDRATE_OPTION).toUInt(&ok);
        if (!ok) {
            std::cout << "The argument provided to --" << UART_BAUDRATE_OPTION.latin1() << " was not a decimal number." << std::endl;
            return 12;
        }
        uint32_t stopbits = parser.value(UART_DATABITS_OPTION).toUInt(&ok);
        if (!ok) {
            std::cout << "The argument provided to --" << UART_DATABITS_OPTION.latin1() << " was not a decimal number." << std::endl;
            return 13;
        }
        if ((stopbits != 7) && (stopbits != 8)) {
            std::cout << "The argument provided to --" << UART_DATABITS_OPTION.latin1() << " was neither 7 nor 8." << std::endl;
            return 14;
        }
        settings.m_uart.m_databits = static_cast<uint8_t>(stopbits);
        QString paritytype = parser.value(UART_PARITYTYPE_OPTION);
        if (QString::compare(paritytype, QObject::tr("None"), Qt::CaseInsensitive) == 0) {
            settings.m_uart.m_paritytype = cydfuht::core::ParityTypeEnum::NONE;
        } else if (QString::compare(paritytype, QObject::tr("Odd"), Qt::CaseInsensitive) == 0) {
            settings.m_uart.m_paritytype = cydfuht::core::ParityTypeEnum::ODD;
        } else if (QString::compare(paritytype, QObject::tr("Even"), Qt::CaseInsensitive) == 0) {
            settings.m_uart.m_paritytype = cydfuht::core::ParityTypeEnum::EVEN;
        } else {
            std::cout << "The argument provided to --" << UART_PARITYTYPE_OPTION.latin1() << " was not \"None\", \"Odd\" or \"Even\"."
                      << std::endl;
            return 15;
        }
        settings.m_uart.m_stopbits = parser.value(UART_STOPBITS_OPTION).toFloat(&ok);
        if (!ok) {
            std::cout << "The argument provided to --" << UART_STOPBITS_OPTION.latin1() << " was not a floating-point number." << std::endl;
            return 16;
        }
        if ((settings.m_uart.m_stopbits != 1.0) && (settings.m_uart.m_stopbits != 1.5) && (settings.m_uart.m_stopbits != 2.0)) {
            std::cout << "The argument provided to --" << UART_STOPBITS_OPTION.latin1() << " was not 1.0, 1.5 or 2.0." << std::endl;
            return 17;
        }
    }
    if (settings.m_name.isEmpty()) {
        std::cout << "No appropriate hardware was found." << std::endl;
        return 18;
    }

    cydfuht::cli::CliObject cliobject;
    if (program_mode) {
        cliobject.program(settings, parser.value(PROGRAM_OPTION));
    } else if (verify_mode) {
        cliobject.verify(settings, parser.value(VERIFY_OPTION));
    } else {
        cliobject.erase(settings, parser.value(ERASE_OPTION));
    }

    return QCoreApplication::exec();
}

int main(int argc, char *argv[]) {
    try {
        // Needed to handle numbers consistently. Locale-awareness is not currently supported.
        static const QLocale ENGLISH(QLocale::Language::English, QLocale::Country::UnitedStates);
        QLocale::setDefault(ENGLISH);

        const QCoreApplication app(argc, argv);
        cydfuht::core::ICyLog::initLog();

#ifdef Q_OS_DARWIN
        // On MacOS QCoreApplication::applicationName is set to CFBundleName by default, change it back.
        QCoreApplication::setApplicationName(QFileInfo(QCoreApplication::applicationFilePath()).baseName());
#endif
        QCoreApplication::setOrganizationDomain(QLatin1String("cypress.com"));
        QCoreApplication::setOrganizationName(cydfuht::newOrganizationName());
        auto version = cydfuht::core::CyVersion::loadFromVersionFile(QCoreApplication::applicationDirPath());
        QCoreApplication::setApplicationVersion(
            QCoreApplication::tr("%1.%2.%3.%4").arg(VERSION_MAJOR).arg(VERSION_MINOR).arg(VERSION_PATCH).arg(version.Build));

        cydfuht::core::CyCommandLineParser parser;
        parser.addHelpOption();
        parser.addVersionOption();

        QCommandLineOption display_hw_option =
            QCommandLineOption(DISPLAY_HW_OPTION, QObject::tr("Output all compatible hardware attached to the computer and exit."));
        parser.addOption(display_hw_option);
        QCommandLineOption program_option =
            QCommandLineOption(PROGRAM_OPTION, QObject::tr("Program the device with the specified file and exit."), CYACD2_FILE_VALUENAME);
        parser.addOption(program_option);
        QCommandLineOption verify_option = QCommandLineOption(
            VERIFY_OPTION, QObject::tr("Verify the programming of the device with the specified file and exit."), CYACD2_FILE_VALUENAME);
        parser.addOption(verify_option);
        QCommandLineOption erase_option =
            QCommandLineOption(ERASE_OPTION, QObject::tr("Erase the specified program from the device and exit."), CYACD2_FILE_VALUENAME);
        parser.addOption(erase_option);

        QCommandLineOption hwid_option =
            QCommandLineOption(HWID_OPTION,
                               QObject::tr("Specifies the ID of the hardware to program/verify/erase.  If this "
                                           "option is skipped, the first appropriate device found will be used."),
                               STRING_VALUENAME);
        parser.addOption(hwid_option);

        QCommandLineOption i2c_address_option = QCommandLineOption(
            I2C_ADDRESS_OPTION, QObject::tr("Sets the address for the I2C protocol.  Valid values are between 8 (0x08) and 120 (0x78)."),
            INTEGER_VALUENAME);
        parser.addOption(i2c_address_option);
        QCommandLineOption i2c_speed_option = QCommandLineOption(
            I2C_SPEED_OPTION, QObject::tr("Sets the speed for the I2C protocol in kHz.  Common values are 50, 100, 400 and 1000."),
            INTEGER_VALUENAME);
        parser.addOption(i2c_speed_option);

        QCommandLineOption spi_clockspeed_option =
            QCommandLineOption(SPI_CLOCKSPEED_OPTION, QObject::tr("Sets the clock speed for the SPI protocol in MHz."), FLOAT_VALUENAME);
        parser.addOption(spi_clockspeed_option);
        QCommandLineOption spi_mode_option = QCommandLineOption(
            SPI_MODE_OPTION, QObject::tr("Sets the mode for the SPI protocol in binary.  Valid values are 00, 01, 10 and 11."),
            INTEGER_VALUENAME);
        parser.addOption(spi_mode_option);
        QCommandLineOption spi_lsb_option = QCommandLineOption(
            SPI_LSB_OPTION, QObject::tr("Specifies that the least-significant bit should be sent first for command-line use "
                                        "of the SPI protocol.  Otherwise, the most-significant bit will be sent first."));
        parser.addOption(spi_lsb_option);

        QCommandLineOption uart_baudrate_option =
            QCommandLineOption(UART_BAUDRATE_OPTION, QObject::tr("Sets the baud rate for the UART protocol."), INTEGER_VALUENAME);
        parser.addOption(uart_baudrate_option);
        QCommandLineOption uart_databits_option =
            QCommandLineOption(UART_DATABITS_OPTION, QObject::tr("Sets the number of data bits for the UART protocol."), INTEGER_VALUENAME);
        parser.addOption(uart_databits_option);
        QCommandLineOption uart_paritytype_option = QCommandLineOption(
            UART_PARITYTYPE_OPTION,
            QObject::tr("Sets the parity type for the UART protocol.  Valid strings are \"None\", \"Odd\" and \"Even\"."),
            STRING_VALUENAME);
        parser.addOption(uart_paritytype_option);
        QCommandLineOption uart_stopbits_option =
            QCommandLineOption(UART_STOPBITS_OPTION,
                               QObject::tr("Sets the stop bits for the UART protocol.  Valid values are 1, 1.5 and 2."), FLOAT_VALUENAME);
        parser.addOption(uart_stopbits_option);

        parser.process(app, true);

        if (parser.isSet(DISPLAY_HW_OPTION)) {
            displayHardware();
            return 0;
        }
        if (!parser.isSet(PROGRAM_OPTION) && !parser.isSet(VERIFY_OPTION) && !parser.isSet(ERASE_OPTION)) {
            std::cout << "Please provide one of --" << PROGRAM_OPTION.latin1() << ", --" << VERIFY_OPTION.latin1() << " and --"
                      << ERASE_OPTION.latin1() << "." << std::endl;
            return 20;
        }
        return commandLineMode(parser);
    } catch (const std::exception &e) {
        qDebug() << "Unexpected failure. " << e.what();
        return QProcess::ExitStatus::CrashExit;
    }
}
