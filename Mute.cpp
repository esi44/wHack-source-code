/*==============================================================================
	Mute.cpp
	
	*created by (c) wc3noobpl (2012)
==============================================================================*/
#include "System.h"
#include "WC3.h"
#include "Mute.h"

// Vars
bool MuteEnabled = false;
bool IsMuteOn = false;

/*------------------------------------------------------------------------------
	InitGameMute
------------------------------------------------------------------------------*/

void InitGameMute()
{
	if(IsMuteOn)
		PrintText(10, "|cFFAAAAAAMute ON|r");
	else
		PrintText(10, "|cFFAAAAAAMute OFF|r");
}

/*------------------------------------------------------------------------------
	GAME_PrintChat_Intercept
------------------------------------------------------------------------------*/

void __fastcall GAME_PrintChat_Intercept(DWORD chatPtrList, DWORD _1, DWORD slot, CHAR* text, DWORD printType, DWORD stayUpTime)
{
	// show only your own messages
	DWORD localSlot = GetLocalPlayerSlot();
	if(slot == localSlot || !IsMuteOn)
	{
		GAME_PrintChat(chatPtrList, _1, slot, text, printType, stayUpTime);
	}
}

/*------------------------------------------------------------------------------
	EnableMute
------------------------------------------------------------------------------*/

void EnableMute(bool isOnByDefault)
{
	if(IsWC3Valid())
	{
		if(!MuteEnabled)
		{
			InterceptCallSafe((DWORD)GAME_PrintChat_I1, (DWORD)GAME_PrintChat, (DWORD)GAME_PrintChat_Intercept);
			InterceptCallSafe((DWORD)GAME_PrintChat_I2, (DWORD)GAME_PrintChat, (DWORD)GAME_PrintChat_Intercept);
			IsMuteOn = isOnByDefault;
			MuteEnabled = true;
		}
	}
}

/*------------------------------------------------------------------------------
	DisableMute
------------------------------------------------------------------------------*/

void DisableMute()
{
	if(IsWC3Valid())
	{
		if(MuteEnabled)
		{
			InterceptCallSafe((DWORD)GAME_PrintChat_I1, (DWORD)GAME_PrintChat_Intercept, (DWORD)GAME_PrintChat);
			InterceptCallSafe((DWORD)GAME_PrintChat_I2, (DWORD)GAME_PrintChat_Intercept, (DWORD)GAME_PrintChat);
			MuteEnabled = false;
		}
	}
}

/*------------------------------------------------------------------------------
	ToggleMute
------------------------------------------------------------------------------*/

void ToggleMute()
{
	if(IsInGame())
	{
		if(!IsMuteOn)
		{
			PrintText(10, "|cFFAAAAAAMute ON|r");
			IsMuteOn = true;
		}
		else
		{
			PrintText(10, "|cFFAAAAAAMute OFF|r");
			IsMuteOn = false;
		}
	}
}
