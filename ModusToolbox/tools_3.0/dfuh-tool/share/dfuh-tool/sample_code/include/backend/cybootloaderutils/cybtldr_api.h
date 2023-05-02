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

#ifndef __CYBTLDR_API_H__
#define __CYBTLDR_API_H__

#ifdef __cplusplus
extern "C" {
#endif

/* NOLINTNEXTLINE(modernize-deprecated-headers) */
#include <stdint.h>

#include "cybtldr_utils.h"

/**
 * This struct defines all of the items necessary for the bootloader
 * host to communicate over an arbitrary communication protocol. The
 * caller must provide implementations of these items to use their
 * desired communication protocol.
 */
typedef struct {
    /** Function used to open the communications connection */
    int (*OpenConnection)(void);
    /** Function used to close the communications connection */
    int (*CloseConnection)(void);
    /** Function used to read data over the communications connection */
    int (*ReadData)(uint8_t*, int);
    /** Function used to write data over the communications connection */
    int (*WriteData)(uint8_t*, int);
    /** Value used to specify the maximum number of bytes that can be transfered at a time */
    unsigned int MaxTransferSize;
} CyBtldr_CommunicationsData;

/*******************************************************************************
 * Function Name: CyBtldr_TransferData
 ********************************************************************************
 * Summary:
 *   This function is responsible for transferring a buffer of data to the target
 *   device and then reading a response packet back from the device.
 *
 * Parameters:
 *   inBuf   - The buffer containing data to send to the target device
 *   inSize  - The number of bytes to send to the target device
 *   outBuf  - The buffer to store the data read from the device
 *   outSize - The number of bytes to read from the target device
 *
 * Returns:
 *   CYRET_SUCCESS  - The transfer completed successfully
 *   CYRET_ERR_COMM - There was a communication error talking to the device
 *
 *******************************************************************************/
int CyBtldr_TransferData(uint8_t* inBuf, int inSize, uint8_t* outBuf, int outSize);

/*******************************************************************************
 * Function Name: CyBtldr_StartBootloadOperation
 ********************************************************************************
 * Summary:
 *   Initiates a new bootload operation. This must be called before any other
 *   request to send data to the bootloader. A corresponding call to
 *   CyBtldr_EndBootloadOperation() should be made once all transactions are
 *   complete. This function is only used for applications using the .cyacd2
 *   format.
 *
 * Parameters:
 *   comm              – Communication struct used for communicating with the target device
 *   expSiId           - The Silicon ID of the device we expect to communicate with
 *   expSiRev          - The Silicon Rev of the device we expect to communicate with
 *   blVer             - The Bootloader version that is running on the device
 *   productID         - The product ID of the cyacd
 *
 * Returns:
 *   CYRET_SUCCESS     - The start request was sent successfully
 *   CYRET_ERR_DEVICE  - The detected device does not match the desired device
 *   CYRET_ERR_VERSION - The detected bootloader version is not compatible
 *   CYRET_ERR_BTLDR   - The bootloader experienced an error
 *   CYRET_ERR_COMM    - There was a communication error talking to the device
 *
 *******************************************************************************/
EXTERN int CyBtldr_StartBootloadOperation(CyBtldr_CommunicationsData* comm, uint32_t expSiId, uint8_t expSiRev, uint32_t* blVer,
                                          uint32_t productID);

/*******************************************************************************
 * Function Name: CyBtldr_EndBootloadOperation
 ********************************************************************************
 * Summary:
 *   Terminates the current bootload operation. This should be called once all
 *   bootload commands have been sent and no more communication is desired.
 *
 * Parameters:
 *   void.
 *
 * Returns:
 *   CYRET_SUCCESS   - The end request was sent successfully
 *   CYRET_ERR_BTLDR - The bootloader experienced an error
 *   CYRET_ERR_COMM  - There was a communication error talking to the device
 *
 *******************************************************************************/
EXTERN int CyBtldr_EndBootloadOperation(void);

/******************************************************************************
 * The following section contains API for applications using the .cyacd2 format
 ******************************************************************************/

/*******************************************************************************
 * Function Name: CyBtldr_ProgramRow
 ********************************************************************************
 * Summary:
 *   Sends a single row of data to the bootloader to be programmed into flash
 *   This function is only used for applications using the .cyacd2 format.
 *
 * Parameters:
 *   address – The flash address that is to be reprogrammed
 *   buf     – The buffer of data to program into the devices flash
 *   size    – The number of bytes in data that need to be sent to the bootloader
 *
 * Returns:
 *   CYRET_SUCCESS    - The row was programmed successfully
 *   CYRET_ERR_LENGTH - The result packet does not have enough data
 *   CYRET_ERR_DATA   - The result packet does not contain valid data
 *   CYRET_ERR_ARRAY  - The array is not valid for programming
 *   CYRET_ERR_ROW    - The array/row number is not valid for programming
 *   CYRET_ERR_BTLDR  - The bootloader experienced an error
 *   CYRET_ERR_ACTIVE - The application is currently marked as active
 *
 *******************************************************************************/
EXTERN int CyBtldr_ProgramRow(uint32_t address, uint8_t* buf, uint16_t size);

/*******************************************************************************
 * Function Name: CyBtldr_EraseRow
 ********************************************************************************
 * Summary:
 *   Erases a single row of flash data from the device. This function is only
 *   used for applications using the .cyacd2 format.
 *
 * Parameters:
 *   address - The flash address that is to be erased
 *
 * Returns:
 *   CYRET_SUCCESS    - The row was erased successfully
 *   CYRET_ERR_LENGTH - The result packet does not have enough data
 *   CYRET_ERR_DATA   - The result packet does not contain valid data
 *   CYRET_ERR_ARRAY  - The array is not valid for programming
 *   CYRET_ERR_ROW    - The array/row number is not valid for programming
 *   CYRET_ERR_BTLDR  - The bootloader experienced an error
 *   CYRET_ERR_COMM   - There was a communication error talking to the device
 *   CYRET_ERR_ACTIVE - The application is currently marked as active
 *
 *******************************************************************************/
EXTERN int CyBtldr_EraseRow(uint32_t address);

/*******************************************************************************
 * Function Name: CyBtldr_VerifyRow
 ********************************************************************************
 * Summary:
 *   Verifies that the data contained within the specified flash array and row
 *   matches the expected value. This function is only used for applications
 *   using the .cyacd2 format.
 *
 * Parameters:
 *   address  - The flash address that is to be verified
 *   buf      - The data to be verified in flash
 *   size     - The amount of data to verify
 *
 * Returns:
 *   CYRET_SUCCESS      - The row was verified successfully
 *   CYRET_ERR_LENGTH   - The result packet does not have enough data
 *   CYRET_ERR_DATA     - The result packet does not contain valid data
 *   CYRET_ERR_ARRAY	   - The array is not valid for programming
 *   CYRET_ERR_ROW      - The array/row number is not valid for programming
 *   CYRET_ERR_CHECKSUM - The checksum does not match the expected value
 *   CYRET_ERR_BTLDR    - The bootloader experienced an error
 *   CYRET_ERR_COMM     - There was a communication error talking to the device
 *
 *******************************************************************************/
EXTERN int CyBtldr_VerifyRow(uint32_t address, uint8_t* buf, uint16_t size);

/*******************************************************************************
 * Function Name: CyBtldr_VerifyApplication
 ********************************************************************************
 * Summary:
 *   Verifies that the checksum for the entire bootloadable application matches
 *   the expected value.  This is used to verify that the entire bootloadable
 *   image is valid and ready to execute. This function is only used for
 *   applications using the .cyacd2 format.
 *
 * Parameters:
 *   appId              - The application number
 *
 * Returns:
 *   CYRET_SUCCESS      - The application was verified successfully
 *   CYRET_ERR_LENGTH   - The result packet does not have enough data
 *   CYRET_ERR_DATA     - The result packet does not contain valid data
 *   CYRET_ERR_CHECKSUM - The checksum does not match the expected value
 *   CYRET_ERR_BTLDR    - The bootloader experienced an error
 *   CYRET_ERR_COMM     - There was a communication error talking to the device
 *
 *******************************************************************************/
EXTERN int CyBtldr_VerifyApplication(uint8_t appId);

/*******************************************************************************
 * Function Name: CyBtldr_SetApplicationStartAddress
 ********************************************************************************
 * Summary:
 *   Set the metadata for the giving application ID. This function is only used
 *   for applications using the .cyacd2 format.
 *
 * Parameters:
 *   appId               - Application ID number
 *   appStartAddr        - The Start Address to put into the metadata
 *   appSize             - The number of bytes in the application
 *
 * Returns:
 *   CYRET_SUCCESS      - The application was verified successfully
 *   CYRET_ERR_CMD      - ThE command was invalid
 *   CYRET_ERR_LENGTH   - The command length was incorrect
 *   CYRET_ERR_DATA     - The result packet does not contain valid data
 *   CYRET_ERR_CHECKSUM - The checksum does not match the expected value
 *
 *******************************************************************************/
EXTERN int CyBtldr_SetApplicationMetaData(uint8_t appId, uint32_t appStartAddr, uint32_t appSize);

/*******************************************************************************
 * Function Name: CyBtldr_SetEncryptionInitialVector
 ********************************************************************************
 * Summary:
 *   Set Encryption Initial Vector. This function is only used for applications
 *   using the .cyacd2 format.
 *
 * Parameters:
 *   size               - size of encryption initial vector
 *   buf                - encryption initial vector buffer
 *
 * Returns:
 *   CYRET_SUCCESS      - The application was verified successfully
 *   CYRET_ERR_CMD      - ThE command was invalid
 *   CYRET_ERR_LENGTH   - The command length was incorrect
 *   CYRET_ERR_DATA     - The result packet does not contain valid data
 *   CYRET_ERR_CHECKSUM - The checksum does not match the expected value
 *
 *******************************************************************************/
EXTERN int CyBtldr_SetEncryptionInitialVector(uint16_t size, uint8_t* buf);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif
