// WK (Og + Battles) Text drawing fixes

// This fixes in particular a crash when the game tries to render a very long line of text,
// especially when drawing the AI general bio text on 4K and ultrawide screen resolutions.

// Unfortunately something as simple as placing text characters on the screen
// is not exempt from classic and unnecessary bugs:
// * Buffer overflow:
//   When drawing a long text that does not fit the length of a label control,
//   the game has to split it in lines and draw each line separately.
//   For this it has to copy each line to a fixed-size buffer on the stack.
//   But there does not seem to be any boundary check...
// * String formatting:
//   For some reason the game seems to prefer using the formatting version of
//   the text drawing function over the string only one, even if there are no parameters given.
//   This means unfortunately a player can just enter "%i" in a edit text box and see funny things
//   going on. Or worse type "%n" and the game crashes instantly.
//   Even though the devs might have thought of not exceeding the buffer bounds this time,
//   the string may still get truncated, which would not have happened if it was using the
//   unformatted text drawing function instead.

#include "global.h"

#include <algorithm>

void naked __cdecl sub_WKCalcText_WithoutFormatting_WKO(void* interface2d, int* out_x, int* out_y, const wchar_t* text)
{
	__asm {
        mov eax, [esp+4]
		mov edx, [esp+8]
		push dword ptr [esp+16]

        // fontStyle object
		mov eax, [eax]
        mov eax, [eax+0x18]
        add eax, 0x44
		push eax

		push dword ptr [esp+20]
		mov ecx, [eax] // first dword of fontstyle object, which is the Bitmap_font pointer, used as "this" for WKCalcText1
		mov eax, 0x5E32B0 // WKCalcText1
		call eax
        
        mov ecx, [esp+4]
        mov ecx, [ecx+4]
        mov eax, [esp+8]
        fild dword ptr [eax]
        fidiv dword ptr [ecx+0x14]
        fstp dword ptr [eax]
        mov eax, [esp+12]
        fild dword ptr [eax]
        fidiv dword ptr [ecx+0x18]
        fstp dword ptr [eax]
		
        ret
	}
}

void naked __cdecl sub_WKCalcText_WithoutFormatting_WKB(void* interface2d, int* out_x, int* out_y, void* fontStyle, const wchar_t* text)
{
	__asm {
		mov edx, [esp+8]
		push dword ptr [esp+20]
		mov eax, [esp+20]
		push eax
		push dword ptr [esp+20]
		mov ecx, [eax] // first dword of fontstyle object, which is the Bitmap_font pointer, used as "this" for WKCalcText1
		mov eax, 0x5C0540 // WKCalcText1
		call eax
		ret
	}
}

void naked __cdecl sub_WKDrawText2_WithoutFormatting_WKO(void* interface2d, int pos_x, int pos_y, const wchar_t* text)
{
	__asm {
		mov edx, [esp+4]
		push dword ptr [esp+16]
		push dword ptr [esp+16]
		push dword ptr [esp+16]

        // fontStyle object
		mov eax, [edx]
        mov eax, [eax+0x18]
        add eax, 0x44
		push eax

		mov ecx, [eax] // first dword of fontstyle object, which is the Bitmap_font pointer, used as "this" for WKDrawText1
		mov eax, 0x5E3280 // WKDrawText1
		call eax
		ret
	}
}

void naked __cdecl sub_WKDrawText2_WithoutFormatting_WKB(void* interface2d, void* fontStyle, int pos_x, int pos_y, const wchar_t* text)
{
	__asm {
		mov edx, [esp+4]
		push dword ptr [esp+20]
		push dword ptr [esp+20]
		push dword ptr [esp+20]
		mov eax, [esp+20]
		push eax
		mov ecx, [eax] // first dword of fontstyle object, which is the Bitmap_font pointer, used as "this" for WKDrawText1
		mov eax, 0x5C0510 // WKDrawText1
		call eax
		ret
	}
}


void __cdecl wcsncpy_SizeLimited2047AndNullTerminate(wchar_t* dest, const wchar_t* src, size_t count)
{
	size_t actualCount = std::min(count, 2047u);
	wcsncpy(dest, src, actualCount);
	dest[actualCount] = 0;
}

void PatchDrawLabelTextBufferOverflow_WKO()
{
	SetImmediateCall((void*)0x593EEC, (uint)wcsncpy_SizeLimited2047AndNullTerminate);
	NopifyCode((void*)0x593F08, 7);
	SetImmediateCall((void*)0x593F72, (uint)wcsncpy_SizeLimited2047AndNullTerminate);
	NopifyCode((void*)0x593F87, 10);

	// Relocate addresses to line charbuf to word charbuf.
	// The content of the word charbuf is no longer needed at this point of code, so it is safe to merge both buffers into a bigger one.
	*(uint*)0x593F6C -= 0x800;
	*(uint*)0x593F82 -= 0x800;
}

void PatchDrawLabelTextBufferOverflow_WKB()
{
	SetImmediateCall((void*)0x724046, (uint)wcsncpy_SizeLimited2047AndNullTerminate);
	NopifyCode((void*)0x724070, 10);
	SetImmediateCall((void*)0x724122, (uint)wcsncpy_SizeLimited2047AndNullTerminate);
	NopifyCode((void*)0x724140, 8);

	// Relocate addresses to line charbuf to word charbuf.
	// The content of the word charbuf is no longer needed at this point of code, so it is safe to merge both buffers into a bigger one.
	*(uint*)0x724040 -= 0x800;
	*(uint*)0x72404E -= 0x800;
}


void PatchStart_WKO_DrawTextFixes()
{
    static const int drawText2Calls[] = {
        0x40b6bf, 0x40bb52, 0x40cbe9, 0x40cc30, 0x40cc77, 0x40ccbe, 0x40cd05, 0x40cf96, 0x40d418, 0x40d4bf, 0x40d77f,
        0x40d7bb, 0x58d97c, 0x592f44, 0x593fa0, 0x594856, 0x594a5d, 0x594c30, 0x594fdf, 0x59524d, 0x59545b, 0x595c9f,
        0x595fb5, 0x59963c, 0x59a188, 0x59ff24, 0x5a00e7, 0x5a03c0, 0x5a056e, 0x5a07a8, 0x5a0b31, 0x5a0db2, 0x5a100c,
        0x5a12ce, 0x5a1dae, 0x5a24c7, 0x5a2be1, 0x5a4744, 0x5bdd8b, 0x5bddf0, 0x5d46ca, 0x6416f5
    };

    for(int call : drawText2Calls) {
		SetImmediateCall((void*)call, (uint)sub_WKDrawText2_WithoutFormatting_WKO);
	}

	static const int calcTextCalls[] = {
        0x40b631, 0x40bae6, 0x40d38f, 0x40d45a, 0x40d535, 0x40d550, 0x40d70f, 0x58c0bc, 0x58c0f6, 0x592e80, 0x5936b3,
        0x593f0f, 0x5947e4, 0x5949a5, 0x594bbb, 0x594f27, 0x5951d2, 0x5951ee, 0x5953e8, 0x595b85, 0x595e03, 0x5995e1,
        0x599711, 0x59b323, 0x59fe6c, 0x5a0072, 0x5a0306, 0x5a04b4, 0x5a06ee, 0x5a08c9, 0x5a098a, 0x5a09d2, 0x5a0ab9,
        0x5a0b46, 0x5a0dc7, 0x5a0f39, 0x5a109d, 0x5a10bf, 0x5a125b, 0x5a1c94, 0x5a21fc, 0x5a2282, 0x5a2b27, 0x5a46d0,
        0x5bda9b, 0x5bdce9, 0x5d4648, 0x6415c6, 0x641646, 0x641b68, 0x641db0
    };

    for(int call : calcTextCalls) {
		SetImmediateCall((void*)call, (uint)sub_WKCalcText_WithoutFormatting_WKO);
	}

    PatchDrawLabelTextBufferOverflow_WKO();
}


void PatchStart_WKB_DrawTextFixes()
{
	static const int drawText2Calls[] = {
		0x42144F, 0x62C42E, 0x64CF8A, 0x719A04, 0x71B8AF, 0x7203EB, 0x72063B, 0x7208F1, 0x72265C, 0x7248F9, 0x725FFC,
		0x7281D1, 0x7294EC, 0x72B378, 0x72B58B, 0x72C3E3, 0x72EDD7, 0x73034C, 0x73050E, 0x73D7BE, 0x764B2B, 0x77C2EC,
		0x77C340, 0x77C44E, 0x77C471, 0x784081, 0x79AFBB, 0x79B2AE, 0x79B30B, 0x79B357, 0x79B3A3, 0x79B41F
	};
	
	for(int call : drawText2Calls) {
		SetImmediateCall((void*)call, (uint)sub_WKDrawText2_WithoutFormatting_WKB);
	}
	
	static const int calcTextCalls[] = {
		0x62c3f9, 0x64cd0d, 0x64f082, 0x655406, 0x71998e, 0x71b3d0, 0x71b507, 0x71b56e, 0x7201b2, 0x720260, 0x720298,
		0x720370, 0x720408, 0x720662, 0x7207d4, 0x720994, 0x7209c6, 0x7225c3, 0x72320f, 0x72324b, 0x72407a, 0x724159,
		0x7243d9, 0x724486, 0x7244f8, 0x72473e, 0x72817d, 0x729476, 0x72b322, 0x72b532, 0x72c30e, 0x72ed4f, 0x72fa11,
		0x7302cb, 0x730486, 0x739fa5, 0x73d76e, 0x75ad5d, 0x75cd29, 0x764202, 0x764ac7, 0x771ea2, 0x7799a1, 0x779d81,
		0x77a0c1, 0x784019, 0x78da0e, 0x78da73, 0x78df8d, 0x78e1bb, 0x79af54, 0x79b3c5, 0x79d7d5, 0x7a38cb
	};
	
	for(int call : calcTextCalls) {
		SetImmediateCall((void*)call, (uint)sub_WKCalcText_WithoutFormatting_WKB);
	}

    PatchDrawLabelTextBufferOverflow_WKB();

    // Forbid typing the % character in edit boxes
    // Reason is to prevent players to accidently send % to unpatched clients in multiplayer.
    for(uint editBoxAllowedCharsAddress : {0x71FFCC, 0x720030}) {
        // bools of allowed chars from '"' (0x22) to '|' (0x7C)
        uchar* editBoxAllowedChars = (uchar*)editBoxAllowedCharsAddress;
        editBoxAllowedChars['%' - 0x22] = 0;
    }
}
