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

#include "cybtldr_api.h"
#include <stdlib.h>
#include "cybtldr_command.h"


static CyBtldr_CommunicationsData* g_comm;

static uint16_t min_uint16(uint16_t a, uint16_t b) { return (a < b) ? a : b; }

int CyBtldr_TransferData(uint8_t* inBuf, int inSize, uint8_t* outBuf, int outSize) {
    int err = g_comm->WriteData(inBuf, inSize);

    if (CYRET_SUCCESS == err) err = g_comm->ReadData(outBuf, outSize);

    if (CYRET_SUCCESS != err) err |= CYRET_ERR_COMM_MASK;

    return err;
}

int CyBtldr_StartBootloadOperation(CyBtldr_CommunicationsData* comm, uint32_t expSiId, uint8_t expSiRev, uint32_t* blVer,
                                      uint32_t productID) {
    uint32_t inSize = 0;
    uint32_t outSize = 0;
    uint32_t siliconId = 0;
    uint8_t inBuf[MAX_COMMAND_SIZE];
    uint8_t outBuf[MAX_COMMAND_SIZE];
    uint8_t siliconRev = 0;
    uint8_t status = CYRET_SUCCESS;
    int err;

    g_comm = comm;

    err = g_comm->OpenConnection();
    if (CYRET_SUCCESS != err) err |= CYRET_ERR_COMM_MASK;

    if (CYRET_SUCCESS == err) err = CyBtldr_CreateEnterBootLoaderCmd(inBuf, &inSize, &outSize, productID);
    if (CYRET_SUCCESS == err) err = CyBtldr_TransferData(inBuf, inSize, outBuf, outSize);
    if (CYRET_SUCCESS == err)
        err = CyBtldr_ParseEnterBootLoaderCmdResult(outBuf, outSize, &siliconId, &siliconRev, blVer, &status);
    else if (CyBtldr_TryParseParketStatus(outBuf, outSize, &status) == CYRET_SUCCESS)
        err = status | CYRET_ERR_BTLDR_MASK;  // if the response we get back is a valid packet override the err with the response's status

    if (CYRET_SUCCESS == err) {
        if (CYRET_SUCCESS != status) err = status | CYRET_ERR_BTLDR_MASK;
        if (expSiId != siliconId || expSiRev != siliconRev) err = CYRET_ERR_DEVICE;
    }

    return err;
}

int CyBtldr_EndBootloadOperation(void) {
    uint32_t inSize;
    uint32_t outSize;
    uint8_t inBuf[MAX_COMMAND_SIZE];

    int err = CyBtldr_CreateExitBootLoaderCmd(inBuf, &inSize, &outSize);
    if (CYRET_SUCCESS == err) {
        err = g_comm->WriteData(inBuf, inSize);

        if (CYRET_SUCCESS == err) err = g_comm->CloseConnection();

        if (CYRET_SUCCESS != err) err |= CYRET_ERR_COMM_MASK;
    }
    g_comm = NULL;

    return err;
}

static int SendData(uint8_t* buf, uint16_t size, uint16_t* offset, uint16_t maxRemainingDataSize, uint8_t* inBuf, uint8_t* outBuf) {
    uint8_t status = CYRET_SUCCESS;
    uint32_t inSize = 0, outSize = 0;
    // size is the total bytes of data to transfer.
    // offset is the amount of data already transfered.
    // a is maximum amount of data allowed to be left over when this function ends.
    // (we can leave some data for caller (programRow, VerifyRow,...) to send.
    // TRANSFER_HEADER_SIZE is the amount of bytes this command header takes up.
    const uint16_t TRANSFER_HEADER_SIZE = 7;
    uint16_t subBufSize = min_uint16((uint16_t)(g_comm->MaxTransferSize - TRANSFER_HEADER_SIZE), size);
    int err = CYRET_SUCCESS;
    uint16_t cmdLen = 0;
    // Break row into pieces to ensure we don't send too much for the transfer protocol
    while ((CYRET_SUCCESS == err) && ((size - (*offset)) > maxRemainingDataSize)) {
        if ((size - (*offset)) > subBufSize) {
            cmdLen = subBufSize;
        } else {
            cmdLen = size - (*offset);
        }
        err = CyBtldr_CreateSendDataCmd(&buf[*offset], cmdLen, inBuf, &inSize, &outSize);
        if (CYRET_SUCCESS == err) err = CyBtldr_TransferData(inBuf, inSize, outBuf, outSize);
        if (CYRET_SUCCESS == err) err = CyBtldr_ParseSendDataCmdResult(outBuf, outSize, &status);
        if (CYRET_SUCCESS != status) err = status | CYRET_ERR_BTLDR_MASK;

        (*offset) += cmdLen;
    }
    return err;
}

int CyBtldr_ProgramRow(uint32_t address, uint8_t* buf, uint16_t size) {
    const int TRANSFER_HEADER_SIZE = 15;

    uint8_t inBuf[MAX_COMMAND_SIZE];
    uint8_t outBuf[MAX_COMMAND_SIZE];
    uint32_t inSize;
    uint32_t outSize;
    uint16_t offset = 0;
    uint16_t subBufSize;
    uint8_t status = CYRET_SUCCESS;
    int err = CYRET_SUCCESS;

    uint32_t chksum = CyBtldr_ComputeChecksum32bit(buf, size);

    if (CYRET_SUCCESS == err) err = SendData(buf, size, &offset, (uint16_t)(g_comm->MaxTransferSize - TRANSFER_HEADER_SIZE), inBuf, outBuf);

    if (CYRET_SUCCESS == err) {
        subBufSize = (uint16_t)(size - offset);

        err = CyBtldr_CreateProgramDataCmd(address, chksum, &buf[offset], subBufSize, inBuf, &inSize, &outSize);
        if (CYRET_SUCCESS == err) err = CyBtldr_TransferData(inBuf, inSize, outBuf, outSize);
        if (CYRET_SUCCESS == err) err = CyBtldr_ParseDefaultCmdResult(outBuf, outSize, &status);
        if (CYRET_SUCCESS != status) err = status | CYRET_ERR_BTLDR_MASK;
    }

    return err;
}

int CyBtldr_EraseRow(uint32_t address) {
    uint8_t inBuf[MAX_COMMAND_SIZE];
    uint8_t outBuf[MAX_COMMAND_SIZE];
    uint32_t inSize = 0;
    uint32_t outSize = 0;
    uint8_t status = CYRET_SUCCESS;
    int err = CYRET_SUCCESS;

    if (CYRET_SUCCESS == err) err = CyBtldr_CreateEraseDataCmd(address, inBuf, &inSize, &outSize);
    if (CYRET_SUCCESS == err) err = CyBtldr_TransferData(inBuf, inSize, outBuf, outSize);
    if (CYRET_SUCCESS == err) err = CyBtldr_ParseDefaultCmdResult(outBuf, outSize, &status);
    if (CYRET_SUCCESS != status) err = status | CYRET_ERR_BTLDR_MASK;

    return err;
}

int CyBtldr_VerifyRow(uint32_t address, uint8_t* buf, uint16_t size) {
    const int TRANSFER_HEADER_SIZE = 15;

    uint8_t inBuf[MAX_COMMAND_SIZE];
    uint8_t outBuf[MAX_COMMAND_SIZE];
    uint32_t inSize;
    uint32_t outSize;
    uint16_t offset = 0;
    uint16_t subBufSize;
    uint8_t status = CYRET_SUCCESS;
    int err = CYRET_SUCCESS;

    uint32_t chksum = CyBtldr_ComputeChecksum32bit(buf, size);

    if (CYRET_SUCCESS == err) err = SendData(buf, size, &offset, (uint16_t)(g_comm->MaxTransferSize - TRANSFER_HEADER_SIZE), inBuf, outBuf);

    if (CYRET_SUCCESS == err) {
        subBufSize = (uint16_t)(size - offset);

        err = CyBtldr_CreateVerifyDataCmd(address, chksum, &buf[offset], subBufSize, inBuf, &inSize, &outSize);
        if (CYRET_SUCCESS == err) err = CyBtldr_TransferData(inBuf, inSize, outBuf, outSize);
        if (CYRET_SUCCESS == err) err = CyBtldr_ParseDefaultCmdResult(outBuf, outSize, &status);
        if (CYRET_SUCCESS != status) err = status | CYRET_ERR_BTLDR_MASK;
    }

    return err;
}

int CyBtldr_VerifyApplication(uint8_t appId) {
    uint8_t inBuf[MAX_COMMAND_SIZE];
    uint8_t outBuf[MAX_COMMAND_SIZE];
    uint32_t inSize = 0;
    uint32_t outSize = 0;
    uint8_t checksumValid = 0;
    uint8_t status = CYRET_SUCCESS;

    int err = CyBtldr_CreateVerifyChecksumCmd(appId, inBuf, &inSize, &outSize);
    if (CYRET_SUCCESS == err) err = CyBtldr_TransferData(inBuf, inSize, outBuf, outSize);
    if (CYRET_SUCCESS == err) err = CyBtldr_ParseVerifyChecksumCmdResult(outBuf, outSize, &checksumValid, &status);
    if (CYRET_SUCCESS != status) err = status | CYRET_ERR_BTLDR_MASK;
    if ((CYRET_SUCCESS == err) && (!checksumValid)) err = CYRET_ERR_CHECKSUM;

    return err;
}

int CyBtldr_SetApplicationMetaData(uint8_t appId, uint32_t appStartAddr, uint32_t appSize) {
    uint8_t inBuf[MAX_COMMAND_SIZE];
    uint8_t outBuf[MAX_COMMAND_SIZE];
    uint32_t inSize = 0;
    uint32_t outSize = 0;
    uint8_t status = CYRET_SUCCESS;

    uint8_t metadata[8];
    metadata[0] = (uint8_t)appStartAddr;
    metadata[1] = (uint8_t)(appStartAddr >> 8);
    metadata[2] = (uint8_t)(appStartAddr >> 16);
    metadata[3] = (uint8_t)(appStartAddr >> 24);
    metadata[4] = (uint8_t)appSize;
    metadata[5] = (uint8_t)(appSize >> 8);
    metadata[6] = (uint8_t)(appSize >> 16);
    metadata[7] = (uint8_t)(appSize >> 24);
    int err = CyBtldr_CreateSetApplicationMetadataCmd(appId, metadata, inBuf, &inSize, &outSize);
    if (CYRET_SUCCESS == err) err = CyBtldr_TransferData(inBuf, inSize, outBuf, outSize);
    if (CYRET_SUCCESS == err) err = CyBtldr_ParseDefaultCmdResult(outBuf, outSize, &status);
    if (CYRET_SUCCESS != status) err = status | CYRET_ERR_BTLDR_MASK;

    return err;
}

int CyBtldr_SetEncryptionInitialVector(uint16_t size, uint8_t* buf) {
    uint8_t inBuf[MAX_COMMAND_SIZE];
    uint8_t outBuf[MAX_COMMAND_SIZE];

    uint32_t inSize = 0;
    uint32_t outSize = 0;
    uint8_t status = CYRET_SUCCESS;

    int err = CyBtldr_CreateSetEncryptionInitialVectorCmd(buf, size, inBuf, &inSize, &outSize);
    if (CYRET_SUCCESS == err) err = CyBtldr_TransferData(inBuf, inSize, outBuf, outSize);
    if (CYRET_SUCCESS == err) err = CyBtldr_ParseDefaultCmdResult(outBuf, outSize, &status);
    if (CYRET_SUCCESS != status) err = status | CYRET_ERR_BTLDR_MASK;

    return err;
}
