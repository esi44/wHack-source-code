/*==============================================================================
	CloakDLL.cpp

	*created by (c) wc3noobpl (2012)
	*originally based on YourName selfhack sources
==============================================================================*/
#include "System.h"
#include "CloakDLL.h"

// Defines
#define MAX_CLOAKED_DLLS		10
#define UNLINK(x)				(x).Blink->Flink = (x).Flink; (x).Flink->Blink = (x).Blink;
#define RELINK(x,p)				(x).Blink->Flink = (p);       (x).Flink->Blink = (p);

// Types
#pragma pack(push, 1)
struct UNICODE_STRING
{
	USHORT Length;
	USHORT MaxLength;
	PWSTR Buffer;
};
struct ModuleInfoNode
{
	LIST_ENTRY LoadOrder;
	LIST_ENTRY InitOrder;
	LIST_ENTRY MemoryOrder;
	HMODULE BaseAddress;		// base address AKA module handle
	unsigned long EntryPoint;
	unsigned int Size;			// size of the modules image
	UNICODE_STRING FullPath;
	UNICODE_STRING Name;
	unsigned long Flags;
	unsigned short LoadCount;
	unsigned short TlsIndex;
	LIST_ENTRY HashTable;		// linked list of any other modules that have the same first letter
	unsigned long Timestamp;
};
struct ProcessModuleInfo
{
	unsigned int Size;			// size of a ModuleInfo node?
	unsigned int Initialized;
	HANDLE SsHandle;
	LIST_ENTRY LoadOrder;
	LIST_ENTRY InitOrder;
	LIST_ENTRY MemoryOrder;
};
#pragma pack(pop)
struct CloakedDLL
{
	BYTE* Name;
	int NameLength;
    LIST_ENTRY* LoadOrderEntry;
    LIST_ENTRY* InitOrderEntry;
    LIST_ENTRY* MemoryOrderEntry;
    LIST_ENTRY* HashTableEntry;
	void* ModuleInfoAddr;
	ModuleInfoNode ModuleInfo;
	HMODULE FakeModule;
};

// Vars
int CloakedDLLCount = 0;
CloakedDLL CloakedDLLs[MAX_CLOAKED_DLLS];

/*------------------------------------------------------------------------------
	HideDLL
------------------------------------------------------------------------------*/

bool HideDLL(HMODULE hDLL)
{
	// get process module info
    ProcessModuleInfo* pmInfo;
    __asm
    {
        mov eax, fs:[18h]		// TEB
        mov eax, [eax + 30h]	// PEB
        mov eax, [eax + 0Ch]	// PROCESS_MODULE_INFO
        mov pmInfo, eax
    }

    // find hDLL module
    ModuleInfoNode* module = (ModuleInfoNode*)(pmInfo->LoadOrder.Flink);
    while(module->BaseAddress && module->BaseAddress != hDLL)
		module = (ModuleInfoNode*)(module->LoadOrder.Flink);

    if(module->BaseAddress == NULL)
		return false;

	// backup module address
	CloakedDLLs[CloakedDLLCount].ModuleInfoAddr = module;

    // backup links
	CloakedDLLs[CloakedDLLCount].LoadOrderEntry = module->LoadOrder.Blink->Flink;
	CloakedDLLs[CloakedDLLCount].InitOrderEntry = module->InitOrder.Blink->Flink;
	CloakedDLLs[CloakedDLLCount].MemoryOrderEntry = module->MemoryOrder.Blink->Flink;
	CloakedDLLs[CloakedDLLCount].HashTableEntry = module->HashTable.Blink->Flink;
	
	// unlink module from all lists
    UNLINK(module->LoadOrder);
    UNLINK(module->InitOrder);
    UNLINK(module->MemoryOrder);
    UNLINK(module->HashTable);

	// backup module name
	CloakedDLLs[CloakedDLLCount].Name = new BYTE[module->FullPath.Length];
	CloakedDLLs[CloakedDLLCount].NameLength = module->FullPath.Length;
    for(int i = 0; i < module->FullPath.Length; i++)
		CloakedDLLs[CloakedDLLCount].Name[i] = ((BYTE*)module->FullPath.Buffer)[i];

	// zero module name, for case
    for(int i = 0; i < module->FullPath.Length; i++)
		((BYTE*)module->FullPath.Buffer)[i] = 0;
    
	// backup module info node
    for(int i = 0; i < sizeof(ModuleInfoNode); i++)
		((BYTE*)&CloakedDLLs[CloakedDLLCount].ModuleInfo)[i] = ((BYTE*)module)[i];

	// zero module info node, for case
    for(int i = 0; i < sizeof(ModuleInfoNode); i++)
		((BYTE*)module)[i] = 0;
    
	return true;
}

/*------------------------------------------------------------------------------
	CloakDLL
------------------------------------------------------------------------------*/

bool CloakDLL(const char* name, DWORD originalBaseAddress)
{
	// name must be valid and not empty
	if(name != NULL && name[0] != 0 && CloakedDLLCount < MAX_CLOAKED_DLLS)
	{
		// dont hide dll if it's already hided (original base addr not match)
		HMODULE hDLL = GetModuleHandle(name);
		if(hDLL != NULL && (DWORD)hDLL == originalBaseAddress)
		{
			if(HideDLL(hDLL))
			{
				// load fake dll
				CloakedDLLs[CloakedDLLCount].FakeModule = LoadLibraryEx(name, NULL, 0);
				bool result = CloakedDLLs[CloakedDLLCount].FakeModule != NULL;
				CloakedDLLCount++;
				return result;
			}
		}
	}
	return false;
}

/*------------------------------------------------------------------------------
	UncloakDLLs
------------------------------------------------------------------------------*/

void UncloakDLLs()
{
	while(CloakedDLLCount > 0)
	{
		CloakedDLLCount--;
		ModuleInfoNode* module = &CloakedDLLs[CloakedDLLCount].ModuleInfo;	// backuped module

		// close fake dll
		FreeLibrary(CloakedDLLs[CloakedDLLCount].FakeModule);

		// restore module info node
		for(int i = 0; i < sizeof(ModuleInfoNode); i++)
			((BYTE*)CloakedDLLs[CloakedDLLCount].ModuleInfoAddr)[i] = ((BYTE*)module)[i];

		// restore module name
		for(int i = 0; i < CloakedDLLs[CloakedDLLCount].NameLength; i++)
			((BYTE*)module->FullPath.Buffer)[i] = CloakedDLLs[CloakedDLLCount].Name[i];
		
		delete[] CloakedDLLs[CloakedDLLCount].Name;
		CloakedDLLs[CloakedDLLCount].Name = NULL;

		// relink module to all lists
		RELINK(module->LoadOrder,   CloakedDLLs[CloakedDLLCount].LoadOrderEntry);
		RELINK(module->InitOrder,   CloakedDLLs[CloakedDLLCount].InitOrderEntry);
		RELINK(module->MemoryOrder, CloakedDLLs[CloakedDLLCount].MemoryOrderEntry);
		RELINK(module->HashTable,   CloakedDLLs[CloakedDLLCount].HashTableEntry);
	}
}
