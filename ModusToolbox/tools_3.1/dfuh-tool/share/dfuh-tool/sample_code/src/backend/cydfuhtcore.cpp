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

#include "cydfuhtcore.h"

#include <sys/stat.h>

#include <QCoreApplication>
#include <QDir>
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <limits>
#include <sstream>

#include "cybootloaderutils/cybtldr_api.h"
#include "cybootloaderutils/cybtldr_api2.h"
#include "cychanneli2c.h"
#include "cychannelspi.h"
#include "cychanneluart.h"

namespace cydfuht {
namespace core {

static CyChannel* global_current_channel = nullptr;

static int CloseConnectionWrapper() { return global_current_channel->closeConnection(); }
static int OpenConnectionWrapper() { return global_current_channel->openConnection(); }
static int WriteDataWrapper(uint8_t* buff, int size) { return global_current_channel->writeData(buff, size); }
static int ReadDataWrapper(uint8_t* buff, int size) { return global_current_channel->readData(buff, size); }

std::unique_ptr<CyBtldr_CommunicationsData> convertChannelToBtldrComm(CyChannel* channel) {
    global_current_channel = channel;

    auto comm_data = std::make_unique<CyBtldr_CommunicationsData>();
    comm_data->CloseConnection = CloseConnectionWrapper;
    comm_data->OpenConnection = OpenConnectionWrapper;
    comm_data->WriteData = WriteDataWrapper;
    comm_data->ReadData = ReadDataWrapper;
    comm_data->MaxTransferSize = CyChannel::MAX_TRANSFER_SIZE;

    return comm_data;
}

void deleteBtldrComm() {
    delete global_current_channel;
    global_current_channel = nullptr;
}

CyDfuhtCore::CyDfuhtCore(CyBridgeConnectCallback connectcallback, void* callback_p)
    : m_eof_reached(false),
      m_programming_data(false),
      m_file_complete(false),
      m_appRowAddr(0x0),
      m_appStart(UINT_MAX),
      m_appLength(0xFFFF),
      m_startOffset(0),
      m_abort(false),
      m_currentProgressCount(0),
      m_maxProgressCounts(0) {
    m_bridge.reset(createCyBridge());
    m_bridge->setLogCallback(&logEvent, nullptr, CyBridgeLogLevel::Debug);
    if (connectcallback != nullptr) {
        m_bridge->setConnectCallback(connectcallback, callback_p);
    }

    m_bridge->initialize(Bridge_KP3_MP4 | UART);
}

QVector<CyChannelSettings> CyDfuhtCore::retrieveChannels(const char* name) const {
    QVector<CyChannelSettings> channels;

    // On the Mac, calling openDevice() on a serial device takes several seconds.
    // getDeviceList() will tell us that a device is a serial device quickly so we can avoid calling openDevice().
    CyBridgeDevice** dev_list = nullptr;
    const int count = m_bridge->getDeviceList(&dev_list);
    for (int i = 0; i < count; ++i) {
        if (strcmp(dev_list[i]->Name, name) == 0) {
            if (strcmp(dev_list[i]->ComType, "serial") == 0) {
                CyChannelSettings newsettings;
                std::ostringstream ss;
                ss << dev_list[i]->Product << " (" << name << ")"
                   << " - " << interfaceEnumToString(InterfaceEnum::UART);
                newsettings.m_displayName = QLatin1String(ss.str().c_str());
                newsettings.m_name = QLatin1String(name);
                newsettings.m_interface = InterfaceEnum::UART;
                newsettings.m_productName = QLatin1String(dev_list[i]->Product);
                channels.push_back(newsettings);
                m_bridge->releaseDeviceList(dev_list);
                return channels;
            }
            break;
        }
    }
    m_bridge->releaseDeviceList(dev_list);

    const int ret = m_bridge->openDevice(name);
    if (ret == CyBridgeErrors::CyBridgeNoError) {
        CyBridgeDeviceProps* props = nullptr;
        m_bridge->getDeviceProps(&props);

        CyChannelSettings newsettings;
        newsettings.m_name = QLatin1String(name);

        // Get supported interfaces
        std::string iflist(props->ListIfs);
        m_bridge->releaseDeviceProps(props);

        // Allocate one target for each of i2c spi and uart, if supported
        if (iflist.find(CyBridgeIfaceI2CName) != std::string::npos) {
            newsettings.m_interface = InterfaceEnum::I2C;
            newsettings.m_displayName =
                QLatin1String(name) + QLatin1String(" - ") + QString::fromStdString(interfaceEnumToString(InterfaceEnum::I2C));
            channels.push_back(newsettings);
        }

        if (iflist.find(CyBridgeIfaceSPIName) != std::string::npos) {
            newsettings.m_interface = InterfaceEnum::SPI;
            newsettings.m_displayName =
                QLatin1String(name) + QLatin1String(" - ") + QString::fromStdString(interfaceEnumToString(InterfaceEnum::SPI));
            channels.push_back(newsettings);
        }
        m_bridge->closeDevice();
    }
    return channels;
}

QVector<CyChannelSettings> CyDfuhtCore::retrieveAllChannels() const {
    CyBridgeDevice** dev_list = nullptr;
    int count = m_bridge->getDeviceList(&dev_list);
    if (0 == count) {
        return QVector<CyChannelSettings>();
    }

    QVector<CyChannelSettings> channels;

    // Generate CyChannel structures
    for (int i = 0; i < count; ++i) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        QVector<CyChannelSettings> newchannels = retrieveChannels(dev_list[i]->Name);
        channels.append(newchannels);
    }

    m_bridge->releaseDeviceList(dev_list);
    return channels;
}

int CyDfuhtCore::setCurrentChannel(const CyChannelSettings& settings) {
    std::string name = settings.m_name.toStdString();
    switch (settings.m_interface) {
        case InterfaceEnum::I2C:
            global_current_channel = new CyChannelI2c(m_bridge.get(), name, settings.m_i2c.m_freq_hz, settings.m_i2c.m_addr);
            break;
        case InterfaceEnum::SPI:
            global_current_channel =
                new CyChannelSpi(m_bridge.get(), name, settings.m_spi.m_freq_hz, settings.m_spi.m_mode, settings.m_spi.m_bit_order);
            break;
        case InterfaceEnum::UART: {
            QSerialPort::Parity parity = QSerialPort::NoParity;
            if (settings.m_uart.m_paritytype == ParityTypeEnum::ODD) {
                parity = QSerialPort::OddParity;
            } else if (settings.m_uart.m_paritytype == ParityTypeEnum::EVEN) {
                parity = QSerialPort::EvenParity;
            }
            QSerialPort::StopBits stopbits = QSerialPort::OneStop;
            if (settings.m_uart.m_stopbits == 1.5) {
                stopbits = QSerialPort::OneAndHalfStop;
            } else if (settings.m_uart.m_stopbits == 2) {
                stopbits = QSerialPort::TwoStop;
            }
            global_current_channel = new CyChannelUart(m_bridge.get(), name, static_cast<QSerialPort::BaudRate>(settings.m_uart.m_baudrate),
                                                       static_cast<QSerialPort::DataBits>(settings.m_uart.m_databits), parity, stopbits);
            break;
        }
        default:
            // Should never get here
            return CyChannel::CYCHANNEL_UNKNOWN_ERROR;
    }
    return CyChannel::CYCHANNEL_NO_ERROR;
}

int CyDfuhtCore::runAction(const CyBtldr_Action& action, const CyChannelSettings& settings, CyBtldr_ProgressUpdate* update,
                           const char* cyAcd2File) {
    int err;
    err = setCurrentChannel(settings);
    if (err != CyChannel::CYCHANNEL_NO_ERROR) {
        return err;
    }
    auto btldrComm = convertChannelToBtldrComm(global_current_channel);

    switch (action) {
        case CyBtldr_Action::PROGRAM:
            // coverity[escape]
            err = CyBtldr_Program(cyAcd2File, btldrComm.get(), update);
            break;
        case CyBtldr_Action::VERIFY:
            // coverity[escape]
            err = CyBtldr_Verify(cyAcd2File, btldrComm.get(), update);
            break;
        case CyBtldr_Action::ERASE:
            // coverity[escape]
            err = CyBtldr_Erase(cyAcd2File, btldrComm.get(), update);
            break;
        default:
            // Should never get here
            err = CyChannel::CYCHANNEL_UNKNOWN_ERROR;
    }

    if (CYRET_ERR_COMM_MASK == (CYRET_ERR_COMM_MASK & err)) {
        // BHT module does not cancel comm if it was a communication error hence if we error out due to a comm err we close
        global_current_channel->closeConnection();
    }
    deleteBtldrComm();
    return err;
}

int CyDfuhtCore::runCustomCommand(const CyChannelSettings& settings, CyBtldr_ProgressUpdate* update, QJsonObject& command,
                                  CyBtldr_CustomCommandHeaderData& metaHeaderData) {
    int err = CyChannel::CYCHANNEL_NO_ERROR;
    bool ok;

    QStringList commandKeys = command.keys();
    int repeatCount = 1;
    if (commandKeys.contains(RepeatField)) {
        QString repeatString = command[RepeatField].toString();
        err = getRepeatCount(repeatCount, commandKeys, repeatString);
    }
    if (m_maxProgressCounts == 0) {
        if (repeatCount == std::numeric_limits<int>::max()) {
            // count lines in the file
            m_maxProgressCounts = countDataFileLines(command[DataFileField].toString());
        } else {
            m_maxProgressCounts = repeatCount;
        }
    }
    if (err != CyChannel::CYCHANNEL_NO_ERROR) return err;

    for (int i = 0; i < repeatCount; i++) {
        if (m_abort) {
            err = CYRET_ABORT;
            m_abort = false;
            break;
        }
        CyBtldr_CustomCommandData cmdData;
        cmdData.data = new uint8_t[CyChannel::MAX_TRANSFER_SIZE];
        // get cmdId from JSON object
        if (commandKeys.contains(CmdIdField)) {
            cmdData.cmdId = command[CmdIdField].toString().toUInt(&ok, 16);
            if (!ok) {
                delete[] cmdData.data;
                QString errorMsg = QString::fromLatin1("cmdId must be defined as valid single-byte hex value starting with '0x'.");
                setMtbDfuConfigErrorMessage(errorMsg);
                return core::CyChannel::CYCHANNEL_MTBDFU_CONFIGURATION_ERROR;
            }
        } else {
            delete[] cmdData.data;
            QString errorMsg = QString::fromLatin1("cmdId must be defined for every command.");
            setMtbDfuConfigErrorMessage(errorMsg);
            return core::CyChannel::CYCHANNEL_MTBDFU_CONFIGURATION_ERROR;
        }

        // get data
        err = getCommandDataBuffer(command, cmdData, update);
        if (err != CyChannel::CYCHANNEL_NO_ERROR) {
            return err;
        }
        // set offset for file
        if (commandKeys.contains(StartOffsetField)) {
            m_startOffset = command[StartOffsetField].toString().toUInt(&ok, 16);
        }

        // get outCli parameter from JSON object
        cmdData.outCli = 0;
        if (commandKeys.contains(OutCliField)) {
            if (command[OutCliField].toString() == QLatin1String("True")) {
                cmdData.outCli = 1;
            }
        }

        // get outFile parameter from JSON object
        if (commandKeys.contains(OutFileField)) {
            cmdData.outFile = fopen(command[OutFileField].toString().toStdString().data(), "a");
        } else {
            cmdData.outFile = NULL;
        }

        // send command
        if (!(cmdData.cmdId == CMD_ERASE_DATA && m_eof_reached)) err = CyBtldr_CustomCommand(&cmdData);

        delete[] cmdData.data;
        // closing files
        if (cmdData.outFile != NULL) {
            fclose(cmdData.outFile);
        }
        if (err != CyChannel::CYCHANNEL_NO_ERROR) {
            return err;
        }
        if (m_eof_reached && m_file_complete) {
            return err;
        }
    }
    return err;
}

int CyDfuhtCore::runCustomSession(const CyChannelSettings& settings, CyBtldr_ProgressUpdate* update, QJsonArray& session,
                                  CyBtldr_CustomCommandHeaderData& metaHeaderData) {
    int err;
    bool ok;
    double sendingCommandFinished = 100.0;
    err = setCurrentChannel(settings);
    if (err != CyChannel::CYCHANNEL_NO_ERROR) {
        return err;
    }
    auto btldrComm = convertChannelToBtldrComm(global_current_channel);

    uint32_t blVer = 0;
    uint32_t siliconId = 0;
    uint8_t siliconRev = 0;

    // read basic parameters
    uint32_t productId = metaHeaderData.productId;
    uint8_t chksumtype = metaHeaderData.checksumType;
    // uint8_t appId = metaHeaderData.applicationId;
    CyBtldr_SetCheckSumType(static_cast<CyBtldr_ChecksumType>(chksumtype));

    // send ENTER DFU command to start communication
    err = CyBtldr_StartBootloadOperation(btldrComm.get(), siliconId, siliconRev, &blVer, productId);

    // sending commands

    // parse commands from session JSON array
    if (err == CYRET_SUCCESS)
        for (auto commandElement : session) {
            m_currentProgressCount = 0;
            // commandObject can be either a single command or a set commands
            QJsonObject commandObject = commandElement.toObject();
            QStringList commandKeys = commandObject.keys();
            if (commandKeys.contains(CommandSetField)) {
                // object in "commands" section includes set of multiple commands
                int repeatCount = 1;
                if (commandKeys.contains(RepeatField)) {
                    QString repeatString = commandObject[RepeatField].toString();
                    err = getRepeatCount(repeatCount, commandKeys, repeatString);
                }
                if (repeatCount == std::numeric_limits<int>::max()) {
                    // count lines in the file
                    m_maxProgressCounts = countDataFileLines(commandObject[DataFileField].toString());

                } else {
                    m_maxProgressCounts = repeatCount;
                }
                QJsonArray commandSetArray = commandObject[CommandSetField].toArray();
                QString outFileName;
                if (!m_programming_data) {
                    if (commandKeys.contains(OutFileField)) {
                        outFileName = commandObject[OutFileField].toString();
                    }
                    for (auto commandValue : commandSetArray) {
                        if (commandValue.toObject().keys().contains(OutFileField)) {
                            outFileName = commandValue.toObject()[OutFileField].toString();
                        }

                        int cmdId = commandValue.toObject()[CmdIdField].toString().toInt(&ok, HEX_BASE);
                        if (cmdId == CMD_PROGRAM_DATA || cmdId == CMD_VERIFY_DATA) {
                            m_programming_data = true;
                        }
                    }
                }
                if (!outFileName.isEmpty()) {
                    writeLogTimeToFile(outFileName);
                }
                std::cout << std::endl;  // splitting progress bars for different commands/command sets
                for (int i = 0; i < repeatCount; i++) {
                    for (auto commandValue : commandSetArray) {
                        QJsonObject command = commandValue.toObject();

                        err = copyCommandSetOptionsToCommand(command, commandObject);
                        // break cycle if file was passed and EoF reached
                        if (err == CyChannel::CYCHANNEL_NO_ERROR) err = runCustomCommand(settings, update, command, metaHeaderData);
                        if (err != CyChannel::CYCHANNEL_NO_ERROR) {
                            deleteBtldrComm();
                            resetCoreState();
                            return err;
                        }
                    }
                    if (m_eof_reached && m_file_complete) {
                        m_eof_reached = false;
                        m_file_complete = false;
                        break;
                    }
                }
                update(sendingCommandFinished);
                resetCoreState();
                m_currentProgressCount = 0;
                m_maxProgressCounts = 0;
                m_programming_data = false;
            } else {
                if (commandObject.keys().contains(OutFileField)) {
                    QString outFileName = commandObject[OutFileField].toString();
                    writeLogTimeToFile(outFileName);
                }
                std::cout << std::endl;  // splitting progress bars for different commands/command sets
                // object in "commands" section includes single command
                err = runCustomCommand(settings, update, commandObject, metaHeaderData);
                if (err != CyChannel::CYCHANNEL_NO_ERROR) {
                    deleteBtldrComm();
                    resetCoreState();
                    return err;
                }
                if (m_eof_reached && m_file_complete) {
                    m_eof_reached = false;
                    m_file_complete = false;
                }
                update(sendingCommandFinished);
                resetCoreState();
                m_currentProgressCount = 0;
                m_maxProgressCounts = 0;
            }
        }

    // send EXIT DFU command to finish session
    CyBtldr_EndBootloadOperation();

    if (CYRET_ERR_COMM_MASK == (CYRET_ERR_COMM_MASK & err)) {
        // BHT module does not cancel comm if it was a communication error hence if we error out due to a comm err we close
        global_current_channel->closeConnection();
    }
    deleteBtldrComm();
    return err;
}

int CyDfuhtCore::program(const char* filename, const CyChannelSettings& channelsettings, CyBtldr_ProgressUpdate update) {
    return runAction(CyBtldr_Action::PROGRAM, channelsettings, update, filename);
}

int CyDfuhtCore::verify(const char* filename, const CyChannelSettings& channelsettings, CyBtldr_ProgressUpdate update) {
    return runAction(CyBtldr_Action::VERIFY, channelsettings, update, filename);
}

int CyDfuhtCore::erase(const char* filename, const CyChannelSettings& channelsettings, CyBtldr_ProgressUpdate update) {
    return runAction(CyBtldr_Action::ERASE, channelsettings, update, filename);
}

int CyDfuhtCore::sendCommand(const QString& filename, const CyChannelSettings& channelsettings, CyBtldr_ProgressUpdate update) {
    // parsing sessions with commands and header from .mtbdfu file
    int err = core::CyChannel::CYCHANNEL_NO_ERROR;
    CyBtldr_CustomCommandHeaderData metaHeaderData;
    std::vector<QJsonArray> sessions;
    err = parseSessionsFromJson(filename, sessions, metaHeaderData);
    if (err == core::CyChannel::CYCHANNEL_NO_ERROR) {
        for (QJsonArray session : sessions) {
            err = runCustomSession(channelsettings, update, session, metaHeaderData);
        }
    }
    return err;
}

int CyDfuhtCore::abort() {
    m_abort = true;
    if (global_current_channel != nullptr) global_current_channel->abort = true;
    return CyBtldr_Abort();
}

}  // namespace core
}  // namespace cydfuht
