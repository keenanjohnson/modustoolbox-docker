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

#include <QTextStream>
#include <QVector>
#include <boost/core/noncopyable.hpp>

#include "cybridge.h"
#include "cychannel.h"
#include "cychannelsettings.h"
#include "cydfuhtutils.h"

namespace cydfuht {
namespace core {

class CyChannel;

std::unique_ptr<CyBtldr_CommunicationsData> convertChannelToBtldrComm(CyChannel *channel);

/// The class that manages the behavior of the backend.
class CyDfuhtCore : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(CyDfuhtCore)

   public:
    /// The constructor.
    /// \param connectcallback A function to call when devices are connected or disconnected.
    /// \param callback_p A pointer that will be passed as one of the arguments to setConnectCallBack in CyBridge (can be nullptr).
    explicit CyDfuhtCore(CyBridgeConnectCallback connectcallback, void *callback_p = nullptr);

    /// Programs a device with the contents of a .cyacd2 file.
    /// \param filename The .cyacd2 file.
    /// \param channelsettings The channel to use when communicating with the hardware.
    /// \param update The function to call with progress updates.
    /// \return CYRET_SUCCESS (0) on success, other constants from cybtldr_utils.h on failure.
    int program(const char *filename, const CyChannelSettings &channelsettings, CyBtldr_ProgressUpdate update = &genericPrintUpdate);

    /// Verifies that a device holds the contents of a .cyacd2 file.
    /// \param filename The .cyacd2 file.
    /// \param channelsettings The channel to use when communicating with the hardware.
    /// \param update The function to call with progress updates.
    /// \return CYRET_SUCCESS (0) on successful verification, (CYRET_ERR_BTLDR_MASK | CYBTLDR_STAT_ERR_VERIFY) on failed verification.
    int verify(const char *filename, const CyChannelSettings &channelsettings, CyBtldr_ProgressUpdate update = &genericPrintUpdate);

    /// Erases the contents of a .cyacd2 file from a device.
    /// \param filename The .cyacd2 file.
    /// \param channelsettings The channel to use when communicating with the hardware.
    /// \param update The function to call with progress updates.
    /// \return CYRET_SUCCESS (0) on success, other constants from cybtldr_utils.h on failure.
    int erase(const char *filename, const CyChannelSettings &channelsettings, CyBtldr_ProgressUpdate update = &genericPrintUpdate);

    /// Sends a user-defined custom command to the device.
    /// \param filename The file containing the custom command data.
    /// \param channelsettings The channel to use when communicating with the hardware.
    /// \param update The function to call with progress updates.
    /// \return CYRET_SUCCESS (0) on success, other constants either from cybtldr_utils.h (on communication failure) or from cychannel.h (on
    /// input file parsing header).
    int sendCommand(const QString &filename, const CyChannelSettings &channelsettings, CyBtldr_ProgressUpdate update = &genericPrintUpdate);

    /// Tells a program(), verify() or erase() function call running on another thread to stop running.
    /// \return Always CYRET_SUCCESS (0).
    int abort();

    /// Retrieves the channels associated with the named device.
    /// \param name The name of the device to examine.
    /// \return A vector containing the channels discovered by examining that device.
    QVector<CyChannelSettings> retrieveChannels(const char *name) const;

    /// Retrieves all channels.
    /// NOTE: This function is only for command-line mode.  The GUI should use connect/disconnect events instead.
    /// \return A vector containing information about all channels.
    QVector<CyChannelSettings> retrieveAllChannels() const;

   private:
    std::unique_ptr<CyBridge> m_bridge;
    bool m_eof_reached;
    bool m_programming_data;
    bool m_file_complete;
    uint32_t m_appRowAddr;
    quint32 m_appStart;
    quint32 m_appLength;
    QByteArray m_appRowArray;
    QFile m_dataFile;
    QTextStream m_dataFileStream;
    QVector<quint32> m_addressQueue;
    QVector<quint32> m_chksumQueue;
    QByteArray m_dataQueue;
    quint32 m_startOffset;
    bool m_abort;
    int m_currentProgressCount;
    int m_maxProgressCounts;
    int setCurrentChannel(const CyChannelSettings &settings);
    int fillRowWithZeros(QJsonObject &command);
    void parseHexLineRecord(QJsonObject &command, QString &line, int &err);
    int openDataFile(QJsonObject &command);
    void resetCoreState();
    void fillProgramDataCommand(CyBtldr_CustomCommandData &cmdData);
    void getMetadataFromBytes(QByteArray &dataBytesArray, bool &ok);
    int getCommandDataBuffer(QJsonObject &command, CyBtldr_CustomCommandData &cmdData, CyBtldr_ProgressUpdate *update);
    int getDataFromString(QJsonObject &command, CyBtldr_CustomCommandData &cmdData);
    int getDataFromFile(QJsonObject &command, CyBtldr_CustomCommandData &cmdData, CyBtldr_ProgressUpdate *update);
    int runCustomSession(const CyChannelSettings &settings, CyBtldr_ProgressUpdate *update, QJsonArray &session,
                         CyBtldr_CustomCommandHeaderData &metaHeaderData);
    int runCustomCommand(const CyChannelSettings &settings, CyBtldr_ProgressUpdate *update, QJsonObject &command,
                         CyBtldr_CustomCommandHeaderData &metaHeaderData);
    int runAction(const CyBtldr_Action &action, const CyChannelSettings &settings, CyBtldr_ProgressUpdate *update, const char *cyAcd2File);
};

}  // namespace core
}  // namespace cydfuht
