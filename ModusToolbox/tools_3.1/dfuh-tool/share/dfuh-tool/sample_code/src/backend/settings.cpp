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

#include "settings.h"

#include <QApplication>
#include <QStandardPaths>

namespace cydfuht {

QLatin1String oldOrganizationName() {
    static QLatin1String oldName = QLatin1String("Cypress Semiconductor Corporation");
    return oldName;
}

QLatin1String newOrganizationName() {
    static QLatin1String newName = QLatin1String("Infineon_Technologies_AG");
    return newName;
}

QSettings &getSettings(const QSettings::Format &format) {
    static QString dataLocation = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, dataLocation);
    static QSettings settings(QSettings::IniFormat, QSettings::UserScope, newOrganizationName(),
                              QStringLiteral("ModusToolbox/%1").arg(QCoreApplication::applicationName()));

    static bool oldSettingsImported = false;
    if (!oldSettingsImported) {
        QSettings oldSettings(format, QSettings::UserScope, oldOrganizationName(), QApplication::applicationName());
        for (const QString &key : oldSettings.allKeys()) {
            if (!settings.contains(key)) {
                settings.setValue(key, oldSettings.value(key));
            }
        }
        oldSettingsImported = true;
    }

    return settings;
}

}  // namespace cydfuht
