/******************************************************************************
* Copyright (C) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_cdo.c
* @{
*
* This file contains the data structures and routines for low level IO
* operations for cdo backend.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   06/09/2020 Initial creation.
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __AIECDO__ /* AIE simulator */

//#include "cdo_rts.h"

#endif

#include "xaie_helper.h"
#include "xaie_io.h"
#include "xaie_io_common.h"
#include "xaie_io_privilege.h"
#include "xaie_npi.h"

/************************** Constant Definitions *****************************/
/****************************** Type Definitions *****************************/
typedef struct {
	u64 BaseAddr;
	u64 NpiBaseAddr;
} XAie_CdoIO;

/************************** Function Definitions *****************************/
#ifdef __AIECDO__

/*****************************************************************************/
/**
*
* This is the memory IO function to free the global IO instance
*
* @param	IOInst: IO Instance pointer.
*
* @return	None.
*
* @note		The global IO instance is a singleton and freed when
* the reference count reaches a zero. Internal only.
*
*******************************************************************************/
static AieRC XAie_CdoIO_Finish(void *IOInst)
{
	free(IOInst);
	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the memory IO function to initialize the global IO instance
*
* @param	DevInst: Device instance pointer.
*
* @return	XAIE_OK on success. Error code on failure.
*
* @note		Internal only.
*
*******************************************************************************/
static AieRC XAie_CdoIO_Init(XAie_DevInst *DevInst)
{
	XAie_CdoIO *IOInst;

	IOInst = (XAie_CdoIO *)malloc(sizeof(*IOInst));
	if(IOInst == NULL) {
		XAIE_ERROR("Memory allocation failed\n");
		return XAIE_ERR;
	}

	IOInst->BaseAddr = DevInst->BaseAddr;
	IOInst->NpiBaseAddr = XAIE_NPI_BASEADDR;
	DevInst->IOInst = IOInst;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the memory IO function to read 32bit data from the specified address.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: Register offset to read from.
* @param	Data: Pointer to store the 32 bit value
*
* @return	XAIE_OK on success.
*
* @note		None.
* @note		Internal only.
*
*******************************************************************************/
static AieRC XAie_CdoIO_Read32(void *IOInst, u64 RegOff, u32 *Data)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	*Data = 0U;
	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the function to run backend operations
*
* @param	IOInst: IO instance pointer
* @param	DevInst: AI engine partition device instance
* @param	Op: Backend operation code
* @param	Arg: Backend operation argument
*
* @return	XAIE_OK for success and error code for failure.
*
* @note		Internal only.
*
*******************************************************************************/
static AieRC XAie_CdoIO_RunOp(void *IOInst, XAie_DevInst *DevInst,
		     XAie_BackendOpCode Op, void *Arg)
{
	AieRC RC = XAIE_OK;
	(void)IOInst;

	switch(Op) {
		case XAIE_BACKEND_OP_ASSERT_SHIMRST:
		{
			u8 RstEnable = (u8)((uintptr_t)Arg & 0xFF);

			_XAie_NpiSetShimReset(DevInst, RstEnable);
			break;
		}
		case XAIE_BACKEND_OP_SET_PROTREG:
		{
			RC = _XAie_NpiSetProtectedRegEnable(DevInst, Arg);
			break;
		}
		case XAIE_BACKEND_OP_REQUEST_TILES:
			return _XAie_PrivilegeRequestTiles(DevInst,
					(XAie_BackendTilesArray *)Arg);
		case XAIE_BACKEND_OP_REQUEST_RESOURCE:
			return _XAie_RequestRscCommon(DevInst, Arg);
		case XAIE_BACKEND_OP_RELEASE_RESOURCE:
			return _XAie_ReleaseRscCommon(Arg);
		case XAIE_BACKEND_OP_FREE_RESOURCE:
			return _XAie_FreeRscCommon(Arg);
		case XAIE_BACKEND_OP_REQUEST_ALLOCATED_RESOURCE:
			return _XAie_RequestAllocatedRscCommon(DevInst, Arg);
		case XAIE_BACKEND_OP_PARTITION_INITIALIZE:
			return _XAie_PrivilegeInitPart(DevInst,
					(XAie_PartInitOpts *)Arg);
		case XAIE_BACKEND_OP_PARTITION_TEARDOWN:
			return _XAie_PrivilegeTeardownPart(DevInst);
		case XAIE_BACKEND_OP_GET_RSC_STAT:
			return _XAie_GetRscStatCommon(DevInst, Arg);
		case XAIE_BACKEND_OP_UPDATE_NPI_ADDR:
		{
			XAie_CdoIO *CdoIOInst = (XAie_CdoIO *)IOInst;
			CdoIOInst->NpiBaseAddr = *((u64 *)Arg);
			break;
		}
		case XAIE_BACKEND_OP_SET_COLUMN_CLOCK:
			return _XAie_PrivilegeSetColumnClk(DevInst,
					(XAie_BackendColumnReq *)Arg);
		default:
			XAIE_ERROR("CDO backend doesn't support operation"
					" %u.\n", Op);
			RC = XAIE_FEATURE_NOT_SUPPORTED;
			break;
	}

	return RC;
}

#else

static AieRC XAie_CdoIO_Finish(void *IOInst)
{
	/* no-op */
	(void)IOInst;
	return XAIE_OK;
}

static AieRC XAie_CdoIO_Init(XAie_DevInst *DevInst)
{
	/* no-op */
	(void)DevInst;
	XAIE_ERROR("Driver is not compiled with cdo generation "
			"backend (__AIECDO__)\n");
	return XAIE_INVALID_BACKEND;
}

static AieRC XAie_CdoIO_Write32(void *IOInst, u64 RegOff, u32 Value)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Value;

	return XAIE_ERR;
}

static AieRC XAie_CdoIO_Read32(void *IOInst, u64 RegOff, u32 *Data)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Data;
	return 0;
}

static AieRC XAie_CdoIO_MaskWrite32(void *IOInst, u64 RegOff, u32 Mask,
		u32 Value)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Mask;
	(void)Value;

	return XAIE_ERR;
}

static AieRC XAie_CdoIO_MaskPoll(void *IOInst, u64 RegOff, u32 Mask, u32 Value,
		u32 TimeOutUs)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Mask;
	(void)Value;
	(void)TimeOutUs;

	return XAIE_ERR;
}

static AieRC XAie_CdoIO_BlockWrite32(void *IOInst, u64 RegOff, const u32 *Data,
		u32 Size)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Data;
	(void)Size;

	return XAIE_ERR;
}

static AieRC XAie_CdoIO_BlockSet32(void *IOInst, u64 RegOff, u32 Data, u32 Size)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Data;
	(void)Size;

	return XAIE_ERR;
}

static AieRC XAie_CdoIO_RunOp(void *IOInst, XAie_DevInst *DevInst,
		     XAie_BackendOpCode Op, void *Arg)
{
	(void)IOInst;
	(void)DevInst;
	(void)Op;
	(void)Arg;
	return XAIE_FEATURE_NOT_SUPPORTED;
}

#endif /* __AIECDO__ */

static AieRC XAie_CdoIO_CmdWrite(void *IOInst, u8 Col, u8 Row, u8 Command,
		u32 CmdWd0, u32 CmdWd1, const char *CmdStr)
{
	/* no-op */
	(void)IOInst;
	(void)Col;
	(void)Row;
	(void)Command;
	(void)CmdWd0;
	(void)CmdWd1;
	(void)CmdStr;

	return XAIE_OK;
}

static XAie_MemInst* XAie_CdoMemAllocate(XAie_DevInst *DevInst, u64 Size,
		XAie_MemCacheProp Cache)
{
	(void)DevInst;
	(void)Size;
	(void)Cache;
	return NULL;
}

static AieRC XAie_CdoMemFree(XAie_MemInst *MemInst)
{
	(void)MemInst;
	return XAIE_ERR;
}

static AieRC XAie_CdoMemSyncForCPU(XAie_MemInst *MemInst)
{
	(void)MemInst;
	return XAIE_ERR;
}

static AieRC XAie_CdoMemSyncForDev(XAie_MemInst *MemInst)
{
	(void)MemInst;
	return XAIE_ERR;
}

static AieRC XAie_CdoMemAttach(XAie_MemInst *MemInst, u64 MemHandle)
{
	(void)MemInst;
	(void)MemHandle;
	return XAIE_ERR;
}

static AieRC XAie_CdoMemDetach(XAie_MemInst *MemInst)
{
	(void)MemInst;
	return XAIE_ERR;
}

const XAie_Backend CdoBackend =
{
	.Type = XAIE_IO_BACKEND_CDO,
	.Ops.Init = XAie_CdoIO_Init,
	.Ops.Finish = XAie_CdoIO_Finish,
	.Ops.Read32 = XAie_CdoIO_Read32,
	.Ops.CmdWrite = XAie_CdoIO_CmdWrite,
	.Ops.MemAllocate = XAie_CdoMemAllocate,
	.Ops.MemFree = XAie_CdoMemFree,
	.Ops.MemSyncForCPU = XAie_CdoMemSyncForCPU,
	.Ops.MemSyncForDev = XAie_CdoMemSyncForDev,
	.Ops.MemAttach = XAie_CdoMemAttach,
	.Ops.MemDetach = XAie_CdoMemDetach,
	.Ops.GetTid = XAie_IODummyGetTid,
	.Ops.SubmitTxn = NULL,
};

/** @} */
