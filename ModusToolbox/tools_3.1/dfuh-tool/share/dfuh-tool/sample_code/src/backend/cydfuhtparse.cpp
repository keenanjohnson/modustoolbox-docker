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

#include "cydfuhtparse.h"

#include "cychannel.h"
#include "cydfuhtcore.h"

namespace cydfuht {
namespace core {

int parseJsonHeader(const QJsonObject &jsonObject, CyBtldr_CustomCommandHeaderData &metaHeaderData) {
    if (jsonObject.find(MetaSection) == jsonObject.end()) {
        QString errorMsg =
            QString::fromLatin1(".mtbdfu file should contain \"%1\" section with all basic parameters defined.").arg(MetaSection);
        setMtbDfuConfigErrorMessage(errorMsg);
        return core::CyChannel::CYCHANNEL_MTBDFU_CONFIGURATION_ERROR;
    }
    QJsonObject metaHeaderObject = jsonObject[MetaSection].toObject();
    QStringList metaHeaderKeys = metaHeaderObject.keys();
    QRegularExpression validHex(QLatin1String("^0x[0-9A-F]+$"), QRegularExpression::CaseInsensitiveOption);

    // File Version
    if (metaHeaderKeys.contains(FileVersion)) {
        QString fileVersionString = metaHeaderObject[FileVersion].toString();
        if (validHex.match(fileVersionString).hasMatch()) {
            metaHeaderData.fileVersion = (unsigned int)strtol(fileVersionString.toStdString().c_str(), nullptr, 0);
        } else {
            QString errorMsg = QString::fromLatin1("File version must be defined as valid hex value starting with '0x'.");
            setMtbDfuConfigErrorMessage(errorMsg);
            return core::CyChannel::CYCHANNEL_MTBDFU_CONFIGURATION_ERROR;
        }
    } else {
        QString errorMsg = QString::fromLatin1("Section \"APPInfo\" must include \"File Version\" field.");
        setMtbDfuConfigErrorMessage(errorMsg);
        return core::CyChannel::CYCHANNEL_MTBDFU_CONFIGURATION_ERROR;
    }

    // Product Id
    if (metaHeaderKeys.contains(ProductId)) {
        QString productIdString = QLatin1String("0x") + metaHeaderObject[ProductId].toString();
        if (validHex.match(productIdString).hasMatch()) {
            metaHeaderData.productId = (unsigned int)strtol(productIdString.toStdString().c_str(), nullptr, 0);
        } else {
            QString errorMsg = QString::fromLatin1("Product Id must be defined as valid hex value without preceeding '0x'.");
            setMtbDfuConfigErrorMessage(errorMsg);
            return core::CyChannel::CYCHANNEL_MTBDFU_CONFIGURATION_ERROR;
        }

    } else {
        QString errorMsg = QString::fromLatin1("Section \"APPInfo\" must include \"Product Id\" field.");
        setMtbDfuConfigErrorMessage(errorMsg);
        return core::CyChannel::CYCHANNEL_MTBDFU_CONFIGURATION_ERROR;
    }

    // Checksum Type
    if (metaHeaderKeys.contains(ChecksumType)) {
        QString checksumTypeString = metaHeaderObject[ChecksumType].toString();
        if (validHex.match(checksumTypeString).hasMatch()) {
            metaHeaderData.checksumType = (unsigned int)strtol(checksumTypeString.toStdString().c_str(), nullptr, 0);
        } else {
            QString errorMsg = QString::fromLatin1("Checksum Type must be defined as valid hex value starting with '0x'.");
            setMtbDfuConfigErrorMessage(errorMsg);
            return core::CyChannel::CYCHANNEL_MTBDFU_CONFIGURATION_ERROR;
        }
    } else {
        QString errorMsg = QString::fromLatin1("Section \"APPInfo\" must include \"Packet Checksum Type\" field.");
        setMtbDfuConfigErrorMessage(errorMsg);
        return core::CyChannel::CYCHANNEL_MTBDFU_CONFIGURATION_ERROR;
    }
    return core::CyChannel::CYCHANNEL_NO_ERROR;
}

int getJsonObjFromFile(const QString &filename, QJsonObject &jsonObject) {
    QFile jsonFile(filename);
    // Open the file
    if (!jsonFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        cydfuht::core::cyCritical() << "Error: .mtbdfu file does not exist or cannot be read.";
        return core::CyChannel::CYCHANNEL_MTBDFU_NOT_FOUND;
    } else {
        // Get all json data out of the file
        QJsonParseError error{};
        QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonFile.readAll(), &error);
        if (error.error != QJsonParseError::NoError) {
            QString errorMsg = error.errorString() + QString::fromLatin1(" at ") + QString::number(error.offset) + QString::fromLatin1(".");
            setMtbDfuConfigErrorMessage(errorMsg);
            return core::CyChannel::CYCHANNEL_MTBDFU_CONFIGURATION_ERROR;
        } else {
            jsonObject = jsonDoc.object();
            return core::CyChannel::CYCHANNEL_NO_ERROR;
        }
    }
}

int parseJsonHeaderFromFile(const QString &filename, CyBtldr_CustomCommandHeaderData &metaHeaderData) {
    int err;
    QJsonObject jsonObject;
    err = getJsonObjFromFile(filename, jsonObject);
    if (err != core::CyChannel::CYCHANNEL_NO_ERROR)
        return err;
    else
        return parseJsonHeader(jsonObject, metaHeaderData);
}

int parseSessionsFromJson(const QString &filename, std::vector<QJsonArray> &sessions, CyBtldr_CustomCommandHeaderData &metaHeaderData) {
    int err;
    // Read JSON object from .mtbdfu input file
    QJsonObject jsonObject;
    err = getJsonObjFromFile(filename, jsonObject);
    if (err != core::CyChannel::CYCHANNEL_NO_ERROR) return err;
    // Read general parameters from header
    err = parseJsonHeader(jsonObject, metaHeaderData);
    if (err != core::CyChannel::CYCHANNEL_NO_ERROR) {
        return err;
    }
    // Read sessions data
    QStringList jsonKeys = jsonObject.keys();
    if (jsonKeys.contains(CommandsSection)) {
        sessions.reserve(1);
        QJsonArray commandsArray = jsonObject[CommandsSection].toArray();
        sessions.push_back(commandsArray);
    } else if (jsonKeys.contains(SessionsSection)) {
        QJsonArray sessionsArray = jsonObject[SessionsSection].toArray();
        int sessionsCount = sessionsArray.size();
        sessions.reserve(sessionsCount);
        for (auto sessionElement : sessionsArray) {
            QJsonObject sessionObject = sessionElement.toObject();
            if (sessionObject.find(CommandsSection) == sessionObject.end()) {
                QString errorMsg = QString::fromLatin1("Section \"sessions\" must include objects with \"commands\" field.");
                setMtbDfuConfigErrorMessage(errorMsg);
                return core::CyChannel::CYCHANNEL_MTBDFU_CONFIGURATION_ERROR;
            }
            QJsonArray commandsArray = sessionObject[CommandsSection].toArray();
            sessions.push_back(commandsArray);
        }
    }
    return core::CyChannel::CYCHANNEL_NO_ERROR;
}

bool generateMtbDfuJson(const QString filename, QString action, QString &generatedFilename,
                        CyBtldr_CustomCommandHeaderData &metaHeaderData) {
    if (action != NoAction)
        generatedFilename = action + QLatin1String("Hex.mtbdfu");
    else
        action = ProgramAction;
    QFile generatedFile(generatedFilename);
    if (generatedFile.open(QIODevice::WriteOnly)) {
        QJsonObject object;
        // insert app info section
        QJsonObject appInfoObject;
        // object.insert();
        appInfoObject.insert(FileVersion, QStringLiteral("0x%1").arg(metaHeaderData.fileVersion, 0, 16));
        appInfoObject.insert(ProductId, QStringLiteral("%1").arg(metaHeaderData.productId, 8, 16, QLatin1Char('0')));
        appInfoObject.insert(ChecksumType, QStringLiteral("0x%1").arg(metaHeaderData.checksumType, 0, 16));
        object.insert(MetaSection, appInfoObject);

        QJsonArray commandsArray;

        // insert setMetadata command if it was passed from CLI
        if (metaHeaderData.applicationLength != 0x0 || metaHeaderData.applicationId != 0x0) {
            QString defaultMetadataLength = QLatin1String("0x09");
            QJsonObject setMetadataObject;
            setMetadataObject.insert(CmdIdField, CmdSetMetadata);
            setMetadataObject.insert(DataLengthField, defaultMetadataLength);

            QByteArray appStartArray =
                QByteArray::fromHex(QStringLiteral("%1").arg(metaHeaderData.applicationStart, 0, 16).toStdString().data());
            QByteArray reversedAppStartArray(appStartArray.size(), 0);
            std::copy(appStartArray.crbegin(), appStartArray.crend(), reversedAppStartArray.begin());

            const QByteArray appLengthArray =
                QByteArray::fromHex(QStringLiteral("%1").arg(metaHeaderData.applicationLength, 0, 16).toStdString().data());
            QByteArray reversedAppLengthArray(appLengthArray.size(), 0);
            std::copy(appLengthArray.crbegin(), appLengthArray.crend(), reversedAppLengthArray.begin());
            QString reversedAppLengthString = QString::fromStdString(reversedAppLengthArray.toHex().toStdString());
            for (int i = 0; i < 8 - appLengthArray.size() * 2; i++) reversedAppLengthString.append(QLatin1Char('0'));

            setMetadataObject.insert(DataBytesField,
                                     QStringLiteral("0x%1%2%3")
                                         .arg(metaHeaderData.applicationId, 2, 16, QLatin1Char('0'))
                                         .arg(reversedAppStartArray.toHex().toInt(nullptr, HEX_BASE), 8, 16, QLatin1Char('0'))
                                         .arg(reversedAppLengthString));

            commandsArray.append(setMetadataObject);
        }

        // insert commands section
        QString defaultFlashRowLength = QLatin1String("0x200");
        QJsonObject commandObject;
        commandObject.insert(DataFileField, filename);
        commandObject.insert(RepeatField, QLatin1String("EoF"));
        commandObject.insert(FlashRowLengthField, defaultFlashRowLength);

        if (action == ProgramAction || action == VerifyAction) {
            QString defaultSendDataPacketLength = QLatin1String("0x10");
            QString defaultSendDataPacketRepeat = QLatin1String("0x20");
            QJsonArray innerCommandsArray;
            QJsonObject sendDataObject;
            sendDataObject.insert(CmdIdField, CmdSendData);
            sendDataObject.insert(DataLengthField, defaultSendDataPacketLength);
            sendDataObject.insert(RepeatField, defaultSendDataPacketRepeat);
            innerCommandsArray.append(sendDataObject);
            QJsonObject actionCommandObject;
            if (action == ProgramAction)
                actionCommandObject.insert(CmdIdField, CmdProgramData);
            else
                actionCommandObject.insert(CmdIdField, CmdVerifyData);
            QString defaultAddressChecksumLength = QLatin1String("0x08");
            actionCommandObject.insert(DataLengthField, defaultAddressChecksumLength);
            innerCommandsArray.append(actionCommandObject);
            commandObject.insert(CommandSetField, innerCommandsArray);
        } else if (action == EraseAction) {
            QString defaultAddressLength = QLatin1String("0x04");
            commandObject.insert(CmdIdField, CmdEraseData);
            commandObject.insert(DataLengthField, defaultAddressLength);
        }

        commandsArray.append(commandObject);
        object.insert(CommandsSection, commandsArray);

        QJsonDocument jsonDoc;
        jsonDoc.setObject(object);
        generatedFile.write(jsonDoc.toJson());
        return true;
    }
    return false;
}

}  // namespace core
}  // namespace cydfuht
