/*
 * Copyright 2019-2023 Cypress Semiconductor Corporation (an Infineon company)
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

#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QString>
#include <functional>

namespace cydfuht {
namespace core {
//! \brief Represents logging category singletons.
//!
//!     There are 2 message categories:
//!     ---------------------------------------------------------------------------------------------------
//!                                                 Write to File               Display to Screen
//!     ---------------------------------------------------------------------------------------------------
//!     Category DEFAULT (example: QT messages)
//!         message type-debug                      In DEBUG mode               stdout (In DEBUG mode)
//!         message type-info                       Always                      stdout (In DEBUG mode)
//!         message type-waring                     Always                      stdout (In DEBUG mode)
//!         message type-critical/fatal             Always                      stderr (In DEBUG mode)
//!
//!     Category LOCAL (example: messages in configurator code)
//!         message type-debug                      In DEBUG mode               stdout (In DEBUG mode)
//!         message type-info                       Always                      stdout
//!         message type-waring                     Always                      stdout
//!         message type-critical/fatal             Always                      stderr
//!
class CyLogCat {
   public:
    //! \brief Gets the default ModusToolbox logging category.
    static const QLoggingCategory LOCAL;
};

inline QDebug cyDebug() { return qDebug(CyLogCat::LOCAL); }
inline QDebug cyInfo() { return qInfo(CyLogCat::LOCAL); }
inline QDebug cyWarning() { return qWarning(CyLogCat::LOCAL); }
inline QDebug cyCritical() { return qCritical(CyLogCat::LOCAL); }

//! \brief Logging singleton. All operations are thread-safe.
class ICyLog {
   public:
    using Listener = std::function<void(const char *, int)>;  //!< Listener function

    //! Gets the singleton instance
    //! \return singleton instance
    static ICyLog *inst();

    /*!
     * Create/initializes ICyLog's log file. Removes any old log files over 14 days old. Also, hooks the ICyLog file up
     * as a handler for Qt messages (qDebug, qInfo, etc.).
     * @param forceDebugOutput Force outputting info, debug and warning messages into command line.
     * @return A reference to the singleton ICyLog instance after it has been initialized.
     */
    static cydfuht::core::ICyLog *initLog(bool forceDebugOutput = false) {
        ICyLog::inst()->debugOutputMode = forceDebugOutput;
        // Create log directory
        QString appName = QFileInfo(QCoreApplication::applicationFilePath()).completeBaseName();
        QDir tmpDir = QDir::temp();
        tmpDir.mkdir(appName);
        QDir logDir = QDir::temp().filePath(appName);

        // Remove old logs
        QDateTime threshold = QDateTime::currentDateTime().addDays(-14);
        for (const QFileInfo &f : logDir.entryInfoList({QStringLiteral("*.log")}, QDir::Files)) {
            if (f.lastModified() < threshold) QFile::remove(f.absoluteFilePath());
        }

        // Generate log file name
        QString date = QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd.HHmmss.zzz"));
        quint64 pid = QCoreApplication::applicationPid();
        QString logFile = logDir.filePath(QStringLiteral("%1-%2-%3.log").arg(appName, date, QString::number(pid)));

        cydfuht::core::ICyLog *log = cydfuht::core::ICyLog::inst();
        log->setFile(logFile);
        qInfo() << "Log file=" << QDir::fromNativeSeparators(log->logFilePath());
        return log;
    }

    //! For testing only.
    //! \return instance
    static ICyLog *test_construct();

    //! Set log file name
    //! \param path file path
    //! \return true if successful
    virtual bool setFile(const QString &path) = 0;
    //! Close log file
    virtual void closeFile() = 0;
    //! Get log file path
    //! \return log file path (empty if none)
    virtual QString logFilePath() const = 0;
    //! Set debug output mode for all build configurations
    //! \param debugOutputMode debug output mode bool variable
    virtual void setDebugOutputMode(bool debugOutputMode) = 0;
    //! Write data to log
    //! \param str data to write (must be valid UTF-8 sequence with null terminator)
    virtual void write(const char *str) = 0;
    //! Write data to log
    //! \param str data to write  (must be valid UTF-8 sequence)
    //! \param length length of data
    virtual void write(const char *str, int length) = 0;
    //! Write data to log
    //! \param str data to write
    virtual void write(const std::string &str) = 0;
    //! Write data to log
    //! \param str data to write
    virtual void write(const std::wstring &str) = 0;
    //! Write data to log
    //! \param str data to write (must be valid UTF-8 sequence)
    virtual void write(const QByteArray &str) = 0;
    //! Write data to log
    //! \param str data to write
    virtual void write(const QLatin1String &str) = 0;
    //! Write data to log
    //! \param str data to write
    virtual void write(const QString &str) = 0;
    //! Write data to log
    //! \param str data to write
    virtual void write(const QStringView &str) = 0;
    //! Register a callback for log data.
    //! The callback will not be copied; the caller is responsible for managing its lifetime.
    //! The callback must be thread-safe.
    //! \param func callback
    virtual void addListener(const Listener &func) = 0;
    //! Unregister a callback for log data
    //! \param func callback
    virtual void removeListener(const Listener &func) = 0;
    //! Whether or not listeners should be called when a Qt message is being handled.
    //! \return whether or not listeners should be called when a Qt message is being handled.
    virtual bool handleQtMessagesSilently() = 0;
    //! Sets whether or not listeners should be called when a Qt message is being handled.
    //! \param value whether or not listeners should be called when a Qt message is being handled.
    virtual void setHandleQtMessagesSilently(bool value) = 0;
    //! Message handler that logs the message to the log file then calls getDefaultMessageHandler if
    //! setDefaultMessageHandler has been used to get it.
    //! \param type the message type
    //! \param context the message context
    //! \param msg the message
    virtual void customQtMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) = 0;
    //! Force debug output to console
    bool debugOutputMode = false;

   protected:
    ICyLog() = default;  //!< Constructor

   public:
    virtual ~ICyLog() = default;                 //!< Destructor
    ICyLog(const ICyLog &) = delete;             //!< Copy constructor
    ICyLog(ICyLog &&) = delete;                  //!< Move constructor
    ICyLog &operator=(const ICyLog &) = delete;  //!< Copy assignment \return self
    ICyLog &operator=(ICyLog &&) = delete;       //!< Move assignment \return self
};

}  // namespace core
}  // namespace cydfuht
