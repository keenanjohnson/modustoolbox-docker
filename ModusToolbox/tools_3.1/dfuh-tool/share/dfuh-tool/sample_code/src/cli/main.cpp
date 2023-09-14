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

static const QLatin1String MTBDFU_FILE_VALUENAME = QLatin1String("mtbdfu_file");
static const QLatin1String CYACD2_FILE_VALUENAME = QLatin1String("cyacd2_file");
static const QLatin1String JSON_FILE_VALUENAME = QLatin1String("json_file");
static const QLatin1String INTEGER_VALUENAME = QLatin1String("int");
static const QLatin1String FLOAT_VALUENAME = QLatin1String("float");
static const QLatin1String STRING_VALUENAME = QLatin1String("string");
static const bool FORCE_DEBUG_OUTPUT = true;
static const QLatin1String hexExt = QLatin1String("hex");
static const QLatin1String cyAcd2Ext = QLatin1String("cyacd2");
static const QLatin1String mtbDfuExt = QLatin1String("mtbdfu");

static const QLatin1String DEBUG_OUTPUT_OPTION = QLatin1String("debug");
static const QLatin1String DISPLAY_HW_OPTION = QLatin1String("display-hw");
static const QLatin1String GENERATE_MTBDFU_OPTION = QLatin1String("generate-mtbdfu");
static const QLatin1String FILE_VERSION_OPTION = QLatin1String("file-version");
static const QLatin1String PRODUCT_ID_OPTION = QLatin1String("product-id");
static const QLatin1String APPLICATION_ID_OPTION = QLatin1String("application-id");
static const QLatin1String APPLICATION_START_OPTION = QLatin1String("application-start");
static const QLatin1String APPLICATION_LENGTH_OPTION = QLatin1String("application-length");
static const QLatin1String CHECKSUM_TYPE_OPTION = QLatin1String("checksum-type");
static const QLatin1String MTBDFU_DATA_FILE_OPTION = QLatin1String("mtbdfu-data-file");
static const QLatin1String PROGRAM_OPTION = QLatin1String("program-device");
static const QLatin1String VERIFY_OPTION = QLatin1String("verify-device");
static const QLatin1String ERASE_OPTION = QLatin1String("erase-device");
static const QLatin1String CUSTOM_COMMAND_OPTION = QLatin1String("custom-command");
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

static const int CLI_NO_ERROR = 0;
static const int CLI_TOO_MUCH_ACTIONS_ERROR = 1;
static const int CLI_TOO_MUCH_INTERFACES_ERROR = 2;
static const int CLI_I2C_NOT_ENOUGH_PARAMS_ERROR = 3;
static const int CLI_I2C_INCORRECT_SPEED_ERROR = 4;
static const int CLI_I2C_INCORRECT_ADDRESS_FORMAT_ERROR = 5;
static const int CLI_I2C_INCORRECT_ADDRESS_RANGE_ERROR = 6;
static const int CLI_SPI_NOT_ENOUGH_PARAMS_ERROR = 7;
static const int CLI_SPI_INCORRECT_CLOCKSPEED_ERROR = 8;
static const int CLI_SPI_INCORRECT_MODE_FORMAT_ERROR = 9;
static const int CLI_SPI_INCORRECT_MODE_RANGE_ERROR = 10;
static const int CLI_UART_NOT_ENOUGH_PARAMS_ERROR = 11;
static const int CLI_UART_INCORRECT_BAUDRATE_FORMAT_ERROR = 12;
static const int CLI_UART_INCORRECT_DATABITS_FORMAT_ERROR = 13;
static const int CLI_UART_INCORRECT_DATABITS_VALUE_ERROR = 14;
static const int CLI_UART_INCORRECT_PARITY_VALUE_ERROR = 15;
static const int CLI_UART_INCORRECT_STOPBITS_FORMAT_ERROR = 16;
static const int CLI_UART_INCORRECT_STOPBITS_VALUE_ERROR = 17;
static const int CLI_NO_HARDWARE_ERROR = 18;
static const int CLI_NO_INTERFACE_ERROR = 19;
static const int CLI_NO_ACTION_ERROR = 20;
static const int CLI_INCORRECT_FILE_EXNTESION_ERROR = 21;
static const int CLI_NO_MTBDFU_FILE_ERROR = 22;

static void displayHardware() {
    cydfuht::core::CyDfuhtCore core(nullptr);
    auto channels = core.retrieveAllChannels();
    for (const auto &channel : channels) {
        std::cout << cydfuht::core::interfaceEnumToString(channel.m_interface) << ": " << channel.m_name.toStdString();
        if (!channel.m_productName.isEmpty()) {
            std::cout << " (" << channel.m_productName.toStdString() << ")";
        }
        std::cout << std::endl;
    }
}

static void generateMtbDfu(const QCommandLineParser &parser, QString &mtbdfuFilename, QString mtbdfuDataFilename,
                           QString action = cydfuht::core::NoAction) {
    bool ok;
    CyBtldr_CustomCommandHeaderData metaHeaderData;
    metaHeaderData.fileVersion = parser.isSet(FILE_VERSION_OPTION) ? parser.value(FILE_VERSION_OPTION).toUInt(&ok, 0) : 0x1;
    metaHeaderData.productId =
        parser.isSet(PRODUCT_ID_OPTION) ? parser.value(PRODUCT_ID_OPTION).toUInt(&ok, 16) : 0x01020304;  // default value for PSoC 6 devices
    metaHeaderData.applicationId = parser.isSet(APPLICATION_ID_OPTION) ? parser.value(APPLICATION_ID_OPTION).toUInt(&ok, 16) : 0x0;
    metaHeaderData.applicationStart = parser.isSet(APPLICATION_START_OPTION) ? parser.value(APPLICATION_START_OPTION).toUInt(&ok, 16) : 0x0;
    metaHeaderData.applicationLength =
        parser.isSet(APPLICATION_LENGTH_OPTION) ? parser.value(APPLICATION_LENGTH_OPTION).toUInt(&ok, 16) : 0x0;
    metaHeaderData.checksumType = parser.isSet(CHECKSUM_TYPE_OPTION) ? parser.value(CHECKSUM_TYPE_OPTION).toUInt(&ok, 0) : 0x0;

    cydfuht::core::generateMtbDfuJson(mtbdfuDataFilename, action, mtbdfuFilename, metaHeaderData);
    std::cout << mtbdfuFilename.toStdString() << " was generated." << std::endl;
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
    bool custom_command_mode = parser.isSet(CUSTOM_COMMAND_OPTION);
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
        return CLI_TOO_MUCH_ACTIONS_ERROR;
    }
    int i2c_options_given = (int)i2c_address_given + (int)i2c_speed_given;
    int spi_options_given = (int)spi_clockspeed_given + (int)spi_mode_given;
    int uart_options_given = (int)uart_baudrate_given + (int)uart_databits_given + (int)uart_paritytype_given + (int)uart_stopbits_given;
    if ((i2c_options_given == 0) && (spi_options_given == 0) && (uart_options_given == 0)) {
        std::cout << "Please provide configuration parameters for the communication protocol to use (I2C or SPI or UART)." << std::endl;
        return CLI_NO_INTERFACE_ERROR;
    }
    if (((i2c_options_given != 0) && (spi_options_given != 0)) || ((i2c_options_given != 0) && (uart_options_given != 0)) ||
        ((spi_options_given != 0) && (uart_options_given != 0))) {
        std::cout << "Please provide configuration parameters for only one protocol." << std::endl;
        return CLI_TOO_MUCH_INTERFACES_ERROR;
    }

    cydfuht::core::CyChannelSettings settings;
    bool ok;
    settings.m_name = parser.value(HWID_OPTION);

    if (i2c_options_given > 0) {
        if (i2c_options_given == 1) {
            std::cout << "Please provide both --" << I2C_ADDRESS_OPTION.latin1() << " and --" << I2C_SPEED_OPTION.latin1()
                      << " to use the I2C protocol." << std::endl;
            return CLI_I2C_NOT_ENOUGH_PARAMS_ERROR;
        }
        if (settings.m_name.isEmpty()) {
            settings.m_name = findFirstDevice(cydfuht::core::InterfaceEnum::I2C);
        }
        settings.m_interface = cydfuht::core::InterfaceEnum::I2C;
        settings.m_i2c.m_freq_hz = parser.value(I2C_SPEED_OPTION).toUInt(&ok) * 1000;
        if (!ok) {
            std::cout << "The argument provided to --" << I2C_SPEED_OPTION.latin1() << " was not a decimal number." << std::endl;
            return CLI_I2C_INCORRECT_SPEED_ERROR;
        }
        uint addr = parser.value(I2C_ADDRESS_OPTION).toUInt(&ok, 0);
        if (!ok) {
            std::cout << "The argument provided to --" << I2C_ADDRESS_OPTION.latin1() << " was not a decimal, hexadecimal or octal number."
                      << std::endl;
            return CLI_I2C_INCORRECT_ADDRESS_FORMAT_ERROR;
        }
        if ((addr < 8) || (addr > 120)) {
            std::cout << "The argument provided to --" << I2C_ADDRESS_OPTION.latin1() << " was not a number between 8 and 120."
                      << std::endl;
            return CLI_I2C_INCORRECT_ADDRESS_RANGE_ERROR;
        }
        settings.m_i2c.m_addr = static_cast<uint8_t>(addr);
    } else if (spi_options_given > 0) {
        if (spi_options_given == 1) {
            std::cout << "Please provide both --" << SPI_CLOCKSPEED_OPTION.latin1() << " and --" << SPI_MODE_OPTION.latin1()
                      << " to use the SPI protocol." << std::endl;
            return CLI_SPI_NOT_ENOUGH_PARAMS_ERROR;
        }
        if (settings.m_name.isEmpty()) {
            settings.m_name = findFirstDevice(cydfuht::core::InterfaceEnum::SPI);
        }
        settings.m_interface = cydfuht::core::InterfaceEnum::SPI;
        settings.m_spi.m_freq_hz = static_cast<int>(parser.value(SPI_CLOCKSPEED_OPTION).toDouble(&ok) * 1000000.0);
        if (!ok) {
            std::cout << "The argument provided to --" << SPI_CLOCKSPEED_OPTION.latin1() << " was not a floating-point number."
                      << std::endl;
            return CLI_SPI_INCORRECT_CLOCKSPEED_ERROR;
        }
        uint32_t spimode = parser.value(SPI_MODE_OPTION).toUInt(&ok, 2);
        if (!ok) {
            std::cout << "The argument provided to --" << SPI_MODE_OPTION.latin1() << " was not a binary number." << std::endl;
            return CLI_SPI_INCORRECT_MODE_FORMAT_ERROR;
        }
        if (spimode > 3) {
            std::cout << "The argument provided to --" << SPI_MODE_OPTION.latin1() << " was not a binary number between 0 and 11."
                      << std::endl;
            return CLI_SPI_INCORRECT_MODE_RANGE_ERROR;
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
            return CLI_UART_NOT_ENOUGH_PARAMS_ERROR;
        }
        if (settings.m_name.isEmpty()) {
            settings.m_name = findFirstDevice(cydfuht::core::InterfaceEnum::UART);
        }
        settings.m_interface = cydfuht::core::InterfaceEnum::UART;
        settings.m_uart.m_baudrate = parser.value(UART_BAUDRATE_OPTION).toUInt(&ok);
        if (!ok) {
            std::cout << "The argument provided to --" << UART_BAUDRATE_OPTION.latin1() << " was not a decimal number." << std::endl;
            return CLI_UART_INCORRECT_BAUDRATE_FORMAT_ERROR;
        }
        uint32_t stopbits = parser.value(UART_DATABITS_OPTION).toUInt(&ok);
        if (!ok) {
            std::cout << "The argument provided to --" << UART_DATABITS_OPTION.latin1() << " was not a decimal number." << std::endl;
            return CLI_UART_INCORRECT_DATABITS_FORMAT_ERROR;
        }
        if ((stopbits != 7) && (stopbits != 8)) {
            std::cout << "The argument provided to --" << UART_DATABITS_OPTION.latin1() << " was neither 7 nor 8." << std::endl;
            return CLI_UART_INCORRECT_DATABITS_VALUE_ERROR;
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
            return CLI_UART_INCORRECT_PARITY_VALUE_ERROR;
        }
        settings.m_uart.m_stopbits = parser.value(UART_STOPBITS_OPTION).toFloat(&ok);
        if (!ok) {
            std::cout << "The argument provided to --" << UART_STOPBITS_OPTION.latin1() << " was not a floating-point number." << std::endl;
            return CLI_UART_INCORRECT_STOPBITS_FORMAT_ERROR;
        }
        if ((settings.m_uart.m_stopbits != 1.0) && (settings.m_uart.m_stopbits != 1.5) && (settings.m_uart.m_stopbits != 2.0)) {
            std::cout << "The argument provided to --" << UART_STOPBITS_OPTION.latin1() << " was not 1.0, 1.5 or 2.0." << std::endl;
            return CLI_UART_INCORRECT_STOPBITS_VALUE_ERROR;
        }
    }
    if (settings.m_name.isEmpty()) {
        std::cout << "No appropriate hardware was found." << std::endl;
        return CLI_NO_HARDWARE_ERROR;
    }

    QString fileExt;
    if (custom_command_mode) {
        QString fileExt = QFileInfo(QFile(parser.value(CUSTOM_COMMAND_OPTION))).suffix();
        if (fileExt != mtbDfuExt) {
            std::cout << "Incorrect file was passed. Please pass .mtbdfu file as an argument to --" << CUSTOM_COMMAND_OPTION.latin1()
                      << std::endl;
            return CLI_NO_MTBDFU_FILE_ERROR;
        }
    } else {
        QLatin1String actionOption = QLatin1String("");
        if (program_mode) {
            fileExt = QFileInfo(QFile(parser.value(PROGRAM_OPTION))).suffix();
            actionOption = PROGRAM_OPTION;
        } else if (verify_mode) {
            fileExt = QFileInfo(QFile(parser.value(VERIFY_OPTION))).suffix();
            actionOption = VERIFY_OPTION;
        } else if (erase_mode) {
            fileExt = QFileInfo(QFile(parser.value(ERASE_OPTION))).suffix();
            actionOption = ERASE_OPTION;
        }
        if (fileExt != hexExt && fileExt != cyAcd2Ext) {
            std::cout << "Incorrect file was passed. Please pass .cyacd2 or .hex as an argument to --" << actionOption.latin1()
                      << std::endl;
            return CLI_INCORRECT_FILE_EXNTESION_ERROR;
        }
    }

    cydfuht::cli::CliObject cliobject;

    QString mtbdfuDataFilename = QLatin1String("");
    if (program_mode) {
        if (fileExt == hexExt) {
            generateMtbDfu(parser, mtbdfuDataFilename, parser.value(PROGRAM_OPTION), cydfuht::core::ProgramAction);
            cliobject.sendCommand(settings, mtbdfuDataFilename);
        } else if (fileExt == cyAcd2Ext) {
            cliobject.program(settings, parser.value(PROGRAM_OPTION));
        }
    } else if (verify_mode) {
        if (fileExt == hexExt) {
            generateMtbDfu(parser, mtbdfuDataFilename, parser.value(VERIFY_OPTION), cydfuht::core::VerifyAction);
            cliobject.sendCommand(settings, mtbdfuDataFilename);
        } else if (fileExt == cyAcd2Ext) {
            cliobject.verify(settings, parser.value(VERIFY_OPTION));
        }
    } else if (erase_mode) {
        if (fileExt == hexExt) {
            generateMtbDfu(parser, mtbdfuDataFilename, parser.value(ERASE_OPTION), cydfuht::core::EraseAction);
            cliobject.sendCommand(settings, mtbdfuDataFilename);
        } else if (fileExt == cyAcd2Ext) {
            cliobject.erase(settings, parser.value(ERASE_OPTION));
        }
    } else if (custom_command_mode) {
        cliobject.sendCommand(settings, parser.value(CUSTOM_COMMAND_OPTION));
    }

    return QCoreApplication::exec();
}

int main(int argc, char *argv[]) {
    try {
        // Needed to handle numbers consistently. Locale-awareness is not currently supported.
        static const QLocale ENGLISH(QLocale::Language::English, QLocale::Country::UnitedStates);
        QLocale::setDefault(ENGLISH);

        const QCoreApplication app(argc, argv);
        cydfuht::core::ICyLog *iCyLog = cydfuht::core::ICyLog::initLog();
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

        QCommandLineOption debug_output_option =
            QCommandLineOption(DEBUG_OUTPUT_OPTION, QObject::tr("Output all debug messages during DFU process."));
        parser.addOption(debug_output_option);
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
        QCommandLineOption custom_command_option =
            QCommandLineOption(CUSTOM_COMMAND_OPTION, QObject::tr("Send user-defined custom commands."), JSON_FILE_VALUENAME);
        parser.addOption(custom_command_option);

        QCommandLineOption generate_mtbdfu_option =
            QCommandLineOption(GENERATE_MTBDFU_OPTION, QObject::tr("Generate a basic .mtbdfu example."), MTBDFU_FILE_VALUENAME);
        parser.addOption(generate_mtbdfu_option);
        QCommandLineOption file_version_option =
            QCommandLineOption(FILE_VERSION_OPTION, QObject::tr("Define file version for generated .mtbdfu example."), STRING_VALUENAME);
        parser.addOption(file_version_option);
        QCommandLineOption product_id_option =
            QCommandLineOption(PRODUCT_ID_OPTION, QObject::tr("Define product ID for generated .mtbdfu example."), STRING_VALUENAME);
        parser.addOption(product_id_option);
        QCommandLineOption application_id_option = QCommandLineOption(
            APPLICATION_ID_OPTION, QObject::tr("Define application ID for generated .mtbdfu example."), STRING_VALUENAME);
        parser.addOption(application_id_option);
        QCommandLineOption application_start_option = QCommandLineOption(
            APPLICATION_START_OPTION, QObject::tr("Define application start address for generated .mtbdfu example."), STRING_VALUENAME);
        parser.addOption(application_start_option);
        QCommandLineOption application_length_option = QCommandLineOption(
            APPLICATION_LENGTH_OPTION, QObject::tr("Define application length for generated .mtbdfu example."), STRING_VALUENAME);
        parser.addOption(application_length_option);
        QCommandLineOption checksum_type_option = QCommandLineOption(
            CHECKSUM_TYPE_OPTION, QObject::tr("Define packet checksum type for generated .mtbdfu example."), STRING_VALUENAME);
        parser.addOption(checksum_type_option);
        QCommandLineOption mtbdfu_data_file_option = QCommandLineOption(
            MTBDFU_DATA_FILE_OPTION, QObject::tr("Define a hex file as an input data for generated .mtbdfu file."), STRING_VALUENAME);
        parser.addOption(mtbdfu_data_file_option);

        QCommandLineOption hwid_option =
            QCommandLineOption(HWID_OPTION,
                               QObject::tr("Specifies the ID of the hardware to program/verify/erase. If this "
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
            SPI_MODE_OPTION, QObject::tr("Sets the mode for the SPI protocol in binary. Valid values are 00, 01, 10 and 11."),
            INTEGER_VALUENAME);
        parser.addOption(spi_mode_option);
        QCommandLineOption spi_lsb_option = QCommandLineOption(
            SPI_LSB_OPTION, QObject::tr("Specifies that the least-significant bit should be sent first for command line use "
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

        if (parser.isSet(DEBUG_OUTPUT_OPTION)) {
            iCyLog->setDebugOutputMode(FORCE_DEBUG_OUTPUT);
        }

        if (parser.isSet(GENERATE_MTBDFU_OPTION)) {
            QString mtbdfuDataFilename = parser.isSet(MTBDFU_DATA_FILE_OPTION) ? parser.value(MTBDFU_DATA_FILE_OPTION) : QLatin1String("");
            QString mtbdfuFilename = parser.value(GENERATE_MTBDFU_OPTION);
            generateMtbDfu(parser, mtbdfuFilename, mtbdfuDataFilename);
            return CLI_NO_ERROR;
        }

        if (parser.isSet(DISPLAY_HW_OPTION)) {
            displayHardware();
            return CLI_NO_ERROR;
        }

        if (!parser.isSet(PROGRAM_OPTION) && !parser.isSet(VERIFY_OPTION) && !parser.isSet(ERASE_OPTION) &&
            !parser.isSet(CUSTOM_COMMAND_OPTION) && !parser.isSet(GENERATE_MTBDFU_OPTION)) {
            std::cout << "Please provide one of --" << PROGRAM_OPTION.latin1() << ", --" << VERIFY_OPTION.latin1() << ", --"
                      << ERASE_OPTION.latin1() << ", --" << CUSTOM_COMMAND_OPTION.latin1() << " and --" << GENERATE_MTBDFU_OPTION.latin1()
                      << "." << std::endl;
            return CLI_NO_ACTION_ERROR;
        }
        return commandLineMode(parser);
    } catch (const std::exception &e) {
        qDebug() << "Unexpected failure. " << e.what();
        return QProcess::ExitStatus::CrashExit;
    }
}
