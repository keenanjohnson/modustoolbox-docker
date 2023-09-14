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

#include "cychannel.h"

namespace cydfuht {
namespace core {

enum class SpiBitOrder {
    MSB = 0,  ///< Most  Significant Bit
    LSB = 1,  ///< Least Significant Bit
};

enum class SpiMode { MODE0 = 0, MODE1 = 1, MODE2 = 2, MODE3 = 3 };

/// An object that encapsulates communication over a SPI channel.
class CyChannelSpi : public CyChannel {
   public:
    /// The constructor.
    /// \param b The CyBridge associated with this channel.
    /// \param name The interface's name.
    /// \param freq_hz The frequency to set.
    /// \param mode The mode to set.
    /// \param bit_order The bit order to set.
    CyChannelSpi(CyBridge *b, const std::string &name, uint32_t freq_hz, SpiMode mode, SpiBitOrder bit_order)
        : CyChannel(b, InterfaceEnum::SPI, name), m_freq_hz(freq_hz), m_mode(mode), m_bit_order(bit_order) {}

    /// \copydoc CyChannel::readData
    int readData(uint8_t data[], int size) override;
    /// \copydoc CyChannel::writeData
    int writeData(uint8_t data[], int size) override;
    /// \copydoc CyChannel::applyParams
    int applyParams() override;
    /// \copydoc CyChannel::print
    void print() const override;

   private:
    const static char *m_bitorderstrings[3];
    bool m_packet_started = false;
    uint32_t m_freq_hz;  ///< Selected clock frequency in Hz
    SpiMode m_mode;
    SpiBitOrder m_bit_order;

    int readDataInternal(uint8_t data[], int size) const;
};

}  // namespace core
}  // namespace cydfuht
