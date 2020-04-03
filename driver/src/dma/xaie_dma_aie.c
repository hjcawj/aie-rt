/******************************************************************************
*
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
******************************************************************************/
/*****************************************************************************/
/**
* @file xaie_dma_aie.c
* @{
*
* This file contains routines for AIE DMA configuration and controls.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   03/23/2020  Initial creation
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaie_helper.h"
#include "xaiegbl_regdef.h"

/************************** Constant Definitions *****************************/
#define XAIE_DMA_TILEDMA_2DX_DEFAULT_INCR		0U
#define XAIE_DMA_TILEDMA_2DX_DEFAULT_WRAP 		255U
#define XAIE_DMA_TILEDMA_2DX_DEFAULT_OFFSET		1U
#define XAIE_DMA_TILEDMA_2DY_DEFAULT_INCR 		255U
#define XAIE_DMA_TILEDMA_2DY_DEFAULT_WRAP 		255U
#define XAIE_DMA_TILEDMA_2DY_DEFAULT_OFFSET		256U

#define XAIE_TILEDMA_NUM_BD_WORDS			7U
#define XAIE_SHIMDMA_NUM_BD_WORDS			5U

#define XAIE_TILE_DMA_NUM_DIMS_MAX			2U
/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This API initializes the Dma Descriptor for AIE 1 Tile Dma.
*
* @param	DmaDesc: Dma Descriptor
*
* @return	None.
*
* @note		Internal Only.
*
******************************************************************************/
void _XAie_TileDmaInit(XAie_DmaDesc *Desc)
{
	Desc->MultiDimDesc.Gen1MultiDimDesc.X_Incr = XAIE_DMA_TILEDMA_2DX_DEFAULT_INCR;
	Desc->MultiDimDesc.Gen1MultiDimDesc.X_Wrap = XAIE_DMA_TILEDMA_2DX_DEFAULT_WRAP;
	Desc->MultiDimDesc.Gen1MultiDimDesc.X_Offset = XAIE_DMA_TILEDMA_2DX_DEFAULT_OFFSET;
	Desc->MultiDimDesc.Gen1MultiDimDesc.Y_Incr = XAIE_DMA_TILEDMA_2DY_DEFAULT_INCR;
	Desc->MultiDimDesc.Gen1MultiDimDesc.Y_Wrap = XAIE_DMA_TILEDMA_2DY_DEFAULT_WRAP;
	Desc->MultiDimDesc.Gen1MultiDimDesc.Y_Offset = XAIE_DMA_TILEDMA_2DY_DEFAULT_OFFSET;

	return;
}

/*****************************************************************************/
/**
*
* This API initializes the Dma Descriptor for AIE 1 Shim Dma.
*
* @param	DmaDesc: Dma Descriptor
*
* @return	None.
*
* @note		Internal Only.
*
******************************************************************************/
void _XAie_ShimDmaInit(XAie_DmaDesc *Desc)
{
	return;
}

/*****************************************************************************/
/**
*
* This API initializes the Acquire and Release Locks for a a Dma for AIE
* descriptor.
*
* @param	DevInst: Device Instance.
* @param	DmaDesc: Initialized Dma Descriptor.
* @param	Acq: Lock object with acquire lock ID and lock value.
* @param	Rel: Lock object with release lock ID and lock value.
* @param 	AcqEn: XAIE_ENABLE/XAIE_DISABLE for enable or disable acquire
*		lock.
* @param 	RelEn: XAIE_ENABLE/XAIE_DISABLE for enable or disable release
*		lock.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal Only. Should not be called directly. This function is
*		called from the internal Dma Module data structure.
*
******************************************************************************/
AieRC _XAie_DmaSetLock(XAie_DmaDesc *DmaDesc, XAie_Lock Acq, XAie_Lock Rel,
		u8 AcqEn, u8 RelEn)
{
	/* For AIE, Acquire and Release Lock IDs must be the same */
	if((Acq.LockId != Rel.LockId)) {
		XAieLib_print("Error: Lock ID is invalid\n");
		return XAIE_INVALID_LOCK_ID;
	}

	DmaDesc->LockDesc.LockAcqId = Acq.LockId;
	DmaDesc->LockDesc.LockRelId = Rel.LockId;
	DmaDesc->LockDesc.LockAcqEn = AcqEn;
	DmaDesc->LockDesc.LockRelEn = RelEn;

	/* If lock release value is invalid,then lock released with no value */
	if(Rel.LockVal != XAIE_LOCK_WITH_NO_VALUE) {
		DmaDesc->LockDesc.LockRelValEn = XAIE_ENABLE;
		DmaDesc->LockDesc.LockRelVal = Rel.LockVal;
	}

	/* If lock acquire value is invalid,then lock acquired with no value */
	if(Acq.LockVal != XAIE_LOCK_WITH_NO_VALUE) {
		DmaDesc->LockDesc.LockAcqValEn = XAIE_ENABLE;
		DmaDesc->LockDesc.LockAcqVal = Acq.LockVal;
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API setups the DmaDesc with the register fields required for the dma
* addressing mode of AIE.
*
* @param	DmaDesc: Initialized Dma Descriptor.
* @param	Tensor: Dma Tensor describing the address mode of dma.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal API only.
*
******************************************************************************/
AieRC _XAie_DmaSetMultiDim(XAie_DmaDesc *DmaDesc, XAie_DmaTensor *Tensor)
{
	/*
	for(u8 i = 0U; i < Tensor->NumDim; i++) {
		switch(i)
		{
			case 0U:
				DmaDesc->MultiDimDesc.Gen1MultiDimDesc.X_Offset =
					Tensor->Dim[i].AieDimDesc.Offset;
				DmaDesc->MultiDimDesc.Gen1MultiDimDesc.X_Wrap =
					Tensor->Dim[i].AieDimDesc.Wrap - 1U;
				DmaDesc->MultiDimDesc.Gen1MultiDimDesc.X_Incr =
					Tensor.Dim[i].AieDimDesc.Incr - 1U;
				break;
			case 1U:
				DmaDesc->MultiDimDesc.Gen1MultiDimDesc.Y_Offset =
					Tensor->Dim[i].AieDimDesc.Offset;
				DmaDesc->MultiDimDesc.Gen1MultiDimDesc.Y_Wrap =
					Tensor->Dim[i].AieDimDesc.Wrap - 1U;
				DmaDesc->MultiDimDesc.Gen1MultiDimDesc.Y_Incr =
					Tensor->Dim[i].AieDimDesc.Incr - 1U;
				break;
			default:
				break;
		}

	}
	*/
	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API setups DmaDesc with parameters to run dma in interleave mode for AIE
*
* @param	DevInst: Device Instance.
* @param	DmaDesc: Initialized Dma Descriptor.
* @param	DoubleBuff: Double buffer to use(0 - A, 1-B)
* @param	IntrleaveCount: Interleaved count to use(to be 32b word aligned)
* @param	IntrleaveCurr: Interleave current pointer.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only. The API sets up the value in the dma descriptor
*		and does not configure the buffer descriptor field in the
*		hardware.
*
******************************************************************************/
AieRC _XAie_DmaSetInterleaveEnable(XAie_DmaDesc *DmaDesc, u8 DoubleBuff,
		u8 IntrleaveCount, u16 IntrleaveCurr)
{
	DmaDesc->MultiDimDesc.Gen1MultiDimDesc.EnInterleaved = XAIE_ENABLE;
	DmaDesc->MultiDimDesc.Gen1MultiDimDesc.IntrleaveBufSelect = DoubleBuff;
	DmaDesc->MultiDimDesc.Gen1MultiDimDesc.IntrleaveCount = IntrleaveCount;
	DmaDesc->MultiDimDesc.Gen1MultiDimDesc.CurrPtr = IntrleaveCurr;

	return XAIE_OK;
}
/*****************************************************************************/
/**
*
* This API writes a Dma Descriptor which is initialized and setup by other APIs
* into the corresponding registers and register fields in the hardware. This API
* is specific to AIE Shim Tiles only.
*
* @param	DevInst: Device Instance
* @param	DmaDesc: Initialized Dma Descriptor.
* @param	Loc: Location of AIE Tile
* @param	BdNum: Hardware BD number to be written to.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only. For AIE Shim Tiles only.
*
******************************************************************************/
AieRC _XAie_ShimDmaWriteBd(XAie_DevInst *DevInst , XAie_DmaDesc *DmaDesc,
		XAie_LocType Loc, u8 BdNum)
{
	u64 Addr;
	u64 BdBaseAddr;
	u32 BdWord[XAIE_SHIMDMA_NUM_BD_WORDS];
	const XAie_DmaMod *DmaMod;
	const XAie_DmaBdProp *BdProp;

	DmaMod = DevInst->DevProp.DevMod[DmaDesc->TileType].DmaMod;
	BdProp = DmaMod->BdProp;

	BdBaseAddr = DmaMod->BaseAddr + BdNum * DmaMod->IdxOffset;

	BdWord[0U] = XAie_SetField(DmaDesc->AddrDesc.Address,
			BdProp->Buffer->ShimDmaBuff.AddrLow.Lsb,
			BdProp->Buffer->ShimDmaBuff.AddrLow.Mask);

	BdWord[1U] = XAie_SetField(DmaDesc->AddrDesc.Length,
			BdProp->Buffer->ShimDmaBuff.BufferLen.Lsb,
			BdProp->Buffer->ShimDmaBuff.BufferLen.Mask);

	BdWord[2U] = XAie_SetField((DmaDesc->AddrDesc.Address >> 32U),
			BdProp->Buffer->ShimDmaBuff.AddrHigh.Lsb,
			BdProp->Buffer->ShimDmaBuff.AddrHigh.Mask) |
		XAie_SetField(DmaDesc->BdEnDesc.UseNxtBd,
				BdProp->BdEn->UseNxtBd.Lsb,
				BdProp->BdEn->UseNxtBd.Mask) |
		XAie_SetField(DmaDesc->BdEnDesc.NxtBd,
				BdProp->BdEn->NxtBd.Lsb,
				BdProp->BdEn->NxtBd.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockAcqId,
				BdProp->Lock->AieDmaLock.LckId_A.Lsb,
				BdProp->Lock->AieDmaLock.LckId_A.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockRelEn,
				BdProp->Lock->AieDmaLock.LckRelEn_A.Lsb,
				BdProp->Lock->AieDmaLock.LckRelEn_A.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockRelVal,
				BdProp->Lock->AieDmaLock.LckRelVal_A.Lsb,
				BdProp->Lock->AieDmaLock.LckRelVal_A.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockRelValEn,
				BdProp->Lock->AieDmaLock.LckRelUseVal_A.Lsb,
				BdProp->Lock->AieDmaLock.LckRelUseVal_A.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockAcqEn,
				BdProp->Lock->AieDmaLock.LckAcqEn_A.Lsb,
				BdProp->Lock->AieDmaLock.LckAcqEn_A.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockAcqVal,
				BdProp->Lock->AieDmaLock.LckAcqVal_A.Lsb,
				BdProp->Lock->AieDmaLock.LckAcqVal_A.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockAcqValEn,
				BdProp->Lock->AieDmaLock.LckAcqUseVal_A.Lsb,
				BdProp->Lock->AieDmaLock.LckAcqUseVal_A.Mask) |
		XAie_SetField(DmaDesc->BdEnDesc.ValidBd,
				BdProp->BdEn->ValidBd.Lsb,
				BdProp->BdEn->ValidBd.Mask);

	BdWord[3U] = XAie_SetField(DmaDesc->AxiDesc.SMID,
			BdProp->SysProp->SMID.Lsb, BdProp->SysProp->SMID.Mask) |
		XAie_SetField(DmaDesc->AxiDesc.BurstLen,
				BdProp->SysProp->BurstLen.Lsb,
				BdProp->SysProp->BurstLen.Mask) |
		XAie_SetField(DmaDesc->AxiDesc.AxQos,
				BdProp->SysProp->AxQos.Lsb,
				BdProp->SysProp->AxQos.Mask) |
		XAie_SetField(DmaDesc->AxiDesc.SecureAccess,
				BdProp->SysProp->SecureAccess.Lsb,
				BdProp->SysProp->SecureAccess.Mask) |
		XAie_SetField(DmaDesc->AxiDesc.AxCache,
				BdProp->SysProp->AxCache.Lsb,
				BdProp->SysProp->AxCache.Mask);

	BdWord[4U] = XAie_SetField(DmaDesc->PktDesc.PktId,
			BdProp->Pkt->PktId.Lsb, BdProp->Pkt->PktId.Mask) |
		XAie_SetField(DmaDesc->PktDesc.PktType,
				BdProp->Pkt->PktType.Lsb,
				BdProp->Pkt->PktType.Mask) |
		XAie_SetField(DmaDesc->PktDesc.PktEn,
				BdProp->Pkt->EnPkt.Lsb,
				BdProp->Pkt->EnPkt.Mask);

	Addr = DevInst->BaseAddr + BdBaseAddr +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	for(u8 i = 0U; i < XAIE_SHIMDMA_NUM_BD_WORDS; i++) {
		XAieGbl_Write32(Addr + i * 4U, BdWord[i]);
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API writes a Dma Descriptor which is initialized and setup by other APIs
* into the corresponding registers and register fields in the hardware. This API
* is specific to AIE Tiles only.
*
* @param	DevInst: Device Instance
* @param	DmaDesc: Initialized Dma Descriptor.
* @param	Loc: Location of AIE Tile
* @param	BdNum: Hardware BD number to be written to.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only. For AIE Tiles only.
*
******************************************************************************/
AieRC _XAie_TileDmaWriteBd(XAie_DevInst *DevInst , XAie_DmaDesc *DmaDesc,
		XAie_LocType Loc, u8 BdNum)
{
	u64 Addr;
	u64 BdBaseAddr;
	u32 BdWord[XAIE_TILEDMA_NUM_BD_WORDS];
	const XAie_DmaMod *DmaMod;
	const XAie_DmaBdProp *BdProp;

	DmaMod = DevInst->DevProp.DevMod[DmaDesc->TileType].DmaMod;
	BdProp = DmaMod->BdProp;

	BdBaseAddr = DmaMod->BaseAddr + BdNum * DmaMod->IdxOffset;

	/* AcqLockId and RelLockId are the same in AIE */
	BdWord[0U] = XAie_SetField(DmaDesc->LockDesc.LockAcqId,
			BdProp->Lock->AieDmaLock.LckId_A.Lsb,
			BdProp->Lock->AieDmaLock.LckId_A.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockRelEn,
				BdProp->Lock->AieDmaLock.LckRelEn_A.Lsb,
				BdProp->Lock->AieDmaLock.LckRelEn_A.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockRelVal,
				BdProp->Lock->AieDmaLock.LckRelVal_A.Lsb,
				BdProp->Lock->AieDmaLock.LckRelVal_A.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockRelValEn,
				BdProp->Lock->AieDmaLock.LckRelUseVal_A.Lsb,
				BdProp->Lock->AieDmaLock.LckRelUseVal_A.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockAcqEn,
				BdProp->Lock->AieDmaLock.LckAcqEn_A.Lsb,
				BdProp->Lock->AieDmaLock.LckAcqEn_A.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockAcqVal,
				BdProp->Lock->AieDmaLock.LckAcqVal_A.Lsb,
				BdProp->Lock->AieDmaLock.LckAcqVal_A.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockAcqValEn,
				BdProp->Lock->AieDmaLock.LckAcqUseVal_A.Lsb,
				BdProp->Lock->AieDmaLock.LckAcqUseVal_A.Mask) |
		XAie_SetField(DmaDesc->AddrDesc.Address,
				BdProp->Buffer->TileDmaBuff.BaseAddr.Lsb,
				BdProp->Buffer->TileDmaBuff.BaseAddr.Mask);

	BdWord[1U] = XAie_SetField(DmaDesc->LockDesc_2.LockAcqId,
			BdProp->Lock->AieDmaLock.LckId_B.Lsb,
			BdProp->Lock->AieDmaLock.LckId_B.Mask) |
		XAie_SetField(DmaDesc->LockDesc_2.LockRelEn,
				BdProp->Lock->AieDmaLock.LckRelEn_B.Lsb,
				BdProp->Lock->AieDmaLock.LckRelEn_B.Mask) |
		XAie_SetField(DmaDesc->LockDesc_2.LockRelVal,
				BdProp->Lock->AieDmaLock.LckRelVal_B.Lsb,
				BdProp->Lock->AieDmaLock.LckRelVal_B.Mask) |
		XAie_SetField(DmaDesc->LockDesc_2.LockRelValEn,
				BdProp->Lock->AieDmaLock.LckRelUseVal_B.Lsb,
				BdProp->Lock->AieDmaLock.LckRelUseVal_B.Mask) |
		XAie_SetField(DmaDesc->LockDesc_2.LockAcqEn,
				BdProp->Lock->AieDmaLock.LckAcqEn_B.Lsb,
				BdProp->Lock->AieDmaLock.LckAcqEn_B.Mask) |
		XAie_SetField(DmaDesc->LockDesc_2.LockAcqVal,
				BdProp->Lock->AieDmaLock.LckAcqVal_B.Lsb,
				BdProp->Lock->AieDmaLock.LckAcqVal_B.Mask) |
		XAie_SetField(DmaDesc->LockDesc_2.LockAcqValEn,
				BdProp->Lock->AieDmaLock.LckAcqUseVal_B.Lsb,
				BdProp->Lock->AieDmaLock.LckAcqUseVal_B.Mask) |
		XAie_SetField(DmaDesc->AddrDesc_2.Address,
				BdProp->DoubleBuffer->BaseAddr_B.Lsb,
				BdProp->DoubleBuffer->BaseAddr_B.Mask);

	BdWord[2U] = XAie_SetField(
			DmaDesc->MultiDimDesc.Gen1MultiDimDesc.X_Incr,
			BdProp->AddrMode->AieMultiDimAddr.X_Incr.Lsb,
			BdProp->AddrMode->AieMultiDimAddr.X_Incr.Mask) |
		XAie_SetField(DmaDesc->MultiDimDesc.Gen1MultiDimDesc.X_Wrap,
				BdProp->AddrMode->AieMultiDimAddr.X_Wrap.Lsb,
				BdProp->AddrMode->AieMultiDimAddr.X_Wrap.Mask) |
		XAie_SetField(DmaDesc->MultiDimDesc.Gen1MultiDimDesc.X_Offset,
				BdProp->AddrMode->AieMultiDimAddr.X_Offset.Lsb,
				BdProp->AddrMode->AieMultiDimAddr.X_Offset.Mask);

	BdWord[3U] = XAie_SetField(
			DmaDesc->MultiDimDesc.Gen1MultiDimDesc.Y_Incr,
			BdProp->AddrMode->AieMultiDimAddr.Y_Incr.Lsb,
			BdProp->AddrMode->AieMultiDimAddr.Y_Incr.Mask) |
		XAie_SetField(DmaDesc->MultiDimDesc.Gen1MultiDimDesc.X_Wrap,
				BdProp->AddrMode->AieMultiDimAddr.Y_Wrap.Lsb,
				BdProp->AddrMode->AieMultiDimAddr.Y_Wrap.Mask) |
		XAie_SetField(DmaDesc->MultiDimDesc.Gen1MultiDimDesc.Y_Offset,
				BdProp->AddrMode->AieMultiDimAddr.Y_Offset.Lsb,
				BdProp->AddrMode->AieMultiDimAddr.Y_Offset.Mask);

	BdWord[4U] = XAie_SetField(DmaDesc->PktDesc.PktId,
			BdProp->Pkt->PktId.Lsb, BdProp->Pkt->PktId.Mask) |
		XAie_SetField(DmaDesc->PktDesc.PktType,
				BdProp->Pkt->PktType.Lsb,
				BdProp->Pkt->PktType.Mask);

	BdWord[5U] = XAie_SetField(
			DmaDesc->MultiDimDesc.Gen1MultiDimDesc.IntrleaveBufSelect,
			BdProp->DoubleBuffer->BuffSelect.Lsb,
			BdProp->DoubleBuffer->BuffSelect.Mask) |
		XAie_SetField(DmaDesc->MultiDimDesc.Gen1MultiDimDesc.CurrPtr,
				BdProp->AddrMode->AieMultiDimAddr.CurrPtr.Lsb,
				BdProp->AddrMode->AieMultiDimAddr.CurrPtr.Mask);

	BdWord[6U] = XAie_SetField(DmaDesc->BdEnDesc.ValidBd,
			BdProp->BdEn->ValidBd.Lsb, BdProp->BdEn->ValidBd.Mask) |
		XAie_SetField(DmaDesc->BdEnDesc.UseNxtBd,
				BdProp->BdEn->UseNxtBd.Lsb,
				BdProp->BdEn->UseNxtBd.Mask) |
		XAie_SetField(DmaDesc->EnDoubleBuff,
				BdProp->DoubleBuffer->EnDoubleBuff.Lsb,
				BdProp->DoubleBuffer->EnDoubleBuff.Mask) |
		XAie_SetField(DmaDesc->EnFifoMode,
				BdProp->DoubleBuffer->EnFifoMode.Lsb,
				BdProp->DoubleBuffer->EnFifoMode.Mask) |
		XAie_SetField(DmaDesc->PktDesc.PktEn, BdProp->Pkt->EnPkt.Lsb,
				BdProp->Pkt->EnPkt.Mask) |
		XAie_SetField(DmaDesc->MultiDimDesc.Gen1MultiDimDesc.EnInterleaved,
				BdProp->DoubleBuffer->EnIntrleaved.Lsb,
				BdProp->DoubleBuffer->EnIntrleaved.Mask) |
		XAie_SetField((DmaDesc->MultiDimDesc.Gen1MultiDimDesc.IntrleaveCount - 1U),
				BdProp->DoubleBuffer->IntrleaveCnt.Lsb,
				BdProp->DoubleBuffer->IntrleaveCnt.Mask) |
		XAie_SetField(DmaDesc->BdEnDesc.NxtBd,
				BdProp->BdEn->NxtBd.Lsb,
				BdProp->BdEn->NxtBd.Mask) |
		XAie_SetField(DmaDesc->AddrDesc.Length,
				BdProp->Buffer->TileDmaBuff.BufferLen.Lsb,
				BdProp->Buffer->TileDmaBuff.BufferLen.Mask);

	Addr = DevInst->BaseAddr + BdBaseAddr +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	for(u8 i = 0U; i < XAIE_TILEDMA_NUM_BD_WORDS; i++) {
		XAieGbl_Write32(Addr + i * 4U, BdWord[i]);
	}

	return XAIE_OK;
}

/** @} */