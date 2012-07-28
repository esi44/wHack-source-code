/*==============================================================================
	WC3Ptrs.h for version 1.26a

	*created by (c) wc3noobpl (2012) for 1.26a
	*originally based on Sheppard (1.22a) sources
==============================================================================*/

#ifdef _DEFINE_VARS
// declared in WC3.cpp
#define FUNCPTR(name,type,params,offset)	name##_t* name = NULL;
#define VARPTR(name,type,offset)			name##_t* name = NULL;
#else
#ifdef _SET_GAMEBASE
// set on init, used by InitWC3Ptrs
#define FUNCPTR(name,type,params,offset)	name = (name##_t*)((DWORD)_SET_GAMEBASE + (DWORD)offset);
#define VARPTR(name,type,offset)			name = (name##_t*)((DWORD)_SET_GAMEBASE + (DWORD)offset);
#else
// declared in WC3.h
#define FUNCPTR(name,type,params,offset)	typedef type name##_t params;	extern name##_t* name;
#define VARPTR(name,type,offset)			typedef type name##_t;			extern name##_t* name;
#endif
#endif

// Functions
FUNCPTR(GAME_GetPtrList, DWORD __fastcall, (VOID), 0x5FAEC0)
FUNCPTR(GAME_Print, void __fastcall, (DWORD ptrList, DWORD _EDX, CHAR* text, DWORD* color, DWORD stayUpTime, DWORD _1), 0x6049B0)
FUNCPTR(GAME_PrintChat, void __fastcall, (DWORD chatPtrList, DWORD _1, DWORD slot, CHAR* text, DWORD printType, DWORD stayUpTime), 0x2FB480)
FUNCPTR(GAME_GetPlayerColor, DWORD* __fastcall, (DWORD slot), 0x314B60)
FUNCPTR(GAME_GetPlayerName, CHAR* __fastcall, (DWORD slot), 0x2F8F90)
FUNCPTR(GAME_SendPacket, void __fastcall, (Packet* packet, DWORD zero), 0x54D970)
FUNCPTR(GAME_GetLumber, DWORD __fastcall, (Player* playerPtr), 0x340A80)
FUNCPTR(GAME_GetGold, DWORD __fastcall, (Player* playerPtr), 0x340A60)
FUNCPTR(GAME_GetMaxSupply, DWORD __fastcall, (Player* playerPtr), 0x40F850)
FUNCPTR(GAME_GetMinSupply, DWORD __fastcall, (Player* playerPtr), 0x40F5D0)
FUNCPTR(GAME_IsUnitVisible, bool __fastcall, (Unit* unitPtr, DWORD playerSlot, DWORD team, DWORD _4), 0x285080) // used by Safeclick

// Vars
VARPTR(GAME_GameInfo, GameInfo*, 0xAB65F4)
VARPTR(GAME_PacketClassPtr, void, 0x932D2C)	// used by send packet
VARPTR(GAME_ChatPtrList, DWORD, 0xAB4F80)	// used by print chat
//VARPTR(GAME_IsLagScreen, BYTE, 0xACF668)	// [not working sometimes, donno why]
VARPTR(GAME_InGame1, DWORD, 0xAB5738)		// based on YourName offset
VARPTR(GAME_InGame2, DWORD, 0xAB573C)		// based on YourName offset
VARPTR(GAME_GameState, DWORD, 0xACE638)		// based on YourName offset
VARPTR(GAME_UnitArray, DWORD, 0xACE66C)		// used by Safeclick
VARPTR(GAME_GameState2, DWORD, 0xA8B528)
VARPTR(GAME_PrintChat_I1, void, 0x2FB72F)
VARPTR(GAME_PrintChat_I2, void, 0x2FC981)
VARPTR(GAME_SendPacket_I, void, 0x2C9CC7)	// called twice select + unselect
//VARPTR(GAME_SendPacket_I2, void, 0x2C9FC7)	// called once on select
//VARPTR(GAME_SendPacket_I3, void, 0x2C9F07)	// called once on select

// 0888FA98 - is lag player

#undef FUNCPTR
#undef VARPTR
