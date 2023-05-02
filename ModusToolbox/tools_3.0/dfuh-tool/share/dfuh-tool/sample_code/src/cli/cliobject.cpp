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

#include "cliobject.h"

#include <QCoreApplication>
#include <cstdio>
#include <iostream>

#include "printprogress.h"

namespace cydfuht {
namespace cli {

CliObject::CliObject() : m_workerobject() {
    m_workerobject.moveToThread(&m_workerthread);

    qRegisterMetaType<cydfuht::core::CyChannelSettings>();
    connect(this, &CliObject::workerInitialize, &m_workerobject, &backend::WorkerObject::initialize);
    connect(this, &CliObject::workerClose, &m_workerobject, &backend::WorkerObject::close);
    connect(this, &CliObject::workerProgramDevice, &m_workerobject, &backend::WorkerObject::programDevice);
    connect(this, &CliObject::workerVerifyDevice, &m_workerobject, &backend::WorkerObject::verifyDevice);
    connect(this, &CliObject::workerEraseDevice, &m_workerobject, &backend::WorkerObject::eraseDevice);
    connect(&m_workerobject, &backend::WorkerObject::progressReport, this, &CliObject::workerProgressReport);
    connect(&m_workerobject, &backend::WorkerObject::returnValue, this, &CliObject::workerReturnValue);
    m_workerthread.start();
    emit workerInitialize();
}

void CliObject::program(const cydfuht::core::CyChannelSettings &channel, const QString &filename) {
    emit workerProgramDevice(channel, filename);
}

void CliObject::verify(const cydfuht::core::CyChannelSettings &channel, const QString &filename) {
    emit workerVerifyDevice(channel, filename);
}

void CliObject::erase(const cydfuht::core::CyChannelSettings &channel, const QString &filename) {
    emit workerEraseDevice(channel, filename);
}

void CliObject::workerProgressReport(double percentage) { printProgress(percentage); }

void CliObject::workerReturnValue(int rval) {
    if ((rval == CYRET_SUCCESS) || (rval == (CYRET_ERR_BTLDR_MASK | CYBTLDR_STAT_SUCCESS))) {
        workerProgressReport(100.0);
    }
    std::cout << std::endl << cydfuht::core::cyChannelErrToString(rval).toStdString() << std::endl;
    m_workerobject.abort();
    emit workerClose();
    m_workerthread.exit(0);
    m_workerthread.wait();
    QCoreApplication::exit(rval);
}

}  // namespace cli
}  // namespace cydfuht
