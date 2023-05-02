/* (c) 2018-2022, Cypress Semiconductor Corporation (an Infineon company)
or an affiliate of Cypress Semiconductor Corporation.  All rights reserved.

This software, including source code, documentation and related materials
("Software") is owned by Cypress Semiconductor Corporation or one of its
affiliates ("Cypress") and is protected by and subject to worldwide patent
protection (United States and foreign), United States copyright laws and
international treaty provisions.  Therefore, you may use this Software
only as provided in the license agreement accompanying the software package from which
you obtained this Software ("EULA").

If no EULA applies, Cypress hereby grants you a personal, non-exclusive, non-
transferable license to copy, modify, and compile the Software source code solely
for use in connection with Cypress's integrated circuit products.
Any reproduction, modification, translation, compilation, or representation of
this Software except as specified above is prohibited without the express
written permission of Cypress.

Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
Cypress reserves the right to make changes to the Software without notice.
Cypress does not assume any liability arising out of the application or use
of the Software or any product or circuit described in the Software.
Cypress does not authorize its products for use in any products where a malfunction
or failure of the Cypress product may reasonably be expected to result in
significant property damage, injury or death ("High Risk Product").
By including Cypress's product in a High Risk Product, the manufacturer
of such system or application assumes all risk of such use and in doing
so agrees to indemnify Cypress against all liability.
*/

//
// @file cybridge.h
// @brief This file describes user interface of CyBridge library
// @author mykt@cypress.com
//
//  The CyBridge library simplifies operation with KitProg3 and MiniProg4 devices
//

#pragma once
#include <memory>

#if !defined(WIN32) || !defined(WIN64)
#define __cdecl
#endif

/* Defines the get parameters of the CyBridge interface */
enum CyBridgeGetParams : uint8_t {
};

/* Defines parameters that can be set to CyBridge interface */
enum CyBridgeSetParams : uint8_t {
  DisableAllLogs,
  DisableWarningLogs,
  DisableErrorLogs,
  DisableInfoLogs
};

/* Defines the connected device */
struct CyBridgeDevice {
  /* The device name */
  const char* Name;
  /* The short device name */
  const char* ShortName;
  /* The product ID */
  uint16_t Pid;
  /* The vendor ID */
  uint16_t Vid;
  /* The device serial number */
  const char* SerialNumber;
  /* The product name */
  const char* Product;
  /* The manufacturer name */
  const char* Manufacturer;
  /* The USB location */
  const char* Location;
  /* The name of the probe to which the current device belongs */
  const char* ProbeName;
  /* The communication type: hid, bulk, serial, hid+serial */
  const char* ComType;
  /* The operation speed in bps - used only by Wiced Bluetooth devices */
  uint32_t OperationSpeed;
};

/* Defines KitProg3 Programming Options */
struct CyBridgeProgrammingFeatures {
  /* Defines if DAPLink mode is supported */
  bool Daplink;
  /* Defines if KitProg3 SWD Prtocol is supported */
  bool KitProgSwd;
  /* Defines if KitProg3 JTAG Prtocol is supported */
  bool KitProgJtag;
  /* Power Cycle Programming Mode */
  bool PowerMode;
  /* Reset Programming Mode */
  bool ResetMode;
  /* SWO */
  bool Swo;
};
 /* KitProg3 Bridging Options */
struct CyBridgeBridgingFeatures {
  /* Primary UART */
  bool Uart;
  /* Secondary UART */
  bool SecondaryUart;
  /* UART HW Control */
  bool HwControlUart;
  /* Special RTS Behaviour */
  bool UartRts;
  /* I2C Interface */
  bool I2c;
  /* I2C Pull Up */
  bool I2cPu;
  /* SPI Interface*/
  bool Spi;
  /* GPIO Interface */
  bool Gpio;
};

/* Defines KP3 specific features */
struct CyBridgeKp3Features {
  /* One KitProg3 LED */
  bool OneLed;
  /* Two KitProg3 LEDs */
  bool TwoLeds;
  /* One KitProg3 Mode Switching Button */
  bool OneButton;
  /* Two KitProg3 Mode Switching Buttons */
  bool TwoButtons;
};

/* KitProg3 Target type */
enum CyBridgeTarget {
  PSoC4 = 0x00,
  PSoC5,
  PSoC6,
  PSoC62M,
  PSoC6A512,
  PSoC6A256,
  PMG1,
  TVII,
  PSoC4500,
  XMC7000
};

/* KitProg3 FRAM options */
enum CyBridgeFramChip {
  FRAM_UNSUPPORTED = 0x00,
  CY15B104QSN = 0x01,
  FM24V10,
  FM25V10,
};

/* KitProg3 QSPI options */
enum CyBridgeQspiChip {
  QSPI_UNSUPPORTED = 0x00,
  S25FL512S = 0x01,
  S25FL064L = 0x02,
  S25FS512S = 0x03,
  S25HL512T = 0x04,
  S25FL256S = 0x05
};

/* KitProg3 WIFI-BT Options */
enum CyBridgeWFBTchip {
  WFBT_UNSUPPORTED = 0x00,
  LBEE5KL1DX = 0x01,
  CYW43012 = 0x02,
  CYW4343W= 0x03,
  CYW43438 = 0x04,
  CYBLE_416045 = 0x05,
  CYBLE_022001 = 0x06,
  Various = 0x07,
  CYSBSYS_RP01 = 0x08,
  BLE = 0x09,
  CYW20819 = 0x0A,
  CYW94373W = 0x0B,
};

/* Defines KitProg3 target onboard */
struct CyBridgeTargetFeatures {
  /* Defines Target Family */
  const char* TargetType;
  /* Secure */
  bool TargetIsSecure;
};

/* Defines KitProg3 Connectivity options onboard */
struct CyBridgeAdditionalFeatures {
  /* FRAM */
  const char* FramType;
  /* WIFI/BT */
  const char* ConnectivityChip;
};

/* Defines KitProg3 Board Features */
struct CyBridgeBoardFeatures {
  /* Voltage Control */
  bool VoltageControl;
  /* Voltage measurment */
  bool VoltageMeasurement;
  /* Power On/Off */
  bool PowerOnOff;
  /* Default VTARG output disabled */
  bool VTARG;
  /* Expansion Pmod Header */
  bool ExpansionPmodHeaders;
  /* Expansion Arduino Header */
  bool ExpansionArduinoHeaders;
  /* Other Expansion Headers */
  bool ExternalPower;
  /* CapSense */
  bool CapSense;
};

/* Defines KiProg3 QSPI options */
struct CyBridgeQspiSupport {
  const char* QspiType;
};

/* KitProg3 Device Features */
struct CyBridgeKitProg3UidDeviceInfoFeatures {
  /* Bridging options */
  CyBridgeBridgingFeatures BridgingFeatures;
  /* Programming modes and interfaces */
  CyBridgeProgrammingFeatures ProgrammingFeatures;
  /* KP3 operational features */
  CyBridgeKp3Features KitProgFeatures;
  /* Target Family and secure flag */
  CyBridgeTargetFeatures TargetFeatures;
  /* Connectivity and FRAM Features */
  CyBridgeAdditionalFeatures BtFramFeatures;
  /* QSPI Features */
  CyBridgeQspiSupport QspiSupportFeatures;
  /* Additional Board Features */
  CyBridgeBoardFeatures BoardFeatures;
};

/* Defines KitProg3 device */
struct CyBridgeKitProg3UidDeviceInfo {
  /* The device name */
  const char* Name;
  /* List of Features */
  CyBridgeKitProg3UidDeviceInfoFeatures FeaturesList;
  /* The product Hardware ID */
  uint8_t HardwareId;
  /* SIID */
  uint32_t TargetSiliconId;
  /* Unique identifier of KP3 device */
  uint32_t Uid;
  /* MBED Board ID */
  const char* MbedBoardId;
  /* Minor Version of UID */
  uint8_t VerMinor;
  /* Major Version of UID */
  uint8_t VerMajor;
  /* Raw KitProg3 UID Data */
  uint8_t KitProg3UidData[62];

};

/* Defines the properties of the connected device */
struct CyBridgeDeviceProps {
  /* The device USB characteristics */
  CyBridgeDevice Device;
  /* The number of supported interfaces */
  uint8_t NumIfs;
  /* The comma-separated list of supported interfaces */
  const char* ListIfs;
  /* The major firmware version */
  uint32_t FwVerMaj;
  /* The minor firmware version */
  uint32_t FwVerMin;
  /* The FW build number */
  uint32_t FwBuildNumber;
  /* The hardware ID, this field may hold the KP3 HWID, as well as WICED-BT platform ID */
  uint32_t HwId;
  /* The major protocol version */
  uint32_t ProtVerMaj;
  /* The minor protocol version */
  uint32_t ProtVerMin;
  /* If this field contains 'true' the KitProg3 UID is available and
  the next field Kp3UidInfo contains valid data describing
  capabilities of KitProg3 device and the board itself */
  bool UidIsPresent;
  /* Information about device from KitProg3 UID */
  CyBridgeKitProg3UidDeviceInfo Kp3UidInfo;
};

/* Defines the properties of the CyBridge library */
struct CyBridgeProps {
  /* If true - changes default HID device access mode from (Exclusive + Shared) to only Exclusive on Windows 8/10.
     This property has no effect on Linux or macOS */
  bool ExclusiveHidAccessMode;
};

/* Defines device types to be reported by CyBridge */
enum CyBridgeDevices : uint32_t {
  /* All devices */
  All = 0x0000,
  /* CMSIS-DAP devices */
  CMSIS_DAP = 0x0001,
  /* All Bridge devices */
  Bridge = 0x0002,
  /* KitProg2 Bridge devices  */
  Bridge_KP2 = 0x0004,
  /* KitProg3 and MiniProg4 Bridge devices */
  Bridge_KP3_MP4 = 0x0008,
  /* UART devices */
  UART = 0x0010,
  /* JLink devices */
  JLink = 0x0020,
  /* WICED WiFi devices */
  WICED_WiFi = 0x0040,
  /* WICED BlueTooth devices */
  WICED_BT = 0x0080,
  /* KitProg bridge with UART devices. Automatically enables
   * Bridge_KP3_MP4 | UART if not enabled */
  Bridge_with_UART = 0x1000,
};

/* Defines the levels of the logger */
enum CyBridgeLogLevel {
  Debug = 1,
  Info = 2,
  Warning = 5,
  Error = 6,
};
/// \brief Defaut timeout for UART operations
const uint32_t DefaultTimeout = 1000;

using CyBridgeConnectCallback = void(__cdecl*)(CyBridgeDevice* device_p, bool is_connected, void* client_p);
using CyBridgeLogCallback = void(__cdecl*)(const char* msg_p, CyBridgeLogLevel level, void* client_p);
using CyBridgeUartRxCallback = void(__cdecl*)(uint8_t const* data_p, uint32_t num_bytes, void* client_p);

/* Defines the get parameters of the power-monitor interface */
enum CyBridgePowerMonitorGetParams : uint8_t {
  /* Defines if the power control supports the voltage measurement.
   Contains the "true" or "false" values */
  CanMeasureVoltage = 0
};

/* Defines the get parameters of the GPIO Bridge interface */
enum CyBridgeGPIOGetParams : uint8_t {
  HaveGPIO = 0
};

/* Defines the get parameters of the power control interface */
enum CyBridgePowerControlGetParams : uint8_t {
  /* Defines if the power on/off is supported.  Contains the "true" or "false" values */
  IsOnOffSupported = 0,
  /* Defines if the voltage control is supported.  Contains the "true" or "false" values */
  IsVoltCtrlSupported,
  /* Defines if the power control is continuous. Contains the "true" or "false" values */
  IsContinuous,
  /* The min supported voltage in mV */
  MinVoltMv,
  /* The max supported voltage in mV*/
  MaxVoltMv,
  /* The current set voltage in mV*/
  LastSetVoltMv,
  /* The comma-separated list of supported voltages for discreet power controls.
  The list is empty for the continuous power controls*/
  DiscreteVoltListMv,
  /* "true" if this interface powers the board,
  "false" if an external supply powers the board*/
  IsPoweredByThisInterface
};

/* Defines the get parameters of the I2C interface */
enum CyBridgeI2cGetParams : uint8_t {
  /* The list of supported frequencies in Hz */
  I2cFreqListHz = 0,
  /* The currently selected frequency*/
  CurrentI2cFreqHz = 1,
  /* checks if KP3 firmware supports the I2C multiple transaction mode. Returns "true" if yes. */
  IsMultipleTransactionSupported = 2,
  /* Returns number of available bytes for the next single command to be appended to i2c multiple transaction.
   * Note: Number of available bytes are associated only with a single i2c command's payload. You must not
   * assume that several commands may fit into this buffer. */
  BytesAvailableForNextCmd  = 3,
  /* Returns current state of multiple I2C transaction. Possible values are 0 - stopped, 1 - started,
    see corresponding CyBridgeI2cMultipleTransactionState */
  I2cTransactionState  = 4,
  /* Checks whether current transaction contains commands to be executed */
  HasCommandsToExecute = 5,
  /* Returns "true" if i2c command can be appended to the multiple transaction buffer */
  CanAppendCommand = 6
};

/* Defines the set parameters of the I2C interface */
enum CyBridgeI2cSetParams : uint8_t {
  /* The I2C frequency in Hz */
  I2cFreqHz = 0,
  /* Disables the I2C hardware, waits for the bus to be
  released, and re-enables the I2C hardware */
  RestartI2cMaster
};

/* I2C Modes. Passed as the mode argument to the i2cDataTransfer API. Can be ORed together */
enum CyBridgeI2CMode : uint8_t {
  /* The current operation is I2C Write */
  Write = 0x00,
  /* The current operation is I2C Read */
  Read = 0x01,
  /* Generate the I2C Start condition */
  GenStart = 0x02,
  /* Generate the I2C Restart condition */
  GenReStart = 0x04,
  /* Generate the I2C Stop condition */
  GenStop = 0x08
};

/* Defines the get parameters of the SPI interface */
enum CyBridgeSpiGetParams : uint8_t {
  /* The list of the supported predefined frequencies in Hz.
   * Use the IsSpiFreqPredefined parameter to check if the device supports such a list of frequencies.
   * If not, then the min and max frequencies are returned */
  SpiFreqListHz = 0,
  /* The currently selected frequency */
  CurrentSpiFreqHz = 1,
  /* SPI Mode: 0-3 */
  SpiModeGet = 2,
  /* Bit Order (MSB=0, LSB=1) */
  SpiBitOrderGet = 3,
  /* Defines if the device supports the pre-defined SPI frequencies.
   * If supports, then the SpiFreqListHz parameter contains a list of this frequencies.
   * If not, then the min and max frequencies are returned by the SpiFreqListHz parameter*/
  IsSpiFreqPredefined = 4
};

/* Defines the set parameters of the SPI interface */
enum CyBridgeSpiSetParams : uint8_t {
  /* The SPI frequency in Hz */
  SpiFreqHz = 0,
  /* SPI mode (0-3) */
  SpiModeSet = 1,
  /* The SPI bit order (MSB first = 0, LSB first =1) */
  SpiBitOrderSet = 2,
  /* The SPI SS*/
  SpiSlaveSelect = 3,
};

/* SPI Modes. Passed as the mode argument to the spiDataTransfer API. Can be ORed together */
enum CyBridgeSPIMode : uint8_t {
  /* Generates the High->Low transition on the SS pin before a data transfer */
  AassertSSOnStart = 0x01,
  /* Generates the Low->High transition on the SS pin after a data transfer */
  DeassertSSOnStop = 0x02,
};

/* Defines the get parameters for the SWD and JTAG interfaces */
enum CyBridgeSWDJtagGetParams : uint8_t {
  /* The list of the supported SWD/JTAG frequencies in Hz. If strlen(out_buff_p) == 0,
   * then the interface does not support configurable frequencies */
  SWDJtagFreqListHz = 0,
  /* The supported reset types. Any of "software" or "hardware", or both */
  ResetType = 1,
};

/* Defines the set parameters of the UART interface */
enum CyBridgeUartSetParams : uint8_t {
  /* Baud Rate */
  BaudRate = 0,
  /* The number of the stop bits */
  StopBits = 1,
  /* The parity type */
  ParityType = 2,
  /* The number of the data bits. The supported values are in the range from 5 to 8 */
  DataBits = 3,
};

/* Defines the get parameters of the UART interface */
enum CyBridgeUartGetParams : uint8_t {
  /* The list of supported baud rates,
     Note: If list is empty, the supported baud rates are not specified for the device
           and client code may choose any baud rate valid for its platform */
  BaudRates = 0,
};

/* Defines the number of stop bits supoprted by the UART bridge */
enum CyBridgeUartStopBits : uint8_t {
  /* One stop Bit */
  One = 0,
  /* One and half stop Bits */
  OnePointFive = 1,
  /* Two stop Bits */
  Two = 2
};

/* Defines the parity types supported by the UART bridge */
enum CyBridgeUartParityType : uint8_t {
  /* Odd parity */
  Odd = 0,
  /* Even parity */
  Even = 1,
  /* No parity */
  None = 2
};

/* This enum defines valid GPIO pin numbers for KitProg3 devices with HWID 0x0D.
   The list of KP3 devices with HWID 0x0D:
   CY8CKIT-062S2-43012, CYW9P62S1-43438EVB-01, CYW9P62S1-43012EVB-01,
   CY8CKIT-064B0S2-4343W, CY8CKIT-064S0S2-4343W
*/
enum CyBridgeGpioPinsKp3HwId_0D : uint8_t {
Pin1_hwid_0D = 0x35,
Pin2_hwid_0D = 0x36
};

/*  Drive GPIO Pin modes. */
enum CyBridgeGpioMode : uint8_t {
  /* PIN_DM_DIG_HIZ */
  HiZ = 0x00,
  /* PIN_DM_RES_UP */
  ResUp = 0x01,
  /* PIN_DM_RES_DWN */
  ResDw = 0x02,
  /* PIN_DM_OD_LO */
  OdLo = 0x03,
  /* PIN_DM_OD_HI */
  OdHi = 0x04,
  /* PIN_DM_STRONG */
  DmStr = 0x05,
  /* PIN_DM_RES_UPDWN */
  ResUpdwn = 0x06
};

/* GPIO Pin State Change. */
enum CyBridgeGpioState : uint8_t {
  /* Clear */
  Clear = 0x00,
  /* Set */
  Set = 0x01
};

/* GPIO Pin States. */
enum CyBridgeGpioStateTransition  : uint8_t {
  /* State did not change since previous read */
  Unchanged = 0x00,
  /* Transition from low to high occured */
  LowToHigh = 0x01,
  /* Transition from high to low occured */
  HighToLow = 0x02
};

/// \brief CyBridge errors and statuses
enum CyBridgeErrors : int32_t {
  /* Recomended to upgrade the FW. Returned by openDevice() only */
  CyBridgeRecommendedFWUpgrade = 1,
  /* The operation completed successfully */
  CyBridgeNoError = 0,
  /* Not enough memory to complete the operation */
  CyBridgeNoMem = -1,
  /* An error of the execution. See the operation logs for details */
  CyBridgeOperationError = -2,
  /* Requested an invalid argument */
  CyBridgeInvalidArgument = -3,
  /* Requested an unsupported parameter value */
  CyBridgeUnsupportedValue = -4,
  /* The specified device was not found */
  CyBridgeDeviceNotFound = -5,
  /* Requested an unsupported interface number */
  CyBridgeNotSupportedInterfaceNumber = -6,
  /* The device is not opened */
  CyBridgeDeviceNotOpened = -7,
  /* The device does not have a power-monitor interface */
  CyBridgeNoPowerMonitor = -8,
  /* The device does not have a power-control interface */
  CyBridgeNoPowerControl = -9,
  /* The device does not have an SWD interface */
  CyBridgeNoSwdBridge = -10,
  /* The device does not have an SPI interface */
  CyBridgeNoSpiBridge = -11,
  /* The device does not have an I2C interface */
  CyBridgeNoI2cBridge = -12,
  /* The requested operation is not supported */
  CyBridgeNotSupported = -13,
  /* The size of the output buffer is not enough to contain the whole result */
  CyBridgeNotEnoughOutBuffer = -14,
  /* The device is already opened by either another instance of the CyBridge library or by another application */
  CyBridgeDeviceInUse = -15,
  /* The "initialize API" was not called */
  CyBridgeNotInitialized = -16,
  /* The operation has timed out */
  CyBridgeOperationTimeout = -17,
  /* The FW update is forbidden . Returned by upgradeFw() only */
  CyBridgeFwUpdateForbidden = -18,
  /* Access denied. Check the access permissions for the device */
  CyBridgeAccessDenied = -19,
  /* The device does not have a JTAG interface */
  CyBridgeNoJtagBridge = -20,
  /* The device does not have a UART interface */
  CyBridgeNoUartBridge = -21,
  /* The operation was aborted due to a timeout */
  CyBridgeOperationIsTimedOut = -22,
  /* The FW must be upgraded. Returned by openDevice() only.
     The device is opened, and only upgradeFw() API can be called */
  CyBridgeNeedFWUpgrade = -23,
  /* The CyBridge must be restarted to support the device. Returned by openDevice() only */
  CyBridgeNeedRestart = -24,
  /* The CyBridge failed to set the requested voltage. Returned by setVoltage() only */
  CyBridgeFailToSetVoltage = -25,
  /* The device does not have a USB interface */
  CyBridgeNoUsbBridge = -26,
  /* The data transfer operation was cancelled */
  CyBridgeOperationCancelled = -27,
  /* The requested UART baud rate value is not supported */
  CyBridgeUnsupportedUARTBaudRate = -28,
  /* The requested value of UART data bits is not supported */
  CyBridgeUnsupportedUARTDataBits = -29,
  /* The requested value of UART stop bits is not supported */
  CyBridgeUnsupportedUARTStopBits = -30,
  /* The requested UART parity value is not supported */
  CyBridgeUnsupportedUARTParity = -31,
  /* The device does not have a GPIO interface */
  CyBridgeNoGpioBridge = -32,
  /* No valid FW path provided */
  CyBridgeInvalidFwPath = -33
};

/* Defines the supported KitProg modes */
enum CyBridgeKitProgModeSwitch : uint8_t {
  /* KitProg3 Bootloader mode */
  KitProg3Bootloader = 0,
  /* KitProg3 CMSIS-DAP BULK mode */
  KitProg3Bulk = 1,
  /* KitProg3 CMSIS-DAP HID mode */
  KitProg3Hid = 2,
  /* KitProg3 DAPLink mode */
  KitProg3DAPLink = 3,
  /* KitProg3 Daul Uart mode */
  KitProg3DualUart = 4,
};

enum CyBridgeI2cMultipleTransactionState : uint8_t {
  /* Stopped i2c multiplpe transaction */
  i2cTransactionStopped = 0,
  /* Started i2c multiple transaction.
   This state allows appending commands to the current transaction */
  i2cTransactionStarted = 1,
  /* Multiple transaction is completed.
     It is ready to return the response data */
  i2cTransactionCompleted = 2,
};

/// \brief Defines the CyBridge object capabilities
class CyBridge {
 public:
  /// \brief destroys the CyBridge class
  virtual ~CyBridge() = default;

  /// \brief performs initialization of the CyBridge library.
  /// Without the initialization, the library is not operable
  /// \param devices - the mask of devices to be reported (optional).
  /// \param props - the pointer to the CyBridgeProps structure (optional).
  /// A few devices can be OR-ed. All masks are listed in the
  /// CyBridgeDevices enum. By default, all supported devices are reported
  /// \return error code
  virtual int32_t initialize(uint32_t devices = 0, CyBridgeProps* props = nullptr) = 0;

  /// \brief releases CyBridge object by calling "delete this"
  virtual void exit() = 0;

  /// \brief returns version of CyBridge
  /// \param major - the returned CyBridge major version
  /// \param minor - the returned CyBridge minor version
  /// \param patch - the returned CyBridge path version
  /// \param build - the returned CyBridge build number
  /// \return error code
  virtual int32_t getVersion(uint32_t& major, uint32_t& minor, uint32_t& patch, uint32_t& build) = 0;

  /// \brief returns a description of the passed error code
  /// \param error - the error code from the CyBridgeErrors enum whose
  /// description should be returned
  /// \return - a description of the passed error code
  virtual const char* getErrorDescription(int32_t error) = 0;

  /// \brief Sets the callback function for the Connect Event.
  /// The Connect Event is raised on connect and disconnect of
  /// supported devices. The supported devices are defined in the initialize API
  /// \param func - the pointer to the function to be called if a device is connected/disconnected
  /// \param client_p - the client-supplied data passed to the callback function when called
  virtual void setConnectCallback(CyBridgeConnectCallback func, void* client_p) = 0;

  /// \brief sets the callback function for the CyBridge Logger.
  /// It helps understand the state of the CyBridge and connected device
  /// \param func - the pointer to the function to be called if a log message arrives
  /// \param client_p -the client-supplied data passed to the callback function when called
  /// \param level - the max level of log messages to be displayed. The default log level is Info
  virtual void setLogCallback(CyBridgeLogCallback func, void* client_p, CyBridgeLogLevel level = CyBridgeLogLevel::Info) = 0;

  /// \brief returns a list of connected devices. The returned list must be released by the releaseDeviceList  API
  /// \param list_p - the pointer to return a list of CyBridgeDevice structures.
  /// If nullptr is passed, then only the number of the connected devices is returned
  /// \return if >= 0 - the number of connected devices, otherwise - error code
  virtual int32_t getDeviceList(CyBridgeDevice*** list_p) = 0;

  /// \brief releases a list of connected devices returned by the getDeviceList API
  /// \param list_p - the pointer to the list of CyBridgeDevice structures returned by the getDeviceList API
  /// \return error code
  virtual int32_t releaseDeviceList(CyBridgeDevice** list_p) = 0;

  /// \brief opens a specific device for operation. The list of connected devices
  /// can be obtained via the getDeviceList API
  /// \param device_name_p - the pointer to the name of the device to be opened
  /// \return error code
  virtual int32_t openDevice(const char* device_name_p) = 0;

  /// \brief closes the device opened by the openDevice API.
  /// The API call is not mandatory. Added for the consistency only
  virtual void closeDevice() = 0;

  /// \brief gets the properties of the device opened by the openDevice API.
  /// The returned properties must be released by the releaseDeviceProps API
  /// \param props_p - the pointer to the CyBridgeDeviceProps structure
  /// \return error code
  virtual int32_t getDeviceProps(CyBridgeDeviceProps** props_p) = 0;

  /// \brief releases the device properties returned by the getDeviceProps API
  /// \param props_p - the pointer to the device properties to be released
  /// \return error code
  virtual int32_t releaseDeviceProps(CyBridgeDeviceProps* props_p) = 0;

  /// \brief gets the parameter value of the CyBridge library
  /// The parameter value is returned as a string
  /// \param param - the specific parameter of the CyBridgeGetParams enum to be returned
  /// \param out_buff_p - the pointer to the buffer to store the parameter value in
  /// \param out_buff_len - the size of the buffer pointed by the out_buff_p argument
  /// \return If >= 0 number of bytes in out_buff_p filled by the return value, otherwise - error code
  virtual int32_t getCyBridgeParam(CyBridgeGetParams param, char* out_buff_p, uint32_t out_buff_len) = 0;

  /// \brief sets the parameter value to the CyBridge library
  /// The parameter value is passed as integer value
  /// \param param - the specific parameter of the CyBridgeSetParams enum to be set
  /// \param value - the value to be set
  /// \return error code
  virtual int32_t setCyBridgeParam(CyBridgeSetParams param, uint32_t value) = 0;

  /// \brief gets the parameter value of a power monitor associated with the opened device.
  /// The parameter value is returned as a string
  /// \param param - the specific parameter of the CyBridgePowerMonitorGetParams enum to be returned
  /// \param out_buff_p - the pointer to the buffer to store the parameter value in
  /// \param out_buff_len - the size of the buffer pointed by the out_buff_p argument
  /// \param interface_num - the number of the power monitor interface whose properties should be returned.
  /// By default, a zero interface is used
  /// \return If >= 0 number of bytes in out_buff_p filled by the return value, otherwise - error code
  virtual int32_t getPowerMonitorParam(CyBridgePowerMonitorGetParams param, char* out_buff_p, uint32_t out_buff_len,
                                       uint8_t interface_num = 0) = 0;

  /// \brief gets the parameter value of a power control associated with the opened device.
  /// The parameter value is returned as a string
  /// \param param - the specific parameter of the CyBridgePowerControlGetParams enum to be returned.
  /// \param out_buff_p - the pointer to the buffer to store a parameter value in
  /// \param out_buff_len - the size of the buffer pointed by the out_buff_p argument
  /// \param interface_num - the number of a power control interface whose properties should be returned.
  /// By default, a zero interface is used
  /// \return If >= 0 number of bytes in out_buff_p filled by return value, otherwise - error code
  virtual int32_t getPowerControlParam(CyBridgePowerControlGetParams param, char* out_buff_p, uint32_t out_buff_len,
                                       uint8_t interface_num = 0) = 0;

  /// \brief gets the parameter value of the I2C interface associated with the opened device.
  /// The parameter value is returned as a string
  /// \param param - the specific parameter of the CyBridgeI2cGetParams enum to be returned
  /// \param out_buff_p - the pointer to the buffer to store a parameter value in
  /// \param out_buff_len - The size of the buffer pointed by the out_buff_p argument
  /// \param interface_num - the number of  i2c bridge interface which properties has to be returned
  /// \return if < 0 - error code, else - number of bytes in out_buff_p filled by return value
  virtual int32_t getI2cParam(CyBridgeI2cGetParams param, char* out_buff_p, uint32_t out_buff_len, uint8_t interface_num = 0) = 0;

  /// \brief sets the parameter value of the I2C interface associated with the opened device
  /// \param param - the specific parameter of the CyBridgeI2cSetParams enum to be set
  /// \param value - the value of the parameter to be set
  /// \param interface_num - the number of the I2C interface whose properties should be set.
  /// By default, a zero interface is used
  /// \return error code
  virtual int32_t setI2cParam(CyBridgeI2cSetParams param, uint32_t value, uint8_t interface_num = 0) = 0;

  /// \brief gets the parameter value of the SPI interface associated with the opened device.
  /// The parameter value is returned as a string
  /// \param param - the specific parameter of the CyBridgeSpiGetParams enum to be returned
  /// \param out_buff_p - the pointer to the buffer to store a parameter value in
  /// \param out_buff_len - the size of the buffer pointed by the out_buff_p argument
  /// \param interface_num - the number of the SPI interface whose properties should be returned.
  /// By default, a zero interface is used
  /// \return if >= 0 number of bytes in out_buff_p filled by a return value, otherwise - error code
  virtual int32_t getSpiParam(CyBridgeSpiGetParams param, char* out_buff_p, uint32_t out_buff_len, uint8_t interface_num = 0) = 0;

  /// \brief sets the parameter value of the SPI interface associated with the opened device
  /// \param param - the specific parameter of the CyBridgeSpiSetParams enum to be set
  /// \param value - the value of the parameter to be set
  /// \param interface_num - the number of the SPI interface whose properties should be set.
  /// By default, a zero interface is used
  /// \return error code
  virtual int32_t setSpiParam(CyBridgeSpiSetParams param, uint32_t value, uint8_t interface_num = 0) = 0;

  /// \brief gets the parameter value of the SWD interface associated with the opened device.
  /// The parameter value is returned as a string
  /// \param param - the specific parameter of the CyBridgeSWDJtagGetParams enum to be returned
  /// \param out_buff_p - the pointer to the buffer to store a parameter value in
  /// \param out_buff_len - the size of the buffer pointed by the out_buff_p argument
  /// \param interface_num - the number of the SWD interface whose properties should be returned. By default, a zero interface is used
  /// \return if >= 0 number of bytes in out_buff_p filled by a return value, otherwise - error code
  virtual int32_t getSwdParam(CyBridgeSWDJtagGetParams param, char* out_buff_p, uint32_t out_buff_len, uint8_t interface_num = 0) = 0;

  /// \brief gets the parameter value of the JTAG interface associated with the opened device.
  /// The parameter value is returned as a string
  /// \param param - the pointer to the buffer to store a parameter value in
  /// \param out_buff_p - the size of the buffer pointed by the out_buff_p argument
  /// \param out_buff_len - the size of buffer pointed by out_buff_p argument
  /// \param interface_num - The number of the JTAG interface whose properties should be returned.
  /// By default, a zero interface is used
  /// \return if >= 0 number of bytes in out_buff_p filled by a return value, otherwise - error code
  virtual int32_t getJtagParam(CyBridgeSWDJtagGetParams param, char* out_buff_p, uint32_t out_buff_len, uint8_t interface_num = 0) = 0;

  /// \brief returns a parameter's value of a GPIO bridge associated with opened device
  /// \param param - the specific parameter of GPIO bridge interface to be returned
  /// \param out_buff_p - the pointer to a buffer where parameter's value will be stored
  /// \param out_buff_len - the size of buffer pointed by out_buff_p argument
  /// \param interface_num - the number of GPIO bridge interface which properties has to be returned
  /// \return if >= 0 number of bytes in out_buff_p filled by a return value, otherwise - error code
  virtual int32_t getGPIOParam(CyBridgeGPIOGetParams param, char *out_buff_p, uint32_t out_buff_len,
                                               uint8_t interface_num = 0) = 0;
  /// \brief returns the voltage measured by the opened device in mV
  /// \param measured_volt_p - the measured voltage
  /// \param interface_num - the number of the power meter interface that should measure voltage.
  /// By default, a zero interface is used
  /// \return error code
  virtual int32_t getVoltage(int32_t* measured_volt_p, uint8_t interface_num = 0) = 0;

  /// \brief Sets the voltage in mV by the power control interface associated with the opened device
  /// \param mV - the voltage in mV to be set.
  /// \param interface_num - The number of the power control interface that should set the voltage.
  /// By default, a zero interface is used
  /// \return error code
  virtual int32_t setVoltage(uint32_t mV, uint8_t interface_num = 0) = 0;

  /// \brief enables/disables the output power
  /// \param onOff - defines if the power should be On or Off
  /// \param interface_num - The number of the power control interface to toggle the power.
  /// By default, a zero interface is used
  /// \return error code
  virtual int32_t powerOn(bool onOff, uint8_t interface_num = 0) = 0;

  /// \brief upgrades the FW of the opened device. It closes the opened device before an FW upgrade
  /// \param fw_file_p - The path to the firmware upgrade file.
  /// The allowed values: path to the FW upgrade file or programming file as null terminated string
  /// Also it as allowed to pass the whole programming file as null terminant string. In this case
  /// of passing the whole programming file, string should have the following format:
  /// <"File in buffer>:<format of the file>:<file content>.
  /// E.g. for cyacd file passed as the string "File in buffer:cyacd:2E12706900....."
  /// Note Only cycacd file format is supported for passing via null terminant string
  /// \return error code
  virtual int32_t upgradeFw(const char* fw_file_p) = 0;

  /// \brief sends or receives data over the I2C protocol
  /// \param addr - the 7-bit address of the I2C slave device
  /// \param mode - the mask bits that define the I2C bus protocol signals. Different masks can be ORed.
  /// Listed in the CyBridgeI2CMode enum
  /// \param out_len - the number of bytes to be read from a slave device. This parameter is used only for Read operation.
  /// The value should not exceed (out_buff_len - 1)
  /// \param in_buff_p - The data to send to the I2C slave device. This parameter is used only for Write operation.
  ///  - Its size should be equal to or bigger than write_len.
  ///  - For Read operation, this array is disregarded and its size can be set to 0.
  /// \param in_buff_len - The number of bytes to send to the I2C slave device. This parameter is used only for Write operation
  /// \param out_buff_p - data returned as a result of transaction. It has different meanings for Read and
  /// Write operations. The only common meaning is the first byte - the acknowledgment result of the address byte(1 - ACK, 0 - NACK).
  /// Its size should be equal to or bigger than write_len.
  ///   - Read operation - Contains (1 + out_len) bytes, where out_len is the bytes actually read from the device.
  /// They are meaningful if the address byte is ACKed.
  ///   - Write operation - Contains (1 + in_buff_len) bytes. The second part of the packet contains the ACK / NACK
  /// result for every sent byte. Normally, when the first NACK byte occurs, all others are NACKed as well.
  /// \param out_buff_len - the size of the out buffer
  /// \param interface_num - the number of the I2C interface to be used. By default, a zero interface is used
  /// \return error code
  virtual int32_t i2cDataTransfer(uint8_t addr, uint8_t mode, uint32_t out_len, const uint8_t* in_buff_p, uint32_t in_buff_len,
                                  uint8_t* out_buff_p, uint32_t out_buff_len, uint8_t interface_num = 0) = 0;

  /// \brief sends and receives data over the SPI protocol
  /// \param mode - the mask bits that define the SPI bus protocol signals. Different masks can be ORed.
  ///  Listed in the CyBridgeSPIMode enum
  /// \param in_buff_p - the data to send to the SPI slave device.
  /// \param out_buff_p - the data returned as a result of transaction.
  /// \param buff_len - the number of bytes to transfer. The In and Out buffers should be equal to or bigger than the number
  /// \param interface_num - the number of the SPI interface to be used. By default, a zero interface is used
  /// \return error code
  virtual int32_t spiDataTransfer(uint8_t mode, const uint8_t* in_buff_p, uint8_t* out_buff_p, uint32_t buff_len,
                                  uint8_t interface_num = 0) = 0;

  /// \brief sends data over the UART protocol
  /// \param buff_p - the pointer to the data to send
  /// \param buff_len - the number of the bytes to send
  /// \return error code
  virtual int32_t uartWrite(const uint8_t* buff_p, uint32_t buff_len) = 0;

  /// \brief reads data from the UART protocol until the provided buffer is filled. It is a blocking function that does not
  /// exit until a timeout or receipt of the expected number of bytes
  /// \param buff_p - the pointer to the Read buffer
  /// \param buff_len - the number of bytes to read, must be <= of the size of the buff_p buffer
  /// \param timeout_ms - the timeout for waiting for Read data. The default value is 1000 ms
  /// \return if >= 0 number of read bytes, otherwise - error code
  virtual int32_t uartRead(uint8_t* buff_p, uint32_t buff_len, uint32_t timeout_ms = DefaultTimeout) = 0;

  /// \brief reads the single packet data from the UART protocol. It is a blocking function that does not exit
  /// until a timeout or receipt of a packet of data
  /// \param buff_p - the pointer to the Read buffer
  /// \param buff_len - the size of the buff_p buffer.
  /// \param timeout_ms - the timeout for waiting for Read data. The default value is 1000 ms.
  /// \return if >= 0 number of read bytes, otherwise - error code
  virtual int32_t uartReadSinglePacket(uint8_t* buff_p, uint32_t buff_len, uint32_t timeout_ms = DefaultTimeout) = 0;

  /// \brief starts/stops the Async Read of data from the UART protocol
  /// \param func - the function to be called when new data arrives.
  /// If nullptr is passed, then the Async Read is stopped
  /// \param buff_p - the buffer for Read data
  /// \param buff_len - the size of the buff_p buffer
  /// \param client_p - the client-supplied data passed to the callback function when called
  /// \return error code
  virtual int32_t uartReadAsync(CyBridgeUartRxCallback func, uint8_t* buff_p, uint32_t buff_len, void* client_p) = 0;

  /// \brief sets the parameter value of a the UART interface associated with the opened device
  /// \param param - the specific parameter of the CyBridgeUartSetParams enum to be set
  /// \param value - the value of the parameter to be set
  /// \return error code
  virtual int32_t setUartParam(CyBridgeUartSetParams param, uint32_t value) = 0;

  /// \brief gets the parameter value of the UART interface associated with the opened device.
  /// The parameter value is returned as a string
  /// \param param - the specific parameter of the CyBridgeUartGetParams enum to be returned
  /// \param out_buff_p - the pointer to the buffer to store a parameter value in
  /// \param out_buff_len - the size of the buffer pointed by the out_buff_p argument
  /// \param interface_num - the number of the UART interface whose properties should be returned. By default, a zero interface is used
  /// \return if >= 0 number of bytes in out_buff_p filled by a return value, otherwise - error code
  virtual int32_t getUartParam(CyBridgeUartGetParams param, char *out_buff_p, uint32_t out_buff_len, uint8_t interface_num = 0) = 0;

  /// \brief sends raw data over USB protocol
  /// \param buff_p - the pointer to data to be sent
  /// \param buff_len - the number of bytes to send
  /// \return error code
  virtual int32_t usbWrite(const char* buff_p, uint32_t buff_len) = 0;

  /// \brief sends raw data over USB protocol and receives response
  /// \param in_buff_p - the pointer to data to be sent
  /// \param in_buff_len - the number of bytes to send
  /// \param out_buff_p - the pointer to response data to be received
  /// \param out_buff_len - the number of bytes to receive
  /// \return error code
  virtual int32_t usbWriteRead(const char* in_buff_p, uint32_t in_buff_len, uint8_t* out_buff_p, uint32_t out_buff_len) = 0;

  /// \brief switches KitProg3 kit into the desired mode
  /// \param mode - specifies which mode KitProg3 kit should be switched to.
  /// All the supported modes are defined in the CyBridgeKitProgModeSwitch enum
  /// \return error code
  virtual int32_t modeSwitch(CyBridgeKitProgModeSwitch mode) = 0;

  /// \brief Starts multiple transaction mode. This call initializes the internal library state and allows to start appending I2C commands.
  /// \param interface_num - The number of the I2C interface of device. By default, a zero interface is used.
  /// \return the status of the operation as a standard error_code.
  virtual int32_t i2cStartMultipleTransaction(uint8_t interface_num = 0) = 0;

  /// \brief Stops the previously started multiple I2C transaction.
  /// \return the status of the operation as a standard error_code.
  virtual int32_t i2cStopMultipleTransaction() = 0;

  /// \brief Appends a short I2C command to internal buffer of the current multiple transaction.
  /// \param addr - the 7-bit address of the I2C slave device
  /// \param mode - The mask bits that define the I2C bus protocol signals. Different masks can be ORed. Listed in the CyBridgeI2CMode enum.
  /// \param in_buff_p - The data to send to the I2C slave device. This parameter is used only for Write operation.
  ///                  Its size should be equal to or bigger than length param.
  ///                  For Read operation, this array is disregarded and the buffer pointer can be set to nullptr.
  /// \param in_buff_len - For Write operation - the number of bytes to send to the I2C slave device.
  ///                      For Read operation - the number of bytes to be read from slave device.
  /// \return The status of the operation as a standard error_code.
  ///         The CyBridgeNoMem error is returned if internal buffer is not large enough to store the given command.
  virtual int32_t i2cAppendCommand(uint8_t addr, uint8_t mode, const uint8_t* in_buff_p, uint32_t in_buff_len) = 0;

  /// \brief Executes all commands combined in a multiple I2C transaction.
  /// This call makes firmware to execute all commands previously collected in the current I2C transaction.
  /// \return if >= 0 - the number of actually executed commands, otherwise the status of the operation as a standard error_code.
  virtual int32_t i2cExecuteCommands() = 0;

  /// \brief Returns the response of I2C command by the given index.
  /// \param cmd_index - The index of the command. This index corresponds to the position of the I2C command being added to the transaction
  /// \param resp_buffer_p - The pointer to the buffer where response of the command is returned.
  ///                        For Read operation - Contains number of bytes actually read from the device.
  ///                        For Write operation - Contains the ACK / NACK result for every sent byte. Normally, when the first NACK byte occurs,
  ///                        all others are NACKed as well.
  /// \param buff_len - The length of the response buffer.
  ///                   Note - this buffer size must not be less than number of bytes requested by the command in i2cAppendCommand()
  /// \return If >= 0 - the number of data bytes written to the response buffer.
  ///                   Otherwise - the status of the operation as the standard error_code
  virtual int32_t i2cGetCmdResponse(uint32_t cmd_index, uint8_t * resp_buffer_p, uint32_t buff_len) = 0;

  /// \brief Cancels the (possibly stuck) I2C/SPI data transfer operation.
  /// This call aborts only a single pending/following data transfer operation.
  /// The call to this method affects only next API: i2cDataTransfer(), spiDataTransfer(), i2cExecuteCommands()
  /// \return the status of the operation as the standard error_code
  virtual int32_t cancelTransfer() = 0;
  
  /// \brief Issues the KitProg3 USB Device Reset
  /// \return the status of the operation as the standard error_code
  virtual int32_t resetDevice() = 0;

  /// \brief Issues the Target MCU Reset request via the KitProg3 Bridge or CMSIS-DAP interfaces
  /// \return the status of the operation as the standard error_code
  virtual int32_t resetTarget() = 0;

  /// \brief reads current state of GPIO pin
  /// \param pin - chosen pin
  /// \param state - current state
  /// \return the status of the operation as the standard error_code
  virtual int32_t gpioRead(uint8_t pin, uint8_t &state) = 0;

  /// \brief sets drive mode of chosen GPIO pin
  /// \param pin - chosen pin
  /// \param mode - mode to set to
  /// \return the status of the operation as the standard error_code
  virtual int32_t gpioSetMode(uint8_t pin, CyBridgeGpioMode mode) = 0;

  /// \brief sets state of chosen GPIO pin
  /// \param pin - chosen pin
  /// \param state - state to set to
  /// \return the status of the operation as the standard error_code
  virtual int32_t gpioSetState(uint8_t pin, CyBridgeGpioState state) = 0;

  /// \brief reads whether GPIO pin state has changed from device
  /// \param pin_number defines pin to read state of
  /// \param state returns if GPIO pin state is unchanged,
  /// changed from 1 -> 0 or changed from 0 -> 1
  /// \return the status of the operation as the standard error_code
  virtual int32_t gpioStateChanged(uint8_t pin, CyBridgeGpioStateTransition &state) = 0;

};

typedef CyBridge*(__cdecl* createCyBridge_t)();

#if defined(WIN32) || defined(_WIN64)
#ifdef CYBRIDGE_EXPORT
#define CYBRIDGE_DLL __declspec(dllexport)
#else
#define CYBRIDGE_DLL __declspec(dllimport)
#endif
#else
#define CYBRIDGE_DLL __attribute__((visibility("default")))
#endif

extern "C" CYBRIDGE_DLL CyBridge* createCyBridge();

// --------- Names of interfaces
/// \brief Name of power monitor interface
const char CyBridgeIfacePowMonName[] = "power_monitor";

/// \brief Name of power control interface
const char CyBridgeIfacePowCtrlName[] = "power_control";

/// \brief Name of i2c interface
const char CyBridgeIfaceI2CName[] = "i2c";

/// \brief Name of spi interface
const char CyBridgeIfaceSPIName[] = "spi";

/// \brief Name of swd interface
const char CyBridgeIfaceSWDName[] = "swd";

/// \brief Name of swd interface
const char CyBridgeIfaceJTAGName[] = "jtag";

/// \brief Name of uart interface
const char CyBridgeIfaceUARTName[] = "uart";

/// \brief Name of usb interface
const char CyBridgeIfaceUSBName[] = "usb";

/// \brief Name of GPIO interface
const char CyBridgeIfaceGPIOName[] = "gpio";

// --------- parameters of upgradeFw() API
/// \brief Parameter of upgradeFw() API to upgrade FW from buffer
const char FwInBuffer[] = "File in buffer:";
