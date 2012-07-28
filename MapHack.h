/*==============================================================================
	Maphack.h
	
	*created by (c) wc3noobpl (2012)
==============================================================================*/
#pragma once

/*------------------------------------------------------------------------------
	Types
------------------------------------------------------------------------------*/

enum EMapHackType
{
	MAPHACK_OFF,
	MAPHACK_FOG_STANDARD,
	MAPHACK_FOG_SHARED,
	MAPHACK_FOG_NONE,
};

/*------------------------------------------------------------------------------
	Methods
------------------------------------------------------------------------------*/
void sstats();
void InitMapHack();
void InitGameMapHack();
void EnableMapHack();
void ToggleMapHack();
EMapHackType GetMapHackType();
void DisableMapHack();
void ExitMapHack();
