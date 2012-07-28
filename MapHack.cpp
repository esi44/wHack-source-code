/*==============================================================================
	Maphack.cpp for v1.26a
	
	*created by (c) wc3noobpl (2012)
	*originally based on Dennis(Zero) sources
==============================================================================*/
#include "System.h"
#include "WC3.h"
#include "MapHack.h"
#include "SafeClick.h"
#include <vector>

// Types
struct PatchOffset
{
	DWORD Addr;
	LPVOID Code;
	DWORD Length;
	BYTE* OriginalCode;

	// Construct
	PatchOffset(DWORD addr, LPVOID code, DWORD length)
	{
		Addr = addr;
		Code = code;
		Length = length;
		OriginalCode = NULL;
	}
};

// Vars
EMapHackType CurrentMapHackType = MAPHACK_OFF;
std::vector<PatchOffset> MapHackOffsets;

/*------------------------------------------------------------------------------
	InitMapHack
------------------------------------------------------------------------------*/

void InitMapHack()
{
	if(IsWC3Valid())
	{
		MapHackOffsets.push_back(PatchOffset(GameBase + 0x3A15BA, "\xEB", 1));						// MainUnits
		MapHackOffsets.push_back(PatchOffset(GameBase + 0x3999F9, "\x09\xC3", 2));					// Invisible
		MapHackOffsets.push_back(PatchOffset(GameBase + 0x3A14C0, "\xEB\x30\x90\x90", 4));			// Items
		MapHackOffsets.push_back(PatchOffset(GameBase + 0x282A50, "\x09\xC2", 2));					// Illusions
		MapHackOffsets.push_back(PatchOffset(GameBase + 0x38E9F0, "\xA8\xFF", 2));					// Missles
		MapHackOffsets.push_back(PatchOffset(GameBase + 0x04B7D3, "\x90\x90", 2));					// RallyPoints
		MapHackOffsets.push_back(PatchOffset(GameBase + 0x2851B2, "\xEB", 1));						// Clickable Units
		MapHackOffsets.push_back(PatchOffset(GameBase + 0x34F2A8, "\x90\x90", 2));					// Skills #1
		MapHackOffsets.push_back(PatchOffset(GameBase + 0x2026DC, "\x90\x90\x90\x90\x90\x90", 6));	// Skills #2
		MapHackOffsets.push_back(PatchOffset(GameBase + 0x0C838C, "\x0F\x8C\xFC\x00\x00\x00", 6));	// Skills #3
		MapHackOffsets.push_back(PatchOffset(GameBase + 0x34F2E8, "\x90\x90", 2));					// Cooldowns
		MapHackOffsets.push_back(PatchOffset(GameBase + 0x28E1DE, "\xEB\x31", 2));					// Status

		//MapHackOffsets.push_back(PatchOffset(GameBase + 0x425DA0, "\x90\x90", 2));					// Enemy Clicks [bug], 421690 - nevermind just note for me
		//MapHackOffsets.push_back(PatchOffset(GameBase + 0x425DA0, "\x90\x90\x90\x90\x90\x90\x90\x90\x90", 9));					// Enemy Clicks [bug], 421690 - nevermind just note for me
		//MapHackOffsets.push_back(PatchOffset(GameBase + 0x381CC6, "\x90\x90", 2));					// Enemy Clicks [bug], 421690 - nevermind just note for me
		//MapHackOffsets.push_back(PatchOffset(GameBase + 0x424C7C, "\x90\x90", 2));					// Enemy Clicks [bug], 421690 - nevermind just note for me
		//MapHackOffsets.push_back(PatchOffset(GameBase + 0x424D45, "\x90\x90", 2));					// Enemy Clicks [bug], 421690 - nevermind just note for me

		MapHackOffsets.push_back(PatchOffset(GameBase + 0x43EE8B, "\xEB\x24\x90\x90\x90\x90", 6));	// Pings
		MapHackOffsets.push_back(PatchOffset(GameBase + 0x361176, "\xEB", 1));						// Creepdots
		MapHackOffsets.push_back(PatchOffset(GameBase + 0x406B50, "\xE9\xED\x00\x00\x00\x90", 6));	// Ud Blight
		MapHackOffsets.push_back(PatchOffset(GameBase + 0x0EE9A0, "\x90\x90", 2));					// Build Blueprints #1
		MapHackOffsets.push_back(PatchOffset(GameBase + 0x35C0E5, "\x90\x90", 2));					// Build Blueprints #2
		MapHackOffsets.push_back(PatchOffset(GameBase + 0x35FA2B, "\xEB\x1F", 2));					// Clickable Resources
		MapHackOffsets.push_back(PatchOffset(GameBase + 0x049F33, "\x90\x90\x90\x90\x90\x90", 6));	// Buildings #1
		MapHackOffsets.push_back(PatchOffset(GameBase + 0x044A90, "\x90\x90\x90\x90\x90\x90", 6));	// Buildings #2
		MapHackOffsets.push_back(PatchOffset(GameBase + 0x04B5FD, "\x90\x90\x90\x90\x90\x90", 6));	// Buildings #3
		MapHackOffsets.push_back(PatchOffset(GameBase + 0x3C639C, "\xB8", 1));						// Dota -ah Bypass #1
		MapHackOffsets.push_back(PatchOffset(GameBase + 0x3C63A1, "\xEB", 1));						// Dota -ah Bypass #2
		MapHackOffsets.push_back(PatchOffset(GameBase + 0x3CB872, "\xEB", 1));						// Dota -ah Bypass #3
		MapHackOffsets.push_back(PatchOffset(GameBase + 0x3A14DB, "\x71", 1));						// Show Runes

		// backup original
		for(DWORD i = 0; i < MapHackOffsets.size(); i++)
		{
			MapHackOffsets[i].OriginalCode = new BYTE[MapHackOffsets[i].Length];
			for(DWORD j = 0; j < MapHackOffsets[i].Length; j++)
				MapHackOffsets[i].OriginalCode[j] = *((BYTE*)(MapHackOffsets[i].Addr + j));
		}
	}
}

/*------------------------------------------------------------------------------
	InitGameMapHack
------------------------------------------------------------------------------*/

void InitGameMapHack()
{
	switch(CurrentMapHackType)
	{
		case MAPHACK_FOG_STANDARD: PrintText(10, "|cFFAAAAAAMapHack FOG STANDARD|r"); break;
		case MAPHACK_FOG_SHARED:   PrintText(10, "|cFFAAAAAAMapHack FOG SHARED|r"); break;
		case MAPHACK_FOG_NONE:     PrintText(10, "|cFFAAAAAAMapHack FOG NONE|r"); break;
		case MAPHACK_OFF:          PrintText(10, "|cFFAAAAAAMapHack OFF|r"); break;
	}
	PrintText(10, "|cFFAAAAAASafeClick ON|r");
}

/*------------------------------------------------------------------------------
	EnableMapHack
------------------------------------------------------------------------------*/

void EnableMapHack()
{
	if(IsWC3Valid() && MapHackOffsets.size() > 0)
	{
		if(CurrentMapHackType == MAPHACK_OFF)
		{
			for(DWORD i = 0; i < MapHackOffsets.size(); i++)
				PatchMemory((void*)MapHackOffsets[i].Addr, MapHackOffsets[i].Code, MapHackOffsets[i].Length);

			EnableSafeClick();
			CurrentMapHackType = MAPHACK_FOG_STANDARD;
		}
	}
}

/*------------------------------------------------------------------------------
	DisableMapHack
------------------------------------------------------------------------------*/

void DisableMapHack()
{
	if(IsWC3Valid() && MapHackOffsets.size() > 0)
	{
		if(CurrentMapHackType != MAPHACK_OFF)
		{
			// original standard fog
			PatchMemory((void*)(GameBase + 0x74C9F1), "\x8B\x54\x24\x28\x52", 5);
			PatchMemory((void*)(GameBase + 0x3564B8), "\x66\x23\xC2", 3);

			// original map code
			DisableSafeClick();
			for(DWORD i = 0; i < MapHackOffsets.size(); i++)
				PatchMemory((void*)MapHackOffsets[i].Addr, MapHackOffsets[i].OriginalCode, MapHackOffsets[i].Length);

			CurrentMapHackType = MAPHACK_OFF;
		}
	}
}

/*------------------------------------------------------------------------------
	ExitMapHack
------------------------------------------------------------------------------*/

void ExitMapHack()
{
	if(MapHackOffsets.size() > 0)
	{
		DisableMapHack();
		for(DWORD i = 0; i < MapHackOffsets.size(); i++)
			delete[] MapHackOffsets[i].OriginalCode;
		MapHackOffsets.clear();
	}
}

/*------------------------------------------------------------------------------
	ToggleMapHack
------------------------------------------------------------------------------*/
void sstats()
{
	if(IsInGame())
	{	
	GetName(DWORD slot)
	PrintText(30, "/stats" GetName(DWORD slot));
	}
}
void ToggleMapHack()
{
	if(IsInGame())
	{
		switch(CurrentMapHackType)
		{
			case MAPHACK_OFF:
			{
				EnableMapHack();
				PrintText(10, "|cFFAAAAAAMapHack FOG STANDARD|r");
				break;
			}
			case MAPHACK_FOG_STANDARD:
			{
				// shared fog
				PatchMemory((void*)(GameBase + 0x74C9F1), "\x68\xFF\x00\x00\x00", 5);
				PatchMemory((void*)(GameBase + 0x3564B8), "\x66\x21\xC0", 3);
				CurrentMapHackType = MAPHACK_FOG_SHARED;
				PrintText(10, "|cFFAAAAAAMapHack FOG SHARED|r");
				break;
			}
			case MAPHACK_FOG_SHARED:
			{
				// none fog
				PatchMemory((void*)(GameBase + 0x74C9F1), "\x8B\x14\x24\x90\x52", 5);
				PatchMemory((void*)(GameBase + 0x3564B8), "\x66\x09\xD0", 3);
				CurrentMapHackType = MAPHACK_FOG_NONE;
				PrintText(10, "|cFFAAAAAAMapHack FOG NONE|r");
				break;
			}
			case MAPHACK_FOG_NONE:
			{
				DisableMapHack();
				PrintText(10, "|cFFAAAAAAMapHack OFF|r");
				break;
			}
		}
	}
}

/*------------------------------------------------------------------------------
	GetMapHackType
------------------------------------------------------------------------------*/

EMapHackType GetMapHackType()
{
	return CurrentMapHackType;
}
