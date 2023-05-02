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

#include "cychanneli2c.h"

#include <QDebug>
#include <chrono>
#include <thread>
#include <vector>

#include "cydfuhtcore.h"

namespace cydfuht {
namespace core {

int CyChannelI2c::readData(uint8_t data[], int size) {
    logOperation("Read");
    const uint8_t MAX_READS = 30;
    const uint8_t BAD_STATUS_DATA = 0xFF;
    uint8_t reads = 0;
    bool good_data = false;

    // The I2C component for the bootloader has gone through multiple different incarnations.
    // Some of which respond with 0xFF if they don't have data, some perform clock stretching.
    // Because there is no way to determine which version we are talking with, we need to handle
    // both versions.  To do this, we attempt to read the full amount of data at the beginning.
    // If we are talking with a version that supports clock stretching, it will clock stretch
    // until data is available and we are done.  If we are talking with a version that responds
    // with 0xFF we may get good data, we may get all bad data, or we may get some of each. If
    // we get all good data, we are done and move on.  If we get some bad and some good data, we
    // extract the good data from the end and issue another read to finish reading the good data.
    // If we get all bad data, we perform multiple tries in order to read the good data, one byte
    // at a time.  Once we get something other than 0xFF, we then read the remaining good data
    // packet.

    int err = readDataInternal(data, size);
    if (err == CyChannel::CYCHANNEL_NO_ERROR) {
        int good_bytes;
        int i;  // Index of first good data received.
        for (i = 0; i < size && data[i] == BAD_STATUS_DATA; i++) {
            // see if any good data
        }

        // If the first byte is 0xFF would mean that the packet is invalid. We check the packet for
        // for useful data.
        if (i != 0) {
            if (i == size) {
                // if entire data is invalid the we read 1 byte at a time until we get good data
                // or the max read count is exceeded.
                while (!good_data && reads < MAX_READS) {
                    err = readDataInternal(data, 1);
                    logPacket("Read data (byte)", data, 1);
                    good_data = (err == CyChannel::CYCHANNEL_NO_ERROR) && (data[0] != BAD_STATUS_DATA);
                    reads++;
                    if (!good_data) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    }
                }
                good_bytes = 1;
            } else {
                // otherwise extract the good data from the existing buffer.
                good_data = true;
                good_bytes = size - i;
                for (int j = 0; j < good_bytes; j++) data[j] = data[i + j];
            }

            if (good_data && good_bytes < size) {
                err = readDataInternal(&data[good_bytes], size - good_bytes);
                logPacket("Read data (Remaining Bytes)", &data[good_bytes], size - good_bytes);
            } else if (!good_data) {
                err = CyChannel::CYCHANNEL_OPERATION_TIMEOUT;
            }
        }
    }
    logPacket("Read packet", data, size);
    logErrorCode(err);
    return err;
}

int CyChannelI2c::readDataInternal(uint8_t data[], int size) const {
    std::vector<uint8_t> response_buff(size + 1);
    int err = cybridgeErrToCyChannelErr(
        m_bridge->i2cDataTransfer(this->m_addr, CyBridgeI2CMode::Read | CyBridgeI2CMode::GenStart | CyBridgeI2CMode::GenStop, size, nullptr,
                                  0, &response_buff[0], size + 1));
    if (err == CyChannel::CYCHANNEL_NO_ERROR) {
        if (response_buff[0] == I2C_ACK) {
            memmove(data, &response_buff[1], size);
        } else {
            err = CyChannel::CYCHANNEL_READ_OPERATION_ERROR;
        }
    }
    return err;
}

int CyChannelI2c::writeData(uint8_t data[], int size) {
    logOperation("Write");
    std::vector<uint8_t> response_buff(size + 1);
    int err = cybridgeErrToCyChannelErr(
        m_bridge->i2cDataTransfer(this->m_addr, CyBridgeI2CMode::Write | CyBridgeI2CMode::GenStart | CyBridgeI2CMode::GenStop, 0, data,
                                  size, &response_buff[0], static_cast<int>(response_buff.size())));
    logPacket("Write packet", data, size);

    // Check for ACK from the response buffer
    if (err == CyBridgeErrors::CyBridgeNoError) {
        for (uint8_t resp_byte : response_buff) {
            if (resp_byte != I2C_ACK) {
                err = CyChannel::CYCHANNEL_WRITE_OPERATION_ERROR;
                break;
            }
        }
    }
    logPacket("Write packet (Response)", &response_buff[0], static_cast<int>(response_buff.size()));
    logErrorCode(err);
    return err;
}

int CyChannelI2c::applyParams() {
    const int err = m_bridge->setI2cParam(CyBridgeI2cSetParams::I2cFreqHz, m_freq_hz);
    return cybridgeErrToCyChannelErr(err);
}

void CyChannelI2c::print() const {
    CyChannel::print();
    qDebug() << QString::fromLatin1("Slave Address: %1\nFrequency: %2\n\n").arg(static_cast<int>(m_addr)).arg(m_freq_hz);
}

}  // namespace core
}  // namespace cydfuht
