/*==============================================================================
	WC3.cpp
	
	*created by (c) wc3noobpl (2012)
	*originally based on Sheppard sources
	*also based on YourName sources (IsInGame, stayUpTime)
==============================================================================*/
#include "System.h"
#include "WC3.h"

// Link extern vars
#define _DEFINE_VARS
#include "WC3Ptrs.h"
#undef _DEFINE_VARS

// Vars
DWORD GameBase = NULL;
DWORD GameVersion = 0;		// 0x1A for 1.26a (low word == number, high word == letter)
DWORD GameStartTime = 0;	// initialized by main module, for now

/*------------------------------------------------------------------------------
	InitWC3Ptrs
------------------------------------------------------------------------------*/

void InitWC3Ptrs()
{
	GameBase = (DWORD)GetModuleHandle("Game.dll");

	// init ptrs before calling other wc3 functions
	#define _SET_GAMEBASE GameBase
	#include "WC3Ptrs.h"
	#undef _SET_GAMEBASE

	// init wc3 version
	DWORD  verHandle = NULL;
	UINT   size      = 0;
	LPBYTE lpBuffer  = NULL;
	DWORD  verSize   = GetFileVersionInfoSize("Game.dll", &verHandle);
	if(verSize != NULL)
	{
		LPSTR verData = new char[verSize];
		if(GetFileVersionInfo("Game.dll", verHandle, verSize, verData))
		{
			if(VerQueryValue(verData, "\\", (VOID FAR* FAR*)&lpBuffer, &size))
			{
				if(size)
				{
					VS_FIXEDFILEINFO* verInfo = (VS_FIXEDFILEINFO*) lpBuffer;
					if(verInfo->dwSignature == 0xfeef04bd)
					{
						// example for 1.26.0.6401
						DWORD majorMS = HIWORD(verInfo->dwFileVersionMS);	// 1
						DWORD minorMS = LOWORD(verInfo->dwFileVersionMS);	// 26
						DWORD majorLS = HIWORD(verInfo->dwFileVersionLS);	// 0
						DWORD minorLS = LOWORD(verInfo->dwFileVersionLS);	// 6401
						
						// we need only middle numbers 26.0
						GameVersion = minorMS | majorLS << 16;
					}
				}
			}
		}
	}
}

/*------------------------------------------------------------------------------
	IsWC3Valid
------------------------------------------------------------------------------*/

bool IsWC3Valid()
{
	// pointers inited and version match
	return GameBase && GameVersion == 26;	// 1.26a
}

/*------------------------------------------------------------------------------
	IsInGame
------------------------------------------------------------------------------*/

bool IsInGame()
{
	return (IsWC3Valid() && (*GAME_GameInfo) && *GAME_InGame1 == 4 && *GAME_InGame2 == 4);
}

/*------------------------------------------------------------------------------
	GetGameState
------------------------------------------------------------------------------*/

EGameState GetGameState()
{
	return (EGameState)(*GAME_GameState);
}

/*------------------------------------------------------------------------------
	GetGameState2
------------------------------------------------------------------------------*/

EGameState2 GetGameState2()
{
	// works much better for main menu than game state 1,
	// game state 1 returns main menu when game loading.
	return (EGameState2)(*GAME_GameState2);
}

/*------------------------------------------------------------------------------
	GetPlayerCount
------------------------------------------------------------------------------*/

DWORD GetPlayerCount()
{
	if(!IsInGame())
		return 0;

	return (*GAME_GameInfo)->PlayerCount;
}

/*------------------------------------------------------------------------------
	GetLocalPlayerSlot
------------------------------------------------------------------------------*/

DWORD GetLocalPlayerSlot()
{
	if(!IsInGame())
		return INVALID_SLOT;

	return (*GAME_GameInfo)->PlayerSlot;
}

/*------------------------------------------------------------------------------
	GetPlayer
------------------------------------------------------------------------------*/

Player* GetPlayer(DWORD slot)
{
	if(!IsInGame())
		return NULL;

	if((*GAME_GameInfo) && (*GAME_GameInfo)->PlayerCount > slot)
		return (*GAME_GameInfo)->PlayerList[slot];

	return NULL;
}

/*------------------------------------------------------------------------------
	GetLocalPlayer
------------------------------------------------------------------------------*/

Player* GetLocalPlayer()
{
	return GetPlayer(GetLocalPlayerSlot());
}

/*------------------------------------------------------------------------------
	PrintText
------------------------------------------------------------------------------*/

void PrintText(float stayUpTime, char* message)
{
	// note: stayUpTime is in seconds (==0 means stay forever, ==10 default)

	if(!IsInGame())
		return;

	DWORD ptrList = GAME_GetPtrList();
	if(!ptrList)
		return;

	// *basicly taken from Sheppard PrintText_ASM and translated to C++,
	// didn't research what exactly means offset 0x3EC, possibly its chatPtrList or smth,
	// it's just working and thats all (c) wc3noobpl.
	static DWORD color = 0xFFFFFFFF;	// (c) Dennis
	GAME_Print(*((DWORD*)(ptrList + 0x3EC)), ptrList, message, &color, *((DWORD*)&stayUpTime), NULL);
}

/*------------------------------------------------------------------------------
	PrintTextFormat
------------------------------------------------------------------------------*/

void PrintTextFormat(float stayUpTime, char* format, ...)
{
	char str[8192] = {0};
	va_list args;

	va_start(args, format);
	vsprintf(str, format, args);
	va_end(args);

	PrintText(stayUpTime, str);
}

/*------------------------------------------------------------------------------
	PrintChat
------------------------------------------------------------------------------*/

void PrintChat(float stayUpTime, EChatRecipient msgType, char* message)
{
	// note: stayUpTime is in seconds (==0 means stay forever, ==10 default)

	if(!IsInGame())
		return;

	GAME_PrintChat(*GAME_ChatPtrList, 0, GetLocalPlayerSlot(), message, (DWORD)msgType, *((DWORD*)&stayUpTime));
}

/*------------------------------------------------------------------------------
	PrintChatFormat
------------------------------------------------------------------------------*/

void PrintChatFormat(float stayUpTime, EChatRecipient msgType, char* format, ...)
{
	char str[8192] = {0};
	va_list args;

	va_start(args, format);
	vsprintf(str, format, args);
	va_end(args);

	PrintChat(stayUpTime, msgType, str);
}

/*------------------------------------------------------------------------------
	SendPacket
------------------------------------------------------------------------------*/

void SendPacket(BYTE* packetData, DWORD size)
{
	// @warning: this function thread-unsafe, do not use it in other thread.
	// note: this is very useful function, in fact this function
	// does wc3 ingame action, so you can use it for anything you want,
	// including unit commands and and gameplay commands,
	// i suppose its wc3 single action W3GS_INCOMING_ACTION (c) wc3noobpl.

	if(!IsInGame())
		return;

	Packet packet;
	memset(&packet, 0, sizeof(Packet));

	packet.PacketClassPtr = (DWORD)GAME_PacketClassPtr; // Packet Class
	packet.PacketData = packetData;
	packet.Size = size;
	packet._3 = 0xFFFFFFFF;
	GAME_SendPacket(&packet, 0);
}

/*------------------------------------------------------------------------------
	Player Info
------------------------------------------------------------------------------*/


const string GetName(DWORD slot)
{
        if(!IsInGame())
                return string();
 
        return string(GAME_GetPlayerName(slot));
}
ERace GetRace(DWORD slot)
{
	if(IsInGame())
	{
		Player* player = GetPlayer(slot);
		if(player != NULL)
			return (ERace)player->Race;
	}
	return RACE_UNKNOWN;
}
const char* GetRaceString(DWORD slot)
{
	ERace race = GetRace(slot);
	switch(race)
	{
		case RACE_HUMAN: return "HUM";
		case RACE_ORC: return "ORC";
		case RACE_NIGHTELVE: return "ELF";
		case RACE_UNDEAD: return "UD";
	}
	return "UNKNOWN";
}
DWORD GetGold(DWORD slot)
{
	if(!IsInGame())
		return 0;

	return GAME_GetGold(GetPlayer(slot));
}
DWORD GetWood(DWORD slot)
{
	if(!IsInGame())
		return 0;

	return GAME_GetLumber(GetPlayer(slot));
}
DWORD GetMinSupply(DWORD slot)
{
	if(!IsInGame())
		return 0;

	return GAME_GetMinSupply(GetPlayer(slot));
}
DWORD GetMaxSupply(DWORD slot)
{
	if(!IsInGame())
		return 0;

	return GAME_GetMaxSupply(GetPlayer(slot));
}
DWORD GetColor(DWORD slot)
{
	if(!IsInGame())
		return 0;

	Player* player = GetPlayer(slot);
	return player != NULL ? *GAME_GetPlayerColor(slot) : 0;
}
const char* GetColorString(char* outStr, DWORD slot)
{
	// @warning: make sure outStr have size >= 8 + 1 for null terminating
	DWORD color = GetColor(slot);
	sprintf(outStr, "%02X%02X%02X%02X", ((color & 0xFF000000)>>24), ((color & 0x00FF0000)>>16), ((color & 0x0000FF00)>>8), (color & 0x000000FF));
	return outStr;
}

/*------------------------------------------------------------------------------
	Feed
------------------------------------------------------------------------------*/

void Feed(DWORD toPlayerSlot, DWORD feedGoldCount, DWORD feedWoodCount)
{
	// @warning: this function doesn't work through thread, its not thread-safe,
	// use windows message hook instead (c) wc3noobpl.
	
	if(!IsInGame())
		return;

	if(feedGoldCount > 0 || feedWoodCount > 0)
	{
		// dont give more than you have
		DWORD feedGold = min(GetGold(GetLocalPlayerSlot()), feedGoldCount);
		DWORD feedWood = min(GetWood(GetLocalPlayerSlot()), feedWoodCount);
		if(feedGold > 0 || feedWood > 0)
		{
			BYTE packet[10] = {0x51,(BYTE)toPlayerSlot};
			*((DWORD*)(&packet[2])) = feedGold;
			*((DWORD*)(&packet[6])) = feedWood;
			SendPacket(packet, 10);
		}
	}
}

/*------------------------------------------------------------------------------
	IsAlly
------------------------------------------------------------------------------*/

bool IsAlly(DWORD slot)
{
	if(slot < GetPlayerCount())
	{
		Player* localPlayer = GetLocalPlayer();
		Player* player = GetPlayer(slot);
		if(localPlayer->Team == player->Team)
			return true;
	}
	return false;
}

/*------------------------------------------------------------------------------
	GetAllySlot
------------------------------------------------------------------------------*/

DWORD GetAllySlot(DWORD allyIndex)
{
	DWORD playerCount = GetPlayerCount();
	Player* localPlayer = GetLocalPlayer();
	if(localPlayer != NULL)
	{
		DWORD currentAllyIndex = 0;
		for(DWORD i = 0; i < playerCount; i++)
		{
			Player* player = GetPlayer(i);
			if(player != NULL && player != localPlayer && player->Team == localPlayer->Team)
			{
				if(currentAllyIndex == allyIndex)
					return player->Slot;

				currentAllyIndex++;
			}
		}
	}
	return INVALID_SLOT;	// not found
}

/*------------------------------------------------------------------------------
	IsHighUpkeep
------------------------------------------------------------------------------*/

bool IsHighUpkeep(DWORD slot)
{
	return GetMinSupply(slot) > 80;
}

/*------------------------------------------------------------------------------
	IsLowOrHighUpkeep
------------------------------------------------------------------------------*/

bool IsLowOrHighUpkeep(DWORD slot)
{
	return GetMinSupply(slot) > 50;
}

/*------------------------------------------------------------------------------
	ConvertGoldToUpkeep
------------------------------------------------------------------------------*/

DWORD ConvertGoldToUpkeep(DWORD goldCount, DWORD slot)
{
	return IsHighUpkeep(slot) ? (DWORD)((double)goldCount / 0.4) : (IsLowOrHighUpkeep(slot) ? (DWORD)((double)goldCount / 0.7) : goldCount);
}

/*------------------------------------------------------------------------------
	ConvertGoldToLowUpkeep
------------------------------------------------------------------------------*/

DWORD ConvertGoldToLowUpkeep(DWORD goldCount, DWORD slot)
{
	// converts to mid for both mid and high supply
	return IsLowOrHighUpkeep(slot) ? (DWORD)((double)goldCount / 0.7) : goldCount;
}

/*------------------------------------------------------------------------------
	IsAllys
------------------------------------------------------------------------------*/

bool IsAllys()
{
	return GetAllySlot(0) != INVALID_SLOT ? true : false;
}

/*------------------------------------------------------------------------------
	IsLagScreen
------------------------------------------------------------------------------*/
/*
bool IsLagScreen()
{
	// @todo: not work properly, donno why.

	if(!IsInGame())
		return false;

	return (*GAME_IsLagScreen) != 0 ? true : false;
}

/*------------------------------------------------------------------------------
	Intercept helpers
------------------------------------------------------------------------------*/

bool PatchMemory(void* addr, void* data, DWORD size)
{
	DWORD oldProtect;
	if(!VirtualProtect(addr, size, PAGE_READWRITE, &oldProtect))
		return false;

	memcpy(addr, data, size);
	return VirtualProtect(addr, size, oldProtect, &oldProtect) ? true : false;
}
bool InterceptCall(DWORD instAddr, DWORD func)
{
	// asm code uses offsets from next position instead of direct address,
	// 5 is current CALL instruction code length (instID + funcOffset).

	// ensure we have CALL instruction here
	DWORD funcOffset = func - (instAddr + 5);
	if(*((BYTE*)instAddr) == 0xE8)
		return PatchMemory((void*)(instAddr + 1), &funcOffset, 4);

	return false;
}
bool InterceptCallSafe(DWORD instAddr, DWORD oldFunc, DWORD func)
{
	// ensure we have old func offset here, to prevent wrong patch
	DWORD oldFuncOffset = oldFunc - (instAddr + 5);
	if(*((DWORD*)(instAddr + 1)) == oldFuncOffset)
		return InterceptCall(instAddr, func);

	return false;
}
