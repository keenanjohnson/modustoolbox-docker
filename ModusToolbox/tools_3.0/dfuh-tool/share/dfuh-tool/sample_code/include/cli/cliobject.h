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
#include <QString>
#include <QThread>

#include "cychannelsettings.h"
#include "workerobject.h"

namespace cydfuht {
namespace cli {

/// An object that handles the Qt signals and slots for the CLI.
class CliObject : public QObject {
    Q_OBJECT

   public:
    CliObject();
    ~CliObject() override = default;

    /// Program the device.
    /// \param channel The channel to communicate over.
    /// \param filename The file containing the data to program.
    void program(const cydfuht::core::CyChannelSettings &channel, const QString &filename);

    /// Verify the programming of the device.
    /// \param channel The channel to communicate over.
    /// \param filename The file containing the data to verify.
    void verify(const cydfuht::core::CyChannelSettings &channel, const QString &filename);

    /// Erase the program from the device.
    /// \param channel The channel to communicate over.
    /// \param filename The file containing the program to erase.
    void erase(const cydfuht::core::CyChannelSettings &channel, const QString &filename);

   signals:
    /// Sends a signal to the worker object telling it to program the device.
    /// \param channel The channel to communicate over.
    /// \param filename The file containing the data to program.
    void workerProgramDevice(cydfuht::core::CyChannelSettings channel, QString filename);

    /// Sends a signal to the worker object telling it to verify the programming of the device.
    /// \param channel The channel to communicate over.
    /// \param filename The file containing the data to verify.
    void workerVerifyDevice(cydfuht::core::CyChannelSettings channel, QString filename);

    /// Sends a signal to the worker object telling it to erase the program from the device.
    /// \param channel The channel to communicate over.
    /// \param filename The file containing the program to erase.
    void workerEraseDevice(cydfuht::core::CyChannelSettings channel, QString filename);

    /// Sends a signal to the worker object telling it that it's on its own thread and that it can safely create QObjects.
    void workerInitialize();

    /// Sends a signal to the worker object telling it to close.
    void workerClose();

   private slots:
    void workerProgressReport(double percentage);
    void workerReturnValue(int rval);

   private:
    QThread m_workerthread;
    backend::WorkerObject m_workerobject;
};

}  // namespace cli
}  // namespace cydfuht
