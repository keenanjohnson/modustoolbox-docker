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

#include "cybtldr_parse.h"
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

/* Pointer to the *.cyacd file containing the data that is to be read */
static FILE* dataFile;

static uint16_t parse2ByteValueLittleEndian(uint8_t* buf) { return ((uint16_t)buf[0]) | (((uint16_t)buf[1]) << 8); }

static uint32_t parse4ByteValueLittleEndian(uint8_t* buf) {
    return ((uint32_t)parse2ByteValueLittleEndian(buf)) | (((uint32_t)parse2ByteValueLittleEndian(buf + 2)) << 16);
}

uint8_t CyBtldr_FromHex(char value) {
    if ('0' <= value && value <= '9') return (uint8_t)(value - '0');
    if ('a' <= value && value <= 'f') return (uint8_t)(10 + value - 'a');
    if ('A' <= value && value <= 'F') return (uint8_t)(10 + value - 'A');
    return 0;
}

int CyBtldr_FromAscii(uint32_t bufSize, char* buffer, uint16_t* rowSize, uint8_t* rowData) {
    uint16_t i;
    int err = CYRET_SUCCESS;

    if (bufSize & 1)  // Make sure even number of bytes
        err = CYRET_ERR_LENGTH;
    else {
        for (i = 0; i < bufSize / 2; i++) {
            rowData[i] = (CyBtldr_FromHex(buffer[i * 2]) << 4) | CyBtldr_FromHex(buffer[i * 2 + 1]);
        }
        *rowSize = i;
    }

    return err;
}

int CyBtldr_ReadLine(uint32_t* size, char* buffer) {
    int err = CYRET_SUCCESS;
    uint32_t len;
    // line that start with '#' are assumed to be comments, continue reading if we read a comment
    do {
        len = 0;
        if (NULL != dataFile && !feof(dataFile)) {
            if (NULL != fgets(buffer, MAX_BUFFER_SIZE * 2, dataFile)) {
                len = (uint32_t)strlen(buffer);

                while (len > 0 && ('\n' == buffer[len - 1] || '\r' == buffer[len - 1])) --len;
            } else
                err = CYRET_ERR_EOF;
        } else
            err = CYRET_ERR_FILE;
    } while (err == CYRET_SUCCESS && buffer[0] == '#');

    *size = len;
    return err;
}

int CyBtldr_OpenDataFile(const char* file) {
#ifdef WIN32
    fopen_s(&dataFile, file, "r");
#else
    dataFile = fopen(file, "r");
#endif

    return (NULL == dataFile) ? CYRET_ERR_FILE : CYRET_SUCCESS;
}

int CyBtldr_CheckCyacdFileVersion(uint32_t bufSize, char* header, uint8_t* version) {
    // check file extension of the file, if extension is cyacd, version 0
    int err = CYRET_SUCCESS;
    if (bufSize == 0) {
        err = CYRET_ERR_FILE;
    }
    
    if (bufSize < 2) err = CYRET_ERR_FILE;
    // .cyacd2 file stores version information in the first byte of the file header.
    if (CYRET_SUCCESS == err) {
        *version = CyBtldr_FromHex(header[0]) << 4 | CyBtldr_FromHex(header[1]);
        if (*version != 1) err = CYRET_ERR_DATA;
    }

    return err;
}

int CyBtldr_ParseHeader(uint32_t bufSize, char* buffer, uint32_t* siliconId, uint8_t* siliconRev, uint8_t* chksum, uint8_t* appID,
                           uint32_t* productID) {
    int err = CYRET_SUCCESS;

    uint16_t rowSize = 0;
    uint8_t rowData[MAX_BUFFER_SIZE];
    if (CYRET_SUCCESS == err) {
        err = CyBtldr_FromAscii(bufSize, buffer, &rowSize, rowData);
    }
    if (CYRET_SUCCESS == err) {
        if (rowSize == 12) {
            *siliconId = parse4ByteValueLittleEndian(rowData + 1);
            *siliconRev = rowData[5];
            *chksum = rowData[6];
            *appID = rowData[7];
            *productID = parse4ByteValueLittleEndian(rowData + 8);
        } else {
            err = CYRET_ERR_LENGTH;
        }
    }
    return err;
}

int CyBtldr_ParseRowData(uint32_t bufSize, char* buffer, uint32_t* address, uint8_t* rowData, uint16_t* size, uint8_t* checksum) {
    const uint16_t MIN_SIZE = 4;  // 4-addr
    const int DATA_OFFSET = 4;

    unsigned int i;
    uint16_t hexSize;
    uint8_t hexData[MAX_BUFFER_SIZE];
    int err = CYRET_SUCCESS;

    if (bufSize <= MIN_SIZE)
        err = CYRET_ERR_LENGTH;
    else if (buffer[0] == ':') {
        err = CyBtldr_FromAscii(bufSize - 1, &buffer[1], &hexSize, hexData);

        if (CYRET_SUCCESS == err) {
            *address = parse4ByteValueLittleEndian(hexData);
            *checksum = 0;

            if (MIN_SIZE < hexSize) {
                *size = hexSize - MIN_SIZE;
                for (i = 0; i < *size; i++) {
                    rowData[i] = (hexData[DATA_OFFSET + i]);
                    *checksum += rowData[i];
                }
            } else
                err = CYRET_ERR_DATA;
        }
    } else
        err = CYRET_ERR_CMD;

    return err;
}

int CyBtldr_ParseAppStartAndSize(uint32_t* appStart, uint32_t* appSize, uint32_t* dataLines, char* buf) {
    const uint32_t APPINFO_META_HEADER_SIZE = 11;
    const char APPINFO_META_HEADER[] = "@APPINFO:0x";
    const uint32_t APPINFO_META_SEPERATOR_SIZE = 3;
    const char APPINFO_META_SEPERATOR[] = ",0x";
    const char APPINFO_META_SEPERATOR_START[] = ",";

    long fp = ftell(dataFile);
    *appStart = 0xffffffff;
    *appSize = 0;
    *dataLines = 0;
    uint32_t addr = 0;
    uint32_t rowLength;
    uint16_t rowSize;
    uint32_t seperatorIndex;
    uint8_t rowData[MAX_BUFFER_SIZE];
    uint8_t checksum;
    int err = CYRET_SUCCESS;
    uint32_t i;
    bool appInfoFound = false;
    do {
        err = CyBtldr_ReadLine(&rowLength, buf);
        if (err == CYRET_SUCCESS) {
            if (buf[0] == ':') {
                if (!appInfoFound) {
                    err = CyBtldr_ParseRowData(rowLength, buf, &addr, rowData, &rowSize, &checksum);

                    if (addr < (*appStart)) {
                        *appStart = addr;
                    }
                    (*appSize) += rowSize;
                }
                ++(*dataLines);
            } else if (rowLength >= APPINFO_META_HEADER_SIZE && strncmp(buf, APPINFO_META_HEADER, APPINFO_META_HEADER_SIZE) == 0) {
                // find seperator index
                seperatorIndex = (uint32_t)strcspn(buf, APPINFO_META_SEPERATOR_START);
                if (strncmp(buf + seperatorIndex, APPINFO_META_SEPERATOR, APPINFO_META_SEPERATOR_SIZE) == 0) {
                    *appStart = 0;
                    *appSize = 0;
                    for (i = APPINFO_META_HEADER_SIZE; i < seperatorIndex; i++) {
                        *appStart <<= 4;
                        *appStart += CyBtldr_FromHex(buf[i]);
                    }
                    for (i = seperatorIndex + APPINFO_META_SEPERATOR_SIZE; i < rowLength; i++) {
                        *appSize <<= 4;
                        *appSize += CyBtldr_FromHex(buf[i]);
                    }
                } else {
                    err = CYRET_ERR_FILE;
                }
                appInfoFound = true;
            }
        }
    } while (err == CYRET_SUCCESS);
    if (err == CYRET_ERR_EOF) err = CYRET_SUCCESS;
    // reset to the file to where we were
    if (err == CYRET_SUCCESS) {
        if (fseek(dataFile, fp, SEEK_SET) != 0) {
            err = CYRET_ERR_EOF; // shouldn't be possible, we're just going to somewhere that was valid before
        }
    }
    return err;
}

int CyBtldr_CloseDataFile(void) {
    int err = 0;
    if (NULL != dataFile) {
        err = fclose(dataFile);
        dataFile = NULL;
    }
    return (0 == err) ? CYRET_SUCCESS : CYRET_ERR_FILE;
}
