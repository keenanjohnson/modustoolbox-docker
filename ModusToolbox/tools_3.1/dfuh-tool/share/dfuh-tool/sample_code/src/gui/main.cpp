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

#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QMessageBox>
#include <QProcess>
#include <cstdlib>
#include <iostream>
#include <memory>

#include "cybridge.h"
#include "cycommandlineparser.h"
#include "cydfuhtcore.h"
#include "cylog.h"
#include "cyversion.h"
#include "dfuh_tool.h"
#include "mainwindow.h"
#include "settings.h"

static const QLatin1String DEBUG_OPTION = QLatin1String("debug");

int main(int argc, char *argv[]) {
    try {
        // Needed to handle numbers consistently. Locale-awareness is not currently supported.
        static const QLocale ENGLISH(QLocale::Language::English, QLocale::Country::UnitedStates);
        QLocale::setDefault(ENGLISH);

        QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
        const QApplication app(argc, argv);
        cydfuht::core::ICyLog *log = cydfuht::core::ICyLog::initLog();
        QApplication::setAttribute(Qt::AA_DisableWindowContextHelpButton);

        auto version = cydfuht::core::CyVersion::loadFromVersionFile(QApplication::applicationDirPath());
        QCoreApplication::setApplicationVersion(
            QCoreApplication::tr("%1.%2.%3.%4").arg(VERSION_MAJOR).arg(VERSION_MINOR).arg(VERSION_PATCH).arg(version.Build));
        QApplication::setApplicationName(QApplication::tr("Device Firmware Update Host Tool"));
        QApplication::setApplicationDisplayName(
            QApplication::tr("Device Firmware Update Host Tool %1.%2").arg(VERSION_MAJOR).arg(VERSION_MINOR));
        QApplication::setOrganizationDomain(QLatin1String("cypress.com"));
        QApplication::setOrganizationName(cydfuht::newOrganizationName());
        QApplication::setWindowIcon(QIcon(QLatin1String(":images/dfu")));
        // Workaround for Qt bug with widgets not properly inheriting system fonts. See CONFIGURATORS-3449.
        QApplication::setFont(QApplication::font("QMenu"));

        cydfuht::core::CyCommandLineParser parser;
        parser.addHelpOption();
        parser.addVersionOption();
        parser.addOption(QCommandLineOption(DEBUG_OPTION, QApplication::tr("Appends logging information to the specified file."),
                                            QApplication::tr("filename")));
        parser.process(app, true);

        cydfuht::core::ICyLog::Listener debugListener = [](const char *, int) {};
        std::unique_ptr<QFileDevice> debugfile;
        if (parser.isSet(DEBUG_OPTION)) {
            // We want a listener to also write the messages to the 'debug' file if provided so don't perform silent writes.
            log->setHandleQtMessagesSilently(false);
            debugfile = std::make_unique<QFile>(parser.value(DEBUG_OPTION));
            if (!debugfile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text | QIODevice::Unbuffered)) {
                QMessageBox::critical(nullptr, QApplication::applicationDisplayName(),
                                      QObject::tr("Could not open the file %1 for writing.").arg(parser.value(DEBUG_OPTION)));
                return 1;
            }
            debugListener = [&debugfile](const char *str, int size) {
                debugfile->write(str, size);
                debugfile->flush();
            };
            log->addListener(debugListener);
        }

        cydfuht::gui::MainWindow mainwin;
        mainwin.setLogFile(log->logFilePath());
        mainwin.setWindowTitle(QApplication::applicationDisplayName());
        mainwin.show();

        const auto result = QApplication::exec();

        return result;
    } catch (const std::exception &e) {
        cydfuht::core::cyDebug() << "Unexpected failure. " << e.what();
        return QProcess::ExitStatus::CrashExit;
    }
}
