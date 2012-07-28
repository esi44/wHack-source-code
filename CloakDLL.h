/*==============================================================================
	CloakDLL.h

	Used for anti-warden protection (at least for 1.26a).

	*created by (c) wc3noobpl (2012)
	*originally based on YourName selfhack sources
==============================================================================*/
#pragma once

/*------------------------------------------------------------------------------
	Methods
------------------------------------------------------------------------------*/

bool CloakDLL(const char* name, DWORD originalBaseAddress);
void UncloakDLLs();
