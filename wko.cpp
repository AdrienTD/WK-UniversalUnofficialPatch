// Patch code specific to Warrior Kings 1.4

#include "global.h"

#include <initializer_list>

static naked void loc_58444e()
{
	__asm push ecx  // IPin* of Source output
	__asm push eax  // IGraphBuilder*
	__asm call BuildMsMpegGraph
	__asm mov ecx, 0x584453
	__asm jmp ecx
}

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

static void __cdecl Action_Trace(char *self, void *arg)
{
	wchar_t w[256]; char *s;
	s = *(char**)(self+12);
	if(setting_trace_filter) if(s[0] != '@') return;
	atow(s, w, 255); w[255] = 0;
	WriteNotification(w);
}

static void __cdecl Action_Trace_Value(char *self, void *arg)
{
	wchar_t w[256]; float f; void *v; char *s;
	v = *(void**)(self+12);
	s = *(char**)(self+16);
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

naked void loc_5c802e_PatchVersionColor()
{
	__asm {
		// replaced instruction
		mov ecx, [ebx+0x104]

		mov dword ptr [ecx+0x44], 0xFF00FF00

		mov eax, 0x5c8034
		jmp eax
	}
}


naked void loc_584377_DisableDirectShowEvents()
{
	__asm {
		call CoCreateInstance
		push dword ptr [esi]
		call DisableDirectShowEvents
		mov eax, 0x58437D
		jmp eax
	}
}

naked void loc_584698_FixMusicCompletionWaitCausingGhostWindow()
{
	__asm {
		call WaitForMusicCompletion
		test eax, eax
		jz done
	playing:
		mov eax, 0x5846AE
		jmp eax
	done:
		mov eax, 0x5846A2
		jmp eax
	}
}

void PatchVersionDisplay()
{
	// Game version text label coordinates
	*(uchar*)0x5C7EA7 = 40; // left
	*(uchar*)0x5C7EB6 = 50; // top
	*(uchar*)0x5C7EC5 = 0; // right
	*(uchar*)0x5C7ED4 = 30; // bottom

	// UUP Patch version text label
	// This reuses a second version label control that was left empty, how convenient!
	for(uint addr : {0x5c7f50, 0x5c7f5b, 0x5c7f6b, 0x5c7f72, 0x5c7f7f, 0x5c7fe6})
		*(const wchar_t**)addr = g_uupVersionDisplay;
	*(uchar*)0x5C7FFB = 70; // left
	*(uchar*)0x5C800A = 30; // top
	*(uchar*)0x5C8019 = 0; // right
	*(uchar*)0x5C8028 = 10; // bottom
	SetImmediateJump((void*)0x5c802e, (uint)loc_5c802e_PatchVersionColor, 6);
}

void PatchStart_WKO_DrawTextFixes();

void PatchStart_WKO()
{
	// Make the IAT writable.
	MemProtectionChange iatChange((void*)0x698000, PAGE_READWRITE);
	iatChange.apply();

	// Display UUP version in main menu.
	PatchVersionDisplay();

	// Fix the unit teleportation by replacing GetTickCount with our own function in the IAT.
	if(setting_higher_time_precision)
		*(void**)(0x698198) = (void*)myGetTickCount;
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
		SetImmediateJump((void*)0x5cf297, (uint)loc_5cf297, 5);
		SetImmediateJump((void*)0x5cfc04, (uint)loc_5cfc04, 7);
	}
	// Force MS MPEG audio codecs to render music.
	if(setting_dshow_force_ms_mpeg_codecs)
		SetImmediateJump((void*)0x58444e, (uint)loc_58444e, 5);
	// Avoid call to IFilterGraph->SetDefaultSyncSource.
	if(setting_dshow_no_default_syncsrc)
		*(uchar*)0x58446B = 0xEB; // jz short -> jmp short
	// Set first argument to 0 ms when calling IMediaEvent->WaitForCompletion.
	if(setting_dshow_waitforcompletion_immediate)
		*(char*)0x584696 = 0;

	// Fix game window becoming "unresponsive" (ghost window) in windowed mode when music is on.
	if(setting_dshow_unresponsive_window_fix) {
		SetImmediateJump((void*)0x584377, (uint)loc_584377_DisableDirectShowEvents, 6);
		SetImmediateJump((void*)0x584698, (uint)loc_584698_FixMusicCompletionWaitCausingGhostWindow, 8);
	}

	PatchStart_WKO_DrawTextFixes();

	// Make the IAT back to non-writable for security reasons.
	iatChange.restore();
}