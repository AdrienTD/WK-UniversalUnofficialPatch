// Global header file

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define UNICODE
#define _UNICODE

#include <windows.h>
#include <stdio.h>
#include <dshow.h>

#define naked __declspec(naked)

#define callvf(p, f) \
__asm mov ecx, p \
__asm mov eax, [ecx] \
__asm call [eax+f*4]

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;

extern const wchar_t* const g_uupVersionDisplay;
extern const wchar_t* const g_title;

extern bool g_isBattles;

extern char setting_higher_time_precision, setting_custom_campaign_crash_fix,
	setting_use_data_directory, setting_use_multi_bcp,
	setting_enable_trace_action , setting_enable_trace_value_action,
	setting_zero_allocated_memory, setting_sight_range_events_bugfix,
	setting_custom_multiplayer_maps, setting_trace_filter,
	setting_no_tutorial_in_skirmish, setting_map_editor_button,
	setting_map_editor_hacks, setting_show_all_screen_resolutions,
	setting_dshow_force_ms_mpeg_codecs, setting_dshow_no_default_syncsrc,
	setting_dshow_waitforcompletion_immediate, setting_dshow_no_bitrate_limit,
	setting_allow_multiple_instances, setting_apply_bcm_sky_texture_and_fog_color;

extern bool setting_ui_performance_improvements,
            setting_ui_messagebox_double_text_render_fix;

void atow(char *a, wchar_t *w, uint ms);
DWORD WINAPI myGetTickCount(void);

void SetImmediateJump(void *p, uint j);
void SetImmediateCall(void *p, uint j);
void NopifyCode(void* p, uint count);

int __stdcall BuildMsMpegGraph(IGraphBuilder *gb, IPin *psrcout);

void PatchStart_WKO();
void PatchStart_WKB();

class MemProtectionChange
{
public:
	MemProtectionChange(void* address, DWORD protection) : newProtection(protection), applied(false) {
		VirtualQuery(address, &memInfo, sizeof(memInfo));
		apply();
	}
	~MemProtectionChange() {
		restore();
	}

	void apply() {
		if(!applied) {
			VirtualProtect(memInfo.BaseAddress, memInfo.RegionSize, newProtection, &oldProtection);
			applied = true;
		}
	}

	void restore() {
		if(applied) {
			DWORD unused;
			VirtualProtect(memInfo.BaseAddress, memInfo.RegionSize, oldProtection, &unused);
			applied = false;
		}
	}
private:
	MEMORY_BASIC_INFORMATION memInfo;
	DWORD newProtection;
	DWORD oldProtection;
	bool applied;
};