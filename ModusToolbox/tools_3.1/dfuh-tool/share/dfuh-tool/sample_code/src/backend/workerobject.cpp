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

#include "workerobject.h"

#include <QSharedPointer>
#include <QThread>
#include <QVector>

#include "cychannel.h"

namespace cydfuht {
namespace backend {

static WorkerObject *g_masterpointer = nullptr;

static void progressUpdate(double percentage) { emit g_masterpointer->progressReport(percentage); }

static void connectCallback(CyBridgeDevice *device_p, bool is_connected, void * /*client_p*/) {
    emit g_masterpointer->connectionSignal(QLatin1String(device_p->Name), is_connected);
}

WorkerObject::WorkerObject() : QObject(nullptr), m_dfuhtcore_p(nullptr) {
    connect(this, &WorkerObject::connectionSignal, this, &WorkerObject::connectionChange, Qt::QueuedConnection);
    if (g_masterpointer != nullptr) {
        throw std::logic_error("Only create one WorkerObject.");
    }
    g_masterpointer = this;
}

void WorkerObject::abort() {
    if (m_dfuhtcore_p != nullptr) m_dfuhtcore_p->abort();
}

void WorkerObject::initialize() {
    if (m_dfuhtcore_p == nullptr) {
        m_dfuhtcore_p = new core::CyDfuhtCore(connectCallback, nullptr);
    }
}

void WorkerObject::close() { m_dfuhtcore_p->deleteLater(); }

void WorkerObject::connectionChange(const QString &name, bool is_connected) {
    if (!is_connected) {
        emit deviceDisconnected(name);
    } else {
        emit deviceConnected(m_dfuhtcore_p->retrieveChannels(name.toLatin1().data()));
    }
}

void WorkerObject::programDevice(cydfuht::core::CyChannelSettings channel, const QString &filename) {
    const int result = m_dfuhtcore_p->program(filename.toLatin1().data(), channel, &progressUpdate);
    emit returnValue(result);
}

void WorkerObject::verifyDevice(cydfuht::core::CyChannelSettings channel, const QString &filename) {
    const int result = m_dfuhtcore_p->verify(filename.toLatin1().data(), channel, &progressUpdate);
    emit returnValue(result);
}

void WorkerObject::eraseDevice(cydfuht::core::CyChannelSettings channel, const QString &filename) {
    const int result = m_dfuhtcore_p->erase(filename.toLatin1().data(), channel, &progressUpdate);
    emit returnValue(result);
}

void WorkerObject::sendCommand(cydfuht::core::CyChannelSettings channel, const QString &filename) {
    const int result = m_dfuhtcore_p->sendCommand(filename, channel, &progressUpdate);
    emit returnValue(result);
}

}  // namespace backend
}  // namespace cydfuht
