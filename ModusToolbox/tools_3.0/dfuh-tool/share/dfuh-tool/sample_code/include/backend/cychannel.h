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

#include <QDebug>
#include <QSharedPointer>
#include <QVector>
#include <boost/core/noncopyable.hpp>
#include <iomanip>
#include <iostream>
#include <string>

#include "cybridge.h"
#include "cychannelsettings.h"
#include "cydfuhtcore.h"
#include "cydfututils.h"

namespace cydfuht {
namespace core {

/// The virtual parent class for all CyChannel* implementations.
class CyChannel : boost::noncopyable {
   protected:
    /// The constructor that initializes shared fields.
    /// \param b The CyBridge associated with this channel.
    /// \param comm_interface Which interface type the child object implements.
    /// \param name The interface's name.
    CyChannel(CyBridge *b, const InterfaceEnum comm_interface, const std::string &name)
        : m_bridge(b), m_interface(comm_interface), m_name(name), m_dirtyparams(false) {}

   public:
    /// The destructor.
    virtual ~CyChannel() = default;

    /// Operation completed sucessfully.
    const static int CYCHANNEL_NO_ERROR = 0x00;
    /// Operation timed out.
    const static int CYCHANNEL_OPERATION_TIMEOUT = 0x01;
    /// Error occured while reading data.
    const static int CYCHANNEL_READ_OPERATION_ERROR = 0x02;
    /// Error occured while writing data.
    const static int CYCHANNEL_WRITE_OPERATION_ERROR = 0x03;

    /// Device is already opened by another instance of the library or by another application.
    const static int CYCHANNEL_DEVICE_IN_USE = 0x04;
    /// Access denied. Check access permissions to device.
    const static int CYCHANNEL_ACCESS_DENIED = 0x05;
    /// Specified device was not found
    const static int CYCHANNEL_DEVICE_NOT_FOUND = 0x06;
    /// Internal errors that the user should never see.
    const static int CYCHANNEL_INTERNAL_ERROR = 0xFE;
    /// Unknown error type.
    const static int CYCHANNEL_UNKNOWN_ERROR = 0xFF;

    /// Opens a connection using whatever parameters have been specified.
    /// \return CYCHANNEL_NO_ERROR (0) on success, non-zero on failure.
    virtual int openConnection() {
        logOperation("Open Device");
        int err = cybridgeErrToCyChannelErr(m_bridge->openDevice(m_name.c_str()));
        if (err == CYCHANNEL_NO_ERROR) {
            err = applyParams();
        }
        print();
        logErrorCode(err);
        return err;
    }

    /// Closes the connection. It is safe to call this method multiple times.
    /// \return CYCHANNEL_NO_ERROR (0) on success, non-zero on failure.
    virtual int closeConnection() {
        logOperation("Close Device");
        m_bridge->closeDevice();
        return CYCHANNEL_NO_ERROR;
    }

    /// Reads data from an open connection.
    /// \param data The buffer to place the read data into.
    /// \param size The number of bytes to read. (The data buffer must be at least this long.)
    /// \return CYCHANNEL_NO_ERROR (0) on success, non-zero on failure.
    virtual int readData(uint8_t data[], int size) = 0;

    /// Writes data to an open connection.
    /// \param data The buffer that contains the data to be written.
    /// \param size The number of bytes to read.
    /// \return CYCHANNEL_NO_ERROR (0) on success, non-zero on failure.
    virtual int writeData(uint8_t data[], int size) = 0;

    /// Applies any changes made to the configuration of the channel.
    /// \return CYCHANNEL_NO_ERROR (0) on success, negative numbers on failure.
    virtual int applyParams() = 0;

    /// Prints the configuration of the channel to standard output.
    virtual void print() const {
        qDebug() << QString::fromLatin1("Interface:    %1\nTarget Name:  %2\n")
                        .arg(QString::fromStdString(interfaceEnumToString(interface())))
                        .arg(QString::fromStdString(name()));
    }

    /// Getter for the interface type enum.
    /// \return The InterfaceEnum.
    InterfaceEnum interface() const { return m_interface; }

    /// Getter for the interface name.
    /// \return The interface name.
    const std::string &name() const { return m_name; }

    /// Maximum data transfer size in bytes for each channel. Currently this value is set to fixed value for all channels.
    /// The bootloader transfer protocol uses this value to figure out how much data it can transfer per packet.
    const static uint32_t MAX_TRANSFER_SIZE = 32;

   protected:
    CyBridge *m_bridge;               ///< Handle to the CyBridge object created at startup
    const InterfaceEnum m_interface;  ///< The interface supported by this target
    std::string m_name;               ///< Target Name (KitProg/MiniProg/etc + serial number)
    bool m_dirtyparams;               ///< Set dirty = true any time in-memory model of channel is changed
};

}  // namespace core
}  // namespace cydfuht
