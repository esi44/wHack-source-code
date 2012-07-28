/*==============================================================================
	WC3.h
	
	*created by (c) wc3noobpl (2012)
	*originally based on Sheppard sources
	*also base on YourName sources (GameState)
==============================================================================*/
#pragma once

#define INVALID_SLOT ((DWORD)(-1))

/*------------------------------------------------------------------------------
	Types
------------------------------------------------------------------------------*/

enum EGameState : DWORD
{
	GSTATE_LOADING_CUSTOM_GAME_BNET		= 14,
	GSTATE_LOADING_CUSTOM_GAME_SP		= 14,
	GSTATE_LOADING_SAVED_GAME_LAN		= 23,
	GSTATE_LOADING_CUSTOM_GAME_LAN		= 23,
	GSTATE_LOADIND_REGULAR_GAME_BNET	= 26,
	GSTATE_LOADING_REGULAR_GAME_LAN		= 26,
	GSTATE_LOADING_REGULAR_GAME_SP		= 26,
	GSTATE_MAIN_MENU_WC					= 38,
	GSTATE_MAIN_MENU_SP					= 47,
	GSTATE_MAIN_MENU_LAN				= 63,
	GSTATE_CHOOSE_MAP_LAN				= 64,
	GSTATE_SCORE_SCREEN_WC				= 65,
	GSTATE_HOST_LOBBY_BNET				= 85,
	GSTATE_HOST_LOBBY_LAN				= 85,
	GSTATE_CHOOSE_MAP_SP				= 93,
	GSTATE_LOGON_SCREEN_BNET			= 107,
	GSTATE_OPTION_MENU_WC				= 119,
	GSTATE_MAIN_SCREEN_BNET				= 197,
	GSTATE_CHAT_SCREEN_BNET				= 197,
	GSTATE_JOIN_LOBBY_BNET				= 229,
	GSTATE_JOIN_LOBBY_LAN				= 229,
	GSTATE_CHOOSE_MAP_BNET				= 267,
	GSTATE_CUSTOM_GAMES_LIST_BNET		= 269,
	GSTATE_AT_SCREEN_BNET				= 291,
	GSTATE_INGAME_SP					= 388,
	GSTATE_WATCHING_REPLAY_2_SP			= 395,
	GSTATE_INGAME_SAVED_SP				= 400,
	GSTATE_INGAME_BNET					= 404,
	GSTATE_INGAME_LAN					= 404,
	GSTATE_INGAME_2_SP					= 410,
	GSTATE_INGAME_SAVED_BNET			= 416,
	GSTATE_INGAME_SAVED_LAN				= 416,
	GSTATE_AFTER_MODE_SP				= 438,
	GSTATE_WATCHING_REPLAY_SP			= 445,
	GSTATE_AFTER_MODE_BNET				= 454,
	GSTATE_AFTER_MODE_LAN				= 454,
	GSTATE_VIEWING_PROFILE_BNET			= 460,
	GSTATE_VIEWING_PROFILE_2_BNET		= 464,
	GSTATE_INGAME_3_BNET				= 476,
	GSTATE_INGAME_2_LAN					= 500,
	GSTATE_INGAME_2_BNET				= 508
};

enum EGameState2 : DWORD
{
	GSTATE2_MAIN_MENU_WC = 24
};

enum EChatRecipient : DWORD
{
	CHAT_RECIPIENT_ALL			= 0,
	CHAT_RECIPIENT_ALLIES		= 1,
	CHAT_RECIPIENT_OBSERVERS	= 2,
	CHAT_RECIPIENT_REFEREES	    = 2,
	CHAT_RECIPIENT_PRIVATE		= 3		// basicly this is used
};

enum ERace : DWORD
{
	RACE_UNKNOWN = 0,
	RACE_HUMAN,
	RACE_ORC,
	RACE_UNDEAD,
	RACE_NIGHTELVE,
};

#pragma pack(push,1)

// total size = 0x430
struct Player
{
	BYTE _1[0x30];			//0x00
	WORD Slot;				//0x30
	WORD _2;				//0x32
	DWORD PtrList;			//0x34, Necessary to get ahold of the current selected units!
	BYTE _3[0x228];			//0x38
	DWORD Race;				//0x260
	DWORD ColorIndex;		//0x264
	BYTE _4[0x10];			//0x268
	DWORD Team;				//0x278
};

struct GameInfo
{
    DWORD PtrList;			//0x00, PtrList means pointer to some unknown list of pointers
    DWORD _1[0x9];			//0x24, _1 _2 _3 ... means unknown skipped data
    WORD PlayerSlot;		//0x28 
	WORD _2;				//0x30
    DWORD _3[0x6];			//0x40 
    DWORD MaxPlayerCount;	//0x44 
    DWORD _4;				//0x48 
    DWORD PlayerCount;		//0x4C 
    DWORD _5[0x2];			//0x50 
    Player* PlayerList[15];	//0x58 
};

struct Packet
{
	DWORD PacketClassPtr;	//+00, some unknown, but needed, Class Pointer
	BYTE* PacketData;		//+04
	DWORD _1;				//+08, zero
	DWORD _2;				//+0C, ??
	DWORD Size;				//+10, size of PacketData
	DWORD _3;				//+14, 0xFFFFFFFF
};

struct UnitInfo
{
	DWORD Ptr;				//0x0
	DWORD _1[3];			//0x4
    DWORD XTargetCoord;		//0x8 <- Not sure about these two, but they only appear when a unit is moving
	DWORD YTargetCoord;		//0xC more investigation is needed!
	BYTE _2[0x24];
	DWORD _3[2];			// 0x30
	DWORD MapX;				// 0x34
	DWORD MapY;				// 0x38
};

struct Unit
{
	DWORD _1[3];			//0x00
	DWORD ID1;				//0xC
	DWORD ID2;				//0x10
	BYTE _2[0x1C];			//0x2C (0x28 -> Unit Type ..?)
	DWORD ClassId;			//0x30
	BYTE _3[0x24];			//0x54
	DWORD OwnerSlot;		//0x58
	BYTE _4[0x108];			//0x5C
	//1A0
	UnitInfo Info;			//0x164
};

#pragma pack(pop)

/*------------------------------------------------------------------------------
	Includes
------------------------------------------------------------------------------*/

#include "WC3Ptrs.h"

/*------------------------------------------------------------------------------
	Vars
------------------------------------------------------------------------------*/

extern DWORD GameBase;
extern DWORD GameVersion;
extern DWORD GameStartTime;

/*------------------------------------------------------------------------------
	Methods
------------------------------------------------------------------------------*/

void InitWC3Ptrs();
bool IsWC3Valid();
bool IsInGame();
EGameState GetGameState();
EGameState2 GetGameState2();
DWORD GetPlayerCount();
DWORD GetLocalPlayerSlot();
Player* GetPlayer(DWORD slot);
Player* GetLocalPlayer();
void PrintText(float stayUpTime, char* message);
void PrintTextFormat(float stayUpTime, char* format, ...);
void PrintChat(float stayUpTime, EChatRecipient msgType, char* message);
void PrintChatFormat(float stayUpTime, EChatRecipient msgType, char* format, ...);
void SendPacket(BYTE* packetData, DWORD size);
const char* GetName(DWORD slot);
ERace GetRace(DWORD slot);
DWORD GetColor(DWORD slot);
const char* GetColorString(char* outStr, DWORD slot);
DWORD GetGold(DWORD slot);
DWORD GetWood(DWORD slot);
DWORD GetMinSupply(DWORD slot);
DWORD GetMaxSupply(DWORD slot);

// Feed helpers
void Feed(DWORD toPlayerSlot, DWORD feedGoldCount, DWORD feedWoodCount);
bool IsAlly(DWORD slot);
DWORD GetAllySlot(DWORD allyIndex);
bool IsHighUpkeep(DWORD slot);
bool IsLowOrHighUpkeep(DWORD slot);
DWORD ConvertGoldToUpkeep(DWORD goldCount, DWORD slot);
DWORD ConvertGoldToLowUpkeep(DWORD goldCount, DWORD slot);
bool IsAllys();
//bool IsLagScreen();

// Intercept helpers
bool PatchMemory(void* addr, void* data, DWORD size);
bool InterceptCall(DWORD instAddr, DWORD func);
bool InterceptCallSafe(DWORD instAddr, DWORD oldFunc, DWORD func);
