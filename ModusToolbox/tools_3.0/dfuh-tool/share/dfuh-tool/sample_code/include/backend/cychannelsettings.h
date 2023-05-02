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

#include <QMetaType>
#include <QVector>
#include <cstdint>

namespace cydfuht {
namespace core {

/// Enum for supported hardware interfaces. Can be bitwise OR-ed together */
enum class InterfaceEnum { I2C = 0x1, SPI = 0x2, UART = 0x4, NONE = 0x0 };

enum class InterfaceEnum;
enum class SpiBitOrder;
enum class SpiMode;

/// Defines the parity of a UART connection independently of the implementation used.
enum class ParityTypeEnum { NONE, ODD, EVEN };

/// Contains the data needed to create an I2C channel.
struct CyChannelSettingsI2c {
    uint32_t m_freq_hz;  ///< The frequency of the communication, in hertz.
    uint8_t m_addr;      ///< The I2C address (8-120).
};

/// Contains the data needed to create a SPI channel.
struct CyChannelSettingsSpi {
    uint32_t m_freq_hz;       ///< The frequency of the communication, in hertz.
    SpiMode m_mode;           ///< The SPI mode, which determines the clock polarity and clock phase of the serial line.
    SpiBitOrder m_bit_order;  ///< Whether the most or least significant bit is transmitted first.
};

/// Contains the data needed to create a UART channel.
struct CyChannelSettingsUart {
    uint32_t m_baudrate;          ///< The frequency of the communication, in bits per second.
    uint8_t m_databits;           ///< The number of data bits transmitted before parity/stop bits.
    ParityTypeEnum m_paritytype;  ///< Whether a parity bit is being used, and if so, whether it is an even or odd parity bit.
    float m_stopbits;             ///< The number of cycles that the sender must wait before sending the next data grouping.
    // Note: m_stopbits is a float because 1.5 stop bits is a thing that we might want to implement later.
};

/// This contains the data needed to create a channel.
struct CyChannelSettings {
    /// Only one of these will be valid at a time.
    CyChannelSettingsI2c m_i2c = {0};    ///< The I2C settings, if m_interface is I2C.
    CyChannelSettingsSpi m_spi = {0};    ///< The SPI settings, if m_interface is SPI.
    CyChannelSettingsUart m_uart = {0};  ///< The UART settings, if m_interface is UART.

    InterfaceEnum m_interface = InterfaceEnum::NONE;  ///< The interface type (I2C, SPI, UART).
    QString m_name;                                   ///< The name of the device.
    QString m_displayName;                            ///< The display name for the device.
};

}  // namespace core
}  // namespace cydfuht

Q_DECLARE_METATYPE(QVector<cydfuht::core::CyChannelSettings>)
Q_DECLARE_METATYPE(cydfuht::core::CyChannelSettings)
