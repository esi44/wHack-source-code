/*==============================================================================
	SafeClick.h
	
	*created by (c) wc3noobpl (2012)
	*originally based on el patron sources
==============================================================================*/
#pragma once

/*------------------------------------------------------------------------------
	Types
------------------------------------------------------------------------------*/

#pragma pack(push,1)
struct SelectionAction
{
	BYTE OpCode;			//0x16
	BYTE Mode;				//1 - select, 2 unselect
	WORD Count;
	DWORD UnitIDs[12][2];
};
#pragma pack(pop)

/*------------------------------------------------------------------------------
	Methods
------------------------------------------------------------------------------*/

Unit* GetUnitByIDs(DWORD id1, DWORD id2);
void EnableSafeClick();
void DisableSafeClick();
