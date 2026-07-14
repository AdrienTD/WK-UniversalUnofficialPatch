// WKB UI Performance improvements and fixes

// In WKB, UI elements are drawn by rendering textured quads.
// The DX8 driver prepares the vertex data and uses the IDirect3DDevice8::DrawPrimitiveUP method
// to render one quad.
// I precise, **ONE** quad per draw call!
// This can be verified with any graphics analyzing tool, such as PIX from the (latest but old) DirectX SDK.
//
// Worse, when rendering a full image, this image may be split in several ones to respect
// power-of-2 texture sizes (for example, a 48x48 texture is split in 4 32x32 textures).
// So whenever the game renders an image multiple times for background filling, it keeps switching
// the current texture to the different parts for every single quad, which may also hurt
// performance.
//
// So the performance improvements here basically tries to batch multiple quads of same
// texture (part) and submit them in a single draw call.
//
// WK1 may have similar issues, but its UI does not render as many quads as WKB does.
// This is why for now these improvements are exclusive to WKB.

#include "global.h"

#include <map>
#include <vector>
#include <d3d9.h> // need some enums for D3D8, luckily some have same values in D3D9

struct UiBox
{
	float left, top, right, bottom;
};

struct TexCoord
{
	float u, v;
};

struct Vertex_PosColUv
{
	float x, y, z, w;
	int color;
	TexCoord uv;
};

struct UiQuadBuffer {
	std::vector<Vertex_PosColUv> uiVertexBuffer;
	std::vector<ushort> uiIndexBuffer;
};
std::vector<void*> uiQuadBufferKeys;
std::vector<UiQuadBuffer> uiQuadBuffers;
int uiSkipFlushAtRectRender = 0;
int uiClearBufferAfterDrawText = 1;

void __stdcall UiAddQuadToBuffers(void* tex, const UiBox* box, const TexCoord* uv0, const TexCoord* uv1, const TexCoord* uv2, const TexCoord* uv3, uint color)
{
	const auto itKey = std::find(uiQuadBufferKeys.begin(), uiQuadBufferKeys.end(), tex);
	size_t bufferIndex;
	if(itKey != uiQuadBufferKeys.end()) {
		bufferIndex = itKey - uiQuadBufferKeys.begin();
	}
	else {
		bufferIndex = uiQuadBufferKeys.size();
		uiQuadBufferKeys.push_back(tex);
		if(uiQuadBuffers.size() <= bufferIndex) {
			uiQuadBuffers.resize(bufferIndex + 1);
		}
	}

	auto& quadBuffers = uiQuadBuffers[bufferIndex];
	auto& uiVertexBuffer = quadBuffers.uiVertexBuffer;
	auto& uiIndexBuffer = quadBuffers.uiIndexBuffer;

	ushort firstIndex = (ushort)uiVertexBuffer.size();
	uiVertexBuffer.resize(uiVertexBuffer.size() + 4);
	Vertex_PosColUv& v0 = uiVertexBuffer[firstIndex+0];
	Vertex_PosColUv& v1 = uiVertexBuffer[firstIndex+1];
	Vertex_PosColUv& v2 = uiVertexBuffer[firstIndex+2];
	Vertex_PosColUv& v3 = uiVertexBuffer[firstIndex+3];
	v0.x = box->right; v0.y = box->bottom; v0.uv = *uv3;
	v1.x = box->left; v1.y = box->bottom; v1.uv = *uv2;
	v2.x = box->left; v2.y = box->top; v2.uv = *uv0;
	v3.x = box->right; v3.y = box->top; v3.uv = *uv1;
	for(auto* v : {&v0, &v1, &v2, &v3}) {
		v->z = 0.0f;
		v->w = 1.0f;
		v->color = color;
	}
	uiIndexBuffer.push_back(firstIndex);
	uiIndexBuffer.push_back(firstIndex+1);
	uiIndexBuffer.push_back(firstIndex+2);
	uiIndexBuffer.push_back(firstIndex);
	uiIndexBuffer.push_back(firstIndex+2);
	uiIndexBuffer.push_back(firstIndex+3);
}

void naked __stdcall Fake_IDirect3DDevice8_DrawIndexedPrimitiveUP(void* device, int primitiveType, int minVertex, int numVertices, int numPrims, void* indexData, int indexType, void* vertexData, int vertexStride)
{
	__asm {
		mov eax, [esp+4]
		mov eax, [eax]
		jmp dword ptr [eax+124h]
	}
}

void naked __stdcall UiConfigureStreamSource(void* somePointer, int someInt)
{
	// unfortunately __thiscall cannot be used unless it is a class method...
	__asm {
		mov eax, [esp]
		add esp, 4
		mov ecx, [esp]
		mov [esp], eax
		mov eax, 0x5F3C70
		jmp eax
	}
}

void naked __stdcall UiConfigureRenderStates(void* driver, void* renderStateObject)
{
	// normally this was called through the 2dInterface method at 5F0F50,
	// which just redirects to driver method 5E8460:DX8_SetRenderStatesIfDifferent
	// because we get the driver pointer here, why not call the driver method directly
	__asm {
		mov edx, [esp+8]
		mov ecx, [esp+4]
		mov eax, [ecx]
		call dword ptr [eax+4Ch] // calls 5E8460:DX8_SetRenderStatesIfDifferent
		ret 8
	}
}

void __stdcall UiClearBuffers()
{
	const size_t numBuffers = uiQuadBufferKeys.size();
	for(size_t bufferIndex = 0; bufferIndex < numBuffers; ++bufferIndex) {
		auto& buffers = uiQuadBuffers[bufferIndex];
		buffers.uiVertexBuffer.clear();
		buffers.uiIndexBuffer.clear();
	}
	uiQuadBufferKeys.clear();
}

void __stdcall UiFlushBuffersEx(void* interface2d, int clear)
{
	// Interface2d is usually an instance of class DirectX_8::Interface_2d
	// But when the window is minimized, it is changed to an instance of Managed_rendering_interface_2d
	// So we need to be careful when trying to get the D3D8 driver as it may not be available.
	uint interface2dVirtualTablePtr = *(uint*)interface2d;
	if(interface2dVirtualTablePtr != 0x843B14) { // DirectX_8::Interface_2d
		if(clear) {
			UiClearBuffers();
		}
		return;
	}

	void* const driver = *(void**)((uchar*)interface2d + 0x68);
	void* const device = *(void**)((uchar*)driver + 0x94);
	void* const somePointer = *(void**)((uchar*)driver + 0x1D0);

	const size_t numBuffers = uiQuadBufferKeys.size();
	bool streamSourceSet = false;
	for(size_t bufferIndex = 0; bufferIndex < numBuffers; ++bufferIndex) {
		void* const tex = uiQuadBufferKeys[bufferIndex];
		auto& buffers = uiQuadBuffers[bufferIndex];
		if(buffers.uiIndexBuffer.empty())
			continue;

		if(!streamSourceSet) {
			UiConfigureStreamSource(somePointer, 0x144);
			streamSourceSet = true;
		}

		UiConfigureRenderStates(driver, tex);

		auto& uiVertexBuffer = buffers.uiVertexBuffer;
		auto& uiIndexBuffer = buffers.uiIndexBuffer;
		Fake_IDirect3DDevice8_DrawIndexedPrimitiveUP(device, D3DPT_TRIANGLELIST, 0, uiVertexBuffer.size(), uiIndexBuffer.size() / 3, uiIndexBuffer.data(), D3DFMT_INDEX16, uiVertexBuffer.data(), sizeof(Vertex_PosColUv));
		if(clear) {
			uiVertexBuffer.clear();
			uiIndexBuffer.clear();
		}
	}

	if(clear) {
		uiQuadBufferKeys.clear();
	}
}

void __stdcall UiFlushBuffers(void* interface2d)
{
	UiFlushBuffersEx(interface2d, uiClearBufferAfterDrawText);
}

void __stdcall UiTweakBuffer(float x, float y, uint color)
{
	const size_t numBuffers = uiQuadBufferKeys.size();
	for(size_t bufferIndex = 0; bufferIndex < numBuffers; ++bufferIndex) {
		for(auto& vertex : uiQuadBuffers[bufferIndex].uiVertexBuffer) {
			vertex.x += x;
			vertex.y += y;
			vertex.color = color;
		}
	}
}

static naked void loc_5c58e8_2DInterfaceRenderHook()
{
	__asm {
		mov     ecx, [esp+80h]
		//mov     eax, [edi]
		push    ecx
		lea     edx, [esp+1Ch]
		push    edx
		lea     ecx, [esp+28h]
		push    ecx
		lea     edx, [esp+34h]
		push    edx
		lea     ecx, [esp+40h]
		push    ecx
		lea     edx, [esp+64h]
		//mov     ecx, edi
		//call    dword ptr [eax+30h] // this calls 5F0F90:Dx8Int2D_DrawTexturedQuad
		push edx
		push dword ptr [esi+20h]
		call UiAddQuadToBuffers

		mov eax, 0x5C5919
		jmp eax
	}
}

static naked void loc_5c591e_2DInterfaceRenderLoopBreakHook()
{
	__asm {
		// the replaced instruction, jumps to beginning of loop if 'for' condition remains true
		jz     breakLoop
		mov eax, 0x5C5597
		jmp eax

	breakLoop:
		// if the loop ends, we want to flush the buffers, if buffers are enabled
		mov eax, uiSkipFlushAtRectRender
		test eax, eax
		jnz noFlush

		push edi
		call UiFlushBuffers

	noFlush:
		mov eax, 0x5C5924 // next to replaced instruction
		jmp eax
	}
}

static naked void loc_5bec85_StartOfDrawText()
{
	__asm {
		sub esp, 208h

		inc uiSkipFlushAtRectRender

		mov eax, 0x5BEC8B
		jmp eax
	}
}

static naked void loc_5bf031_EndOfDrawText()
{
	__asm {
		mov eax, [esp+20h] // pointer to DX8Interface2d object
		test eax, eax
		jz noDriver

		push eax
		call UiFlushBuffers

	noDriver:
		dec uiSkipFlushAtRectRender

		mov ecx, [esp+218h] // the replaced instruction
		mov eax, 0x5BF038
		jmp eax
	}
}

static naked void loc_77c5aa_StartOfDrawWindowBackground()
{
	__asm {
		//lea ebx, [ebx+0] // yes, that was the replaced instruction :)
		mov uiSkipFlushAtRectRender, 1

		mov eax, 0x77C5B0
		jmp eax

	}
}

static naked void loc_77c63c_EndOfDrawWindowBackground()
{
	__asm {
		// esi = pointer to DX8Interface2d object
		push esi
		call UiFlushBuffers

		mov uiSkipFlushAtRectRender, 0

		// the replaced instructions:
		mov edx, [esi]
		mov ecx, esi
		call dword ptr [edx+28h]

		mov eax, 0x77c643
		jmp eax
	}
}

static naked void loc_77c650_StartOfDrawWindowBorder()
{
	__asm {
		// replaced instructions
		sub esp, 30h
		fld dword ptr [edx]

		mov uiSkipFlushAtRectRender, 1

		mov eax, 0x77C655
		jmp eax

	}
}

static naked void loc_77cad6_EndOfDrawWindowBorder()
{
	__asm {
		// esi = pointer to DX8Interface2d object
		push esi
		call UiFlushBuffers

		mov uiSkipFlushAtRectRender, 0

		// the replaced instructions:
		mov edx, [esi]
		mov ecx, esi
		call dword ptr [edx+28h]

		mov eax, 0x77CADD
		jmp eax
	}
}

static naked void loc_77caf4_StartOfDrawButtonBackground()
{
	__asm {
		// replaced instructions
		mov ebx, [esp+80h]

		mov uiSkipFlushAtRectRender, 1

		mov eax, 0x77cafb
		jmp eax

	}
}

static naked void loc_77ce3c_EndOfDrawButtonBackground()
{
	__asm {
		// edi = pointer to DX8Interface2d object
		push edi
		call UiFlushBuffers

		mov uiSkipFlushAtRectRender, 0

		// the replaced instructions:
		pop edi
		pop esi
		pop ebp
		pop ebx
		add esp, 78h
		ret 0Ch
	}
}

float uiOutlinePosChanges[8] = {
	0.0f, -2.0f,
	1.0f, 1.0f,
	-2.0f, 0.0f,
	1.0f, 0.0f
};
uint uiOutlineColorChanges[4] = {0, 0, 0, 0xFFFFFFFF};

void __cdecl sub_WKDrawText2_WithoutFormatting_WKB(void* interface2d, void* fontStyle, int pos_x, int pos_y, const wchar_t* text);

static naked void loc_77c435_Outlined_Text_Optim()
{
	__asm {
		mov uiClearBufferAfterDrawText, 0

		// original code:
		mov     edx, [esp+esi*8+1Ch]
		mov     eax, [esp+esi*8+18h]
		push    edi             ; wchar_t *
		add     edx, ebx
		push    edx             ; int
		mov     edx, [esp+18h]
		add     eax, ebp
		push    eax             ; int
		lea     ecx, [esp+44h]
		push    ecx             ; int
		push    edx             ; int
		//mov     eax, 0x5C5E70
		//call    eax  // WKDrawText2
		call sub_WKDrawText2_WithoutFormatting_WKB
		add     esp, 14h

		mov edi, 0
	outlineLoop:
		mov eax, [dword ptr uiOutlineColorChanges + edi*4]
		and eax, [esp+14h] // local variable containing text color
		push eax
		push [dword ptr uiOutlinePosChanges + 4 + edi*8]
		push [dword ptr uiOutlinePosChanges + edi*8]
		call UiTweakBuffer

		mov eax, [esp+10h] // pointer to 2dInterface object
		push eax
		call UiFlushBuffers

		inc edi
		cmp edi, 4
		jnz outlineLoop

		mov uiClearBufferAfterDrawText, 1
		call UiClearBuffers

		mov eax, 0x77c479
		jmp eax
	}
}

static naked void loc_778fed_StartOfDrawScrollBar()
{
	__asm {
		// replaced instructions
		lea edx, [esp+20h]
		push edx

		mov uiSkipFlushAtRectRender, 1

		mov eax, 0x778ff2
		jmp eax
	}
}

static naked void loc_7790ae_EndOfDrawScrollBar()
{
	__asm {
		// edi = pointer to DX8Interface2d object
		push edi
		call UiFlushBuffers

		mov uiSkipFlushAtRectRender, 0

		// the replaced instructions
		mov eax, [edi]
		mov ecx, edi
		call dword ptr [eax+28h]

		mov eax, 0x7790B5
		jmp eax
	}
}

void naked __cdecl WKDrawText2(void* interface2d, void* fontStyle, int pos_x, int pos_y, const wchar_t* text, ...)
{
	__asm {
		mov eax, 0x5C5E70
		jmp eax
	}
}

void __stdcall DrawPerfStatsExtension(void* interface2d, void* fontStyle)
{
	auto h = [](int i) -> int { return 180 + 12*i; };
	WKDrawText2(interface2d, fontStyle, 8, h(1), L"=== UUP Stats ===");
	WKDrawText2(interface2d, fontStyle, 8, h(2), L"Num UI buffers: %u", uiQuadBuffers.size());
}

void naked loc_5c528e_CallDrawPerfStatsExtension()
{
	__asm {
		lea ecx, [esp+10h] // pointer to font style object (allocated in stack)
		push ecx
		push esi
		call DrawPerfStatsExtension

		// replaced instructions:
		lea ecx, [esp+10h]
		mov [esp+38h], 0xFFFFFFFF

		mov eax, 0x5c529a
		jmp eax
	}
}

void PatchStart_WKB_UiPerformanceImprovements()
{
	// UI Rendering performance improvement
	if(setting_ui_performance_improvements) {
		SetImmediateJump((void*)0x5c58e8, (uint)loc_5c58e8_2DInterfaceRenderHook);
		SetImmediateJump((void*)0x5c591e, (uint)loc_5c591e_2DInterfaceRenderLoopBreakHook);
		SetImmediateJump((void*)0x77c435, (uint)loc_77c435_Outlined_Text_Optim);

		SetImmediateJump((void*)0x5bec85, (uint)loc_5bec85_StartOfDrawText);
		SetImmediateJump((void*)0x5bf031, (uint)loc_5bf031_EndOfDrawText);
		SetImmediateJump((void*)0x77c5aa, (uint)loc_77c5aa_StartOfDrawWindowBackground);
		SetImmediateJump((void*)0x77c63c, (uint)loc_77c63c_EndOfDrawWindowBackground);
		SetImmediateJump((void*)0x77c650, (uint)loc_77c650_StartOfDrawWindowBorder);
		SetImmediateJump((void*)0x77cad6, (uint)loc_77cad6_EndOfDrawWindowBorder);
		SetImmediateJump((void*)0x77caf4, (uint)loc_77caf4_StartOfDrawButtonBackground);
		SetImmediateJump((void*)0x77ce3c, (uint)loc_77ce3c_EndOfDrawButtonBackground);
		SetImmediateJump((void*)0x778fed, (uint)loc_778fed_StartOfDrawScrollBar);
		SetImmediateJump((void*)0x7790ae, (uint)loc_7790ae_EndOfDrawScrollBar);

		// Extension to the Performance Stats (that can be enabled via registry)
		SetImmediateJump((void*)0x5c528e, (uint)loc_5c528e_CallDrawPerfStatsExtension);
	}

	// Fix UI message box text drawn twice
	if(setting_ui_messagebox_double_text_render_fix) {
		*(uchar*)0x76483C = 0xEB; // turn a conditional jump to always jump to skip second text drawing
	}
}