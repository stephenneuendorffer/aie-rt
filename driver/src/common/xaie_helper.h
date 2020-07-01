/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_helper.h
* @{
*
* This file contains inline helper functions for AIE drivers.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   09/24/2019  Initial creation
* 1.1   Tejus   12/09/2019  Include correct header file to avoid cyclic
*			    dependancy
* 1.2   Tejus   03/22/2020  Remove helper functions used by initial dma
*			    implementations
* 1.3   Tejus   04/13/2020  Add api to get tile type from Loc
* 1.4   Tejus   04/13/2020  Remove helper functions for range apis
* 1.5   Tejus   06/10/2020  Add helper functions for IO backend.
* </pre>
*
******************************************************************************/
#ifndef XAIEHELPER_H
#define XAIEHELPER_H

/***************************** Include Files *********************************/
#include "xaie_io.h"
#include "xaiegbl_regdef.h"

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* Calculates the Tile Address from Row, Col of the AIE array/partition
*
* @param	DevInst: Device Instance
* @param	R: Row
* @param	C: Column
* @return	TileAddr
*
* @note		Internal API only.
*
******************************************************************************/
static inline u64 _XAie_GetTileAddr(XAie_DevInst *DevInst, int R, int C)
{
	return (R << DevInst->DevProp.RowShift) | (C << DevInst->DevProp.ColShift);
}

static inline void XAie_Write32(XAie_DevInst *DevInst, u64 RegOff, u32 Value)
{
	const XAie_Backend *Backend = DevInst->Backend;

	Backend->Ops.Write32((void*)(DevInst->IOInst), RegOff, Value);
}

static inline u32 XAie_Read32(XAie_DevInst *DevInst, u64 RegOff)
{
	const XAie_Backend *Backend = DevInst->Backend;

	return Backend->Ops.Read32((void*)(DevInst->IOInst), RegOff);
}

static inline void XAie_MaskWrite32(XAie_DevInst *DevInst, u64 RegOff, u32 Mask,
		u32 Value)
{
	const XAie_Backend *Backend = DevInst->Backend;

	Backend->Ops.MaskWrite32((void *)(DevInst->IOInst), RegOff, Mask,
			Value);
}

static inline u32 XAie_MaskPoll(XAie_DevInst *DevInst, u64 RegOff, u32 Mask,
		u32 Value, u32 TimeOutUs)
{
	const XAie_Backend *Backend = DevInst->Backend;

	return Backend->Ops.MaskPoll((void*)(DevInst->IOInst), RegOff, Mask,
			Value, TimeOutUs);
}

static inline void XAie_BlockWrite32(XAie_DevInst *DevInst, u64 RegOff,
		u32 *Data, u32 Size)
{
	const XAie_Backend *Backend = DevInst->Backend;

	Backend->Ops.BlockWrite32((void *)(DevInst->IOInst), RegOff, Data,
			Size);
}

static inline void XAie_BlockSet32(XAie_DevInst *DevInst, u64 RegOff, u32 Data,
		u32 Size)
{
	const XAie_Backend *Backend = DevInst->Backend;

	Backend->Ops.BlockSet32((void *)(DevInst->IOInst), RegOff, Data, Size);
}

static inline void XAie_CmdWrite(XAie_DevInst *DevInst, u8 Col, u8 Row,
		u8 Command, u32 CmdWd0, u32 CmdWd1, const char *CmdStr)
{
	const XAie_Backend *Backend = DevInst->Backend;

	Backend->Ops.CmdWrite((void *)(DevInst->IOInst), Col, Row, Command,
			CmdWd0, CmdWd1, CmdStr);
}

u8 _XAie_GetTileTypefromLoc(XAie_DevInst *DevInst, XAie_LocType Loc);
AieRC _XAie_CheckModule(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module);
#endif		/* end of protection macro */
/** @} */
