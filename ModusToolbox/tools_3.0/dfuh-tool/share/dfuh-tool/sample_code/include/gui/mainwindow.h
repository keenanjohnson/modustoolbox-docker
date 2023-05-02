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

#include <QDateTime>
#include <QDockWidget>
#include <QMainWindow>
#include <QThread>

#include "cychannelsettings.h"
#include "cychannelspi.h"
#include "cydfuhtcore.h"
#include "filterdialog.h"
#include "ui_mainwindow.h"
#include "workerobject.h"

class QLabel;
class QProgressBar;

namespace cydfuht {
namespace gui {

/// The main window of the Device Firmware Update Tool.
class MainWindow : public QMainWindow {
    Q_OBJECT

   public:
    /// The constructor for the main window and all of its widgets.
    MainWindow();

    /*!
     * Sets the location of the log file for the application.
     * @param log The canonical path to the log file(s).
     */
    void setLogFile(const QString &log) { m_logfile = log; }

   protected:
    /// A function that Qt will call if the user attempts to close the window through any method.
    /// \param event An object with accept() and ignore() functions which allow the function to allow or disallow closing the window.
    void closeEvent(QCloseEvent *event) override;

   signals:
    /// Sends a signal to the worker object telling it to program the device.
    /// \param channel The channel to communicate over.
    /// \param filename The file containing the data to program.
    void workerProgramDevice(cydfuht::core::CyChannelSettings channel, QString filename);

    /// Sends a signal to the worker object telling it to verify the programming of the device.
    /// \param channel The channel to communicate over.
    /// \param filename The file containing the data to verify.
    void workerVerifyDevice(cydfuht::core::CyChannelSettings channel, QString filename);

    /// Sends a signal to the worker object telling it to erase the program from the device.
    /// \param channel The channel to communicate over.
    /// \param filename The file containing the program to erase.
    void workerEraseDevice(cydfuht::core::CyChannelSettings channel, QString filename);

    /// Sends a signal to the worker object telling it that it's on its own thread and that it can safely create QObjects.
    void workerInitialize();

    /// Sends a signal to the object telling it to close.
    void workerClose();

   private slots:
    void portRowChanged(int row);
    void filterButtonPressed();
    void filterChanged();
    void findCyacd2File();
    void saveLogFile();
    void programDevice();
    void verifyDevice();
    void eraseDevice();
    void abortOperation();
    void showDocumentation();
    void showAboutBox();
    void validateData();
    void workerProgressReport(double percentage);
    void workerReturnValue(int rval);
    void workerDeviceConnected(QVector<cydfuht::core::CyChannelSettings> newchannels);
    void workerDeviceDisconnected(QString name);

   private:
    static const constexpr int ProgressBarWidth = 430;

    static const QLatin1String WindowState;

    static const QLatin1String UartEndString;
    static const QLatin1String I2CEndString;
    static const QLatin1String SpiEndString;

    static const QLatin1String Cyacd2Filename;

    static const QLatin1String I2CIsDisplayed;
    static const QLatin1String I2CAddress;
    static const QLatin1String I2CSpeed;

    static const QLatin1String SpiIsDisplayed;
    static const QLatin1String SpiMode;
    static const QLatin1String SpiBitOrder;
    static const QLatin1String SpiFreq;

    static const QLatin1String UartIsDisplayed;
    static const QLatin1String UartBaudRate;
    static const QLatin1String UartDataBits;
    static const QLatin1String UartStopBits;
    static const QLatin1String UartParityBits;

    static const int ONE_MHZ_IN_HZ = 1000000;
    static const int FOUR_HUNDRED_KHZ_IN_HZ = 400000;
    static const int HUNDRED_KHZ_IN_HZ = 100000;
    static const int FIFTY_KHZ_IN_HZ = 50000;

    core::CyChannelSettings initializeCurrentChannel();
    void enableOrDisableActions();
    void logMessage(const QString &message);
    int getI2cSpeed() const;
    core::SpiBitOrder getSpiBitOrder() const;
    core::SpiMode getSpiMode() const;
    static QString getChannelDisplayName(const core::CyChannelSettings &channelsettings);

    Ui::mainwindow m_ui;
    FilterDialog m_filterdialog;
    QAction *m_fileerror_p;
    QAction *m_i2caddresserror_p;
    QThread m_workerthread;
    backend::WorkerObject m_workerobject;
    std::vector<core::CyChannelSettings> m_channellist;
    QLabel *m_statusbar_label_p;
    QProgressBar *m_statusbar_progressbar_p;
    QDateTime m_startofoperation;  ///< a null datetime if no operation is underway
    QString m_logfile;
};

}  // namespace gui
}  // namespace cydfuht
