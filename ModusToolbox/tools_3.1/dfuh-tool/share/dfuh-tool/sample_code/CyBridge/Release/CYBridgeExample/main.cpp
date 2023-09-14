/* (c) 2018-2020, Cypress Semiconductor Corporation or a subsidiary
of Cypress Semiconductor Corporation.All rights reserved.

This software, including source code, documentation and related materials
('Software'), is owned by Cypress Semiconductor Corporation or one of its
subsidiaries('Cypress') and is protected by and subject to worldwide patent
protection(United States and foreign), United States copyright laws and
international treaty provisions.Therefore, you may use this Software only as
provided in the license agreement accompanying the software package from which
you obtained this Software('EULA').

If no EULA applies, Cypress hereby grants you a personal, non-exclusive, non-
transferable license to copy, modify, and compile the Software source code solely
for use in connection with Cypress's integrated circuit products.Any reproduction,
modification, translation, compilation, or representation of this Software except as
specified above is prohibited without the express written permission of Cypress.

Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress reserves the right to
make changes to the Software without notice.Cypress does not assume any liability
arising out of the application or use of the Software or any product or circuit
described in the Software.Cypress does not authorize its products for use in any
products where a malfunction or failure of the Cypress product may reasonably be
expected to result in significant property damage, injury or death ('High Risk
Product').By including Cypress's product in a High Risk Product, the manufacturer of
such system or application assumes all risk of such use and in doing so agrees to
indemnify Cypress against all liability.
*/


//
// @file main.cpp
// @brief This file shows how to use basic capabilities of CyBridge Library
// @author mykt@cypress.com
//
//  Example of use of CyBridge Library
//

#include <algorithm>
#include <chrono>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>
#include <thread>
#include <vector>
#include "cybridge.h"

void i2cShortReadTest(CyBridge* bridge_p);
void i2cLedWriteTest(CyBridge* bridge_p);
void i2cLongReadWriteTest(CyBridge* bridge_p);

void spiLongReadWriteTest(CyBridge* bridge_p);

void setMeasureVoltage(CyBridge* bridge_p, uint32_t voltage);
void powerOnOffTest(CyBridge* bridge_p);

void uint8ToHexStrm(const uint8_t* data_p, size_t num_bytes_to_convert, std::stringstream& out_strm);
void connectEvent(CyBridgeDevice* dev_p, bool is_connected, void* client_p);
void logEvent(const char* msg_p, CyBridgeLogLevel level, void* client_p);
void uartRxCallback(uint8_t const* data_p, uint32_t num_bytes, void* client_p);

static const size_t BufferSize = 5000;
static const uint8_t I2CAddr = 8;

int main(int /*argc*/, char** /*argv[]*/) {
  CyBridge* bridge_p = createCyBridge();
  if (nullptr == bridge_p) {
    std::cout << "Error: Cannot create CyBridge !" << std::endl;
    return -1;
  }

  // register log and connect callbacks
  bridge_p->setLogCallback(&logEvent, nullptr, CyBridgeLogLevel::Info);
  bridge_p->setConnectCallback(&connectEvent, nullptr);

  int ret;
  char buff[BufferSize];

  ret = bridge_p->initialize(CyBridgeDevices::Bridge | CyBridgeDevices::UART);
  if (ret < 0) {
    std::cout << "\n\n Cannot initialize CyBridge. Err = " << bridge_p->getErrorDescription(ret) << "\n\n" << std::endl;
    return 1;
  }

  uint32_t major = 0;
  uint32_t minor = 0;
  uint32_t patch = 0;
  uint32_t build = 0;

  ret = bridge_p->getVersion(major, minor, patch, build);
  if (ret < 0) {
    std::cout << "\n\n Cannot get version of CyBridge. Err = " << bridge_p->getErrorDescription(ret) << "\n\n" << std::endl;
    return 1;
  }

  std::cout << "CyBridge version: " << major << "." << minor << "." << patch << "." << build << std::endl;

  ret = bridge_p->getCyBridgeParam(CyBridgeGetParams::KitProg3FwVersion, buff, BufferSize);
  if (ret < 0) {
    std::cout << "\n\n Cannot get KitProg3 FW version. Err = " << bridge_p->getErrorDescription(ret) << "\n\n" << std::endl;
    return 1;
  }

  std::cout << "KitProg3 FW version: " << buff << std::endl;
  CyBridgeDevice** dev_list = nullptr;
  int count = bridge_p->getDeviceList(&dev_list);

  std::cout << "\ngetDeviceList: Num devices = " << (int)count << std::endl;
  for (int i = 0; i < count; i++) {
    std::cout << "\nN = " << i + 1 << " Name = " << dev_list[i]->Name << ", vid = 0x" << std::hex << dev_list[i]->Vid << ", pid = 0x"
              << dev_list[i]->Pid << std::dec << ", SN = " << dev_list[i]->SerialNumber << ", Manufacturer = " << dev_list[i]->Manufacturer
              << ", Product = " << dev_list[i]->Product << std::endl;

    ret = bridge_p->openDevice(dev_list[i]->Name);
    if (ret == CyBridgeErrors::CyBridgeNeedFWUpgrade) {
      std::cout << "\n\t Need to upgrade Firmware of '" << dev_list[i]->Name << "'" << std::endl;
      bridge_p->upgradeFw();
      continue;
    } else if (ret < 0) {
      std::cout << "\n\t Cannot open device '" << dev_list[i]->Name << "'. Err = " << bridge_p->getErrorDescription(ret) << "\n\n"
                << std::endl;
      continue;
    } else if (ret == CyBridgeErrors::CyBridgeRecommendedFWUpgrade) {
      std::cout << "\n\t Recommended to upgrade Firmware of '" << dev_list[i]->Name << "'" << std::endl;
    } else {
      std::cout << "\n    Firmware of '" << dev_list[i]->Name << "' is up-to-date" << std::endl;
    }

    CyBridgeDeviceProps* props;
    ret = bridge_p->getDeviceProps(&props);
    if (ret < 0) {
      std::cout << "\n\n\t Cannot get device '" << dev_list[i]->Name << "' props. Err = " << bridge_p->getErrorDescription(ret) << "\n\n"
                << std::endl;
      continue;
    }

    std::cout << "\n    Device props: "
              << "Num of interfaces = " << (int)(props->NumIfs) << ", Interfaces = " << props->ListIfs << std::endl;

    if (props->NumIfs > 0) {
      if (strstr(props->ListIfs, CyBridgeIfaceUSBName)) {
        char hex_data[] = "0x8301"; // Turn On programming LED
        ret = bridge_p->usbWrite(hex_data, static_cast<uint32_t>(strlen(hex_data)));
        if (ret < 0) {
          std::cout << "\n\n\t Error of USB Write. Err = " << bridge_p->getErrorDescription(ret) << "\n\n" << std::endl;
          break;
        }
      }

      if (strstr(props->ListIfs, CyBridgeIfaceSWDName)) {
        ret = bridge_p->getSwdParam(CyBridgeSWDJtagGetParams::SWDJtagFreqListHz, buff, BufferSize);
        if (ret >= 0) {
          if (0 == strlen(buff)) {
            std::cout << "\n    SWD Bridge Interface does not support configurable frequencies " << buff << std::endl;
          } else {
            std::cout << "\n    Supported SWD frequencies, Hz = " << buff << std::endl;
          }
        } else {
          std::cout << "\n\n\t Error of retrieving 'CyBridgeSWDGetParams::SwdFreqListKHz' parameter. Err = "
                    << bridge_p->getErrorDescription(ret) << "\n\n"
                    << std::endl;
        }
      }

      if (strstr(props->ListIfs, CyBridgeIfaceJTAGName)) {
        ret = bridge_p->getJtagParam(CyBridgeSWDJtagGetParams::SWDJtagFreqListHz, buff, BufferSize);
        if (ret >= 0) {
          if (0 == strlen(buff)) {
            std::cout << "\n    JTAG Bridge Interface does not support configurable frequencies " << buff << std::endl;
          } else {
            std::cout << "\n    Supported JTAG frequencies, Hz = " << buff << std::endl;
          }
        } else {
          std::cout << "\n\n\t Error of retrieving 'CyBridgeSWDJtagGetParams::SWDJtagFreqListHz' parameter. Err = "
                    << bridge_p->getErrorDescription(ret) << "\n\n"
                    << std::endl;
        }
      }

      bool is_power_mon = false;
      bool can_measure_voltage = false;

      if (strstr(props->ListIfs, CyBridgeIfacePowMonName)) {
        ret = bridge_p->getPowerMonitorParam(CyBridgePowerMonitorGetParams::CanMeasureVoltage, buff, BufferSize);
        if (ret >= 0) {
          std::cout << "\n    Power Monitor: Can measure voltage = " << buff;

          is_power_mon = true;

          if (std::string(buff) == "true") {
            can_measure_voltage = true;
            int32_t voltage;
            bridge_p->getVoltage(&voltage);
            std::cout << ",  Measured voltage = " << voltage << " mV" << std::endl;
          }
        } else {
          std::cout << "\n\n\t Error of retrieving 'CyBridgePowerMonitorGetParams::CanMeasureVoltage' parameter. Err = "
                    << bridge_p->getErrorDescription(ret) << "\n\n"
                    << std::endl;
        }
      }

      if (strstr(props->ListIfs, CyBridgeIfacePowCtrlName)) {
        do {
          bool is_on_off_supported = false;
          bool is_voltage_control_supported = false;
          std::string discrete_voltages;

          // Check if power control supports power on/off feature
          ret = bridge_p->getPowerControlParam(CyBridgePowerControlGetParams::IsOnOffSupported, buff, BufferSize);

          if (ret < 0) {
            std::cout << "\n\n\t Error of retrieving 'CyBridgePowerControlGetParams::IsOnOffSupported' parameter. Err = "
                      << bridge_p->getErrorDescription(ret) << "\n\n"
                      << std::endl;
            break;
          }

          std::cout << "\n    Power Control: Is On/Off supported = " << buff << std::endl;
          if (std::string(buff) == "true") is_on_off_supported = true;

          // Check if power control supports voltage control
          ret = bridge_p->getPowerControlParam(CyBridgePowerControlGetParams::IsVoltCtrlSupported, buff, BufferSize);

          if (ret < 0) {
            std::cout << "\n\n\t Error of retrieving 'CyBridgePowerControlGetParams::IsVoltCtrlSupported' parameter. Err = "
                      << bridge_p->getErrorDescription(ret) << "\n\n"
                      << std::endl;
            break;
          }

          std::cout << "\n    Power Control: Is Voltage Control supported = " << buff << std::endl;
          if (std::string(buff) == "true") is_voltage_control_supported = true;

          // get additional properties of voltage control
          if (is_voltage_control_supported) {
            ret = bridge_p->getPowerControlParam(CyBridgePowerControlGetParams::IsContinuous, buff, BufferSize);

            if (ret < 0) {
              std::cout << "\n\n\t Error of retrieving 'CyBridgePowerControlGetParams::IsContinuous' parameter. Err = "
                        << bridge_p->getErrorDescription(ret) << "\n\n"
                        << std::endl;
              break;
            }

            std::cout << "\n    Power Control: Is continuous = " << buff << std::endl;

			bool is_power_ctrl_continuous = false;			
            if (std::string(buff) == "true") is_power_ctrl_continuous = true;

            ret = bridge_p->getPowerControlParam(CyBridgePowerControlGetParams::MinVoltMv, buff, BufferSize);
            if (ret < 0) {
              std::cout << "\n\n\t Error of retrieving 'CyBridgePowerControlGetParams::MinVoltMv' parameter. Err = "
                        << bridge_p->getErrorDescription(ret) << "\n\n"
                        << std::endl;
              break;
            }

            std::cout << ", Min voltage = " << buff;
            uint32_t min_voltage = std::stoi(buff);

            ret = bridge_p->getPowerControlParam(CyBridgePowerControlGetParams::MaxVoltMv, buff, BufferSize);
            if (ret < 0) {
              std::cout << "\t Error of retrieving 'CyBridgePowerControlGetParams::MaxVoltMv' parameter. Err = "
                        << bridge_p->getErrorDescription(ret) << std::endl;
              break;
            }

            std::cout << ", Max voltage = " << buff;
            uint32_t max_voltage = std::stoi(buff);

            if (!is_power_ctrl_continuous) {
              ret = bridge_p->getPowerControlParam(CyBridgePowerControlGetParams::DiscreteVoltListMv, buff, BufferSize);
              if (ret < 0) {
                std::cout << "\t Error of retrieving 'CyBridgePowerControlGetParams::DiscreteVoltListMv' parameter. Err = "
                          << bridge_p->getErrorDescription(ret) << std::endl;
                break;
              }
              std::cout << ",  Supported voltages = " << buff;
              discrete_voltages = buff;
            }

            ret = bridge_p->getPowerControlParam(CyBridgePowerControlGetParams::IsPoweredByThisInterface, buff, BufferSize);
            if (ret < 0) {
              std::cout << "\t Error of retrieving 'CyBridgePowerControlGetParams::IsPoweredByThisInterface' parameter. Err = "
                        << bridge_p->getErrorDescription(ret) << std::endl;
              break;
            }
            std::cout << ",  Powered by this Interface = " << buff << std::endl;

            if (std::string(buff) == "false") {
              bridge_p->setVoltage(min_voltage);
              bridge_p->powerOn(true);
            }

            if (is_power_mon && can_measure_voltage) {
                if (is_power_ctrl_continuous) {
                  // std::cout << "\t Set min voltage = " << min_voltage << " mV" << std::endl;
                  setMeasureVoltage(bridge_p, min_voltage);

                  uint32_t middle_voltage = (max_voltage - min_voltage) / 2 + min_voltage;

                  // std::cout << "\t Set middle voltage = " << middle_voltage << " mV" << std::endl;
                  setMeasureVoltage(bridge_p, middle_voltage);

                  // std::cout << "\t Set max voltage = " << max_voltage << " mV" << std::endl;
                  setMeasureVoltage(bridge_p, max_voltage);

                } else {
                  // Parsing of comma separated string
                  std::vector<uint32_t> discrete_volt_list;
                  size_t start = 0;
                  size_t end = discrete_voltages.find(",");
                  while (end != std::string::npos) {
                    std::string substr = discrete_voltages.substr(start, end - start);
                    discrete_volt_list.push_back(std::stoi(substr));
                    start = end + 1;
                    end = discrete_voltages.find(",", start);
                    if (end == std::string::npos && start < discrete_voltages.length()) end = discrete_voltages.length();
                  }
                  if (!discrete_volt_list.empty()) {
                    for (uint32_t voltage : discrete_volt_list) setMeasureVoltage(bridge_p, voltage);
                  }
                }
              if (is_on_off_supported) powerOnOffTest(bridge_p);
            }
          }
        } while (0);
      }

      if (strstr(props->ListIfs, CyBridgeIfaceI2CName)) {
        do {
          ret = bridge_p->getI2cParam(CyBridgeI2cGetParams::I2cFreqListHz, buff, BufferSize);
          if (ret < 0) {
            std::cout << "\t Error of retrieving 'CyBridgeI2cGetParams::I2cFreqListHz' parameter. Err = "
                      << bridge_p->getErrorDescription(ret) << std::endl;
            break;
          }
          std::cout << "\n\n    I2C Master: Supported frequencies, Hz = " << buff << std::endl;

          bridge_p->setI2cParam(CyBridgeI2cSetParams::I2cFreqHz, 100000);

          i2cShortReadTest(bridge_p);
          i2cLedWriteTest(bridge_p);
          i2cLongReadWriteTest(bridge_p);
        } while (0);
      }

      if (strstr(props->ListIfs, CyBridgeIfaceSPIName)) {
        do {
          ret = bridge_p->getSpiParam(CyBridgeSpiGetParams::IsSpiFreqPredefined, buff, BufferSize);
          if (ret < 0) {
            std::cout << "\t Error of retrieving 'CyBridgeSpiGetParams::IsSpiFreqPredefined' parameter. Err = "
                      << bridge_p->getErrorDescription(ret) << std::endl;
            break;
          }
          std::cout << "\n\n    SPI Master: Supports predefined frequencies = " << buff << std::endl;

          bool isPredefinedSpiFreqs = false;

          if (std::string(buff) == "true") isPredefinedSpiFreqs = true;

          ret = bridge_p->getSpiParam(CyBridgeSpiGetParams::SpiFreqListHz, buff, BufferSize);
          if (ret < 0) {
            std::cout << "\t Error of retrieving 'CyBridgeSpiGetParams::SpiFreqListHz' parameter. Err = "
                      << bridge_p->getErrorDescription(ret) << std::endl;
            break;
          }
          if (isPredefinedSpiFreqs) {
            std::cout << "\n    SPI Master: Supported frequencies, Hz = " << buff << std::endl;
          } else {
            std::stringstream freqs(buff);
            int number;
            std::vector<int> freqs_num;
            while (freqs >> number) {
              freqs_num.push_back(number);
              if (freqs.peek() == ',') freqs.ignore();
            }
            std::cout << "\n    SPI Master: Supported frequencies, Hz: Min = " << freqs_num[0] << " Max = " << freqs_num[1] << std::endl;
          }

          uint32_t spi_freq = 120000;

          std::cout << "\n\t Setting SPI Frequency to " << spi_freq << " Hz" << std::endl;

          ret = bridge_p->setSpiParam(CyBridgeSpiSetParams::SpiFreqHz, spi_freq);
          if (ret < 0) {
            std::cout << "\t Error of setting 'CyBridgeSpiSetParams::SpiFreqHz' parameter. Err = " << bridge_p->getErrorDescription(ret)
                      << std::endl;
            break;
          }

          ret = bridge_p->getSpiParam(CyBridgeSpiGetParams::CurrentSpiFreqHz, buff, BufferSize);
          if (ret < 0) {
            std::cout << "\t Error of retrieving 'CyBridgeSpiGetParams::CurrentSpiFreqHz' parameter. Err = "
                      << bridge_p->getErrorDescription(ret) << std::endl;
            break;
          }
          std::cout << "\n\t Current SPI Frequency, Hz = " << buff << std::endl;

          spiLongReadWriteTest(bridge_p);
        } while (0);
      }

      if (strstr(props->ListIfs, CyBridgeIfaceUARTName)) {
        do {
          if (strstr(dev_list[i]->Name, "COM13") == nullptr) break;

          uint32_t baudRate = 115200;
          ret = bridge_p->setUartParam(CyBridgeUartSetParams::BaudRate, baudRate);
          if (ret < 0) {
            std::cout << "\t Error of setting 'CyBridgeUartSetParams::BaudRate' parameter. Err = " << bridge_p->getErrorDescription(ret)
                      << std::endl;
            break;
          }
          std::cout << "\n\t BaudRate set to " << baudRate << std::endl;

          std::vector<uint8_t> uart_write_data{0x30, 0x31, 0x32, 0x20};

          // UART Write
          ret = bridge_p->uartWrite(&uart_write_data[0], static_cast<uint32_t>(uart_write_data.size()));
          if (ret == 0) {
            std::stringstream print_str;
            uint8ToHexStrm(&uart_write_data[0], uart_write_data.size(), print_str);
            std::cout << "\n\t Write data = " << print_str.str() << std::endl;
          } else {
            std::cout << "\n\t Error of UART Write data" << std::endl;
          }

          std::vector<uint8_t> uart_read_data(10);

          // UART Read
          std::cout << "\n\t Waiting for " << uart_read_data.size() << " bytes of Rx data... " << std::endl;
          ret = bridge_p->uartRead(&uart_read_data[0], static_cast<uint32_t>(uart_read_data.size()), 2000);

          if (ret >= 0) {
            std::stringstream print_str;
            uint8ToHexStrm(&uart_read_data[0], uart_read_data.size(), print_str);
            std::cout << "\n\t\t Read data = " << print_str.str() << std::endl;
          } else {
            std::cout << "\n\t\t Error of UART Read:" << bridge_p->getErrorDescription(ret) << std::endl;
            if (ret != CyBridgeErrors::CyBridgeOperationIsTimedOut) break;
          }

          // UART Read Async
          std::cout << "\n\t Starting async UART read " << std::endl;
          bridge_p->uartReadAsync(&uartRxCallback, &uart_read_data[0], static_cast<uint32_t>(uart_read_data.size()), nullptr);

          std::this_thread::sleep_for(std::chrono::seconds(5));

          std::cout << "\n\t Stopping async UART read " << std::endl;
          bridge_p->uartReadAsync(nullptr, nullptr, 0, nullptr);

          // UART Read Single Packet

          std::cout << "\n\t\t Waiting for Rx data... " << std::endl;
          ret = bridge_p->uartReadSinglePacket(&uart_read_data[0], static_cast<uint32_t>(uart_read_data.size()), 2000);

          if (ret >= 0) {
            std::stringstream print_str;
            uint8ToHexStrm(&uart_read_data[0], static_cast<size_t>(ret), print_str);
            std::cout << "\n\t\t\t Read data = " << print_str.str() << std::endl;
          } else {
            std::cout << "\n\t\t Error of UART Read:" << bridge_p->getErrorDescription(ret) << std::endl;
            if (ret != CyBridgeErrors::CyBridgeOperationIsTimedOut) break;
          }
        } while (0);
      }

      bridge_p->releaseDeviceProps(props);

      // the API call is not mandatory as opening of next device closes opened device
      bridge_p->closeDevice();
    }
  }

  bridge_p->releaseDeviceList(dev_list);

  std::this_thread::sleep_for(std::chrono::seconds(5));

  delete bridge_p;

  return 0;
}

//
// spiLongReadWriteTest() -
//
// Arguments:
//		bridge_p - the pointer to cyBridge used for communication with device
//
// Shows how to use long SPI transactions
//
void spiLongReadWriteTest(CyBridge* bridge_p) {
  do {
    uint8_t num_writes = 2;
    const uint8_t offset = 10;
    const uint8_t num_bytes_to_write = 240;
    std::cout << "\n\t Perform " << (int)num_writes << " SPI Writes, " << (int)num_bytes_to_write << " bytes each" << std::endl;

    std::vector<uint8_t> write_buff(num_bytes_to_write + offset);
    std::vector<uint8_t> response_buff(num_bytes_to_write + offset);

    for (int j = 0; j < num_writes; j++) {
      if (j == 0) {
        for (size_t index = 0; index < num_bytes_to_write; index++) {
          write_buff[offset + index] = (uint8_t)index;
        }
      } else {
        for (size_t index = 0; index < num_bytes_to_write; index++) {
          write_buff[offset + index] = (uint8_t)(num_bytes_to_write - index - 1);
        }
      }

      int ret = bridge_p->spiDataTransfer(CyBridgeSPIMode::AassertSSOnStart | CyBridgeSPIMode::DeassertSSOnStop, &write_buff[0],
                                          &response_buff[0], num_bytes_to_write + offset);
      if (ret < 0) {
        std::cout << "\t Error of spiDataTransfer. Err = " << ret << std::endl;
        break;
      }

      std::stringstream print_str;
      uint8ToHexStrm(&write_buff[offset], num_bytes_to_write, print_str);
      std::cout << "\n\t\t Write data = " << print_str.str() << std::endl;

      ret = bridge_p->spiDataTransfer(CyBridgeSPIMode::AassertSSOnStart | CyBridgeSPIMode::DeassertSSOnStop, &write_buff[0],
                                      &response_buff[0], num_bytes_to_write + offset);
      if (ret < 0) {
        std::cout << "\t Error of spiDataTransfer. Err = " << ret << std::endl;
        break;
      }

      uint8ToHexStrm(&response_buff[offset], num_bytes_to_write, print_str);
      std::cout << "\n\t\t Read data = " << print_str.str() << std::endl;

      if (std::equal(write_buff.begin() + offset, write_buff.begin() + offset + num_bytes_to_write, response_buff.begin() + offset)) {
        std::cout << "\n\t\t SPI Read/Write Status PASSED. Read and Write data are the same" << std::endl;
      } else {
        std::cout << "\n\t\t SPI Read/Write Status FAILED. Read and Write data are not the same" << std::endl;
      }
    }
  } while (0);
}

//
// i2cShortReadTest() -
//
// Arguments:
//		bridge_p - the pointer to cyBridge used for communication with device
//
// Shows how to use short I2C transactions
//
void i2cShortReadTest(CyBridge* bridge_p) {
  uint8_t num_bytes_to_read = 5;
  uint8_t num_reads = 5;

  std::cout << "\n\t Perform " << (int)num_reads << " I2C Reads, " << (int)num_bytes_to_read << " bytes each, addr = " << (int)I2CAddr
            << std::endl;

  uint32_t read_buff_size = num_bytes_to_read + 1;
  std::vector<uint8_t> read_buff(read_buff_size);

  for (int j = 0; j < num_reads; j++) {
    int ret = bridge_p->i2cDataTransfer(I2CAddr, CyBridgeI2CMode::Read | CyBridgeI2CMode::GenStart | CyBridgeI2CMode::GenStop,
                                        num_bytes_to_read, nullptr, 0, &read_buff[0], read_buff_size);
    if (ret < 0) {
      std::cout << "\t Error of i2cDataTransfer. Err = " << bridge_p->getErrorDescription(ret) << std::endl;
      break;
    }

    std::stringstream read_str;
    uint8ToHexStrm(&read_buff[1], read_buff.size(), read_str);
    std::cout << " \t\t Addr " << (read_buff[0] ? "ACK" : "NACK") << ", Read data = " << read_str.str() << std::endl;
  }
}

//
// i2cLedWriteTest() -
//
// Arguments:
//		bridge_p - the pointer to cyBridge used for communication with device
//
// Shows how to use I2C to control LEDs on the board
//
void i2cLedWriteTest(CyBridge* bridge_p) {
  uint8_t num_writes = 4;
  uint8_t num_bytes_to_write = 1;
  std::cout << "\n\t Perform " << (int)num_writes << " I2C Writes, " << (int)num_bytes_to_write << " bytes each, addr = " << (int)I2CAddr
            << std::endl;
  std::cout << "\n\t Different LEDs should be Turn on/off" << std::endl;

  std::vector<uint8_t> write_buff(num_bytes_to_write);
  uint32_t response_buff_size = num_bytes_to_write + 1;
  std::vector<uint8_t> response_buff(response_buff_size);
  write_buff[0] = 0;
  for (int j = 0; j < num_writes; j++) {
    int ret = bridge_p->i2cDataTransfer(I2CAddr, CyBridgeI2CMode::Write | CyBridgeI2CMode::GenStart | CyBridgeI2CMode::GenStop, 0,
                                        &write_buff[0], num_bytes_to_write, &response_buff[0], response_buff_size);
    if (ret < 0) {
      std::cout << "\t Error of i2cDataTransfer. Err = " << bridge_p->getErrorDescription(ret) << std::endl;
      break;
    }

    std::stringstream print_str;
    for (size_t index = 0; index < write_buff.size(); index++) {
      print_str << std::hex << std::setfill('0') << std::setw(2) << std::uppercase << (int)write_buff[index]
                << (response_buff[index + 1] ? "+" : "-") << " ";
    }

    std::cout << " \t\t Addr " << (response_buff[0] ? "ACK" : "NACK") << ", Write data = " << print_str.str() << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));

    write_buff[0]++;
  }
}

//
// i2cLongReadWriteTest() -
//
// Arguments:
//		bridge_p - the pointer to cyBridge used for communication with device
//
// Shows how to use long I2C transactions
//
void i2cLongReadWriteTest(CyBridge* bridge_p) {
  do {
    uint8_t num_writes = 2;
    const uint8_t offset = 10;
    const uint8_t num_bytes_to_write = 240;
    std::cout << "\n\t Perform " << (int)num_writes << " I2C Writes, " << (int)num_bytes_to_write << " bytes each, addr = " << (int)I2CAddr
              << std::endl;

    std::vector<uint8_t> write_buff(num_bytes_to_write + offset);
    uint32_t response_buff_size = num_bytes_to_write + offset + 1;
    std::vector<uint8_t> response_buff(response_buff_size);

    for (int j = 0; j < num_writes; j++) {
      if (j == 0) {
        for (size_t index = 0; index < num_bytes_to_write; index++) {
          write_buff[offset + index] = (uint8_t)index;
        }
      } else {
        for (size_t index = 0; index < num_bytes_to_write; index++) {
          write_buff[offset + index] = (uint8_t)(num_bytes_to_write - index - 1);
        }
      }

      int ret = bridge_p->i2cDataTransfer(I2CAddr, CyBridgeI2CMode::Write | CyBridgeI2CMode::GenStart | CyBridgeI2CMode::GenStop, 0,
                                          &write_buff[0], num_bytes_to_write + offset, &response_buff[0], response_buff_size);
      if (ret < 0) {
        std::cout << "\t Error of i2cDataTransfer. Err = " << bridge_p->getErrorDescription(ret) << std::endl;
        break;
      }

      std::stringstream print_str;
      for (size_t index = 0; index < num_bytes_to_write; index++) {
        print_str << std::hex << std::setfill('0') << std::setw(2) << std::uppercase << (int)write_buff[index + offset]
                  << (response_buff[index + offset + 1] ? "+" : "-") << " ";
      }
      std::cout << "\n\t\t Addr " << (response_buff[0] ? "ACK" : "NACK") << ", Write data = " << print_str.str() << std::endl;

      ret = bridge_p->i2cDataTransfer(I2CAddr, CyBridgeI2CMode::Read | CyBridgeI2CMode::GenStart | CyBridgeI2CMode::GenStop,
                                      num_bytes_to_write + offset, nullptr, 0, &response_buff[0], response_buff_size);
      if (ret < 0) {
        std::cout << "\t Error of i2cDataTransfer. Err = " << bridge_p->getErrorDescription(ret) << std::endl;
        break;
      }

      uint8ToHexStrm(&response_buff[offset] + 1, num_bytes_to_write, print_str);
      std::cout << "\n\t\t Addr " << (response_buff[0] ? "ACK" : "NACK") << ", Read data = " << print_str.str() << std::endl;

      if (std::equal(write_buff.begin() + offset, write_buff.begin() + offset + num_bytes_to_write, response_buff.begin() + offset + 1)) {
        std::cout << "\n\t\t I2C Read/Write Status PASSED. Read and Write data are the same" << std::endl;
      } else {
        std::cout << "\n\t\t I2C Read/Write Status FAILED. Read and Write data are not the same" << std::endl;
      }
    }
  } while (0);
}

//
// setMeasureVoltage() -
//
// Arguments:
//		bridge_p - the pointer to cyBridge used for communication with device
//      voltage - the volatge to be set
//
// Shows how to set and measure voltage
//
void setMeasureVoltage(CyBridge* bridge_p, uint32_t voltage) {
  std::cout << "\t Set voltage = " << voltage << " mV" << std::endl;
  int ret = bridge_p->setVoltage(voltage);
  if (ret < 0) {
    std::cout << "\t Cannot set voltage " << voltage << " mV. Err = " << bridge_p->getErrorDescription(ret) << std::endl;
    return;
  }

  int32_t measured_voltage;
  bridge_p->getVoltage(&measured_voltage);
  std::cout << "\t\t Measured voltage = " << measured_voltage << " mV";

  char buff[BufferSize];

  ret = bridge_p->getPowerControlParam(CyBridgePowerControlGetParams::LastSetVoltMv, buff, BufferSize);
  if (ret < 0) {
    std::cout << "\t Error of retrieving 'cy_bridge_power_control_get_params::LastSetVoltMv' parameter. Err = "
              << bridge_p->getErrorDescription(ret) << std::endl;
    return;
  }
  std::cout << "\t Set voltage = " << buff << " mV" << std::endl;
}

//
// powerOnOffTest() -
//
// Arguments:
//		bridge_p - the pointer to cyBridge used for communication with device
//
// Shows how to perform power on/off
//
void powerOnOffTest(CyBridge* bridge_p) {
  int ret = bridge_p->powerOn(false);
  if (ret < 0) {
    std::cout << "\t Cannot toggle power Off. Err = " << bridge_p->getErrorDescription(ret) << std::endl;
    return;
  }

  std::this_thread::sleep_for(std::chrono::seconds(1));

  int32_t measured_voltage;
  bridge_p->getVoltage(&measured_voltage);
  std::cout << "\n\t Power Off. Measured voltage = " << measured_voltage << " mV";

  ret = bridge_p->powerOn(true);
  if (ret < 0) {
    std::cout << "\t Cannot toggle power On. Err = " << bridge_p->getErrorDescription(ret) << std::endl;
    return;
  }

  std::this_thread::sleep_for(std::chrono::seconds(1));

  bridge_p->getVoltage(&measured_voltage);
  std::cout << "\t Power On. Measured voltage = " << measured_voltage << " mV";
}

//
// connectEvent() -
//
// Arguments:
//		dev_p - the pointer to CyBridgeDevice structure with description of connected device
//      is_connected - true if device is connected, false - if disconnected
//      client_p - the user supplied data to setConnectCallback API
//
// User's registered callback for device Connect/Disconnect events
//
void connectEvent(CyBridgeDevice* dev_p, bool is_connected, void* /*client_p*/) {
  if (is_connected) {
    std::cout << "\nConnected ";
  } else {
    std::cout << "\nDisconnected ";
  }

  std::cout << "device: " << dev_p->Name << std::endl;
}

//
// logEvent() -
//
// Arguments:
//		msg_p - the received log message
//      level - the log level of the received message
//      client_p - the user supplied data to setLogCallback API
//
// User's registered callback for displaying CyBridge logs
//
void logEvent(const char* msg_p, CyBridgeLogLevel /*level*/, void* /*client_p*/) { std::cout << "Log: " << msg_p; }

//
// uartRxCallback() -
//
// Arguments:
//		data_p - the pointer to received  data
//      num_bytes - number of received bytes in data_p buffer
//      client_p - the user supplied data to setUartRxCallback API
//
// User's registered callback for displaying UART RX logs logs
//
void uartRxCallback(uint8_t const* data_p, uint32_t num_bytes, void* /*client_p*/) {
  std::stringstream print_str;
  uint8ToHexStrm(data_p, num_bytes, print_str);
  std::cout << "\n\t\t Uart Read data = " << print_str.str() << std::endl;
}

//
// uint8ToHexStrm() -
//
// Arguments:
//		data_p - the pointer to data to convert
//      num_bytes_to_convert - number of bytes to convert
//      out_strm - the stringstream buffer to contain result of conversion
//
// Converts uint8_t buffer to stringstream buffer
//
void uint8ToHexStrm(const uint8_t* data_p, size_t num_bytes_to_convert, std::stringstream& out_strm) {
  out_strm.str("");
  for (uint32_t index = 0; index < num_bytes_to_convert; index++) {
    out_strm << std::hex << std::setfill('0') << std::setw(2) << std::uppercase << static_cast<int>(data_p[index]) << " ";
  }
}
