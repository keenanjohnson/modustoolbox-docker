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

/// An object that encapsulates communication over I2C.
class CyChannelI2c : public CyChannel {
   public:
    /// The constructor.
    /// \param b The CyBridge associated with this channel.
    /// \param name The interface's name.
    /// \param freq_hz The frequency to set.
    /// \param addr The address to set.
    CyChannelI2c(CyBridge* b, const std::string& name, uint32_t freq_hz, uint8_t addr)
        : CyChannel(b, InterfaceEnum::I2C, name), m_freq_hz(freq_hz), m_addr(addr) {}

    /// \copydoc CyChannel::readData
    int readData(uint8_t data[], int size) override;
    /// \copydoc CyChannel::writeData
    int writeData(uint8_t data[], int size) override;
    /// \copydoc CyChannel::applyParams
    int applyParams() override;
    /// \copydoc CyChannel::print
    void print() const override;

   private:
    const static uint8_t DFLT_ADDR = 8;
    const static uint32_t DFLT_FREQ_HZ = 100000;
    const uint8_t I2C_ACK = 0x01;
    uint32_t m_freq_hz;
    uint8_t m_addr;
    bool m_packet_started = false;

    int readDataInternal(uint8_t data[], int size) const;
    bool readFirstGoodData(uint8_t data[]);
};

}  // namespace core
}  // namespace cydfuht
