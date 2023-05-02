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

#include "cychannel.h"
#include "ui_filterdialog.h"

namespace cydfuht {
namespace gui {

static const QLatin1String UartPortsVisible = QLatin1String("UartPortsVisible");
static const QLatin1String I2CPortsVisible = QLatin1String("I2CPortsVisible");
static const QLatin1String SpiPortsVisible = QLatin1String("SpiPortsVisible");

/// The memory stat section in the main window, and the Create/Open *.cymem... dialogs.
class FilterDialog : public QDialog {
    Q_OBJECT

   public:
    /// The constructor.
    /// \param parent The parent widget of this dialog.
    explicit FilterDialog(QWidget *parent);

    /// The destructor, which saves changes to the QSettings device.
    ~FilterDialog() override;

    /// This class is not copyable.
    /// \param other The object that would be copied, if this was a non-deleted copy constructor.
    /// \return The copied object, if this was a non-deleted copy constructor.
    FilterDialog &operator=(FilterDialog &other) = delete;

    /// This class is not copyable.
    /// \param other The object that would be copied, if this was a non-deleted copy constructor.
    FilterDialog(FilterDialog &other) = delete;

    /// A function to determine whether a specific kind of ports should be displayed.
    /// \param interface The interface type being queried.
    /// \return Whether those ports should be displayed.
    bool shouldInterfaceBeDisplayed(cydfuht::core::InterfaceEnum interface) const;

    /// Tells the filter dialog to check or uncheck its checkboxes (for use at application startup).
    /// \param interface The interface whose visibility is being set.
    /// \param displayed Whether the interface should be visible or not.
    void setInterfaceVisibility(cydfuht::core::InterfaceEnum interface, bool displayed);

   signals:
    /// Notify anyone who cares to know that the filter might have changed.
    void filterChanged();

   public slots:
    /// Received signal sent by the "Cancel" button of the button box.
    void changesRejected();

    /// Receives signal sent by the "OK" button of the button box.
    void changesAccepted();

   private:
    void readConfig();
    Ui::filterdialog m_ui;
    bool m_uartvisible;
    bool m_i2cvisible;
    bool m_spivisible;
};

}  // namespace gui
}  // namespace cydfuht
