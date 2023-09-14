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

#ifndef __CYBTLDR_PARSE_H__
#define __CYBTLDR_PARSE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "cybtldr_utils.h"

/* Maximum number of bytes to allocate for a single row.  */
/* NB: Rows should have a max of 592 chars (2-arrayID, 4-rowNum, 4-len, 576-data (512-flash, 64-ecc), 2-checksum, 4-newline) */
#define MAX_BUFFER_SIZE 768

/*******************************************************************************
 * Function Name: CyBtldr_FromHex
 ********************************************************************************
 * Summary:
 *   Converts the provided ASCII char into its hexadecimal numerical equivalent.
 *
 * Parameters:
 *   value - the ASCII char to convert into a number
 *
 * Returns:
 *   The hexadecimal numerical equivalent of the provided ASCII char.  If the
 *   provided char is not a valid ASCII char, it will return 0.
 *
 *******************************************************************************/
uint8_t CyBtldr_FromHex(char value);

/*******************************************************************************
 * Function Name: CyBtldr_FromAscii
 ********************************************************************************
 * Summary:
 *   Converts the provided ASCII array into its hexadecimal numerical equivalent.
 *
 * Parameters:
 *   bufSize - The length of the buffer to convert
 *   buffer  - The buffer of ASCII characters to convert
 *   rowSize - The number of bytes of equivalent hex data generated
 *   rowData - The hex data generated for the buffer
 *
 * Returns:
 *   CYRET_SUCCESS    - The buffer was converted successfully
 *   CYRET_ERR_LENGTH - The buffer does not have an even number of chars
 *
 *******************************************************************************/
int CyBtldr_FromAscii(uint32_t bufSize, char* buffer, uint16_t* rowSize, uint8_t* rowData);

/*******************************************************************************
 * Function Name: CyBtldr_ReadLine
 ********************************************************************************
 * Summary:
 *   Reads a single line from the open data file.  This function will remove
 *   any Windows, Linux, or Unix line endings from the data.
 *
 * Parameters:
 *   size - The number of bytes of data read from the line and stored in buffer
 *   file - The preallocated buffer, with MAX_BUFFER_SIZE bytes, to store the
 *          read data in.
 *
 * Returns:
 *   CYRET_SUCCESS  - The file was opened successfully.
 *   CYRET_ERR_FILE - An error occurred opening the provided file.
 *   CYRET_ERR_EOF  - The end of the file has been reached
 *
 *******************************************************************************/
EXTERN int CyBtldr_ReadLine(uint32_t* size, char* buffer);

/*******************************************************************************
 * Function Name: CyBtldr_OpenDataFile
 ********************************************************************************
 * Summary:
 *   Opens the provided file for reading.  Once open, it is expected that the
 *   first call will be to ParseHeader() to read the first line of data.  After
 *   that, successive calls to ParseRowData() are possible to read each line
 *   of data, one at a time, from the file.  Once all data has been read from
 *   the file, a call to CloseDataFile() should be made to release resources.
 *
 * Parameters:
 *   file - The full canonical path to the *.cyacd file to open
 *
 * Returns:
 *   CYRET_SUCCESS  - The file was opened successfully.
 *   CYRET_ERR_FILE - An error occurred opening the provided file.
 *
 *******************************************************************************/
EXTERN int CyBtldr_OpenDataFile(const char* file);

/*******************************************************************************
 * Function Name: CyBtldr_ParseCyacdFileVersion
 ********************************************************************************
 * Summary:
 *   Parses the header information from the *.cyacd file.  The header information
 *   is stored as the first line, so this method should only be called once,
 *   and only immediately after calling OpenDataFile and reading the first line.
 *
 * Parameters:
 *   bufSize    - The number of bytes contained within buffer
 *   buffer     - The buffer containing the header data to parse
 *   version    - The version number of the cyacd file
 * Returns:
 *   CYRET_SUCCESS    - The file was opened successfully.
 *   CYRET_ERR_LENGTH - The line does not contain enough data
 *
 *******************************************************************************/
EXTERN int CyBtldr_CheckCyacdFileVersion(uint32_t bufSize, char* buffer, uint8_t* version);

/*******************************************************************************
 * Function Name: CyBtldr_ParseHeader
 ********************************************************************************
 * Summary:
 *   Parses the header information from the *.cyacd2 file.  The header information
 *   is stored as the first line, so this method should only be called once,
 *   and only immediately after calling OpenDataFile and reading the first line.
 *   This function is only used for applications using the .cyacd2 format.
 *
 * Parameters:
 *   bufSize        - The number of bytes contained within buffer
 *   buffer         - The buffer containing the header data to parse
 *   siliconId      - The silicon ID that the provided *.cyacd file is for
 *   siliconRev     - The silicon Revision that the provided *.cyacd file is for
 *   chksum         - The type of checksum to use for packet integrity check
 *   appID          - The application ID number
 *   product ID     - The product ID number of the cyacd
 *
 * Returns:
 *   CYRET_SUCCESS    - The file was opened successfully.
 *   CYRET_ERR_LENGTH - The line does not contain enough data
 *
 *******************************************************************************/
EXTERN int CyBtldr_ParseHeader(uint32_t bufSize, char* buffer, uint32_t* siliconId, uint8_t* siliconRev, uint8_t* chksum, uint8_t* appID,
                               uint32_t* productID);

/*******************************************************************************
 * Function Name: CyBtldr_ParseCyAcd2RowData
 ********************************************************************************
 * Summary:
 *   Parses the contents of the provided buffer which is expected to contain
 *   the row data from the *.cyacd2 file.  This is expected to be called multiple
 *   times.  Once for each row of the *.cyacd2 file, excluding the header row.
 *   This function is only used for applications using the .cyacd2 format.
 *
 * Parameters:
 *   bufSize     - The number of bytes contained within buffer
 *   buffer      - The buffer containing the row data to parse
 *   address     - The flash address that the data belongs in
 *   rowData     - The preallocated buffer to store the flash row data
 *   size        - The number of bytes of rowData
 *   checksum    - The sum-checksum value for the entire row's data rowData
 *
 * Returns:
 *   CYRET_SUCCESS    - The file was opened successfully.
 *   CYRET_ERR_LENGTH - The line does not contain enough data
 *   CYRET_ERR_DATA   - The line does not contain a full row of data
 *   CYRET_ERR_CMD    - The line does not start with the cmd identifier ':'
 *
 *******************************************************************************/
EXTERN int CyBtldr_ParseCyAcd2RowData(uint32_t bufSize, char* buffer, uint32_t* address, uint8_t* rowData, uint16_t* size,
                                      uint8_t* checksum);

/*******************************************************************************
 * Function Name: CyBtldr_ParseCyAcdAppStartAndSize
 ********************************************************************************
 * Summary:
 *   Parse the cyacd2 application's start address and size.
 *   This function need to be called after parsing the header and EIV row and before any data row.
 *   When this function returns, the file point will be set to the row after header row.
 *
 * Parameters:
 *   appStart     - The application start address
 *   appSize      - The number of bytes in the application.
 *   dataLines    - The number of data lines in the application's file.
 *   buf          - The buffer containing the data to parse.
 *
 * Returns:
 *   CYRET_SUCCESS    - The file was opened successfully.
 *   CYRET_ERR_FILE   - The file cannot be read successfully.
 *
 *******************************************************************************/
EXTERN int CyBtldr_ParseCyAcdAppStartAndSize(uint32_t* appStart, uint32_t* appSize, uint32_t* dataLines, char* buf);

/*******************************************************************************
 * Function Name: CyBtldr_CloseDataFile
 ********************************************************************************
 * Summary:
 *   Closes the data file pointer.
 *
 * Parameters:
 *   void.
 *
 * Returns:
 *   CYRET_SUCCESS  - The file was opened successfully.
 *   CYRET_ERR_FILE - An error occurred opening the provided file.
 *
 *******************************************************************************/
EXTERN int CyBtldr_CloseDataFile(void);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif
