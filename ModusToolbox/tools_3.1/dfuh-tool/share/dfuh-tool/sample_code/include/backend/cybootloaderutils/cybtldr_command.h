/*
 * Copyright 2011-2023 Cypress Semiconductor Corporation (an Infineon company)
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

#ifndef __CYBTLDR_COMMAND_H__
#define __CYBTLDR_COMMAND_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "cybtldr_utils.h"

/* Maximum number of bytes to allocate for a single command.  */
#define MAX_COMMAND_SIZE 512

// STANDARD PACKET FORMAT:
// Multi byte entries are encoded in LittleEndian.
/*******************************************************************************
 * [1-byte] [1-byte ] [2-byte] [n-byte] [ 2-byte ] [1-byte]
 * [ SOP  ] [Command] [ Size ] [ Data ] [Checksum] [ EOP  ]
 *******************************************************************************/

/* The first byte of any boot loader command. */
#define CMD_START 0x01
/* The last byte of any boot loader command. */
#define CMD_STOP 0x17
/* The minimum number of bytes in a bootloader command. */
#define BASE_CMD_SIZE 0x07

/* Command identifier for verifying the checksum value of the bootloadable project. */
#define CMD_VERIFY_CHECKSUM 0x31
/* Command identifier for erasing a row of flash data from the target device. */
#define CMD_ERASE_ROW 0x34
/* Command identifier for making sure the bootloader host and bootloader are in sync. */
#define CMD_SYNC 0x35
/* Command identifier for sending a block of data to the bootloader without doing anything with it yet. */
#define CMD_SEND_DATA 0x37
/* Command identifier for sending a block of data to the bootloader without sending the response back. */
#define CMD_SEND_DATA_NO_RSP 0x47
/* Command identifier for starting the boot loader.  All other commands ignored until this is sent. */
#define CMD_ENTER_BOOTLOADER 0x38
/* Command identifier for exiting the bootloader and restarting the target program. */
#define CMD_EXIT_BOOTLOADER 0x3B
/* Command to erase data */
#define CMD_ERASE_DATA 0x44
/* Command to program data. */
#define CMD_PROGRAM_DATA 0x49
/* Command to verify data */
#define CMD_VERIFY_DATA 0x4A
/* Command to set application metadata in bootloader SDK */
#define CMD_SET_METADATA 0x4C
/* Command to set encryption initial vector */
#define CMD_SET_EIV 0x4D

/* Data source is .cyacd2 file */
#define CYACD2_DATA 0
/* Data source is hex file defined in .json file */
#define HEX_DATA 1
/* Data source is string of bytes from .json file */
#define STRING_DATA 2
/* There is no payload data specified in .json file */
#define NO_DATA 3
/*
 * This enum defines the different types of checksums that can be
 * used by the bootloader for ensuring data integrity.
 */
typedef enum {
    /* Checksum type is a basic inverted summation of all bytes */
    SUM_CHECKSUM = 0x00,
    /* 16-bit CRC checksum using the CCITT implementation */
    CRC_CHECKSUM = 0x01,
} CyBtldr_ChecksumType;

EXTERN void fillData32(uint8_t* buf, uint32_t data);

/*******************************************************************************
 * Function Name: CyBtldr_ComputeChecksum16bit
 ********************************************************************************
 * Summary:
 *   Computes the 2 byte checksum for the provided command data.  The checksum is
 *   either 2's complement or CRC16.
 *
 * Parameters:
 *   buf  - The data to compute the checksum on
 *   size - The number of bytes contained in buf.
 *
 * Returns:
 *   The checksum for the provided data.
 *
 *******************************************************************************/
uint16_t CyBtldr_ComputeChecksum16bit(uint8_t* buf, uint32_t size);

/*******************************************************************************
 * Function Name: CyBtldr_ComputeChecksum32bit
 ********************************************************************************
 * Summary:
 *   Computes the 4 byte checksum for the provided command data.  The checksum is
 *   computed using CRC32-C
 *
 * Parameters:
 *   buf  - The data to compute the checksum on
 *   size - The number of bytes contained in buf.
 *
 * Returns:
 *   The checksum for the provided data.
 *
 *******************************************************************************/
uint32_t CyBtldr_ComputeChecksum32bit(uint8_t* buf, uint32_t size);

/*******************************************************************************
 * Function Name: CyBtldr_SetCheckSumType
 ********************************************************************************
 * Summary:
 *   Updates what checksum algorithm is used when generating packets
 *
 * Parameters:
 *   chksumType - The type of checksum to use when creating packets
 *
 * Returns:
 *   NA
 *
 *******************************************************************************/
void CyBtldr_SetCheckSumType(CyBtldr_ChecksumType chksumType);

/*******************************************************************************
 * Function Name: CyBtldr_ParseDefaultCmdResult
 ********************************************************************************
 * Summary:
 *   Parses the output from any command that returns the default result packet
 *   data.  The default result is just a status byte
 *
 * Response Size: 7
 *
 * Parameters:
 *   cmdBuf  - The preallocated buffer to store command data in.
 *   cmdSize - The number of bytes in the command.
 *   status  - The status code returned by the bootloader.
 *
 * Returns:
 *   CYRET_SUCCESS    - The command was constructed successfully
 *   CYRET_ERR_LENGTH - The packet does not contain enough data
 *   CYRET_ERR_DATA   - The packet's contents are not correct
 *
 *******************************************************************************/
EXTERN int CyBtldr_ParseDefaultCmdResult(uint8_t* cmdBuf, uint32_t cmdSize, uint8_t* status);

/*******************************************************************************
 * Function Name: CyBtldr_ParseDefaultCmdResult
 ********************************************************************************
 * Summary:
 *   Parses the output from custom command that returns the response packet
 *   of any length.
 *
 * Parameters:
 *   cmdBuf     - The preallocated buffer to store command data in.
 *   dataSize   - The number of bytes which represent real data.
 *   cmdSize    - The number of all bytes in the command.
 *   status     - The status code returned by the bootloader.
 *
 * Returns:
 *   CYRET_SUCCESS    - The command was constructed successfully
 *   CYRET_ERR_LENGTH - The packet does not contain enough data
 *   CYRET_ERR_DATA   - The packet's contents are not correct
 *
 *******************************************************************************/
EXTERN int CyBtldr_ParseCustomCmdResult(uint8_t* cmdBuf, uint32_t dataSize, uint32_t cmdSize, uint8_t* status);

/*******************************************************************************
 * Function Name: CyBtldr_CreateEnterBootLoaderCmd
 ********************************************************************************
 * Summary:
 *   Creates the command used to startup the bootloader. This function is only
 *   used for applications using the .cyacd2 format.
 *   NB: This command must be sent before the bootloader will respond to any
 *       other command.
 *
 * Command Size: 13
 *
 * Parameters:
 *   protect         - The flash protection settings.
 *   cmdBuf          - The preallocated buffer to store command data in.
 *   cmdSize         - The number of bytes in the command.
 *   resSize         - The number of bytes expected in the bootloader's response packet.
 *   productID       - The product ID of the cyacd
 *
 * Returns:
 *   CYRET_SUCCESS  - The command was constructed successfully
 *
 *******************************************************************************/
EXTERN int CyBtldr_CreateEnterBootLoaderCmd(uint8_t* cmdBuf, uint32_t* cmdSize, uint32_t* resSize, uint32_t productID);

/*******************************************************************************
 * Function Name: CyBtldr_ParseEnterBootLoaderCmdResult
 ********************************************************************************
 * Summary:
 *   Parses the output from the EnterBootLoader command to get the resultant
 *   data.
 *
 * Response Size: 15
 *
 * Parameters:
 *   cmdBuf     - The buffer containing the output from the bootloader.
 *   cmdSize    - The number of bytes in cmdBuf.
 *   siliconId  - The silicon ID of the device being communicated with.
 *   siliconRev - The silicon Revision of the device being communicated with.
 *   blVersion  - The bootloader version being communicated with.
 *   status     - The status code returned by the bootloader.
 *
 * Returns:
 *   CYRET_SUCCESS    - The command was constructed successfully
 *   CYRET_ERR_LENGTH - The packet does not contain enough data
 *   CYRET_ERR_DATA   - The packet's contents are not correct
 *
 *******************************************************************************/
EXTERN int CyBtldr_ParseEnterBootLoaderCmdResult(uint8_t* cmdBuf, uint32_t cmdSize, uint32_t* siliconId, uint8_t* siliconRev,
                                                 uint32_t* blVersion, uint8_t* status);

/*******************************************************************************
 * Function Name: CyBtldr_CreateExitBootLoaderCmd
 ********************************************************************************
 * Summary:
 *   Creates the command used to stop communicating with the boot loader and to
 *   trigger the target device to restart, running the new bootloadable
 *   application.
 *
 * Command Size: 7
 *
 * Parameters:
 *   cmdBuf    - The preallocated buffer to store command data in.
 *   cmdSize   - The number of bytes in the command.
 *   resSize   - The number of bytes expected in the bootloader's response packet.
 *
 * Returns:
 *   CYRET_SUCCESS  - The command was constructed successfully
 *
 *******************************************************************************/
EXTERN int CyBtldr_CreateExitBootLoaderCmd(uint8_t* cmdBuf, uint32_t* cmdSize, uint32_t* resSize);

/*******************************************************************************
 * Function Name: CyBtldr_ParseVerifyChecksumCmdResult
 ********************************************************************************
 * Summary:
 *   Parses the output from the VerifyChecksum command to get the resultant
 *   data.
 *
 * Response Size: 8
 *
 * Parameters:
 *   cmdBuf           - The preallocated buffer to store command data in.
 *   cmdSize          - The number of bytes in the command.
 *   checksumValid    - Whether or not the full checksums match (1 = valid, 0 = invalid)
 *   status           - The status code returned by the bootloader.
 *
 * Returns:
 *   CYRET_SUCCESS    - The command was constructed successfully
 *   CYRET_ERR_LENGTH - The packet does not contain enough data
 *   CYRET_ERR_DATA   - The packet's contents are not correct
 *
 *******************************************************************************/
EXTERN int CyBtldr_ParseVerifyChecksumCmdResult(uint8_t* cmdBuf, uint32_t cmdSize, uint8_t* checksumValid, uint8_t* status);

/*******************************************************************************
 * Function Name: CyBtldr_CreateSendDataCmd
 ********************************************************************************
 * Summary:
 *   Creates the command used to send a block of data to the target.
 *
 * Command Size: greater than 7
 *
 * Parameters:
 *   buf     - The buffer of data to program into the flash row.
 *   size    - The number of bytes in data for the row.
 *   cmdBuf  - The preallocated buffer to store command data in.
 *   cmdSize - The number of bytes in the command.
 *   resSize - The number of bytes expected in the bootloader's response packet.
 *
 * Returns:
 *   CYRET_SUCCESS  - The command was constructed successfully
 *
 *******************************************************************************/
EXTERN int CyBtldr_CreateSendDataCmd(uint8_t* buf, uint16_t size, uint8_t* cmdBuf, uint32_t* cmdSize, uint32_t* resSize);

/*******************************************************************************
 * Function Name: CyBtldr_ParseSendDataCmdResult
 ********************************************************************************
 * Summary:
 *   Parses the output from the SendData command to get the resultant
 *   data.
 *
 * Response Size: 7
 *
 * Parameters:
 *   cmdBuf  - The preallocated buffer to store command data in.
 *   cmdSize - The number of bytes in the command.
 *   status  - The status code returned by the bootloader.
 *
 * Returns:
 *   CYRET_SUCCESS    - The command was constructed successfully
 *   CYRET_ERR_LENGTH - The packet does not contain enough data
 *   CYRET_ERR_DATA   - The packet's contents are not correct
 *
 *******************************************************************************/
EXTERN int CyBtldr_ParseSendDataCmdResult(uint8_t* cmdBuf, uint32_t cmdSize, uint8_t* status);

/*******************************************************************************
 * Function Name: CyBtldr_CreateProgramDataCmd
 ********************************************************************************
 * Summary:
 *   Creates the command used to program data.
 *
 * Command Size: At least 15
 *
 * Parameters:
 *   address - The address to program.
 *   chksum  - The checksum all the data being programmed by this command the preceding send data commands.
 *   buf     - The buffer of data to program into the flash row.
 *   size    - The number of bytes in data for the row.
 *   cmdBuf  - The preallocated buffer to store command data in.
 *   cmdSize - The number of bytes in the command.
 *   resSize - The number of bytes expected in the bootloader's response packet.
 *
 * Returns:
 *   CYRET_SUCCESS  - The command was constructed successfully
 *
 *******************************************************************************/
EXTERN int CyBtldr_CreateProgramDataCmd(uint32_t address, uint32_t chksum, uint8_t* buf, uint16_t size, uint8_t* cmdBuf, uint32_t* cmdSize,
                                        uint32_t* resSize);

/*******************************************************************************
 * Function Name: CyBtldr_CreateVerifyDataCmd
 ********************************************************************************
 * Summary:
 *   Creates the command used to verify data.
 *
 * Command Size: At least 15
 *
 * Parameters:
 *   address - The address to verify.
 *   chksum  - The checksum all the data being verified by this command the preceding send data commands.
 *   buf     - The buffer of data to verify against.
 *   size    - The number of bytes in data for the row.
 *   cmdBuf  - The preallocated buffer to store command data in.
 *   cmdSize - The number of bytes in the command.
 *   resSize - The number of bytes expected in the bootloader's response packet.
 *
 * Returns:
 *   CYRET_SUCCESS  - The command was constructed successfully
 *
 *******************************************************************************/
EXTERN int CyBtldr_CreateVerifyDataCmd(uint32_t address, uint32_t chksum, uint8_t* buf, uint16_t size, uint8_t* cmdBuf, uint32_t* cmdSize,
                                       uint32_t* resSize);

/*******************************************************************************
 * Function Name: CyBtldr_CreateEraseDataCmd
 ********************************************************************************
 * Summary:
 *   Creates the command used to erase data.
 *
 * Command Size: 11
 *
 * Parameters:
 *   address     - The address to erase.
 *   cmdBuf      - The preallocated buffer to store command data in.
 *   cmdSize     - The number of bytes in the command.
 *   resSize     - The number of bytes expected in the bootloader's response packet.
 *
 * Returns:
 *   CYRET_SUCCESS  - The command was constructed successfully
 *
 *******************************************************************************/
EXTERN int CyBtldr_CreateEraseDataCmd(uint32_t address, uint8_t* cmdBuf, uint32_t* cmdSize, uint32_t* resSize);

/*******************************************************************************
 * Function Name: CyBtldr_CreateCustomDataCmd
 ********************************************************************************
 * Summary:
 *   Creates the command used to send custom command.
 *
 * Command Size: At least 15
 *
 * Parameters:
 *   buf     - The buffer of data to send into the flash row.
 *   size    - The number of bytes in data for the row.
 *   cmdBuf  - The preallocated buffer to store command data in.
 *   cmdSize - The number of bytes in the command.
 *   resSize - The number of bytes expected in the bootloader's response packet.
 *
 * Returns:
 *   CYRET_SUCCESS  - The command was constructed successfully
 *
 *******************************************************************************/
EXTERN int CyBtldr_CreateCustomDataCmd(uint8_t* buf, uint16_t size, uint8_t* cmdBuf, uint32_t* cmdSize, uint32_t* resSize, uint8_t cmdCode);

/*******************************************************************************
 * Function Name: CyBtldr_CreateVerifyChecksumCmd
 ********************************************************************************
 * Summary:
 *   Creates the command used to verify application. This function is only used
 *   for applications using the .cyacd2 format.
 *
 * Command Size: 8
 *
 * Parameters:
 *   appId   - The application number.
 *   cmdBuf  - The preallocated buffer to store command data in.
 *   cmdSize - The number of bytes in the command.
 *   resSize - The number of bytes expected in the bootloader's response packet.
 *
 * Returns:
 *   CYRET_SUCCESS  - The command was constructed successfully
 *
 *******************************************************************************/
EXTERN int CyBtldr_CreateVerifyChecksumCmd(uint8_t appId, uint8_t* cmdBuf, uint32_t* cmdSize, uint32_t* resSize);

/*******************************************************************************
 * Function Name: CyBtldr_CreateSetApplicationMetadataCmd
 ********************************************************************************
 * Summary:
 *   Set the bootloader SDK's metadata field for a specific application ID. This
 *   function is only used for applications using the .cyacd2 format.
 *
 * Command Size: 16
 *
 * Parameters:
 *   appID       - The ID number of the application.
 *   buf         - The buffer containing the application metadata (8 bytes).
 *   cmdBuf      - The preallocated buffer to store the command data in.
 *   cmdSize     - The number of bytes in the command.
 *   resSize     - The number of bytes expected in the bootloader's response packet.
 *
 * Returns:
 *   CYRET_SUCCESS -The command was constructed successfully
 *******************************************************************************/
EXTERN int CyBtldr_CreateSetApplicationMetadataCmd(uint8_t appID, uint8_t* buf, uint8_t* cmdBuf, uint32_t* cmdSize, uint32_t* resSize);

/*******************************************************************************
 * Function Name: CyBtldr_CreateSetEncryptionInitialVectorCmd
 ********************************************************************************
 * Summary:
 *   Set the bootloader SDK's encryption initial vector (EIV). This function is
 *   only used for applications using the .cyacd2 format.
 *
 * Command Size: 7 or 15 or 23
 *
 * Parameters:
 *   buf         - The buffer containing the EIV.
 *   size        - The number bytes of the EIV. (Should be 0 or 8 or 16)
 *   cmdBuf      - The preallocated buffer to store the command data in.
 *   cmdSize     - The number of bytes in the command.
 *   resSize     - The number of bytes expected in the bootloader's response packet.
 *
 * Returns:
 *   CYRET_SUCCESS -The command was constructed successfully
 *******************************************************************************/
EXTERN int CyBtldr_CreateSetEncryptionInitialVectorCmd(uint8_t* buf, uint16_t size, uint8_t* cmdBuf, uint32_t* cmdSize, uint32_t* resSize);

/*******************************************************************************
 * Function Name: CyBtldr_TryParseParketStatus
 ********************************************************************************
 * Summary:
 *   Parses the output packet data
 *
 * Parameters:
 *   packet      - The preallocated buffer to store command data in.
 *   packetSize  - The number of bytes in the command.
 *   status      - The status code returned by the bootloader.
 *
 * Returns:
 *   CYRET_SUCCESS           - The packet is a valid packet
 *   CYBTLDR_STAT_ERR_UNK    - The packet is not a valid packet
 *
 *******************************************************************************/
EXTERN int CyBtldr_TryParseParketStatus(uint8_t* packet, int packetSize, uint8_t* status);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif
