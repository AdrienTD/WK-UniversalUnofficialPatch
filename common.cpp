// Universal unofficial patch for Warrior Kings and Warrior Kings - Battles
// By AdrienTD

#include "global.h"
#include <io.h>

typedef void *(WINAPI *ftDirect3DCreate8)(int SDKVersion);

char title[] = "WK Universal Unofficial Patch";
HMODULE d3d8 = 0;
ftDirect3DCreate8 oriDirect3DCreate8;
char *exeep, oldepcode[5];
HINSTANCE exehi;
char tbuf[512];
char battles;

char setting_higher_time_precision = 1, setting_custom_campaign_crash_fix = 1,
	setting_use_data_directory = 1,	setting_use_multi_bcp = 1,
	setting_enable_trace_action = 0, setting_enable_trace_value_action = 0,
	setting_zero_allocated_memory = 0, setting_sight_range_events_bugfix = 1,
	setting_custom_multiplayer_maps = 1, setting_trace_filter = 0,
	setting_no_tutorial_in_skirmish = 1, setting_map_editor_button = 1,
	setting_map_editor_hacks = 1, setting_dshow_force_ms_mpeg_codecs = 1,
	setting_dshow_no_default_syncsrc = 0, setting_show_all_screen_resolutions = 1,
	setting_dshow_waitforcompletion_immediate = 1;

bool setting_ui_performance_improvements = true,
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

void *WINAPI myDirect3DCreate8(int SDKVersion)
{
	char tbuf[128];
	if(!d3d8)
	{
		if(_access("apd3d8.dll", 0) != -1) // If apd3d8.dll exists.
		{
			d3d8 = LoadLibrary("apd3d8.dll");
		}
		else
		{
			GetSystemDirectory(tbuf, sizeof(tbuf)/sizeof(tbuf[0]));
			strcat_s(tbuf, sizeof(tbuf)/sizeof(tbuf[0])-1, "\\d3d8.dll");
			d3d8 = LoadLibrary(tbuf);
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

void msmpegfailedmsg(int x)
{
	char b[256];
	sprintf(b, "Failed to build graph with only MS MPEG codecs!\nReason: %i\nPlease tell me about this problem! Thanks!", x);
	MessageBox(0, b, title, 16);
	ExitProcess(-1);
}

#define msmpegfail(x) if(FAILED(hr)) msmpegfailedmsg(x);

int __stdcall BuildMsMpegGraph(IGraphBuilder *gb, IPin *psrcout)
{
	HRESULT hr;
	IBaseFilter *mss, *mad;
	IPin *pmssin, *pmssout, *pmadin, *pmadout;

	hr = CoCreateInstance(CLSID_MPEG1Splitter, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&mss);
	msmpegfail(1);
	hr = CoCreateInstance(CLSID_CMpegAudioCodec, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&mad);
	msmpegfail(2);

	hr = gb->AddFilter(mss, L"WKUUP MPEG Stream Splitter"); msmpegfail(3);
	hr = gb->AddFilter(mad, L"WKUUP MPEG Audio Decoder"); msmpegfail(4);

	hr = mss->FindPin(L"Input", &pmssin);		msmpegfail(5);
	hr = mad->FindPin(L"In", &pmadin);			msmpegfail(6);
	hr = gb->ConnectDirect(psrcout, pmssin, NULL);	msmpegfail(7);
	hr = mss->FindPin(L"Audio", &pmssout);		msmpegfail(8);
	hr = gb->ConnectDirect(pmssout, pmadin, NULL);	msmpegfail(9);
	hr = mad->FindPin(L"Out", &pmadout);		msmpegfail(10);
	hr = gb->Render(pmadout);				msmpegfail(11);

	pmssin->Release();
	pmssout->Release();
	pmadin->Release();
	pmadout->Release();
	mss->Release();
	mad->Release();

	//MessageBox(0, "Success!", 0, 64);
	//hr = -1; msmpegfail(1999);
	return 0;
}

void SetImmediateJump(void *p, uint j)
{
	*(char*)p = 0xE9;
	*(uint*)((char*)p+1) = j - ((uint)p + 5);
}

void SetMemProtection(void *mem, int flags)
{
	MEMORY_BASIC_INFORMATION mbi; DWORD unused;
	VirtualQuery(mem, &mbi, sizeof(mbi));
	VirtualProtect(mem, mbi.RegionSize, (mbi.Protect&0xFFFFFF00) | flags, &unused);
}

int VerifyVersion()
{
	char mname[256];
	DWORD vsize, unk, v;
	UINT valueOutSize;
	void *vpnt;
	VS_FIXEDFILEINFO *ffi;
	
	GetModuleFileName(NULL, mname, 127);
	vsize = GetFileVersionInfoSize(mname, &unk);
	if(!vsize) return -1;
	vpnt = (void*)malloc(vsize);
	if(!vpnt) return -1;
	if(!GetFileVersionInfo(mname, 0, vsize, vpnt)) return -1;
	if(!VerQueryValue(vpnt, "\\", (void**)&ffi, &valueOutSize)) return -1;
	if(valueOutSize == 0) return -1;
	v = LOWORD(ffi->dwFileVersionLS);
	battles = v <= 152;
	if((v != 152) && (v != 366))
	{
		sprintf(mname, "You are using WK%s Build %u.\nBut this patch only works on:\n - WK v1.4 (Build 366)\n - WK - Battles v1.23 (Build 152)\nYou can continue, but the game will still not be patched.", battles ? " - Battles" : "", v);
		if(MessageBox(0, mname, title, MB_ICONERROR | MB_OKCANCEL) != IDOK)
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
		else if(!stricmp(s, "dshow_force_ms_mpeg_codecs"))
			setting_dshow_force_ms_mpeg_codecs = p;
		else if(!stricmp(s, "dshow_no_default_syncsrc"))
			setting_dshow_no_default_syncsrc = p;
		else if(!stricmp(s, "show_all_screen_resolutions"))
			setting_show_all_screen_resolutions = p;
		else if(!stricmp(s, "dshow_waitforcompletion_immediate"))
			setting_dshow_waitforcompletion_immediate = p;
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

	if(!battles) PatchStart_WKO();
	else PatchStart_WKB(); //MessageBox(0, "Battles!", title, 64);

	// Restore entry point code.
	memcpy(exeep, oldepcode, 5);

	// Make the .text section back to non-writable for security reasons.
	SetMemProtection((void*)0x401000, PAGE_EXECUTE_READ);
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
		// Make the .text section writable.
		SetMemProtection((void*)0x401000, PAGE_EXECUTE_READWRITE);

		// Find the entry point address in the PE header.
		exehi = GetModuleHandle(0);
		mz = (char*)exehi;
		if(*(ushort*)mz != 'ZM') return FALSE;
		pe = mz + *(uint*)(mz+0x3C);
		if(*(uint*)pe != 'EP') return FALSE;
		exeep = (char*)( *(uint*)(pe+0x28) + *(uint*)(pe+0x34) );

		// Save the first 5 bytes of the entry point code.
		memcpy(oldepcode, exeep, 5);

		// Put a jump to our function at the beginning of the entry point code.
		SetImmediateJump(exeep, (uint)EntryPointInterception);
	}
	return TRUE;
}
