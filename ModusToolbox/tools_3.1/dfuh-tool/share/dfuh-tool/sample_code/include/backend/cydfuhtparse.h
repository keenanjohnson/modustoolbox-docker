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

#pragma once

#include <QChar>
#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>

#include "cybridge.h"
#include "cybtldr_api2.h"
#include "cychannelsettings.h"

namespace cydfuht {
namespace core {

/// Parses JSON object and gets data about custom command header
/// \param jsonObject JSON object read out from the file with custom command data
/// \param metaHeaderData The metadata for current DFU session
/// \return CYCHANNEL_NO_ERROR(0) on success, CYCHANNEL_MTBDFU_CONFIGURATION_ERROR(E) on incorrect header
int parseJsonHeader(const QJsonObject &jsonObject, CyBtldr_CustomCommandHeaderData &metaHeaderData);

/// Parses JSON file and gets data about custom command header
/// \param filename JSON file with custom command data
/// \param metaHeaderData The metadata for current DFU session
/// \return CYCHANNEL_NO_ERROR(0) on success, CYCHANNEL_MTBDFU_CONFIGURATION_ERROR(E), CYCHANNEL_MTBDFU_NOT_FOUND(D) on failure
int parseJsonHeaderFromFile(const QString &filename, CyBtldr_CustomCommandHeaderData &metaHeaderData);

/// Extracts QJsonObject from passed JSON-compatible file
/// \param filename JSON file with custom command data
/// \param jsonObject QJsonObject that contains all json objects from the file
/// \return CYCHANNEL_NO_ERROR(0) on success, CYCHANNEL_MTBDFU_CONFIGURATION_ERROR(E) on incorrect JSON format,
/// CYCHANNEL_MTBDFU_NOT_FOUND(D) on non-existent file
int getJsonObjFromFile(const QString &filename, QJsonObject &jsonObject);

/// Parses JSON file and gets data about sessions that include custom commands
/// \param filename JSON file with custom command data
/// \param sessions The data for every session with custom commands in it
/// \param metaHeaderData The metadata for current DFU session
/// \return CYCHANNEL_NO_ERROR(0) on success, CYCHANNEL_MTBDFU_CONFIGURATION_ERROR(E), CYCHANNEL_MTBDFU_NOT_FOUND(D) on failure
int parseSessionsFromJson(const QString &filename, std::vector<QJsonArray> &sessions, CyBtldr_CustomCommandHeaderData &metaHeaderData);

/// Generate .mtbdfu filled with JSON custom command data.
/// \param filename The file containing the custom command data.
/// \param action The action that should be written to .mtbdfu file.
/// \param generatedFilename The filename of generated .mtbdfu file.
/// \return Bool variable whether file was generated or not
bool generateMtbDfuJson(const QString filename, const QString action, QString &generatedFilename,
                        CyBtldr_CustomCommandHeaderData &metaHeaderData);

static const QLatin1String ProgramAction = QLatin1String("Program");
static const QLatin1String ExecuteAction = QLatin1String("Execute");
static const QLatin1String VerifyAction = QLatin1String("Verify");
static const QLatin1String EraseAction = QLatin1String("Erase");
static const QLatin1String NoAction = QLatin1String("None");

static const QLatin1String FileVersion = QLatin1String("File Version");
static const QLatin1String ProductId = QLatin1String("Product Id");
static const QLatin1String ApplicationId = QLatin1String("Application Id");
static const QLatin1String ChecksumType = QLatin1String("Packet Checksum Type");

static const QLatin1String MetaSection = QLatin1String("APPInfo");
static const QLatin1String CommandsSection = QLatin1String("commands");
static const QLatin1String SessionsSection = QLatin1String("sessions");

static const QLatin1String CmdIdField = QLatin1String("cmdId");
static const QLatin1String DataLengthField = QLatin1String("dataLength");
static const QLatin1String DataBytesField = QLatin1String("dataBytes");
static const QLatin1String DataFileField = QLatin1String("dataFile");
static const QLatin1String RspField = QLatin1String("rsp");
static const QLatin1String StatusCodeField = QLatin1String("statusCode");
static const QLatin1String ResponseLengthField = QLatin1String("responseLength");
static const QLatin1String ResponseField = QLatin1String("response");
static const QLatin1String ChecksumField = QLatin1String("checksum");
static const QLatin1String OutFileField = QLatin1String("outFile");
static const QLatin1String OutCliField = QLatin1String("outCli");
static const QLatin1String FlashRowLengthField = QLatin1String("flashRowLength");
static const QLatin1String StartOffsetField = QLatin1String("startOffset");
static const QLatin1String RepeatField = QLatin1String("repeat");
static const QLatin1String CommandSetField = QLatin1String("commandSet");

static const QLatin1String CmdVerifyChecksum = QLatin1String("0x31");
static const QLatin1String CmdEraseRow = QLatin1String("0x34");
static const QLatin1String CmdSync = QLatin1String("0x35");
static const QLatin1String CmdSendData = QLatin1String("0x37");
static const QLatin1String CmdSendDataNoRsp = QLatin1String("0x47");
static const QLatin1String CmdEraseData = QLatin1String("0x44");
static const QLatin1String CmdProgramData = QLatin1String("0x49");
static const QLatin1String CmdVerifyData = QLatin1String("0x4A");
static const QLatin1String CmdSetMetadata = QLatin1String("0x4C");

static const int HEX_BASE = 16;

}  // namespace core
}  // namespace cydfuht
