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

#include "cydfuhtcore.h"

#include <QCoreApplication>
#include <QDir>
#include <boost/algorithm/string.hpp>
#include <sstream>

#include "cybootloaderutils/cybtldr_api.h"
#include "cybootloaderutils/cybtldr_api2.h"
#include "cychannel.h"
#include "cychanneli2c.h"
#include "cychannelspi.h"
#include "cychanneluart.h"
#include "cydfututils.h"

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

CyDfuhtCore::CyDfuhtCore(CyBridgeConnectCallback connectcallback, void* callback_p) {
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
                channels.push_back(newsettings);
                m_bridge->releaseDeviceList(dev_list);
                return channels;
            }
            break;
        }
    }
    m_bridge->releaseDeviceList(dev_list);

    const int ret = m_bridge->openDevice(name);
    if (ret == CyBridgeErrors::CyBridgeNoError || ret == CyBridgeErrors::CyBridgeRecommendedFWUpgrade) {
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

int CyDfuhtCore::runAction(const CyBtldr_Action& action, const char* filename, const CyChannelSettings& settings,
                           CyBtldr_ProgressUpdate* update) {
    int err;
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
    auto btldrComm = convertChannelToBtldrComm(global_current_channel);

    switch (action) {
        case CyBtldr_Action::PROGRAM:
            // coverity[escape]
            err = CyBtldr_Program(filename, btldrComm.get(), update);
            break;
        case CyBtldr_Action::VERIFY:
            // coverity[escape]
            err = CyBtldr_Verify(filename, btldrComm.get(), update);
            break;
        case CyBtldr_Action::ERASE:
            // coverity[escape]
            err = CyBtldr_Erase(filename, btldrComm.get(), update);
            break;
        default:
            // Should never get here
            err = CyChannel::CYCHANNEL_UNKNOWN_ERROR;
    }

    if (CYRET_ERR_COMM_MASK == (CYRET_ERR_COMM_MASK & err)) {
        // BHT module does not cancel comm if it was a communication error hence if we error out due to a comm err we close
        global_current_channel->closeConnection();
    }
    delete global_current_channel;
    global_current_channel = nullptr;
    return err;
}

int CyDfuhtCore::program(const char* filename, const CyChannelSettings& channelsettings, CyBtldr_ProgressUpdate update) {
    return runAction(CyBtldr_Action::PROGRAM, filename, channelsettings, update);
}

int CyDfuhtCore::verify(const char* filename, const CyChannelSettings& channelsettings, CyBtldr_ProgressUpdate update) {
    return runAction(CyBtldr_Action::VERIFY, filename, channelsettings, update);
}

int CyDfuhtCore::erase(const char* filename, const CyChannelSettings& channelsettings, CyBtldr_ProgressUpdate update) {
    return runAction(CyBtldr_Action::ERASE, filename, channelsettings, update);
}

int CyDfuhtCore::abort() { return CyBtldr_Abort(); }

}  // namespace core
}  // namespace cydfuht
