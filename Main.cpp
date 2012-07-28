/*==============================================================================
	Main.cpp

	Note: have fun with #ifdefs,
	used instead of usual ifs only for lesser binary sizes (c) wc3noobpl.

	*created by (c) wc3noobpl (2012)
==============================================================================*/
#include "System.h"
#include "WC3.h"
#include "CloakDLL.h"
#include "MapHack.h"
#include "Mute.h"
#include "Feed.h"
#include "SafeClick.h"

// Defines
#define IS_KEY_DOWN(lParam) ((lParam & ((DWORD)1<<30)) == 0 && (lParam & ((DWORD)1<<31)) == 0)

// Config
#define IS_MAPHACK	1
#define IS_MUTE		1
#define IS_FEEDBOT	1
#define IS_sstats 1

// Vars
//HANDLE Thread = NULL;
HHOOK KeyboardHook = NULL;
HHOOK MessageHook = NULL;			// we use it instead of another thread, so we stay syncronized with wc3
bool GameStarted = false;
bool Injected = false;				// injected only in battle.net or lan
//bool IsExitThread = false;

/*------------------------------------------------------------------------------
	InitGame
------------------------------------------------------------------------------*/

void InitGame(DWORD currentTime)
{
#if(IS_MAPHACK && IS_FEEDBOT && IS_MUTE)
	PrintText(30, "|cFFFFFF00WC3 wHack v1.3|r (F1/F2/F3/F5/F6)");
#else
#if(IS_FEEDBOT)
	PrintText(30, "|cFFFFFF00WC3 FeedBot v2.2|r (F1/F2/F3)");
#endif
#if(IS_MAPHACK)
	PrintText(30, "|cFFFFFF00WC3 MapHack v1.2|r (F5)");
#endif
#if(IS_MUTE)
	PrintText(30, "|cFFFFFF00WC3 Mute v1.3|r (F6)");
#endif
#endif

#if(IS_FEEDBOT)
	InitGameFeed();
#endif
#if(IS_MAPHACK)
	InitGameMapHack();
#endif
#if(IS_MUTE)
	InitGameMute();
#endif
}

/*------------------------------------------------------------------------------
	ExitGame
------------------------------------------------------------------------------*/

void ExitGame()
{
#if(IS_FEEDBOT)
	StopFeed();
#endif
}

/*------------------------------------------------------------------------------
	Update
------------------------------------------------------------------------------*/

void Update(DWORD currentTime)
{
	if(!Injected)
	{
		// cloak game dll, helps for anti-warden protection in 1.26a.
		if(IsWC3Valid() && !Injected && (GetGameState() == GSTATE_LOGON_SCREEN_BNET || GetGameState() == GSTATE_MAIN_MENU_LAN))
		{
#if(IS_MAPHACK || IS_MUTE)
			// cloak before enable any hacks
			CloakDLL("Game.dll", GameBase);
#if(IS_MAPHACK)
			EnableMapHack();
#endif
#if(IS_MUTE)
#if(!IS_MAPHACK && !IS_FEEDBOT)
			EnableMute(true);	// on by default only if single mute
#else
			EnableMute(false);
#endif
#endif
#endif
			Injected = true;
			//MessageBox(NULL, "inject", "inject", 0);
		}
	}
	else
	{
		// uncloak game dll
		if(IsWC3Valid() && Injected && GetGameState2() == GSTATE2_MAIN_MENU_WC)
		{
			// disable any hacks before uncloak
#if(IS_MAPHACK || IS_MUTE)
#if(IS_MUTE)
			DisableMute();
#endif
#if(IS_MAPHACK)
			DisableMapHack();
#endif
			UncloakDLLs();
#endif
			Injected = false;
			//MessageBox(NULL, "uninject", "uninject", 0);
		}
	}

	// detect game start/end
	if(Injected && IsInGame())
	{
		if(!GameStarted)
		{
			// just entered game
			InitGame(currentTime);
			GameStartTime = currentTime;
			GameStarted = true;
		}

#if(IS_FEEDBOT)
		// update feed
		UpdateFeed(currentTime);
#endif
	}
	else
	{
		if(GameStarted)
		{
			// just closed game
			ExitGame();
			GameStarted = false;
		}
	}
}

/*------------------------------------------------------------------------------
	KeyboardProc
------------------------------------------------------------------------------*/

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if(nCode == HC_ACTION)
	{
		if(wParam == VK_F1 && IS_KEY_DOWN(lParam))
		{
#if(IS_FEEDBOT)
			if(Injected)
				SwitchFeed(FEEDSTATE_ALL);
#endif
		}
		else if(wParam == VK_F2 && IS_KEY_DOWN(lParam))
		{
#if(IS_FEEDBOT)
			if(Injected)
				SwitchFeed(FEEDSTATE_SINGLE);
#endif
		}
		else if(wParam == VK_F3 && IS_KEY_DOWN(lParam))
		{
#if(IS_FEEDBOT)
			if(Injected)
				StopFeed();
#endif
		}
		else if(wParam == VK_F5 && IS_KEY_DOWN(lParam))
		{
#if(IS_MAPHACK)
			if(Injected)
				ToggleMapHack();
#endif
		}
		else if(wParam == VK_F6 && IS_KEY_DOWN(lParam))
		{
#if(IS_MUTE)
			if(Injected)
				ToggleMute();
#endif
		}
		else if(IS_KEY_DOWN(lParam))
		{
			
		}
	}
	
	return CallNextHookEx(KeyboardHook, nCode, wParam, lParam);
}

/*------------------------------------------------------------------------------
	MessageProc
------------------------------------------------------------------------------*/

LRESULT CALLBACK MessageProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	// note: because of this hook we won't receive DLL_PROCESS_DETACH until unhook.
	// i use MessageProc instead of Thread, because Thread not synchronized with
	// some of wc3 functions (for example SendPacket),
	// so i tryed to use windows hook for messages and this works fine,
	// because wc3 also checks windows messages so it dont call other
	// functions while we processing messages, and we can use
	// legal (not hack) windows hook instead of easy detectable wc3 interceptions
	// and hooks (c) wc3noobpl.

	Update(timeGetTime());

	return CallNextHookEx(MessageHook, nCode, wParam, lParam);
}

/*------------------------------------------------------------------------------
	ThreadFunc
------------------------------------------------------------------------------*/
/*
DWORD WINAPI ThreadFunc(LPVOID pvParam)
{
	// note: if we dont use thread-unsafe wc3 functions,
	// then we better use thread instead of MessageHook,
	// because with thread we will get DLL_PROCESS_DETACH notiication,
	// and we dont have to use tricks with DLL_THREAD_DETACH
	// to detect wc3 closing (c) wc3noobpl.

	while(!IsExitThread)
	{
		Update(timeGetTime());
		Sleep(10);
	}

	return 0;
}

/*------------------------------------------------------------------------------
	DllMain
------------------------------------------------------------------------------*/

BOOL APIENTRY DllMain(HINSTANCE hDLL, DWORD reason, LPVOID reserved)
{
	// hDLL is also wc3 instance
	switch(reason)
	{
		case DLL_PROCESS_ATTACH:
		{
			//DisableThreadLibraryCalls(hDLL);	// disable DLL_THREAD_ATTACH / DETACH
			InitWC3Ptrs();
#if(IS_MAPHACK)
			InitMapHack();
#endif
			KeyboardHook = SetWindowsHookEx(WH_KEYBOARD, KeyboardProc, hDLL, GetCurrentThreadId());
			MessageHook = SetWindowsHookEx(WH_GETMESSAGE, MessageProc, hDLL, GetCurrentThreadId());
			//Thread = CreateThread(NULL, NULL, ThreadFunc, NULL, NULL, NULL);
			break;
		}
		case DLL_THREAD_DETACH:
		{
			// DLL_PROCESS_DETACH not called untill we uhnhook WH_GETMESSAGE,
			// other hooks is ok, but WH_GETMESSAGE prevent DLL_PROCESS_DETACH to show up,
			// so we make idiot trick: when wc3 exits it calls DLL_THREAD_DETACH
			// many times and fast, about 15-16 times, but sometimes even when
			// wc3 not exit DLL_THREAD_DETACH also happen, and can vary up to ~3 times at once
			// i suppose not more, so i just use strike below 15~16 as indicator that wc3 exits.
			// @todo: this is untill we can figure out better way to detect that wc3/process closing
			// (c) wc3noobpl.

			static DWORD strikeTime = timeGetTime();
			static DWORD strike = 0;
			static bool detached = false;
			if(!detached)
			{
				if(strikeTime + 1000 > timeGetTime())
				{
					strike++;
					if(strike >= 12)
					{
						// unhook WH_GETMESSAGE if we want to see DLL_PROCESS_DETACH,
						// also unhook other hooks for case.
						// @todo: we could also intercept some wc3 exit function,
						// instead of using this shittrick (c) wc3noobpl.

						UnhookWindowsHookEx(KeyboardHook);
						UnhookWindowsHookEx(MessageHook);
						
#if(IS_MAPHACK || IS_MUTE)
#if(IS_MUTE)
						DisableMute();
#endif
#if(IS_MAPHACK)
						ExitMapHack();
#endif
						UncloakDLLs();
#endif
						detached = true;
						//MessageBox(NULL, "STRIKE UNHOOK", "STRIKE UNHOOK", 0);
					}
				}
				else
				{
					strike = 0;
					strikeTime = timeGetTime();
				}
			}
			break;
		}
		/*
		case DLL_PROCESS_DETACH:
		{
			UnhookWindowsHookEx(KeyboardHook);
			ExitMapHack();
			UncloakDLLs();
			IsExitThread = true;
			Sleep(100);	// wait for thread close a little, to avoid exception on wc3 close
			MessageBox(NULL, "PROCESS DETACH", "PROCESS DETACH", 0);
			break;
		}
		/**/
	}
	return TRUE;
}
