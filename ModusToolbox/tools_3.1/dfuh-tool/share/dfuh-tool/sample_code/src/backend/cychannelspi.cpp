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

#include "cychannelspi.h"

#include <QThread>
#include <chrono>
#include <thread>
#include <vector>

#include "cydfuhtcore.h"

namespace cydfuht {
namespace core {

const char *CyChannelSpi::m_bitorderstrings[3] = {"MSB", "LSB", "UNSET"};

int CyChannelSpi::readData(uint8_t data[], int size) {
    logOperation("Read");
    const static uint8_t TIME_MS = 10;
    const static uint8_t MAX_READS = 30;
    const uint8_t PACKET_START = 0x01;
    const uint8_t PACKET_END = 0x17;
    int err = CyChannel::CYCHANNEL_NO_ERROR;
    uint8_t reads = 0;
    bool good_data = false;

    // After custom commands were added, we cannot know the length of the response packet for sure.
    // For that, we need to read first 4 bytes for some custom commands, get the length of
    // the data bytes from 3rd and 4th bytes, and read data bytes + 2 bytes of checksum + end of
    // the packet. The problem is that the second ReadData operation tries to read data, which
    // may start from 0xFF(e.g. 0xFFFF is the checksum for empty data, which happens quite often)
    // and as a result good data becomes bad data. Because of that packetStarted variable was added.
    if (!m_packet_started) {  // reading the first part of the packet(or the full packet)
        // Note: this is needed for backwards compatibility with newer firmware running older middleware.  If you want to remove this delay
        // when using newer firmware, please integrate our latest changes to spi_transfer.c into your middleware.
        QThread::msleep(1);

        while (!good_data && reads < MAX_READS) {
            err = readDataInternal(data, 1);
            logPacket("Read data (1 byte)", data, 1);
            good_data = (err == CYCHANNEL_NO_ERROR) && (data[0] == PACKET_START);
            if (!good_data) {
                std::this_thread::sleep_for(std::chrono::milliseconds(TIME_MS));
                if (abort) {
                    abort = false;
                    return CYRET_ABORT;
                }
            } else {
                m_packet_started = true;
            }
            reads++;
        }

        if (!good_data) {
            err = CyChannel::CYCHANNEL_OPERATION_TIMEOUT;
        }

        if (err == CyChannel::CYCHANNEL_NO_ERROR && size > 1) {
            err = readDataInternal(&data[1], size - 1);
            if (data[size - 1] == PACKET_END) m_packet_started = false;
            logPacket("Read data (Remaining Bytes)", &data[1], size - 1);
        }
    } else {  // reading the second part of the packet
        err = readDataInternal(data, size);
        if (data[size - 1] == PACKET_END) m_packet_started = false;
    }

    logPacket("Read packet", data, size);
    logErrorCode(err);
    return err;
}

int CyChannelSpi::readDataInternal(uint8_t data[], int size) const {
    std::vector<uint8_t> write_buff(size, 0);

    return cybridgeErrToCyChannelErr(
        m_bridge->spiDataTransfer(CyBridgeSPIMode::AassertSSOnStart | CyBridgeSPIMode::DeassertSSOnStop, &write_buff[0], data, size));
}

int CyChannelSpi::writeData(uint8_t data[], int size) {
    logOperation("Write");
    // The response buffer is ignored on a write.
    std::vector<uint8_t> response_buff(size);

    // Note: this is needed for backwards compatibility with newer firmware running older middleware.  If you want to remove this delay
    // when using newer firmware, please integrate our latest changes to spi_transfer.c into your middleware.
    QThread::msleep(1);

    const int err = cybridgeErrToCyChannelErr(
        m_bridge->spiDataTransfer(CyBridgeSPIMode::AassertSSOnStart | CyBridgeSPIMode::DeassertSSOnStop, data, &response_buff[0], size));

    if (data[1] == CMD_SEND_DATA_NO_RSP)
        // Command 0x47 sends data too fast since it doesn't wait for the response and
        // the device doesn't keep up with updating the buffer with data, which makes
        // Program Data/Verify Data command fail due to wrong data in the buffer
        // if no delay is used
        QThread::msleep(1);
    logPacket("Write packet", data, size);
    logErrorCode(err);
    return err;
}

int CyChannelSpi::applyParams() {
    int err = m_bridge->setSpiParam(CyBridgeSpiSetParams::SpiFreqHz, m_freq_hz);
    if (err != CyBridgeErrors::CyBridgeNoError) {
        return cybridgeErrToCyChannelErr(err);
    }
    err = m_bridge->setSpiParam(CyBridgeSpiSetParams::SpiModeSet, static_cast<int>(m_mode));
    if (err != CyBridgeErrors::CyBridgeNoError) {
        return cybridgeErrToCyChannelErr(err);
    }
    err = m_bridge->setSpiParam(CyBridgeSpiSetParams::SpiBitOrderSet, static_cast<int>(m_bit_order));
    return cybridgeErrToCyChannelErr(err);
}

void CyChannelSpi::print() const {
    CyChannel::print();
    cydfuht::core::cyDebug() << QString::fromLatin1("SPI Current Mode: %1\nSPI Current Freq: %2\nSPI Current Bit Order: %3\n\n")
                                    .arg(static_cast<int>(m_mode))
                                    .arg(m_freq_hz)
                                    .arg(static_cast<int>(m_bit_order));
}

}  // namespace core
}  // namespace cydfuht
