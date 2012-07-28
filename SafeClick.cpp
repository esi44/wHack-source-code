/*==============================================================================
	SafeClick.cpp for 1.26a
	
	*created by (c) wc3noobpl (2012)
	*originally based on el patron safeclick sources
		(was totally rewritten that shitcode (c) wc3noobpl)
==============================================================================*/
#include "System.h"
#include "WC3.h"
#include "SafeClick.h"

// Vars
bool SafeClickEnabled = false;

/*------------------------------------------------------------------------------
	GetUnitByIDs
------------------------------------------------------------------------------*/

Unit* GetUnitByIDs(DWORD id1, DWORD id2)
{
	if(IsInGame())
	{
		Unit** units = (Unit**)*(DWORD*)(*(DWORD*)(*GAME_UnitArray + 0x3BC) + 0x608);
		DWORD count = *(DWORD*)(*(DWORD*)(*GAME_UnitArray + 0x3BC) + 0x604);

		for(DWORD i = 0; i < count; i++)
			if( units[i]->ID1 == id1 && units[i]->ID2 == id2 )
				return units[i];
	}
	return NULL;
}

/*------------------------------------------------------------------------------
	IsSelectedUnitVisible
------------------------------------------------------------------------------*/

bool IsSelectedUnitVisible(Packet* packet, bool* isVisible)
{
	*isVisible = false;
	if(packet->PacketData[0] == 0x16)
	{
		SelectionAction* selectionAction = (SelectionAction*)packet->PacketData;
		//PrintTextFormat(10, "    action = %u, %u, %u", (DWORD)selectionAction->OpCode, (DWORD)selectionAction->Mode, (DWORD)selectionAction->Count);

		// we able to select in fog only one unit or building,
		// if we block Mode == 2 too, then we will get critical error after saving our own replay (c) wc3noobpl.
		if(selectionAction->Count == 1 && selectionAction->Mode == 1)
		{
			Unit* unit = GetUnitByIDs(selectionAction->UnitIDs[0][0], selectionAction->UnitIDs[0][1]);
			Player* player = GetLocalPlayer();
			if(unit && player)
			{
				if(unit && GAME_IsUnitVisible(unit, player->Slot, player->Team, 4))
					*isVisible = true;

				return true; // this is selection action with 1 unit
			}
		}
	}
	return false; // this is not selection action with 1 unit
}

/*------------------------------------------------------------------------------
	GAME_SendPacket_Intercept
------------------------------------------------------------------------------*/

void __fastcall GAME_SendPacket_Intercept(Packet* packet, DWORD zero)
{
	// @warning: sometimes in custom games send packet called before game info initialized, i.e. IsInGame == false.
	//PrintTextFormat(10, "packet = %u, %u, %u, %u, %u   [%u]", packet->Size, packet->PacketClassPtr, packet->_1, packet->_2, packet->_3, zero);
	
	bool isVisible = false;
	if(IsInGame() && IsSelectedUnitVisible(packet, &isVisible))
	{
		if(isVisible)
			GAME_SendPacket(packet, zero);
		//else
			//PrintText(10, "    invisible");
	}
	else
		GAME_SendPacket(packet, zero);
}

/*------------------------------------------------------------------------------
	EnableSafeClick
------------------------------------------------------------------------------*/

void EnableSafeClick()
{
	if(IsWC3Valid())
	{
		if(!SafeClickEnabled)
		{
			InterceptCallSafe((DWORD)GAME_SendPacket_I, (DWORD)GAME_SendPacket, (DWORD)GAME_SendPacket_Intercept);
			SafeClickEnabled = true;
		}
	}
}

/*------------------------------------------------------------------------------
	DisableSafeClick
------------------------------------------------------------------------------*/

void DisableSafeClick()
{
	if(IsWC3Valid())
	{
		if(SafeClickEnabled)
		{
			InterceptCallSafe((DWORD)GAME_SendPacket_I, (DWORD)GAME_SendPacket_Intercept, (DWORD)GAME_SendPacket);
			SafeClickEnabled = false;
		}
	}
}
