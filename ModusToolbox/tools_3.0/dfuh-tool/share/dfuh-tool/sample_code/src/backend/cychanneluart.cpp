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

#include "cychanneluart.h"

#include <QDebug>
#include <QElapsedTimer>
#include <QMetaEnum>

#include "cydfuhtcore.h"

namespace cydfuht {
namespace core {

CyChannelUart::~CyChannelUart() {
    if (m_serial_port_p != nullptr) {
        m_serial_port_p->deleteLater();
    }
}

int CyChannelUart::openConnection() {
    logOperation("Open Device");
    // create serial port
    if (m_serial_port_p == nullptr) {
        m_serial_port_p = new QSerialPort();
    }
    // apply params
    int err = applyParams();
    if (err == CyChannel::CYCHANNEL_NO_ERROR) {
        // open port
        if (m_serial_port_p->open(QIODevice::ReadWrite)) {
            // clear read write buffer
            m_serial_port_p->clear();
        } else {
            err = qserialErrToCyChannelErr(m_serial_port_p->error());
        }
    }
    print();
    logErrorCode(err);
    return err;
}

int CyChannelUart::closeConnection() {
    logOperation("Close Device");
    if ((m_serial_port_p != nullptr) && m_serial_port_p->isOpen()) {
        // close serial port
        m_serial_port_p->close();

        // delete the serial port object.
        delete m_serial_port_p;
        m_serial_port_p = nullptr;
    }
    return CyChannel::CYCHANNEL_NO_ERROR;
}

int CyChannelUart::readData(uint8_t data[], int size) {
    logOperation("Read");
    QElapsedTimer timeout;
    timeout.start();
    int err = CyChannel::CYCHANNEL_NO_ERROR;

    qDebug() << QString::fromLatin1("Waiting for %1 bytes\n").arg(size);

    // Ideally the while loop should have been controlled by the waitForReadReady() function.
    // But due to unusual behavior of that method in Mac machines, where it does not return true
    // when there is new data available in the buffer, that version of the function sometimes
    // incorrectly times out. We work around that by using our own timeout, and continue reading
    // through the buffer either until we get all the data or we time out.
    QByteArray total_bytes_read;
    while (total_bytes_read.size() < size && !timeout.hasExpired(READ_WRITE_TIMEOUT_MS)) {
        m_serial_port_p->waitForReadyRead(10);
        const QByteArray readbytes = m_serial_port_p->read(size - total_bytes_read.size());
        if ((readbytes.size() > 0)) {
            qDebug() << QString::fromLatin1(" Read %1 bytes\n").arg(readbytes.size());
        }
        total_bytes_read += readbytes;
    }

    qDebug() << QString::fromLatin1(" Read operation terminated with %1 bytes\n").arg(total_bytes_read.size());

    if (total_bytes_read.size() == size) {
        std::copy(total_bytes_read.begin(), total_bytes_read.end(), data);
    } else {
        err = CyChannel::CYCHANNEL_OPERATION_TIMEOUT;
    }
    logPacket("Read packet", reinterpret_cast<const uint8_t *>(total_bytes_read.data()), total_bytes_read.size());
    logErrorCode(err);
    return err;
}

int CyChannelUart::writeData(uint8_t data[], int size) {
    logOperation("Write");
    const int bytes_written = m_serial_port_p->write(reinterpret_cast<char *>(data), size);
    const int err = (bytes_written == size) && (m_serial_port_p->waitForBytesWritten(READ_WRITE_TIMEOUT_MS))
                        ? CyChannel::CYCHANNEL_NO_ERROR
                        : qserialErrToCyChannelErr(m_serial_port_p->error());

    logPacket("Write packet", data, size);
    logErrorCode(err);
    return err;
}

int CyChannelUart::applyParams() {
    m_serial_port_p->setPortName(QString::fromStdString(m_name));
    if (m_serial_port_p->setBaudRate(m_baudrate) && m_serial_port_p->setDataBits(m_databits) && m_serial_port_p->setParity(m_paritytype) &&
        m_serial_port_p->setStopBits(m_stopbits)) {
        return CyChannel::CYCHANNEL_NO_ERROR;
    }
    return qserialErrToCyChannelErr(m_serial_port_p->error());
}

void CyChannelUart::print() const {
    CyChannel::print();
    qDebug() << QString::fromLatin1("Baud Rate: %1\nData bits: %2\nParity: %3\nStop bits: %4\n\n")
                    .arg(static_cast<int>(m_baudrate))
                    .arg(static_cast<int>(m_databits))
                    .arg(QLatin1String(QMetaEnum::fromType<QSerialPort::Parity>().valueToKey(m_paritytype)))
                    .arg(QLatin1String(QMetaEnum::fromType<QSerialPort::StopBits>().valueToKey(m_stopbits)));
}

}  // namespace core
}  // namespace cydfuht
