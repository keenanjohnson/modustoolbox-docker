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

#include <QApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QMessageBox>

namespace cydfuht {
namespace core {

/*!
 * Represents an extensible command line parser which complains about unrecognized command-line flags but does not exit.
 */
class CyCommandLineParser : public QCommandLineParser {
   public:
    /*!
     * Creates an extensible command line parser which complains about unrecognized command-line flags but does not exit.
     */
    explicit CyCommandLineParser() : QCommandLineParser(){};

    /*!
     * Processes the command line arguments.
     *
     * @param app The application instance.
     * @param allowUnrecognized Flag to indicate if unrecognized arguments are allowed.
     * If true, unrecognized arguments generate a warning and the application continues.
     * If false, they generate an error and terminate the application.
     */
    void process(const QCoreApplication &app, bool allowUnrecognized) {
        // QCoreApplication::arguments() is static, but the app instance must exist so we require it as parameter
        Q_UNUSED(app);

        process(QCoreApplication::arguments(), allowUnrecognized);
    }

    /*!
     * Processes the command line arguments.
     *
     * @param arguments The set of arguments.
     * @param allowUnrecognized Flag to indicate if unrecognized arguments are allowed.
     * If true, unrecognized arguments generate a warning and the application continues.
     * If false, they generate an error and terminate the application.
     */
    void process(const QStringList &arguments, bool allowUnrecognized) {
        qInfo() << "Command line=" << QDir::fromNativeSeparators(QCoreApplication::arguments().join(QLatin1Char(' ')));

        QStringList validArgs;
        if (allowUnrecognized) {
            QString errorMsg;
            QStringList optNames;
            QStringList positionalOpts;
            std::vector<std::pair<QString, QString>> argVec;

            bool valid = parse(arguments);
            if (!valid) {
                errorMsg += errorText() + QLatin1Char('\n');
                optNames = optionNames();
                optNames.removeDuplicates();
                for (const auto &optName : optNames) {
                    auto optVals = values(optName);
                    for (const auto &optVal : optVals) {
                        argVec.emplace_back(optName, optVal);
                    }
                }
                positionalOpts = positionalArguments();
            }

            if (valid) {
                validArgs = arguments;
            } else {
                validArgs += arguments.front();  // Have to keep first arg (app name) for QCommandLineParser::process(const
                                                 // QStringList &arguments) to work
            }

            for (const auto &argPair : argVec) {
                const auto &name = argPair.first;
                const auto &value = argPair.second;
                QString dash = name.size() == 1 ? QLatin1String("-") : QLatin1String("--");
                validArgs += QString(QLatin1String("%1%2")).arg(dash, name);
                if (!value.isEmpty()) {
                    validArgs += QString(QLatin1String("%1")).arg(value);
                }
            }

            if (!positionalOpts.isEmpty()) {
                validArgs += QLatin1String("--");
                for (const QString &value : positionalOpts) {
                    validArgs += value;
                }
            }

            if (!errorMsg.isEmpty()) {
                std::cerr << errorMsg.toStdString();
                if (dynamic_cast<QApplication *>(QCoreApplication::instance()) != nullptr) {
                    errorMsg += QLatin1String("\n") + helpText();
                    QMessageBox::warning(QApplication::activeWindow(), QLatin1String("Command line arguments:"), errorMsg);
                }
            }
        } else {
            validArgs = arguments;
        }

        // Call base process method now that we've parsed and filtered out unrecognized arguments
        QCommandLineParser::process(validArgs);
    }
};

}  // namespace core
}  // namespace cydfuht
