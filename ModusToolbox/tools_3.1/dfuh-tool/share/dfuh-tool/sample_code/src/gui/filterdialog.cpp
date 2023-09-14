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

#include "filterdialog.h"

#include <QSettings>

#include "settings.h"

namespace cydfuht {
namespace gui {

FilterDialog::FilterDialog(QWidget *parent) : QDialog(parent), m_ui(), m_uartvisible(true), m_i2cvisible(true), m_spivisible(true) {
    m_ui.setupUi(this);
    QSettings &settings = getSettings();
    setInterfaceVisibility(cydfuht::core::InterfaceEnum::UART, settings.value(UartPortsVisible, true).toBool());
    setInterfaceVisibility(cydfuht::core::InterfaceEnum::I2C, settings.value(I2CPortsVisible, true).toBool());
    setInterfaceVisibility(cydfuht::core::InterfaceEnum::SPI, settings.value(SpiPortsVisible, true).toBool());

    connect(m_ui.m_buttonbox_p, &QDialogButtonBox::rejected, this, &FilterDialog::changesRejected);
    connect(m_ui.m_buttonbox_p, &QDialogButtonBox::accepted, this, &FilterDialog::changesAccepted);
}

FilterDialog::~FilterDialog() {
    QSettings &settings = getSettings();
    settings.setValue(UartPortsVisible, m_uartvisible);
    settings.setValue(I2CPortsVisible, m_i2cvisible);
    settings.setValue(SpiPortsVisible, m_spivisible);
}

void FilterDialog::changesAccepted() {
    m_uartvisible = m_ui.m_uart_checkbox_p->isChecked();
    m_i2cvisible = m_ui.m_i2c_checkbox_p->isChecked();
    m_spivisible = m_ui.m_spi_checkbox_p->isChecked();
    emit filterChanged();
}

void FilterDialog::changesRejected() {
    m_ui.m_uart_checkbox_p->setChecked(m_uartvisible);
    m_ui.m_i2c_checkbox_p->setChecked(m_i2cvisible);
    m_ui.m_spi_checkbox_p->setChecked(m_spivisible);
}

bool FilterDialog::shouldInterfaceBeDisplayed(cydfuht::core::InterfaceEnum interface) const {
    switch (interface) {
        case cydfuht::core::InterfaceEnum::UART:
            return m_uartvisible;
        case cydfuht::core::InterfaceEnum::I2C:
            return m_i2cvisible;
        case cydfuht::core::InterfaceEnum::SPI:
            return m_spivisible;
        default:
            return false;
    }
}

void FilterDialog::setInterfaceVisibility(cydfuht::core::InterfaceEnum interface, bool displayed) {
    switch (interface) {
        case cydfuht::core::InterfaceEnum::I2C:
            m_i2cvisible = displayed;
            m_ui.m_i2c_checkbox_p->setChecked(displayed);
            break;
        case cydfuht::core::InterfaceEnum::SPI:
            m_spivisible = displayed;
            m_ui.m_spi_checkbox_p->setChecked(displayed);
            break;
        case cydfuht::core::InterfaceEnum::UART:
            m_uartvisible = displayed;
            m_ui.m_uart_checkbox_p->setChecked(displayed);
            break;
        default:
            break;
    }
}

}  // namespace gui
}  // namespace cydfuht
