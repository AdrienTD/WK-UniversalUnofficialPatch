// Patch code specific to Warrior Kings - Battles 1.23

#include "global.h"

#include <string>

/////

class File
{
public:
	virtual ~File();
	virtual uint GetSize();
	virtual uint Checksum();
	virtual bool GetTime(FILETIME* outTime);
	virtual void SetTime(const FILETIME* time);
	virtual bool Seek(uint offset);
	virtual bool Read(uint numBytes);
private:
	File() = delete;
};

class Random_access_file;

void* (__cdecl *wkFuncPtr_newOperator)(size_t len);
void (__cdecl *wkFuncPtr_deleteOperator)(void* ptr);
void (__thiscall *wkFuncPtr_Random_access_file_Constructor)(Random_access_file* self);
void (__thiscall *wkFuncPtr_Random_access_file_Destructor)(Random_access_file* self);
bool (__thiscall *wkFuncPtr_Random_access_file_Open)(Random_access_file* self, const wchar_t* filePath, int mode);
int (__thiscall *wkFuncPtr_Random_access_file_GetSize)(Random_access_file* self);
void (__thiscall *wkFuncPtr_Random_access_file_Read)(Random_access_file* self, void* buffer, int size1, int size2);

class Random_access_file
{
public:
	void* operator new(size_t len) { return wkFuncPtr_newOperator(len); }
	void operator delete(void* ptr) { wkFuncPtr_deleteOperator(ptr); }

	Random_access_file() { wkFuncPtr_Random_access_file_Constructor(this); }

	virtual ~Random_access_file() { wkFuncPtr_Random_access_file_Destructor(this); }

	bool Open(const wchar_t* filePath, int mode) { return wkFuncPtr_Random_access_file_Open(this, filePath, mode); }
	int GetFileSize() { return wkFuncPtr_Random_access_file_GetSize(this); }
	void Read(void* buffer, int size1, int size2) { return wkFuncPtr_Random_access_file_Read(this, buffer, size1, size2); }

	void GetTime(FILETIME* outTime) { file->GetTime(outTime); }

private:
	char data[12];
	File* file;
};

/////

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

static const wchar_t messageMultipleInstances[] = L"The game is already running.\n\nAnother instance of the game will launch.";
naked void loc_6BA875_AllowMultipleInstances()
{
	__asm {
		cmp eax, ERROR_ALREADY_EXISTS
		jz mutexExists
	runGame:
		mov eax, 0x6BA87B
		jmp eax

	mutexExists:
		push MB_OKCANCEL
		push offset g_title
		push offset messageMultipleInstances
		push 0
		call MessageBoxW

		cmp eax, IDOK
		jz runGame

		mov eax, 0x6BAE47
		jmp eax
	}
}

bool __stdcall IsCustomMap(const char* mapPath)
{
	size_t mapPathLength = strlen(mapPath);
	std::wstring wMapPath(mapPathLength, 0);
	for(size_t i = 0; i < mapPathLength; ++i)
		wMapPath[i] = (wchar_t)mapPath[i];

	Random_access_file file;
	bool opened = file.Open(wMapPath.c_str(), 0);
	if(!opened) return false;

	FILETIME fileTime;
	file.GetTime(&fileTime);

	return fileTime.dwHighDateTime >= 0x01C5'0000 // after 21st Jan 2005
		|| (fileTime.dwHighDateTime == 0 && fileTime.dwLowDateTime == 0); // maps exported by wkbre have date set to null (lazy Adrien)
}

naked void loc_41CF04_ApplySkyColorAndTextureFromBCM()
{
	__asm {
		rep movsb // replaced instruction
		pushad // don't forget to pop this!

		lea eax, [ebp+0x368]
		push eax
		call IsCustomMap
		mov bl, al

		test al, al
		jz useDefaultBcmFogColor

useBcmFogColor:
		mov esi, [ebp+0x40]
		jmp applySky

useDefaultBcmFogColor:
		mov esi, 0x9fc5e6

applySky:
		// Pretty much what happens in the SNR loading code:

		// ecx <- pointer to Private_rendering_interface singleton instance
		mov eax, 0x920888
		mov ecx, [eax]
		// Call Private_rendering_interface::GetDriver()
		mov eax, 0x5C42F0
		call eax
		// Call virtual Private_rendering_interface::SetSkyColor(color)
		mov ecx, eax
		push esi // the fog color
		mov edx, [ecx]
		call dword ptr [edx+0xB0]

		test bl, bl
		jz useDefaultSkyBox

useBcmSkyBox:
		// we skip the code that overrides the skybox texture path
		popad
		mov eax, 0x41CF1A
		jmp eax

useDefaultSkyBox:
		popad
		add ebp, 0x17A0 // replaced instruction
		mov edx, 0x41CF0C
		jmp edx
	}
}

bool __stdcall VerifyPatch1_1Data()
{
	Random_access_file file;
	bool ok = file.Open(L"Warrior Kings Game Set\\Feature Tests.cpp", 0);
	if(!ok) {
		return true;
	}

	if(file.GetFileSize() == 155371)
	{
		int button = MessageBoxW(nullptr,
			L"v1.0 data detected!\n\n"
			L"New game data have been introduced since Version 1.1, but they are currently missing.\n\n"
			L"Please install the patch 1.1 data, particularly the file \"patch_1_1.bcp\", and keep the v1.23 executable.\n\n"
			L"You can continue, but without the patch 1.1 data, you will get the following issues:\n"
			L" - Some bugs and unbalances from v1.0 will still be present, even with a v1.23 exe. (For example, monks can be converted to peasants, etc.)\n"
			L" - You may also not be able to join multiplayer games.",
			g_title, MB_ICONWARNING | MB_OKCANCEL);
		return button == IDOK;
	}

	return true;
}

naked void loc_6BA7FF_VerifyForPatch1_1Data()
{
	__asm {
		call VerifyPatch1_1Data
		test al, al
		jz patchMissing
	patchPresent:
		push 0xD0 // replaced instruction
		mov eax, 0x6BA804
		jmp eax
	patchMissing:
		mov eax, 0x6BAE4E
		jmp eax
	}
}

void PatchStart_WKB_UiPerformanceImprovements();
void PatchStart_WKB_DrawTextFixes();

void PatchStart_WKB()
{
	// Make the IAT writable.
	MemProtectionChange iatChange((void*)0x838000, PAGE_READWRITE);
	iatChange.apply();

	// Initialize function pointers
	*(size_t*)&wkFuncPtr_newOperator = 0x7D7F57;
	*(size_t*)&wkFuncPtr_deleteOperator = 0x7D4E5E;
	*(size_t*)&wkFuncPtr_Random_access_file_Constructor = 0x7525a0;
	*(size_t*)&wkFuncPtr_Random_access_file_Destructor = 0x752600;
	*(size_t*)&wkFuncPtr_Random_access_file_Open = 0x7524E0;
	*(size_t*)&wkFuncPtr_Random_access_file_GetSize = 0x4BE960;
	*(size_t*)&wkFuncPtr_Random_access_file_Read = 0x7523B0;

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

	// Allow multiple instances of the game running
	if(setting_allow_multiple_instances)
		SetImmediateJump((void*)0x6BA875, (uint)loc_6BA875_AllowMultipleInstances);

	// Avoid BCM sky box file path and fog color from being overidden/ignored.
	if(setting_apply_bcm_sky_texture_and_fog_color)
		SetImmediateJump((void*)0x41CF04, (uint)loc_41CF04_ApplySkyColorAndTextureFromBCM);

	SetImmediateJump((void*)0x6BA7FF, (uint)loc_6BA7FF_VerifyForPatch1_1Data);

	PatchStart_WKB_UiPerformanceImprovements();
	PatchStart_WKB_DrawTextFixes();

	// Make the IAT back to non-writable for security reasons.
	iatChange.restore();
}