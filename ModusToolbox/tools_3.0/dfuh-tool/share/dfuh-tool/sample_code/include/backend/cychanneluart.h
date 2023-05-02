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

#include <QSerialPort>

#include "cychannel.h"

namespace cydfuht {
namespace core {

/// This object encapsulates communication over a specific UART channel.
class CyChannelUart : public CyChannel {
   public:
    /// The constructor.
    /// \param b The CyBridge associated with this channel.
    /// \param name The interface's name.
    /// \param baudrate The baud rate to set.
    /// \param databits The number of data bits.
    /// \param parity The parity.
    /// \param stopbits The number of stop bits (currently always "one").
    CyChannelUart(CyBridge* b, const std::string& name, QSerialPort::BaudRate baudrate, QSerialPort::DataBits databits,
                  QSerialPort::Parity parity, QSerialPort::StopBits stopbits)
        : CyChannel(b, InterfaceEnum::UART, name),
          m_serial_port_p(nullptr),
          m_baudrate(baudrate),
          m_databits(databits),
          m_paritytype(parity),
          m_stopbits(stopbits) {}

    /// The destructor. This deletes the QSerialPort pointer.
    ~CyChannelUart() override;
    /// Copy constructor (deleted). This class is not meant to be copyable.
    CyChannelUart(const CyChannelUart&) = delete;
    /// Copy operator (deleted).
    /// \return reference to the copy object.
    CyChannelUart& operator=(const CyChannelUart&) = delete;

    /// \copydoc CyChannel::openConnection
    int openConnection() override;
    /// \copydoc CyChannel::closeConnection
    int closeConnection() override;
    /// \copydoc CyChannel::readData
    int readData(uint8_t data[], int size) override;
    /// \copydoc CyChannel::writeData
    int writeData(uint8_t data[], int size) override;
    /// \copydoc CyChannel::applyParams
    int applyParams() override;
    /// \copydoc CyChannel::print
    void print() const override;

   private:
    const static QSerialPort::BaudRate DFLT_BAUD = QSerialPort::Baud115200;
    const static QSerialPort::DataBits DFLT_DATA_BITS = QSerialPort::Data8;
    const static QSerialPort::StopBits DFLT_STOP_BITS = QSerialPort::StopBits::OneStop;
    const static QSerialPort::Parity DFLT_PARITY_TYPE = QSerialPort::NoParity;
    const static int READ_WRITE_TIMEOUT_MS = 5000;

    QSerialPort* m_serial_port_p;
    QSerialPort::BaudRate m_baudrate;  ///< Baud Rate
    QSerialPort::DataBits m_databits;  ///< Number of data bits
    QSerialPort::Parity m_paritytype;  ///< Odd, Even, None
    QSerialPort::StopBits m_stopbits;  ///< 1, 1.5, 2
};

}  // namespace core
}  // namespace cydfuht
