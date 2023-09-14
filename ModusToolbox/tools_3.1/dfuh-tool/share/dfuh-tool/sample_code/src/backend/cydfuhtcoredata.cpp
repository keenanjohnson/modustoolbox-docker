/*
 * Copyright 2023 Cypress Semiconductor Corporation (an Infineon company)
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

namespace cydfuht {
namespace core {

int CyDfuhtCore::fillRowWithZeros(QJsonObject& command) {
    int err = CyChannel::CYCHANNEL_NO_ERROR;

    int currentRowSize = m_appRowArray.size();
    int flashRowLength = command[FlashRowLengthField].toString().toInt(nullptr, HEX_BASE);
    if (currentRowSize > 0 && currentRowSize < flashRowLength) {
        m_appRowArray.append(flashRowLength - currentRowSize, 0x00);
        m_dataQueue.append(flashRowLength - currentRowSize, 0x00);
    }

    return err;
}

void getDataBytesFromArray(QByteArray& dataBytesArray, QJsonValueRef& dataBytesObject, bool& ok) {
    QJsonArray dataBytesJsonArray = dataBytesObject.toArray();
    for (int i = 0; i < dataBytesJsonArray.size(); i++) {
        dataBytesArray[i] = dataBytesJsonArray[i].toString().toUInt(&ok, 16);
    }
}

void getDataBytesFromString(QByteArray& dataBytesArray, QJsonValueRef& dataBytesObject) {
    QString dataBytesString = dataBytesObject.toString();
    if (dataBytesString.mid(0, 2) == QLatin1String("0x") || dataBytesString.mid(0, 2) == QLatin1String("0X"))
        dataBytesString = dataBytesString.mid(2);
    dataBytesArray = QByteArray::fromHex(dataBytesString.toStdString().data());
}

void CyDfuhtCore::getMetadataFromBytes(QByteArray& dataBytesArray, bool& ok) {
    QByteArray appStartArray = dataBytesArray.mid(1, 4);
    std::reverse(appStartArray.begin(), appStartArray.end());
    m_appStart = appStartArray.toHex().toInt(&ok, HEX_BASE);
    QByteArray appLengthArray = dataBytesArray.mid(5, 4);
    std::reverse(appLengthArray.begin(), appLengthArray.end());
    m_appLength = appLengthArray.toHex().toInt(&ok, HEX_BASE);
}

int CyDfuhtCore::getDataFromString(QJsonObject& command, CyBtldr_CustomCommandData& cmdData) {
    bool ok;
    QByteArray dataBytesArray;
    QStringList commandKeys = command.keys();
    QJsonValueRef dataBytesObject = command[DataBytesField];
    if (!commandKeys.contains(DataLengthField)) {
        QString errorMsg = QString::fromLatin1("\"dataLength\" should be defined when additional data is passed.");
        setMtbDfuConfigErrorMessage(errorMsg);
        return core::CyChannel::CYCHANNEL_MTBDFU_CONFIGURATION_ERROR;
    }

    if (dataBytesObject.isArray()) {
        getDataBytesFromArray(dataBytesArray, dataBytesObject, ok);
    } else if (dataBytesObject.isString()) {
        getDataBytesFromString(dataBytesArray, dataBytesObject);
    } else {
        QString errorMsg = QString::fromLatin1("\"dataBytes\" should be either array or string.");
        setMtbDfuConfigErrorMessage(errorMsg);
        return core::CyChannel::CYCHANNEL_MTBDFU_CONFIGURATION_ERROR;
    }

    cmdData.dataLength = command[DataLengthField].toString().toUInt(&ok, 16);
    if (command[DataLengthField].toString().toInt(&ok, HEX_BASE) >= dataBytesArray.size()) {
        // fill array with leading zeros, if dataLength is bigger than actual dataLength
        int leadingZerosCount = cmdData.dataLength - dataBytesArray.size();
        for (int i = 0; i < leadingZerosCount; i++) {
            dataBytesArray.insert(0, (char)0);
        }
    } else if (command[DataLengthField].toString().toInt(&ok, HEX_BASE) < dataBytesArray.size()) {
        cydfuht::core::cyWarning().noquote() << QString::fromLatin1("Warning: \"dataLength\" is smaller than actual data length.");
    }
    if (cmdData.cmdId == CMD_SET_METADATA) {
        getMetadataFromBytes(dataBytesArray, ok);
    }
    cmdData.data = new uint8_t[cmdData.dataLength];
    // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
    for (int i = 0; i < cmdData.dataLength; i++) {
        *(cmdData.data + i) = dataBytesArray[i];
    }
    return CyChannel::CYCHANNEL_NO_ERROR;
}

void CyDfuhtCore::parseHexLineRecord(QJsonObject& command, QString& line, int& err) {
    int DATA_RECORD_TYPE = 0x00;
    int EOF_RECORD_TYPE = 0x01;
    int ADDRESS_RECORD_TYPE = 0x04;
    bool ok;
    bool erasing_data = false;
    quint32 flashRowLength = command[FlashRowLengthField].toString().toInt(&ok, HEX_BASE);
    if (command[CmdIdField].toString().toUInt(&ok, 16) == CMD_ERASE_DATA) erasing_data = true;
    int dataType = line.mid(7, 2).toInt(&ok, HEX_BASE);
    if (dataType == EOF_RECORD_TYPE || m_dataFileStream.atEnd() ||
        (dataType == ADDRESS_RECORD_TYPE && (m_programming_data || erasing_data) &&
         (line.mid(9, 4).toUInt(&ok, 16) << 16) > (m_appStart | m_appLength))) {  // End of File
        if (m_programming_data) err = fillRowWithZeros(command);

        m_eof_reached = true;
        m_dataFileStream.setDevice(nullptr);
        m_dataFile.close();
        return;
    } else if (dataType == ADDRESS_RECORD_TYPE) {
        if (m_programming_data) err = fillRowWithZeros(command);
        int lineDataSize = line.mid(1, 2).toInt(&ok, HEX_BASE) * 2;
        m_appRowAddr = line.mid(9, lineDataSize).toInt(&ok, HEX_BASE) << 16;
        if (m_appStart != UINT_MAX && m_appRowAddr < m_appStart) {
            m_appRowAddr = UINT_MAX;
        }
    } else if (dataType == DATA_RECORD_TYPE && m_appRowAddr != UINT_MAX) {
        int lineDataSize = line.mid(1, 2).toInt(&ok, HEX_BASE) * 2;
        if (m_startOffset > 0) {
            if (lineDataSize >= (int)m_startOffset) {
                lineDataSize -= m_startOffset;
                m_startOffset = 0;
            } else {
                m_startOffset -= lineDataSize;
                return;
            }
        }
        // get data from line string and keep it in intermediate buffer
        QByteArray lineData = QByteArray::fromHex(line.mid(9, lineDataSize).toStdString().data());

        if (m_programming_data || erasing_data) {
            quint16 lineAddress = line.mid(3, 4).toInt(&ok, HEX_BASE);
            if ((m_addressQueue.isEmpty() && !erasing_data) ||
                (quint32)((m_appRowAddr | lineAddress) - m_addressQueue.last()) >= flashRowLength) {
                m_addressQueue.append(m_appRowAddr | (lineAddress - (lineAddress % flashRowLength)));
            } else if (lineAddress % flashRowLength > (quint32)m_appRowArray.size()) {
                int zerosSize = (lineAddress % flashRowLength) - m_appRowArray.size();
                m_appRowArray.append(zerosSize, 0x00);
                m_dataQueue.append(zerosSize, 0x00);
            }
            m_appRowArray.append(lineData);
            if (m_programming_data) m_dataQueue.append(lineData);
        }
    }
}

void CyDfuhtCore::resetCoreState() {
    m_dataFile.close();
    m_dataFileStream.setDevice(nullptr);
    m_dataFileStream.flush();
    m_addressQueue.clear();
    m_chksumQueue.clear();
    m_dataQueue.remove(0, m_dataQueue.size());
    m_appRowArray.remove(0, m_appRowArray.size());
    m_appRowAddr = 0x0;
    m_eof_reached = false;
    m_programming_data = false;
    m_file_complete = false;
    m_startOffset = 0;
    m_abort = false;
    m_currentProgressCount = 0;
    m_maxProgressCounts = 0;
}

int CyDfuhtCore::openDataFile(QJsonObject& command) {
    if (m_dataFileStream.device() == nullptr && !m_eof_reached) {
        m_dataFile.setFileName(command[DataFileField].toString());
        if (!m_dataFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QString errorMsg = QString::fromLatin1("input .hex file does not exist or cannot be read.");
            setMtbDfuConfigErrorMessage(errorMsg);
            return core::CyChannel::CYCHANNEL_MTBDFU_CONFIGURATION_ERROR;
        } else {
            m_dataFileStream.setDevice(&m_dataFile);
        }
    }
    return CyChannel::CYCHANNEL_NO_ERROR;
}

void CyDfuhtCore::fillProgramDataCommand(CyBtldr_CustomCommandData& cmdData) {
    // add address and row checksum to the packet
    quint32 rowNumber = m_addressQueue.takeFirst();
    quint32 chksum = m_chksumQueue.takeFirst();
    fillData32(cmdData.data, rowNumber);
    fillData32(cmdData.data + 4, chksum);
    // add rest of the data to the packet
    // address and checksum take 4 bytes each
    for (int i = 8; i < (cmdData.dataLength); i++) {
        *(cmdData.data + i) = m_dataQueue[i - 8];
    }
    m_dataQueue.remove(0, cmdData.dataLength - 0x8);
}

int CyDfuhtCore::getDataFromFile(QJsonObject& command, CyBtldr_CustomCommandData& cmdData, CyBtldr_ProgressUpdate* update) {
    bool ok;
    uint32_t PACKET_WRAPPER_SIZE = 7;  // first 4 bytes + last 3 bytes
    int err = CyChannel::CYCHANNEL_NO_ERROR;
    QStringList commandKeys = command.keys();
    bool erasing_data = false;
    if (command[CmdIdField].toString().toUInt(&ok, 16) == CMD_ERASE_DATA) erasing_data = true;

    // calculating length of the data in packet
    cmdData.dataLength = CyChannel::MAX_TRANSFER_SIZE - PACKET_WRAPPER_SIZE;
    if (commandKeys.contains(DataLengthField)) {
        uint8_t jsonDataLength = command[DataLengthField].toString().toUInt(&ok, 16);
        if (jsonDataLength < cmdData.dataLength) {
            cmdData.dataLength = jsonDataLength;
        }
    }

    err = openDataFile(command);
    if (err != CyChannel::CYCHANNEL_NO_ERROR) return err;

    QString line;
    quint32 flashRowLength = command[FlashRowLengthField].toString().toInt(&ok, HEX_BASE);
    // reading new line if necessary
    while (!m_eof_reached &&  // the new line will not be read if m_dataQueue has enough data for the next operation
           ((uint32_t)m_dataQueue.size() < cmdData.dataLength || erasing_data) && m_appRowArray.size() < (int)flashRowLength) {
        line = m_dataFileStream.readLine();
        m_currentProgressCount++;
        update(m_currentProgressCount * 100.0 / m_maxProgressCounts);
        parseHexLineRecord(command, line, err);
    }
    if (m_appRowArray.size() >= (int)flashRowLength && (m_programming_data || erasing_data)) {
        QByteArray tempArray = m_appRowArray.mid(flashRowLength);
        if (!erasing_data)
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            m_chksumQueue.append(CyBtldr_ComputeChecksum32bit((uint8_t*)m_appRowArray.mid(0, flashRowLength).data(), flashRowLength));
        m_appRowArray = tempArray;
    }

    if (cmdData.cmdId == CMD_PROGRAM_DATA || cmdData.cmdId == CMD_VERIFY_DATA) {
        fillProgramDataCommand(cmdData);
    } else if (cmdData.cmdId == CMD_ERASE_DATA) {
        if (!m_addressQueue.isEmpty()) {
            quint32 rowNumber = m_addressQueue.takeFirst();
            fillData32(cmdData.data, rowNumber);
        }
    } else {
        for (int i = 0; i < cmdData.dataLength; i++) {
            *(cmdData.data + i) = m_dataQueue[i];
        }
        m_dataQueue.remove(0, cmdData.dataLength);
    }
    if (m_eof_reached && m_dataQueue.size() == 0) m_file_complete = true;

    return err;
}

int CyDfuhtCore::getCommandDataBuffer(QJsonObject& command, CyBtldr_CustomCommandData& cmdData, CyBtldr_ProgressUpdate* update) {
    int err = CyChannel::CYCHANNEL_NO_ERROR;
    QStringList commandKeys = command.keys();

    if (commandKeys.contains(DataBytesField)) {
        // "dataBytes" used for passing data
        err = getDataFromString(command, cmdData);

    } else if (commandKeys.contains(DataFileField)) {
        // "dataFile" used for passing data
        err = getDataFromFile(command, cmdData, update);

    } else if (commandKeys.contains(DataLengthField) && command[DataLengthField].toInt() != 0) {
        cydfuht::core::cyCritical().noquote()
            << "Error: payload data is not defined. Please use \"dataBytes\" or \"dataFile\" JSON field to set data.";
        err = core::CyChannel::CYCHANNEL_MTBDFU_CONFIGURATION_ERROR;
    } else {
        cmdData.dataLength = 0;
    }

    return err;
}

}  // namespace core
}  // namespace cydfuht