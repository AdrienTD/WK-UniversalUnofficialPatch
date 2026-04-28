// Patch code specific to Warrior Kings 1.4

#include "global.h"

static naked void call_5d4ad6()
{
	__asm mov eax, [esp+12]
	__asm test eax, eax
	__asm jz notxtnocall
	__asm mov eax, 0x41e890
	__asm jmp eax
notxtnocall:
	__asm ret 12
}

static void __stdcall LoadMultiBCP(uint arg1, uint arg2)
{
	HANDLE hfnd; WIN32_FIND_DATAW w32fd; wchar_t *fndfname, *pp;
	hfnd = FindFirstFileW(L"*.bcp", &w32fd);
	if(hfnd)
	{
		do
		{
			fndfname = w32fd.cFileName;
			pp = wcsrchr(fndfname, '.');
			if(pp) *pp = 0;
			if(wcsicmp(fndfname, L"data"))
			{
				__asm
				{
					push eax
					push 1
					push 1
					push fndfname
					push arg2
					mov ecx, arg1
					mov eax, 0x5726A0
					call eax
					pop eax
				}
			}
		} while(FindNextFileW(hfnd, &w32fd));
		FindClose(hfnd);
	}
}

static naked void call_57294c()
{
	__asm {
		pushad
		push ebp
		push esi
		call LoadMultiBCP
		popad
		mov eax, 0x5726A0
		jmp eax
	}
}

static void WriteNotification(wchar_t *txt)
{
	__asm {
		push txt
		mov eax, 0x723DFC
		push [eax]
		mov eax, 0x4078D0
		call eax
		add esp, 8
	}
}

static void __cdecl Action_Trace(char *this, void *arg)
{
	wchar_t w[256]; uchar *s;
	s = *(uchar**)(this+12);
	if(setting_trace_filter) if(s[0] != '@') return;
	atow(s, w, 255); w[255] = 0;
	WriteNotification(w);
}

static void __cdecl Action_Trace_Value(char *this, void *arg)
{
	wchar_t w[256]; float f; void *v; uchar *s;
	v = *(void**)(this+12);
	s = *(uchar**)(this+16);
	if(setting_trace_filter) if(s[0] != '@') return;

	__asm push arg
	callvf(v, 8)
	__asm fstp f

	_snwprintf(w, 255, L"%S: %f", s, f); w[255] = 0;
	WriteNotification(w);
}

naked void pointer_69f15c()
{
	__asm {
		push dword ptr [esp+4]
		push ecx
		call Action_Trace
		pop ecx
		add esp, 4
		ret 4
	}
}

naked void pointer_69f1ac()
{
	__asm {
		push dword ptr [esp+4]
		push ecx
		call Action_Trace_Value
		pop ecx
		add esp, 4
		ret 4
	}
}

char *mapfnamewop;

naked void loc_5cf297()
{
	__asm {
		mov ecx, [esp+36] // fnamewop
		push ecx
		mov mapfnamewop, ecx
		mov edx, 0x5cf29c
		jmp edx
	}
}

naked void loc_5cfc04()
{
	__asm {
		mov eax, mapfnamewop
		mov ecx, 0x5cfbe8
		jmp ecx
	}
}

void PatchStart_WKO()
{
	// Make the IAT writable.
	SetMemProtection((void*)0x698000, PAGE_READWRITE);

	// Fix the unit teleportation by replacing GetTickCount with our own function in the IAT.
	if(setting_higher_time_precision)
		*(void**)(0x698198) = (void*)myGetTickCount;
	// Replace 'v' with 'a' in the version text of the main menu.
	((wchar_t*)0x7151e8)[0] = 'a';
	// This will fix the crash when opening a custom campaign.
	if(setting_custom_campaign_crash_fix)
		*(uint*)(0x5d4ad6) = (uint)call_5d4ad6 - 0x5d4ada;
	// This will enable the data directory.
	if(setting_use_data_directory)
		*(uchar*)(0x57290a) = 0;
	// Allow multi BCP.
	if(setting_use_multi_bcp)
		*(uint*)(0x57294c) = (uint)call_57294c - 0x572950;
	// Enables TRACE action.
	if(setting_enable_trace_action)
		*(uint*)(0x69F15C) = (uint)pointer_69f15c;
	// Enables TRACE_VALUE action.
	if(setting_enable_trace_value_action)
		*(uint*)(0x69F1AC) = (uint)pointer_69f1ac;
	// Full memory with 0 after allocation.
	if(setting_zero_allocated_memory)
		*(uchar*)(0x42296E) = 8;
	// Sight range events not sent bug fix
	if(setting_sight_range_events_bugfix)
		*(ushort*)(0x45858c) = 0x9090;
	// Allow custom multiplayer maps.
	if(setting_custom_multiplayer_maps) {
		SetImmediateJump((void*)0x5cf297, (uint)loc_5cf297);
		SetImmediateJump((void*)0x5cfc04, (uint)loc_5cfc04);
	}

	// Make the IAT back to non-writable for security reasons.
	SetMemProtection((void*)0x698000, PAGE_READONLY);
}