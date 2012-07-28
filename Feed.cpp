/*==============================================================================
	Feed.cpp

	*created by (c) wc3noobpl (2012)
==============================================================================*/
#include "System.h"
#include "WC3.h"
#include "Feed.h"

// Feed config
#define KEEP_MIN_INTERVAL 3 * 60 * 1000 // keep minimum only first interval of game
#define KEEP_MIN_GOLD 100		// keep if MaxKeeping = false (default)
#define KEEP_MIN_WOOD 50		// keep if MaxKeeping = false (default)
#define KEEP_MAX_GOLD 400		// keep if MaxKeeping = true
#define KEEP_MAX_WOOD 250		// keep if MaxKeeping = true
#define FEED_INTERVAL 10000		// should be same as text uptime
#define FEED_DELAY 3000			// starts feed only after some delay, so you may fast switch without feeding
#define TEXT_UPTIME 10			// default wc3 text delay is 10, in seconds
#define MAX_TARGET_GOLD 1500	// if target player have more than this, delay feed him
#define MAX_TARGET_WOOD 750		// if target player have more than this, delay feed him
#define MIN_TARGET_GOLD 400		// if target player corrupted or high upkeep
#define MIN_TARGET_WOOD 250		// if target player corrupted
#define MAX_ALLYS 16			// for case
#define FEED_COLOR "FFAAAAAA"	// default feed text color string

// Internal types
struct AllyData
{
	DWORD Slot;
	ERace Race;
	DWORD TargetGold;
	DWORD TargetWood;
	DWORD TargetNeedsGold;
	DWORD TargetNeedsWood;
	DWORD FeedGoldCount;
	DWORD FeedWoodCount;
	bool IsCorrupted;
	bool IsHighUpkeep;
};

// Vars
DWORD NextFeedTime = 0;
DWORD CurrentAllyIndex = 0;
DWORD AllyCount = 0;
AllyData Allys[MAX_ALLYS];
EFeedState FeedState = FEEDSTATE_STOPPED;

/*------------------------------------------------------------------------------
	InitGameFeed
------------------------------------------------------------------------------*/

void InitGameFeed()
{
	if(IsInGame())
	{
		// init allys
		AllyCount = 0;
		DWORD count = GetPlayerCount();
		for( DWORD i = 0; i < count; i++ )
		{
			DWORD slot = GetAllySlot(i);
			if(slot == INVALID_SLOT)
				break;

			Allys[AllyCount].Slot = slot;
			Allys[AllyCount].Race = GetRace(slot);
			AllyCount++;
		}
	}
}

/*------------------------------------------------------------------------------
	StopFeed
------------------------------------------------------------------------------*/

void StopFeed()
{
	if(FeedState != FEEDSTATE_STOPPED)
	{
		if(IsInGame() && IsAllys())
			PrintTextFormat(TEXT_UPTIME, "|c%sFeed stopped|r", FEED_COLOR);

		FeedState = FEEDSTATE_STOPPED;
	}
	else
	{
		if(IsInGame() && IsAllys())
			PrintTextFormat(TEXT_UPTIME, "|c%sFeed stopped already|r", FEED_COLOR);
	}
}

/*------------------------------------------------------------------------------
	SwitchFeed
------------------------------------------------------------------------------*/

void SwitchFeed(EFeedState state)
{
	if(IsInGame() && IsAllys())
	{
		if(state == FEEDSTATE_ALL)
		{
			if(FeedState != FEEDSTATE_ALL)
			{
				// start feed all
				PrintTextFormat(TEXT_UPTIME, "|c%sFeed all|r", FEED_COLOR);
				NextFeedTime = timeGetTime() + FEED_DELAY;
				FeedState = state;
			}
			else
				StopFeed();
		}
		else if(state == FEEDSTATE_SINGLE)
		{
			if(FeedState != FEEDSTATE_SINGLE)
				CurrentAllyIndex = 0;
			else if(CurrentAllyIndex + 1 < AllyCount)	// not last ally
				CurrentAllyIndex++;
			else
			{
				StopFeed();
				return;
			}

			// start feed single
			char colorBuffer[16];
			PrintTextFormat(TEXT_UPTIME, "|c%sFeed|r |c%s%s|r", FEED_COLOR, GetColorString(colorBuffer, Allys[CurrentAllyIndex].Slot), GetName(Allys[CurrentAllyIndex].Slot));
			NextFeedTime = timeGetTime() + FEED_DELAY;
			FeedState = state;
		}
	}
}

/*------------------------------------------------------------------------------
	InternalCountFeeds
------------------------------------------------------------------------------*/

void InternalCountFeeds(DWORD* goldCount, DWORD* woodCount, DWORD allyIndex, bool isMinimum, bool isUndead)
{
	// init constants
	DWORD keepGold = isMinimum ? KEEP_MIN_GOLD : KEEP_MAX_GOLD;
	DWORD keepWood = isMinimum ? KEEP_MIN_WOOD : KEEP_MAX_WOOD;
	
	// count available local resources
	DWORD availableGold = (*goldCount) > keepGold ? (*goldCount) - keepGold : 0;
	DWORD availableWood = (*woodCount) > keepWood ? (*woodCount) - keepWood : 0;
	//PrintTextFormat(TEXT_UPTIME, "available = %u/%u", availableGold, availableWood);

	// count needs
	DWORD totalNeedsGold = 0;
	DWORD totalNeedsWood = 0;
	DWORD needersGoldCount = 0;
	DWORD needersWoodCount = 0;
	for(DWORD i = 0; i < AllyCount; i++)
	{
		if(allyIndex == i || (allyIndex == INVALID_SLOT && ((isUndead && Allys[i].Race == RACE_UNDEAD) || (!isUndead && Allys[i].Race != RACE_UNDEAD))))
		{
			DWORD needsGold = Allys[i].IsCorrupted || Allys[i].IsHighUpkeep ? MIN_TARGET_GOLD : MAX_TARGET_GOLD;
			DWORD needsWood = Allys[i].IsCorrupted ? MIN_TARGET_WOOD : MAX_TARGET_WOOD;
			Allys[i].TargetNeedsGold = Allys[i].TargetGold < needsGold ? ConvertGoldToUpkeep(needsGold - Allys[i].TargetGold, Allys[i].Slot) : 0;
			Allys[i].TargetNeedsWood = Allys[i].TargetWood < needsWood ? needsWood - Allys[i].TargetWood : 0;	// wood doesn't afected by upkeep
			//char colorBuffer[16];
			//PrintTextFormat(TEXT_UPTIME, "|c%s%s|r needs = %u/%u", GetColorString(colorBuffer, Allys[i].Slot), GetName(Allys[i].Slot), Allys[i].TargetNeedsGold, Allys[i].TargetNeedsWood);

			// next calls to InternalCountFeeds will think that we filled last needs, if not then available == 0 anyway
			Allys[i].TargetGold = max(Allys[i].TargetGold, needsGold);
			Allys[i].TargetWood = max(Allys[i].TargetWood, needsWood);
			
			// need this for distributing rest available for last needer, to avoid any multiply accuracy
			if(Allys[i].TargetNeedsGold > 0){ totalNeedsGold += Allys[i].TargetNeedsGold; needersGoldCount++; }
			if(Allys[i].TargetNeedsWood > 0){ totalNeedsWood += Allys[i].TargetNeedsWood; needersWoodCount++; }
		}
	}

	// distribute gold needs
	if(totalNeedsGold > availableGold)
	{
		double multiplier = (double)availableGold / (double)totalNeedsGold;
		for(DWORD i = 0; i < AllyCount; i++)
		{
			if(Allys[i].TargetNeedsGold > 0 && (allyIndex == i || (allyIndex == INVALID_SLOT && ((isUndead && Allys[i].Race == RACE_UNDEAD) || (!isUndead && Allys[i].Race != RACE_UNDEAD)))))
			{
				if(needersGoldCount > 1)
				{
					Allys[i].TargetNeedsGold = (DWORD)((double)Allys[i].TargetNeedsGold * multiplier);
					availableGold -= Allys[i].TargetNeedsGold;
					needersGoldCount--;
				}
				else
					Allys[i].TargetNeedsGold = availableGold;	// last gets rest available
			}
		}
	}

	// distribute wood needs
	if(totalNeedsWood > availableWood)
	{
		double multiplier = (double)availableWood / (double)totalNeedsWood;
		for(DWORD i = 0; i < AllyCount; i++)
		{
			if(Allys[i].TargetNeedsWood > 0 && (allyIndex == i || (allyIndex == INVALID_SLOT && ((isUndead && Allys[i].Race == RACE_UNDEAD) || (!isUndead && Allys[i].Race != RACE_UNDEAD)))))
			{
				if(needersWoodCount > 1)
				{
					Allys[i].TargetNeedsWood = (DWORD)((double)Allys[i].TargetNeedsWood * multiplier);
					availableWood -= Allys[i].TargetNeedsWood;
					needersWoodCount--;
				}
				else
					Allys[i].TargetNeedsWood = availableWood;	// last gets rest available
			}
		}
	}

	// add to feeds, subtract from local gold/wood
	for(DWORD i = 0; i < AllyCount; i++)
	{
		if(allyIndex == i || (allyIndex == INVALID_SLOT && ((isUndead && Allys[i].Race == RACE_UNDEAD) || (!isUndead && Allys[i].Race != RACE_UNDEAD))))
		{
			Allys[i].FeedGoldCount += Allys[i].TargetNeedsGold;
			Allys[i].FeedWoodCount += Allys[i].TargetNeedsWood;
			*goldCount -= Allys[i].TargetNeedsGold;
			*woodCount -= Allys[i].TargetNeedsWood;
		}
	}
}

/*------------------------------------------------------------------------------
	FeedSmart
------------------------------------------------------------------------------*/

void FeedSmart(DWORD allyIndex, DWORD currentTime)
{
	// init text constants
	char colorBuffer[16];
	const char* colorStr = allyIndex == INVALID_SLOT ? FEED_COLOR : GetColorString(colorBuffer, Allys[allyIndex].Slot);
	const char* allStr = allyIndex == INVALID_SLOT ? " all" : "";

	// init temporary info
	bool isAllHighUpkeep = true;
	bool isAllCorrupted = true;
	bool isAllFullGold = true;
	bool isAllFullWood = true;
	for( DWORD i = 0; i < AllyCount; i++ )
	{
		if(allyIndex == INVALID_SLOT || allyIndex == i)
		{
			Allys[i].IsHighUpkeep = IsHighUpkeep(Allys[i].Slot);
			Allys[i].TargetGold = GetGold(Allys[i].Slot);
			Allys[i].TargetWood = GetWood(Allys[i].Slot);
			//PrintTextFormat(TEXT_UPTIME, "|c%s%s|r total = %u/%u", GetColorString(colorBuffer, Allys[i].Slot), GetName(Allys[i].Slot), Allys[i].TargetGold, Allys[i].TargetWood);

			// any not high upkeep
			if(!Allys[i].IsHighUpkeep)
				isAllHighUpkeep = false;

			// any not corrupted
			DWORD minSupply = GetMinSupply(Allys[i].Slot);
			DWORD maxSupply = GetMaxSupply(Allys[i].Slot);
			Allys[i].IsCorrupted = minSupply <= 5 || minSupply > maxSupply || maxSupply == 0;
			DWORD totalNeedsGold = Allys[i].IsCorrupted || Allys[i].IsHighUpkeep ? MIN_TARGET_GOLD : MAX_TARGET_GOLD;
			DWORD totalNeedsWood = Allys[i].IsCorrupted ? MIN_TARGET_WOOD : MAX_TARGET_WOOD;
			if(!Allys[i].IsCorrupted)
				isAllCorrupted = false;

			// any not full gold
			if(Allys[i].TargetGold < totalNeedsGold)
				isAllFullGold = false;

			// any not full wood
			if(Allys[i].TargetWood < totalNeedsWood)
				isAllFullWood = false;
		}
	}

	// check feed conditions
	if(isAllFullGold)
	{
		if(isAllFullWood)
		{
			PrintTextFormat(TEXT_UPTIME, "|c%sFull%s...|r", colorStr, allStr);
			return;
		}
		else
			PrintTextFormat(TEXT_UPTIME, "|c%sNeed wood%s...|r", colorStr, allStr);
	}
	else if(isAllHighUpkeep)
	{
		PrintTextFormat(TEXT_UPTIME, "|c%sHigh upkeep%s...|r", colorStr, allStr);
	}
	/*
	else if(IsLagScreen())
	{
		// @todo: doesn't detect lag screen properly
		PrintTextFormat(TEXT_UPTIME, "|c%sLag screen%s...|r", colorStr, allStr);
		return;
	}
	/**/
	else
		PrintTextFormat(TEXT_UPTIME, "|c%sFeeding%s...|r", colorStr, allStr);

	// count feeds
	DWORD goldCount = GetGold(GetLocalPlayerSlot());
	DWORD woodCount = GetWood(GetLocalPlayerSlot());
	bool isMinimum = currentTime - GameStartTime < KEEP_MIN_INTERVAL;
	if(allyIndex == INVALID_SLOT)
	{
		InternalCountFeeds(&goldCount, &woodCount, allyIndex, isMinimum, true);		// undead priority
		InternalCountFeeds(&goldCount, &woodCount, allyIndex, isMinimum, false);	// other races
	}
	else
	{
		InternalCountFeeds(&goldCount, &woodCount, allyIndex, isMinimum, false);	// specified ally
	}

	// feed if any
	for( DWORD i = 0; i < AllyCount; i++ )
	{
		if(allyIndex == INVALID_SLOT || allyIndex == i)
		{
			if(Allys[i].FeedGoldCount > 0 || Allys[i].FeedWoodCount > 0)
			{
				//PrintTextFormat(TEXT_UPTIME, "|c%s%s|r feed = %u/%u", GetColorString(colorBuffer, Allys[i].Slot), GetName(Allys[i].Slot), Allys[i].FeedGoldCount, Allys[i].FeedWoodCount);
				Feed(Allys[i].Slot, Allys[i].FeedGoldCount, Allys[i].FeedWoodCount);
				Allys[i].FeedGoldCount = 0;
				Allys[i].FeedWoodCount = 0;
			}
		}
	}
}

/*------------------------------------------------------------------------------
	UpdateFeed
------------------------------------------------------------------------------*/

void UpdateFeed(DWORD currentTime)
{
	if(IsInGame() && IsAllys())
	{
		// update feed
		if(FeedState != FEEDSTATE_STOPPED && NextFeedTime <= currentTime)
		{
			if(FeedState == FEEDSTATE_SINGLE)
				FeedSmart(CurrentAllyIndex, currentTime);
			else if(FeedState == FEEDSTATE_ALL)
				FeedSmart(INVALID_SLOT, currentTime);
			
			NextFeedTime = currentTime + FEED_INTERVAL;
		}
	}
}
