// Universal unofficial patch for Warrior Kings and Warrior Kings - Battles
// By AdrienTD

#include "global.h"
#include <io.h>
#include <array>
#include <string>
#include <shellapi.h>

typedef void *(WINAPI *ftDirect3DCreate8)(int SDKVersion);

#define STRVER_WIDE_LITERAL2(x) L##x
#define STRVER_WIDE_LITERAL(x) STRVER_WIDE_LITERAL2(x)
#define STRVER_STRINGIFY2(x) #x
#define STRVER_STRINGIFY(x) STRVER_STRINGIFY2(x)
#define STRINGIFY_VERSION(a,b) STRVER_STRINGIFY(PATCH_VERSION_MAJOR) "." STRVER_STRINGIFY(PATCH_VERSION_MINOR)
static const wchar_t* const g_uupVersionDisplay = L"UUP " STRVER_WIDE_LITERAL(STRINGIFY_VERSION(PATCH_VERSION_MAJOR, PATCH_VERSION_MINOR));
static const wchar_t* const g_title = L"WK Universal Unofficial Patch";

bool g_isBattles;

HMODULE d3d8 = 0;
ftDirect3DCreate8 oriDirect3DCreate8;
char *exeep, oldepcode[5];
HINSTANCE exehi;

char setting_higher_time_precision = 1, setting_custom_campaign_crash_fix = 1,
	setting_use_data_directory = 1,	setting_use_multi_bcp = 1,
	setting_enable_trace_action = 0, setting_enable_trace_value_action = 0,
	setting_zero_allocated_memory = 0, setting_sight_range_events_bugfix = 1,
	setting_custom_multiplayer_maps = 1, setting_trace_filter = 0,
	setting_no_tutorial_in_skirmish = 1, setting_map_editor_button = 0,
	setting_map_editor_hacks = 1, setting_show_all_screen_resolutions = 1,
	setting_dshow_force_ms_mpeg_codecs = 1, setting_dshow_no_default_syncsrc = 0,
	setting_dshow_waitforcompletion_immediate = 1, setting_dshow_no_bitrate_limit = 1,
	setting_dshow_unresponsive_window_fix = 1,
	setting_allow_multiple_instances = 0, setting_apply_bcm_sky_texture_and_fog_color = 1;

bool setting_ui_performance_improvements = false,
	setting_ui_messagebox_double_text_render_fix = true;

void atow(char *a, wchar_t *w, uint ms)
{
	uint i, l;
	l = strlen(a);
	if(l > ms) l = ms;
	for(i = 0; i < l; i++)
		w[i] = a[i];
	w[l] = 0;
}

void CALLBACK UupMessageCallback(LPHELPINFO lpHelpInfo)
{
	ShellExecuteW(NULL, NULL, L"https://github.com/AdrienTD/WK-UniversalUnofficialPatch/wiki", NULL, NULL, SW_SHOWNORMAL);
}

int __stdcall UupMessage(const wchar_t* text, UINT flags)
{
	MSGBOXPARAMSW param;
	memset(&param, 0, sizeof(param));
	param.cbSize = sizeof(param);
	param.lpszText = text;
	param.lpszCaption = g_title;
	param.dwStyle = flags | MB_HELP;
	param.lpfnMsgBoxCallback = UupMessageCallback;
	return MessageBoxIndirectW(&param);
}

void *WINAPI myDirect3DCreate8(int SDKVersion)
{
	wchar_t tbuf[MAX_PATH];
	if(!d3d8)
	{
		if(_waccess(L"apd3d8.dll", 0) != -1) // If apd3d8.dll exists.
		{
			d3d8 = LoadLibraryW(L"apd3d8.dll");
		}
		else
		{
			GetSystemDirectoryW(tbuf, std::size(tbuf));
			wcscat_s(tbuf, L"\\d3d8.dll");
			d3d8 = LoadLibraryW(tbuf);
		}
	}
	oriDirect3DCreate8 = (ftDirect3DCreate8)GetProcAddress(d3d8, "Direct3DCreate8");
	return oriDirect3DCreate8(SDKVersion);
}

DWORD WINAPI myGetTickCount(void)
{
	LARGE_INTEGER freq, count;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&count);
	return (count.QuadPart * 1000) / freq.QuadPart;
}

void CheckMsMpegFailure(HRESULT hr, int x)
{
	if(!FAILED(hr))
		return;
	wchar_t b[256];
	swprintf_s(b, L"Failed to build graph with only MS MPEG codecs!\nReason: %i\nPlease tell me about this problem! Thanks!", x);
	UupMessage(b, MB_ICONERROR);
	ExitProcess(-1);
}

int __stdcall BuildMsMpegGraph(IGraphBuilder *gb, IPin *psrcout)
{
	HRESULT hr;
	IBaseFilter *mss, *mad;
	IPin *pmssin, *pmssout, *pmadin, *pmadout;

	hr = CoCreateInstance(CLSID_MPEG1Splitter, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&mss);
	CheckMsMpegFailure(hr, 1);
	hr = CoCreateInstance(CLSID_CMpegAudioCodec, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&mad);
	CheckMsMpegFailure(hr, 2);

	hr = gb->AddFilter(mss, L"WKUUP MPEG Stream Splitter"); CheckMsMpegFailure(hr, 3);
	hr = gb->AddFilter(mad, L"WKUUP MPEG Audio Decoder"); CheckMsMpegFailure(hr, 4);

	hr = mss->FindPin(L"Input", &pmssin);		CheckMsMpegFailure(hr, 5);
	hr = mad->FindPin(L"In", &pmadin);			CheckMsMpegFailure(hr, 6);
	hr = gb->ConnectDirect(psrcout, pmssin, NULL);	CheckMsMpegFailure(hr, 7);
	hr = mss->FindPin(L"Audio", &pmssout);		CheckMsMpegFailure(hr, 8);
	hr = gb->ConnectDirect(pmssout, pmadin, NULL);	CheckMsMpegFailure(hr, 9);
	hr = mad->FindPin(L"Out", &pmadout);		CheckMsMpegFailure(hr, 10);
	hr = gb->Render(pmadout);				CheckMsMpegFailure(hr, 11);

	pmssin->Release();
	pmssout->Release();
	pmadin->Release();
	pmadout->Release();
	mss->Release();
	mad->Release();

	return 0;
}

void __stdcall DisableDirectShowEvents(IFilterGraph* filterGraph)
{
	if(!filterGraph)
		return;

	IMediaEventEx* mediaEventEx = nullptr;
	HRESULT hr = filterGraph->QueryInterface(IID_IMediaEventEx, (void**)&mediaEventEx);
	CheckMsMpegFailure(hr, 47);
	// With NONOTIFY, the event handle is only set when the music is completed, nothing else.
	// So we no longer need to use GetEvent and check the event code anymore.
	mediaEventEx->SetNotifyFlags(AM_MEDIAEVENT_NONOTIFY);
	mediaEventEx->Release();
}

int __stdcall WaitForMusicCompletion(IMediaEvent* mediaEvent, long msTimeout, long* pEvCode)
{
	// Game was using IMediaEvent::WaitForCompletion to check if music playing is complete.
	// But for some reason, using it causes the game window to become unresponsive.
	// Reasons could be:
	//  - the hidden "ActiveMovie Window" and FilterGraph's thread do not run the message loop?
	//  - mysterious "AMUnblock" message that is sometimes sent to the main thread,
	//    but shouldn't it be sent to the FilterGraph's thread?
	// Either the DirectShow implementation is buggy or it's not clear how it should be used.
	// Anyway, here we just get the event handle and wait for it. This way we minimize
	// running the possibly bugged DirectShow code.
	// At least "AMUnblock" should no longer be sent to the main thread anymore.
	HANDLE event;
	mediaEvent->GetEventHandle((OAEVENT*)&event);
	DWORD wait = WaitForSingleObject(event, 0);
	return (wait == WAIT_OBJECT_0) ? 0 : -1;
}

void SetImmediateJump(void *p, uint j, uint size)
{
	*(char*)p = 0xE9;
	*(uint*)((char*)p+1) = j - ((uint)p + 5);
	NopifyCode((char*)p + 5, size - 5);
}

void SetImmediateCall(void *p, uint j, uint size)
{
	*(char*)p = 0xE8;
	*(uint*)((char*)p+1) = j - ((uint)p + 5);
	NopifyCode((char*)p + 5, size - 5);
}

void NopifyCode(void* p, uint count)
{
	memset(p, 0x90, count);
}

bool IsBuildBattles(int buildNumber)
{
	return buildNumber <= 155;
}

int GetBuildMinorVersion(int buildNumber)
{
	switch(buildNumber) {
		// Battles:
		case 125: return 0; // Demo
		case 128: return 0;
		case 141: return 1;
		case 152: return 23;
		case 155: return 23; // Traditional Chinese

		// WK1:
		case 334: return 0; // Demo
		case 345: return 0;
		case 355: return 2;
		case 356: return 3;
		case 366: return 4;
	}
	return -1;
}

std::string GetBuildName(int buildNumber)
{
	std::string name = IsBuildBattles(buildNumber) ? "Warrior Kings - Battles" : "Warrior Kings";
	name += " v1.";

	const int minorVersion = GetBuildMinorVersion(buildNumber);
	name += minorVersion != -1 ? std::to_string(minorVersion) : std::string("?");

	if(buildNumber == 155)
		name += " (Traditional Chinese)";

	name += " (Build ";
	name += std::to_string(buildNumber);
	name += ")";
		
	return name;
}

int VerifyVersion()
{
	wchar_t mname[MAX_PATH];
	DWORD vsize, unk, v;
	UINT valueOutSize;
	void *vpnt;
	VS_FIXEDFILEINFO *ffi;

	GetModuleFileNameW(NULL, mname, std::size(mname)-1);
	vsize = GetFileVersionInfoSizeW(mname, &unk);
	if(!vsize) return -1;
	vpnt = (void*)malloc(vsize);
	if(!vpnt) return -1;
	if(!GetFileVersionInfoW(mname, 0, vsize, vpnt)) return -1;
	if(!VerQueryValueW(vpnt, L"\\", (void**)&ffi, &valueOutSize)) return -1;
	if(valueOutSize == 0) return -1;
	v = LOWORD(ffi->dwFileVersionLS);
	g_isBattles = IsBuildBattles(v);
	if((v != 152) && (v != 366))
	{
		const std::string buildName = GetBuildName(v);
		const std::string reqBuildName = GetBuildName(g_isBattles ? 152 : 366);
		wchar_t buf[512];
		swprintf_s(buf,
			L"You are currently using:\n - %S\n\n"
			L"But the WK Universal Unofficial Patch requires the following game version:\n - %S\n\n"
			L"The game first needs to be updated to the required version with official patches, before using this unofficial patch. Click \"Help\" for more details.\n\n"
			L"You can continue, but the game will still not be patched.",
			buildName.c_str(), reqBuildName.c_str());
		if(UupMessage(buf, MB_ICONERROR | MB_OKCANCEL) != IDOK)
			ExitProcess('UUP0');
	}
	return 1;
}

void ReadSettings()
{
	FILE *f; char li[128], s[64]; int p;
	li[127] = s[63] = 0;
	f = fopen("wkuup_settings.txt", "r");
	if(!f) return;
	while(!feof(f))
	{
		fgets(li, 127, f);
		sscanf(li, "%63s %i", s, &p);
		if(!stricmp(s, "higher_time_precision"))
			setting_higher_time_precision = p;
		else if(!stricmp(s, "custom_campaign_crash_fix"))
			setting_custom_campaign_crash_fix = p;
		else if(!stricmp(s, "use_data_directory"))
			setting_use_data_directory = p;
		else if(!stricmp(s, "use_multi_bcp"))
			setting_use_multi_bcp = p;
		else if(!stricmp(s, "enable_trace_action"))
			setting_enable_trace_action = p;
		else if(!stricmp(s, "enable_trace_value_action"))
			setting_enable_trace_value_action = p;
		else if(!stricmp(s, "sight_range_events_bugfix"))
			setting_sight_range_events_bugfix = p;
		else if(!stricmp(s, "zero_allocated_memory"))
			setting_zero_allocated_memory = p;
		else if(!stricmp(s, "custom_multiplayer_maps"))
			setting_custom_multiplayer_maps = p;
		else if(!stricmp(s, "trace_filter"))
			setting_trace_filter = p;
		else if(!stricmp(s, "no_tutorial_in_skirmish"))
			setting_no_tutorial_in_skirmish = p;
		else if(!stricmp(s, "map_editor_button"))
			setting_map_editor_button = p;
		else if(!stricmp(s, "map_editor_hacks"))
			setting_map_editor_hacks = p;
		else if(!stricmp(s, "show_all_screen_resolutions"))
			setting_show_all_screen_resolutions = p;
		else if(!stricmp(s, "dshow_force_ms_mpeg_codecs"))
			setting_dshow_force_ms_mpeg_codecs = p;
		else if(!stricmp(s, "dshow_no_default_syncsrc"))
			setting_dshow_no_default_syncsrc = p;
		else if(!stricmp(s, "dshow_waitforcompletion_immediate"))
			setting_dshow_waitforcompletion_immediate = p;
		else if(!stricmp(s, "dshow_no_bitrate_limit"))
			setting_dshow_no_bitrate_limit = p;
		else if(!stricmp(s, "dshow_unresponsive_window_fix"))
			setting_dshow_unresponsive_window_fix = p;
		else if(!stricmp(s, "allow_multiple_instances"))
			setting_allow_multiple_instances = p;
		else if(!stricmp(s, "apply_bcm_sky_texture_and_fog_color"))
			setting_apply_bcm_sky_texture_and_fog_color = p;
		else if(!stricmp(s, "ui_performance_improvements"))
			setting_ui_performance_improvements = p;
		else if(!stricmp(s, "ui_messagebox_double_text_render_fix"))
			setting_ui_messagebox_double_text_render_fix = p;
	}
	fclose(f);
}

void PatchStart()
{
	// Verify the version of the game.
	VerifyVersion();

	// Read the settings file (wkuup_settings.txt).
	ReadSettings();

	// Make the .text section writable.
	MemProtectionChange textSectionProtectionChange((void*)0x401000, PAGE_EXECUTE_READWRITE);
	textSectionProtectionChange.apply();

	if(!g_isBattles) PatchStart_WKO();
	else PatchStart_WKB();

	// Restore entry point code.
	memcpy(exeep, oldepcode, 5);

	// Make the .text section back to non-writable for security reasons.
	textSectionProtectionChange.restore();
}

naked void EntryPointInterception()
{
	__asm {
		call PatchStart
		mov eax, exeep
		jmp eax
	}
}

BOOL WINAPI DllMain(HINSTANCE hinstDll, DWORD fdwReason, LPVOID lpvReserved)
{
	char *mz, *pe;
	if(fdwReason == DLL_PROCESS_ATTACH)
	{
		// Find the entry point address in the PE header.
		exehi = GetModuleHandle(0);
		mz = (char*)exehi;
		if(*(ushort*)mz != 'ZM') return FALSE;
		pe = mz + *(uint*)(mz+0x3C);
		if(*(uint*)pe != 'EP') return FALSE;
		exeep = (char*)( *(uint*)(pe+0x28) + *(uint*)(pe+0x34) );

		// Make the .text section writable.
		DWORD oldProtection;
		VirtualProtect(exeep, 5, PAGE_EXECUTE_READWRITE, &oldProtection);

		// Save the first 5 bytes of the entry point code.
		memcpy(oldepcode, exeep, 5);

		// Put a jump to our function at the beginning of the entry point code.
		SetImmediateJump(exeep, (uint)EntryPointInterception, 5);

		// Put the .text section back to non-writable.
		DWORD unused;
		VirtualProtect(exeep, 5, oldProtection, &unused);
	}
	return TRUE;
}
