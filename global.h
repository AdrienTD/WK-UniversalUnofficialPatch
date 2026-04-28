// Global header file

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

#define naked __declspec(naked)

#define callvf(p, f) \
__asm mov ecx, p \
__asm mov eax, [ecx] \
__asm call [eax+f*4]

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;

extern char title[];
extern char tbuf[512];
extern char battles;

extern char setting_higher_time_precision, setting_custom_campaign_crash_fix,
	setting_use_data_directory, setting_use_multi_bcp,
	setting_enable_trace_action , setting_enable_trace_value_action,
	setting_zero_allocated_memory, setting_sight_range_events_bugfix,
	setting_custom_multiplayer_maps, setting_trace_filter,
	setting_no_tutorial_in_skirmish, setting_map_editor_button,
	setting_map_editor_hacks;

void atow(char *a, wchar_t *w, uint ms);
DWORD WINAPI myGetTickCount(void);
void SetImmediateJump(void *p, uint j);
void SetMemProtection(void *mem, int flags);

void PatchStart_WKO();
void PatchStart_WKB();
