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

#pragma once

#include <QChar>
#include <QDebug>
#include <QFile>
#include <QJsonObject>
#include <QSerialPort>
#include <QString>

#include "cybridge.h"
#include "cybtldr_api2.h"
#include "cychannelsettings.h"
#include "cydfuhtparse.h"

namespace cydfuht {
namespace core {

/// Converts a string into the matching interface enum.
/// \param str The string to convert.
/// \return The matching enum, or NONE if the string did not match.
// InterfaceEnum stringToInterfaceEnum(const std::string &str);

/// Converts an enum into the matching string.
/// \param e The enum to convert.
/// \return The matching string for the specified enum (or "NONE" if the value does not correspond to any enum).
std::string interfaceEnumToString(InterfaceEnum e);

/// Just prints the progress percentage, for use as a default callback.
void genericPrintUpdate(double percentage);

/// Logs a hex dump of binary data.
/// \param buff_name A brief description of the data being printed
/// \param buff A buffer containing binary data.
/// \param size The number of bytes to print from the buffer.
void logPacket(const std::string &buff_name, const uint8_t buff[], int size);

/// Logs the internal error code.
/// \param err Internal error code (CyBridgeErrors or QSerialPortError codes)
void logErrorCode(int err);

/// Logs operation to be started.
/// \param operation operation to be started.
void logOperation(const std::string &operation);

/// User's registered callback for displaying CyBridge logs
/// \param msg_p the received log message
/// \param level the log level of the received message
/// \param client_p the user supplied data to setLogCallback API
void logEvent(const char *msg_p, CyBridgeLogLevel level, void *client_p);

/// Converts CyBridgeErrors to CyChannelError
/// \param err CyBridgeError to be converted
/// \return The matching CyChannelError
int cybridgeErrToCyChannelErr(int err);

/// Converts QSerialError to CyChannelError
/// \param err QSerialError to be converted
/// \return The matching CyChannelError
int qserialErrToCyChannelErr(QSerialPort::SerialPortError err);

/// Converts CyChannelErrors into text strings
/// \param err CyChannel error to be converted
/// \return The matching QString
QString cyChannelErrToString(int err);

/// Count lines in hex file
/// \param dataFileName The name of the hex file
/// \return The number of lines in the file
int countDataFileLines(const QString &dataFileName);

void setMtbDfuConfigErrorMessage(QString &errorMsg);

QString &getMtbDfuConfigErrorMessage();

///
/// \param
/// \return
void writeLogTimeToFile(QString &outFileName);

///
/// \param
/// \return
int copyCommandSetOptionsToCommand(QJsonObject &command, QJsonObject &session);

///
/// \param
/// \return
int getRepeatCount(int &repeatCount, QStringList &commandKeys, QString &repeatString);

}  // namespace core
}  // namespace cydfuht
