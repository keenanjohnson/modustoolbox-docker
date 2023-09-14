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

#include <stdint.h>
#include <string.h>
#include "cybtldr_api.h"
#include "cybtldr_api2.h"

uint8_t g_abort;

int ProcessDataRow(CyBtldr_Action action, uint32_t rowSize, char* rowData, CyBtldr_ProgressUpdate* update, uint32_t applicationStartAddr,
                   uint32_t applicationDataLines, uint32_t* applicationDataLinesSeen) {
    uint8_t buffer[MAX_BUFFER_SIZE];
    uint16_t bufSize;
    uint32_t address;
    uint8_t checksum;

    int err = CyBtldr_ParseCyAcd2RowData(rowSize, rowData, &address, buffer, &bufSize, &checksum);
    if (CYRET_SUCCESS == err) {
        switch (action) {
            case ERASE:
                err = CyBtldr_EraseRow(address);
                break;
            case PROGRAM:
                err = CyBtldr_ProgramRow(address, buffer, bufSize);
                break;
            case VERIFY:
                err = CyBtldr_VerifyRow(address, buffer, bufSize);
                break;
        }
    }
    if (CYRET_SUCCESS == err && NULL != update) {
        ++(*applicationDataLinesSeen);
        update(*applicationDataLinesSeen * 100.0 / applicationDataLines);
    }

    return err;
}

int ProcessMetaRow(uint32_t rowSize, char* rowData) {
    const uint32_t EIV_META_HEADER_SIZE = 5;
    const char EIV_META_HEADER[] = "@EIV:";

    uint8_t buffer[MAX_BUFFER_SIZE];
    uint16_t bufSize = 0;

    int err = CYRET_SUCCESS;
    if (rowSize >= EIV_META_HEADER_SIZE && strncmp(rowData, EIV_META_HEADER, EIV_META_HEADER_SIZE) == 0) {
        err = CyBtldr_FromAscii(rowSize - EIV_META_HEADER_SIZE, rowData + EIV_META_HEADER_SIZE, &bufSize, buffer);
        if (err == CYRET_SUCCESS)
            err = CyBtldr_SetEncryptionInitialVector(bufSize, buffer);
    }
    return err;
}

int CyBtldr_RunAction(CyBtldr_Action action, CyBtldr_CommunicationsData* comm, CyBtldr_ProgressUpdate* update, const char* file) {
    g_abort = 0;
    uint32_t lineLen;
    char line[MAX_BUFFER_SIZE * 2];  // 2 hex characters per byte
    uint8_t fileVersion = 0;

    int err = CyBtldr_OpenDataFile(file);

    if (CYRET_SUCCESS == err) {
        err = CyBtldr_ReadLine(&lineLen, line);
        // the following functions checks
        if (CYRET_SUCCESS == err) err = CyBtldr_CheckCyacdFileVersion(lineLen, line, &fileVersion);
        if (CYRET_SUCCESS == err) {
            uint32_t blVer = 0;
            uint32_t siliconId = 0;
            uint8_t siliconRev = 0;
            uint8_t chksumtype = SUM_CHECKSUM;
            uint8_t appId = 0;
            uint8_t bootloaderEntered = 0;
            uint32_t applicationStartAddr = 0xffffffff;
            uint32_t applicationSize = 0;
            uint32_t applicationDataLines = 255;
            uint32_t applicationDataLinesSeen = 0;
            uint32_t productId = 0;

            err = CyBtldr_ParseHeader(lineLen, line, &siliconId, &siliconRev, &chksumtype, &appId, &productId);

            if (CYRET_SUCCESS == err) {
                CyBtldr_SetCheckSumType(chksumtype);

                // send ENTER DFU command to start communication
                err = CyBtldr_StartBootloadOperation(comm, siliconId, siliconRev, &blVer, productId);

                // send Set Application Metadata command 
                if (err == CYRET_SUCCESS) 
                    err = CyBtldr_ParseCyAcdAppStartAndSize(&applicationStartAddr, &applicationSize, &applicationDataLines, line);
                if (err == CYRET_SUCCESS) 
                    err = CyBtldr_SetApplicationMetaData(appId, applicationStartAddr, applicationSize);
                bootloaderEntered = 1;
            }


            while (CYRET_SUCCESS == err) {
                if (g_abort) {
                    err = CYRET_ABORT;
                    break;
                }
                
                err = CyBtldr_ReadLine(&lineLen, line);
                if (CYRET_SUCCESS == err) {
                    switch (line[0]) {
                        case '@':
                        err = ProcessMetaRow(lineLen, line);
                            break;
                        case ':':
                            err = ProcessDataRow(action, lineLen, line, update, applicationStartAddr, applicationDataLines,
                                                 &applicationDataLinesSeen);
                        break;
                    }
                } else if (CYRET_ERR_EOF == err) {
                    err = CYRET_SUCCESS;
                    break;
                }
            }
            if (err == CYRET_SUCCESS && (PROGRAM == action || VERIFY == action)) {
                err = CyBtldr_VerifyApplication(appId);
                CyBtldr_EndBootloadOperation();
            } else if (CYRET_ERR_COMM_MASK != (CYRET_ERR_COMM_MASK & err) && bootloaderEntered) {
                CyBtldr_EndBootloadOperation();
            }            
        }
        CyBtldr_CloseDataFile();
    }
    return err;
}

int CyBtldr_Program(const char* file, CyBtldr_CommunicationsData* comm, CyBtldr_ProgressUpdate* update) {
    return CyBtldr_RunAction(PROGRAM, comm, update, file);
}

int CyBtldr_Erase(const char* file, CyBtldr_CommunicationsData* comm, CyBtldr_ProgressUpdate* update) {
    return CyBtldr_RunAction(ERASE, comm, update, file);
}

int CyBtldr_Verify(const char* file, CyBtldr_CommunicationsData* comm, CyBtldr_ProgressUpdate* update) {
    return CyBtldr_RunAction(VERIFY, comm, update, file);
}

int CyBtldr_Abort(void) {
    g_abort = 1;
    return CYRET_SUCCESS;
}
