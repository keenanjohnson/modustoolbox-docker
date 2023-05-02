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

#include "cydfututils.h"

#include <boost/algorithm/string.hpp>

#include "cychannel.h"
#include "cydfuhtcore.h"

namespace cydfuht {
namespace core {

/*
InterfaceEnum stringToInterfaceEnum(const std::string &str) {
    if (boost::iequals(str, CyBridgeIfaceI2CName)) {
        return InterfaceEnum::I2C;
    } else if (boost::iequals(str, CyBridgeIfaceSPIName)) {
        return InterfaceEnum::SPI;
    } else if (boost::iequals(str, CyBridgeIfaceUARTName)) {
        return InterfaceEnum::UART;
    }
    return InterfaceEnum::NONE;
}
*/

std::string interfaceEnumToString(InterfaceEnum e) {
    switch (e) {
        case InterfaceEnum::I2C:
            return boost::to_upper_copy<std::string>(CyBridgeIfaceI2CName);
        case InterfaceEnum::SPI:
            return boost::to_upper_copy<std::string>(CyBridgeIfaceSPIName);
        case InterfaceEnum::UART:
            return boost::to_upper_copy<std::string>(CyBridgeIfaceUARTName);
        default:
            return "NONE";
    }
}

void genericPrintUpdate(double percentage) { qDebug() << QString::fromLatin1("Percentage: %1\n").arg(percentage); }

void logEvent(const char *msg_p, CyBridgeLogLevel level, void *client_p) {
    switch (level) {
        case CyBridgeLogLevel::Debug:
            qDebug() << QString::fromLatin1("Log: ") << msg_p;
            break;
        case CyBridgeLogLevel::Error:
            qCritical() << QString::fromLatin1("Log: ") << msg_p;
            break;
        case CyBridgeLogLevel::Info:
            qDebug() << QString::fromLatin1("Log: ") << msg_p;
            break;
        case CyBridgeLogLevel::Warning:
            qWarning() << QString::fromLatin1("Log: ") << msg_p;
            break;
        default:
            assert(false);
            qDebug() << QString::fromLatin1("Log: ") << msg_p;
            break;
    }
}

void logPacket(const std::string &buff_name, const uint8_t buff[], int size) {
    qDebug() << QString::fromLatin1(" %1: ").arg(QString::fromStdString(buff_name));
    for (int32_t index = 0; index < size; index++) {
        qDebug() << QString::fromLatin1("%1 ").arg(static_cast<ushort>(buff[index]), 2, 16, QLatin1Char('0'));
    }
    qDebug() << QString::fromLatin1("(%1 bytes)\n").arg(size);
}

void logErrorCode(int err) {
    if (err != 0) {
        qCritical() << QString::fromLatin1("Error code: %1\n\n").arg(QString::number(err));
    }
}

void logOperation(const std::string &operation) {
    qDebug() << QString::fromLatin1("Operation: %1\n").arg(QString::fromStdString(operation));
}

int cybridgeErrToCyChannelErr(int err) {
    switch (err) {
        case CyBridgeRecommendedFWUpgrade:
        case CyBridgeNoError:
            return CyChannel::CYCHANNEL_NO_ERROR;
        case CyBridgeOperationIsTimedOut:
        case CyBridgeOperationTimeout:
            return CyChannel::CYCHANNEL_OPERATION_TIMEOUT;
        case CyBridgeDeviceInUse:
            return CyChannel::CYCHANNEL_DEVICE_IN_USE;
        case CyBridgeAccessDenied:
            return CyChannel::CYCHANNEL_ACCESS_DENIED;
        case CyBridgeNoSpiBridge:
        case CyBridgeNoI2cBridge:
        case CyBridgeNoUartBridge:
        case CyBridgeDeviceNotFound:
            return CyChannel::CYCHANNEL_DEVICE_NOT_FOUND;

        // Internal generic cybridge error
        case CyBridgeOperationError:

        // These should be taken care of internally in the DFUHT
        // If one of these occurs then there is a bug in DFHUT.
        case CyBridgeNotInitialized:
        case CyBridgeDeviceNotOpened:
        case CyBridgeNoMem:
        case CyBridgeInvalidArgument:
        case CyBridgeNotSupported:
        case CyBridgeUnsupportedValue:
        case CyBridgeNotSupportedInterfaceNumber:
        case CyBridgeNotEnoughOutBuffer:

        // These are not applicable to DFUHT
        case CyBridgeNoPowerMonitor:
        case CyBridgeNoPowerControl:
        case CyBridgeNoSwdBridge:
        case CyBridgeFwUpdateForbidden:
        case CyBridgeNoJtagBridge:
        case CyBridgeNeedFWUpgrade:
            logErrorCode(err);
            return CyChannel::CYCHANNEL_INTERNAL_ERROR;
        default:
            return CyChannel::CYCHANNEL_UNKNOWN_ERROR;
    }
}

int qserialErrToCyChannelErr(QSerialPort::SerialPortError err) {
    switch (err) {
        case QSerialPort::NoError:
            return CyChannel::CYCHANNEL_NO_ERROR;
        case QSerialPort::DeviceNotFoundError:
            return CyChannel::CYCHANNEL_DEVICE_NOT_FOUND;
        case QSerialPort::PermissionError:
            return CyChannel::CYCHANNEL_ACCESS_DENIED;
        case QSerialPort::OpenError:
            return CyChannel::CYCHANNEL_DEVICE_IN_USE;
        case QSerialPort::WriteError:
            return CyChannel::CYCHANNEL_WRITE_OPERATION_ERROR;
        case QSerialPort::ReadError:
            return CyChannel::CYCHANNEL_READ_OPERATION_ERROR;
        case QSerialPort::TimeoutError:
            return CyChannel::CYCHANNEL_OPERATION_TIMEOUT;

        // These should be taken care of internally in the DFUHT
        // If one of these occurs then there is a bug in DFHUT.
        case QSerialPort::NotOpenError:
        case QSerialPort::ResourceError:
        case QSerialPort::UnsupportedOperationError:
            logErrorCode(err);
            return CyChannel::CYCHANNEL_INTERNAL_ERROR;
        case QSerialPort::UnknownError:
        default:
            return CyChannel::CYCHANNEL_UNKNOWN_ERROR;
    }
}

QString cyChannelErrToString(int err) {
    switch (err) {
        case CYRET_SUCCESS:
        case CYRET_ERR_BTLDR_MASK | CYBTLDR_STAT_SUCCESS:
            return QObject::tr("Operation succeeded.");
        case CYRET_ERR_FILE:
            return QObject::tr("Error accessing the input file. Verify that it exists and has read permissions enabled.");
        case CYRET_ERR_EOF:
            return QObject::tr("Reached the end of the file.");
        case CYRET_ERR_LENGTH:
        case CYRET_ERR_BTLDR_MASK | CYBTLDR_STAT_ERR_LENGTH:
            return QObject::tr("The length of data does not match expected value.");
        case CYRET_ERR_DATA:
        case CYRET_ERR_BTLDR_MASK | CYBTLDR_STAT_ERR_DATA:
            return QObject::tr("The data does not match expected value.");
        case CYRET_ERR_CMD:
        case CYRET_ERR_BTLDR_MASK | CYBTLDR_STAT_ERR_CMD:
            return QObject::tr("The command was not recognized.");
        case CYRET_ERR_DEVICE:
        case CYRET_ERR_BTLDR_MASK | CYBTLDR_STAT_ERR_DEVICE:
            return QObject::tr(
                "The attached device is not compatible with the one specified.  The Silicon ID/Silicon Revision reported by the device do "
                "not match the value expected by the *.cyacd2 file.");
        case CYRET_ERR_VERSION:
        case CYRET_ERR_BTLDR_MASK | CYBTLDR_STAT_ERR_VERSION:
            return QObject::tr("The version of the bootloader is incompatible with the host.");
        case CYRET_ERR_CHECKSUM:
        case CYRET_ERR_BTLDR_MASK | CYBTLDR_STAT_ERR_CHECKSUM:
            return QObject::tr("The row checksum does not match the computed value.");
        case CYRET_ERR_ARRAY:
        case CYRET_ERR_BTLDR_MASK | CYBTLDR_STAT_ERR_ARRAY:
            return QObject::tr("The flash array is not valid.");
        case CYRET_ERR_ROW:
        case CYRET_ERR_BTLDR_MASK | CYBTLDR_STAT_ERR_ROW:
            return QObject::tr("The flash row is not valid for the selected array.");
        case CYRET_ERR_BTLDR:
            return QObject::tr("The bootloader is not ready to process data.");
        case CYRET_ERR_ACTIVE:
        case CYRET_ERR_BTLDR_MASK | CYBTLDR_STAT_ERR_ACTIVE:
            return QObject::tr("The application is currently marked as active and cannot be modified.");
        case CYRET_ERR_COMM_MASK | core::CyChannel::CYCHANNEL_UNKNOWN_ERROR:
        case CYRET_ERR_UNK:
            return QObject::tr("Unknown error.");
        case CYRET_ABORT:
            return QObject::tr("Operation aborted.");
        case CYRET_ERR_BTLDR_MASK | CYBTLDR_STAT_ERR_PROTECT:
            return QObject::tr("The flash row is protected and can not be programmed.");
        case CYRET_ERR_BTLDR_MASK | CYBTLDR_STAT_ERR_APP:
            return QObject::tr("The application is not valid and cannot be set as active.");
        case CYRET_ERR_BTLDR_MASK | CYBTLDR_STAT_ERR_VERIFY:
            return QObject::tr("The verification of the flash row data failed.");
        case CYRET_ERR_BTLDR_MASK | CYBTLDR_STAT_ERR_KEY:
            return QObject::tr("The provided key does not match the expected value.");
        case CYRET_ERR_COMM_MASK | core::CyChannel::CYCHANNEL_OPERATION_TIMEOUT:
            return QObject::tr("Operation timed out.");
        case CYRET_ERR_COMM_MASK | core::CyChannel::CYCHANNEL_ACCESS_DENIED:
            return QObject::tr("Access was denied when trying to access the selected device.");
        case CYRET_ERR_COMM_MASK | core::CyChannel::CYCHANNEL_DEVICE_IN_USE:
            return QObject::tr("The selected device is currently being used by another process.");
        case CYRET_ERR_COMM_MASK | core::CyChannel::CYCHANNEL_INTERNAL_ERROR:
            return QObject::tr("Internal error occured. Please enable logging using --debug to get details.");
        case CYRET_ERR_COMM_MASK | core::CyChannel::CYCHANNEL_READ_OPERATION_ERROR:
            return QObject::tr("Read operation failed.");
        case CYRET_ERR_COMM_MASK | core::CyChannel::CYCHANNEL_WRITE_OPERATION_ERROR:
            return QObject::tr("Write operation failed.");
        case CYRET_ERR_COMM_MASK | core::CyChannel::CYCHANNEL_DEVICE_NOT_FOUND:
            return QObject::tr("The selected device was not found. Please make sure that the device is connected.");
        default:
            return QObject::tr("Operation returned unknown value ") + QString::number(err);
    }
}

}  // namespace core
}  // namespace cydfuht
