// Patch code specific to Warrior Kings - Battles 1.23

#include "global.h"

static naked void loc_442566()
{
	__asm push ecx  // IPin* of Source output
	__asm push eax  // IGraphBuilder*
	__asm call BuildMsMpegGraph
	__asm mov ecx, 0x44256B
	__asm jmp ecx
}

static void WriteNotification(wchar_t *txt)
{
	__asm {
		push txt
		mov eax, 0x96D2D0
		push [eax]
		mov eax, 0x649CA0
		call eax
		add esp, 8
	}
}

static void __cdecl Action_Trace(char *self, void *arg)
{
	wchar_t w[256]; char *s;
	s = *(char**)(self+4);
	if(setting_trace_filter) if(s[0] != '@') return;
	atow(s, w, 255); w[255] = 0;
	WriteNotification(w);
}

static void __cdecl Action_Trace_Value(char *self, void *arg)
{
	wchar_t w[256]; float f; void *v; char *s;
	v = *(void**)(self+4);
	s = *(char**)(self+8);
	if(setting_trace_filter) if(s[0] != '@') return;

	__asm push arg
	callvf(v, 2)
	__asm fstp f

	_snwprintf(w, 255, L"%S: %f", s, f); w[255] = 0;
	WriteNotification(w);
}

naked void pointer_83abc0()
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

naked void pointer_83ac50()
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

int __stdcall IsThisASkirmishLevel(wchar_t *fn)
{
	wchar_t *ext;
	ext = wcsrchr(fn, '.');
	if(!ext) return 0;
	if(wcsicmp(ext, L".lvl")) return 0;
	if(setting_no_tutorial_in_skirmish)
	{
		if(!wcsicmp(fn, L"Interactive Tutorial.lvl")) return 0;
		if(!wcsicmp(fn, L"Basic Economy.lvl")) return 0;
		if(!wcsicmp(fn, L"Advanced Economy.lvl")) return 0;
		if(!wcsicmp(fn, L"Basic Military.lvl")) return 0;
		if(!wcsicmp(fn, L"Advanced Military.lvl")) return 0;
	}
	return 1;
}

naked void loc_6ee346()
{
	__asm {
		// ebp contains ptr to level file name
/*
		mov eax, ebp
		mov ecx, 0x6ee32b
		jmp ecx
*/
		pushad
		push ebp
		call IsThisASkirmishLevel
		test eax, eax
		popad
		jz notamap
		mov eax, ebp
		jmp goback
notamap:	mov eax, 0x91c28c // wkEmptyString
goback:		mov ecx, 0x6ee32b
		jmp ecx
	}
}

// Calling strcpy directly in assembler does not work when using /O2 flag,
// maybe because in this case strcpy is an inline function?
void mystrcpy(char *dst, char *src) {strcpy(dst, src);}

char alltex[] = "Maps\\Map_Textures\\All_Textures.dat";

static naked void loc_41d260()
{
	__asm {
		pushad
		mov al, [ecx]
		test al, al
		jz tdnzero
		push ecx
		jmp scnow
tdnzero:	push offset alltex
scnow:		push edx
		call mystrcpy
		add esp, 8
		popad
		mov eax, 0x41d26a
		jmp eax
	}
}

static naked void loc_41b07d()
{
	__asm {
		pushad
		push offset alltex
		push 0x91ca40  // mlCreateDlgTexDat
		call mystrcpy
		add esp, 8
		popad
		mov eax, 0x41b083
		jmp eax
	}
}

static naked void loc_41cfbc()
{
	__asm {
		lea ebp, [ebx+0x670]
		mov al, [ebp]
		test al, al
		jnz tdnnz
		mov ebp, offset alltex
tdnnz:		mov eax, 0x41cfc2
		jmp eax
	}
}

static naked void loc_771eed_AdditionalUUPVersionDisplay()
{
	__asm {
		mov esi, [esp+4]
		sub dword ptr [esp+4], 24 // move up a few pixels the original version text
		mov eax, 0x77c390 // WKDrawOutlinedText
		call eax

		sub ebx, 32 // move left a few pixels the new text
		mov dword ptr [esp+0x48+0x1C], 0x00FF00 // text color in the FontStyle object
		push dword ptr g_uupVersionDisplay
		push esi
		push ebx
		lea edx, [esp+0x54]
		mov ecx, edi
		mov eax, 0x77c390 // WKDrawOutlinedText
		call eax

		mov eax, 0x771ef2
		jmp eax
	}
}

void PatchStart_WKB_UiPerformanceImprovements();

void PatchStart_WKB()
{
	// Make the IAT writable.
	MemProtectionChange iatChange((void*)0x838000, PAGE_READWRITE);
	iatChange.apply();

	// Display UUP version in main menu.
	SetImmediateJump((void*)0x771eed, (uint)loc_771eed_AdditionalUUPVersionDisplay);

	// Fix the unit teleportation by replacing GetTickCount with our own function in the IAT.
	if(setting_higher_time_precision)
		*(void**)(0x838290) = (void*)myGetTickCount;
	// This will enable the data directory.
	if(setting_use_data_directory)
		*(uchar*)(0x750903) = 0;
	// Enables TRACE action.
	if(setting_enable_trace_action)
		*(uint*)(0x83abc0) = (uint)pointer_83abc0;
	// Enables TRACE_VALUE action.
	if(setting_enable_trace_value_action)
		*(uint*)(0x83ac50) = (uint)pointer_83ac50;
	//// Full memory with 0 after allocation.
	//if(setting_zero_allocated_memory)
	//	*(uchar*)(0x42296E) = 8;
	// Allow custom multiplayer maps.
	if(setting_custom_multiplayer_maps) {
		SetImmediateJump((void*)0x6ee346, (uint)loc_6ee346);
		// This will avoid .sav files to be present in the skirmish map list:
		//memset((void*)0x6ed8a7, 0x90, 12);
	}
	// Enable the map editor button in the main menu.
	if(setting_map_editor_button)
	{
		*(uchar*)0x771741 = 0x90;
		memset((void*)0x771748, 0x90, 5);
	}
	// Makes the map editor usable.
	if(setting_map_editor_hacks)
	{
		*(wchar_t**)0x4038ee = L"Maps\\Map_Textures\\%s";
		SetImmediateJump((void*)0x41d260, (uint)loc_41d260);
		SetImmediateJump((void*)0x41b07d, (uint)loc_41b07d);
		SetImmediateJump((void*)0x41cfbc, (uint)loc_41cfbc);
	}

	// Force MS MPEG audio codecs to render music.
	if(setting_dshow_force_ms_mpeg_codecs)
		SetImmediateJump((void*)0x442566, (uint)loc_442566);
	// Avoid call to IFilterGraph->SetDefaultSyncSource.
	if(setting_dshow_no_default_syncsrc)
		*(uchar*)0x442661 = 0xEB; // jz short -> jmp short
	// Set first argument to 0 ms when calling IMediaEvent->WaitForCompletion.
	if(setting_dshow_waitforcompletion_immediate)
		*(char*)0x442876 = 0;
	// Remove music stream bitrate limit (set CMemReader::dwKBPerSec to INFINITE).
	// This fixes the game freezing for seconds when changing music, depending on the codec, especially on Wine.
	// WK1 already has it set to INFINITE, but WKB set it to 90 KB/s.
	// See https://github.com/microsoft/Windows-classic-samples/blob/main/Samples/Win7Samples/multimedia/directshow/filters/async/memfile/memfile.h
	if(setting_dshow_no_bitrate_limit)
		*(DWORD*)0x4423E6 = INFINITE;

	// Allow a wider range of screen resolution ratios.
	if(setting_show_all_screen_resolutions)
	{
		*(float*)0x843D30 = 0.0f;	// Minimum ratio
		*(float*)0x843D2C = 10.0f;	// Maximum ratio
	}

	PatchStart_WKB_UiPerformanceImprovements();

	// Make the IAT back to non-writable for security reasons.
	iatChange.restore();
}