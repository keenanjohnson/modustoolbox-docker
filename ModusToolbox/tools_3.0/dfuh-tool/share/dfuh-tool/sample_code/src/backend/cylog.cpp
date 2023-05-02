/*
 * Copyright 2011-2022 Cypress Semiconductor Corporation (an Infineon company)
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

#include "cylog.h"

#include <QFile>
#include <memory>
#include <mutex>
#include <vector>

namespace cydfuht {
namespace core {

//! Internal implementation of ICyLog
class CyLog : public ICyLog {
   public:
    CyLog() = default;                         //!< constructor
    CyLog(const CyLog &) = delete;             //!< Copy constructor
    CyLog(CyLog &&) = delete;                  //!< Move constructor
    CyLog &operator=(const CyLog &) = delete;  //!< Copy assignment \return this
    CyLog &operator=(CyLog &&) = delete;       //!< Move assignment \return this

    ~CyLog() override {
        m_listeners.clear();
        closeFileInternal();
    }

    bool setFile(const QString &path) override {
        std::lock_guard<std::recursive_mutex> guard(m_lock);
        closeFile();
        std::unique_ptr<QFileDevice> newFile = std::make_unique<QFile>(path);
        if (newFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
            std::swap(newFile, m_file);
            return true;
        }
        return false;
    }

    QString logFilePath() const override {
        std::lock_guard<std::recursive_mutex> guard(m_lock);
        return m_file != nullptr ? m_file->fileName() : QString();
    }

   private:
    void closeFileInternal()  // Non-virtual implementation
    {
        if (m_file != nullptr) {
            m_file->close();
            m_file.reset();
        }
    }

   public:
    void closeFile() override {
        std::lock_guard<std::recursive_mutex> guard(m_lock);
        closeFileInternal();
    }
    void write(const char *str) override { write(str, int(strlen(str))); }
    void write(const std::string &str) override { write(str.c_str(), int(str.size())); }
    void write(const std::wstring &str) override { write(QString::fromStdWString(str)); }
    void write(const QByteArray &str) override { write(str.constData(), str.size()); }
    void write(const QLatin1String &str) override { write(QString(str)); }
    void write(const QString &str) override { write(str.toUtf8()); }
    void write(const QStringView &str) override { write(str.toUtf8()); }
    void write(const char *str, int length) override {
        std::vector<const Listener *> listeners;
        {
            std::lock_guard<std::recursive_mutex> guard(m_lock);
            listeners = m_listeners;
            silentWrite(str, length);
        }
        // Don't call listeners with mutex held to avoid locking issues
        for (const auto *listener : listeners) (*listener)(str, length);
    }
    void addListener(const Listener &func) override {
        std::lock_guard<std::recursive_mutex> guard(m_lock);
        m_listeners.emplace_back(&func);
    }
    void removeListener(const Listener &func) override {
        std::lock_guard<std::recursive_mutex> guard(m_lock);
        auto it = std::find(m_listeners.begin(), m_listeners.end(), &func);
        if (it != m_listeners.end()) m_listeners.erase(it);
    }

    bool handleQtMessagesSilently() override { return m_handleMessagesSilently; }
    void setHandleQtMessagesSilently(bool value) override { m_handleMessagesSilently = value; }
    void setDefaultQtMessageHandler(QtMessageHandler handler) override { m_defaultMessageHandler = handler; }
    QtMessageHandler getDefaultQtMessageHandler() override { return m_defaultMessageHandler; }
    void customQtMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) override {
        QString prefix;
        switch (type) {
            case QtDebugMsg:
                prefix = QStringLiteral("DEBUG");
                break;
            case QtInfoMsg:
                prefix = QStringLiteral("INFO");
                break;
            case QtWarningMsg:
                prefix = QStringLiteral("WARNING");
                break;
            case QtCriticalMsg:
                prefix = QStringLiteral("CRITICAL");
                break;
            case QtFatalMsg:
                prefix = QStringLiteral("ERROR");
                break;
            default:
                assert(false);
        }

        QString txt = QStringLiteral("[%1] %2\n").arg(prefix).arg(msg);
        if (handleQtMessagesSilently()) {
            silentWrite(txt);
        } else {
            write(txt);
        }

        // Call the default handler.
        if (getDefaultQtMessageHandler() != nullptr) {
            // Only send debug messages to the default hander if not a Release build. See CONFIGURATORS-2174.
            if (type != QtDebugMsg) {
                (*getDefaultQtMessageHandler())(type, context, msg);
            } else {
#if defined(_DEBUG)
                (*getDefaultQtMessageHandler())(type, context, msg);
#endif
            }
        }
    }

   private:
    void silentWrite(const QString &str) { silentWrite(str.toUtf8().constData(), str.size()); }
    void silentWrite(const char *str, int length) {
        std::lock_guard<std::recursive_mutex> guard(m_lock);
        if (m_file != nullptr) {
            m_file->write(str, length);
            m_file->flush();
        }
    }

    mutable std::recursive_mutex m_lock;
    std::unique_ptr<QFileDevice> m_file;
    std::vector<const Listener *> m_listeners;
    QtMessageHandler m_defaultMessageHandler = nullptr;
    bool m_handleMessagesSilently = true;
};

ICyLog *ICyLog::inst() {
    static CyLog inst;
    return &inst;
}

ICyLog *ICyLog::test_construct() { return new CyLog(); }

}  // namespace core
}  // namespace cydfuht
