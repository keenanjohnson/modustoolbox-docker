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

#pragma once

#include <QObject>

#include "cychannelsettings.h"
#include "cydfuhtcore.h"

namespace cydfuht {
namespace backend {

/// An object that runs in its own thread, and runs program/verify/erase operations.
class WorkerObject : public QObject {
    Q_OBJECT

   public:
    WorkerObject();
    ~WorkerObject() override = default;

    /// Tells the worker object to abort its current operation.  Must be called from another thread.
    void abort();

   signals:
    /// Tells the worker object to deal with a change in connected devices.
    /// This exists because some messages come from a thread created in CyBridge, and we want to deal with them in the WorkerObject thread.
    /// \param name The name of the device.
    /// \param is_connected Whether the device has been connected or disconnected.
    void connectionSignal(QString name, bool is_connected);

    /// Report that a new device has been connected.
    /// \param newchannels The channel or channels associated with the new device.
    void deviceConnected(QVector<cydfuht::core::CyChannelSettings> newchannels);

    /// Report that a device has been disconnected.
    /// \param name The name of the device that was disconnected.
    void deviceDisconnected(QString name);

    /// Report progress to anyone who cares to know.
    /// \param percentage The percent completed in the operation. (The returnValue signal is 100% completion.)
    void progressReport(double percentage);

    /// Report that the operation has ended.
    /// \param rval The return value of the operation: CYRET_SUCCESS (0) on success, other constants from cybtldr_utils.h on failure.
    void returnValue(int rval);

   public slots:
    /// Initialize the backend.  Must be triggered after the worker object is on its own thread, but before any other slot is triggered.
    void initialize();

    /// Close the backend. Must be when the worker object is on its own thread and should be the last call before that thread exits.
    void close();

    /// Deal with a change in connected devices.
    /// \param name The name of the device.
    /// \param is_connected Whether the device has been connected or disconnected.
    void connectionChange(const QString &name, bool is_connected);

    /// Program the device.
    /// \param channel The channel to communicate over.
    /// \param filename The file containing the data to program.
    void programDevice(cydfuht::core::CyChannelSettings channel, const QString &filename);

    /// Verify the programming of the device.
    /// \param channel The channel to communicate over.
    /// \param filename The file containing the data to verify.
    void verifyDevice(cydfuht::core::CyChannelSettings channel, const QString &filename);

    /// Erase the program from the device.
    /// \param channel The channel to communicate over.
    /// \param filename The file containing the program to erase.
    void eraseDevice(cydfuht::core::CyChannelSettings channel, const QString &filename);

   private:
    core::CyDfuhtCore *m_dfuhtcore_p;
};

}  // namespace backend
}  // namespace cydfuht
