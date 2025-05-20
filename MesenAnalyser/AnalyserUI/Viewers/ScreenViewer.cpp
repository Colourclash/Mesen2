#include "ScreenViewer.h"

#include <imgui.h>

#include <Windows.h>

#include "../ImGuiSupport/ImGuiTexture.h"
#include "Core/Shared/Emulator.h"
#include "Core/Shared/Video/VideoRenderer.h"
#include "Core/Shared/Video/SystemHud.h"

CRITICAL_SECTION gScreenViewerFrameBufferCS;

bool ScreenViewer::Init(void)
{
	FrameInfo screenSize = _pEmu->GetVideoRenderer()->GetRendererSize();

	ResizeFrameBuffer(256, 240);

	InitializeCriticalSection(&gScreenViewerFrameBufferCS);

	return true;
}

void ScreenViewer::DestroyFrameBuffer()
{
	if (ScreenTexture) 
	{
		ImGui_FreeTexture(ScreenTexture);
	}
	ScreenTexture = 0;
}

void ScreenViewer::ResizeFrameBuffer(int width, int height)
{
	DestroyFrameBuffer();

	// The image flickers when not rounded up to 32 
	const uint32_t widthRounded = (width + 31) & ~31;
	const uint32_t heightRounded = (height + 31) & ~31;

	FrameBufferWidth = widthRounded;
	FrameBufferHeight = heightRounded;

	const size_t pixelBufferSize = FrameBufferWidth * FrameBufferHeight;
	FrameBuffer = new uint32_t[pixelBufferSize * 2]; // what is the * 2 for?
	ScreenTexture = ImGui_CreateTextureRGBA(FrameBuffer, widthRounded, heightRounded);

	ScreenWidth = width;
	ScreenHeight = height;
}

void ScreenViewer::Shutdown(void)
{
	DestroyFrameBuffer();
	DeleteCriticalSection(&gScreenViewerFrameBufferCS);
}

// Chris Covell says 90% of games run at 256 x 239 
void ScreenViewer::SetFrameData(uint32_t* pBuffer, uint32_t width, uint32_t height)
{
	// lock and copy data into frame buffer
	EnterCriticalSection(&gScreenViewerFrameBufferCS);

	if (width != ScreenWidth || height != ScreenHeight)
		ResizeFrameBuffer(width, height);

	uint32_t* pCurDst = FrameBuffer;
	uint32_t* pCurSrc = pBuffer;
	const int diffVal = FrameBufferWidth - ScreenWidth;
	for(int y = 0; y < ScreenHeight; y++) {
		for(int x = 0; x < ScreenWidth; x++) {

			// Red and Blue need swapping round
			const uint8_t byte1 = (*pCurSrc >> 16) & 0xff;
			const uint8_t byte2 = (*pCurSrc >> 8) & 0xff;
			const uint8_t byte3 = *pCurSrc & 0xff;
			*pCurDst = (0xff << 24) | (byte3 << 16) | (byte2 << 8) | byte1;

			pCurDst++;
			pCurSrc++;
		}
		pCurDst += diffVal;
	}

	LeaveCriticalSection(&gScreenViewerFrameBufferCS);
}

void ScreenViewer::DrawUI() 
{
#ifdef DRAW_TEST_PATTERN1
	// Draw test pattern
	// This code presumes that the screen height is divisible by 4..
	ImU32 col[4] = { ImColor(255, 0, 0), ImColor(0, 255, 0),	ImColor(0, 0, 255), ImColor(127, 127, 127) };

	uint32_t* pCur = FrameBuffer;
	const size_t quartScr = (FrameBufferHeight * FrameBufferWidth) >> 2;
	for (int q = 0; q < 4; q++) 
	{
		for(int i = 0; i < quartScr; i++) 
		{
			*pCur = col[q];
			 pCur++;
		}
	}
#endif
#ifdef DRAW_TEST_PATTERN2
	// deals with framebuffer being bigger than screen size
	uint32_t* pCur = FrameBuffer;
	const int diffVal = FrameBufferWidth - ScreenWidth;
	for(int y = 0; y < ScreenHeight; y++) 
	{
		for(int x = 0; x < ScreenWidth; x++)
		{
			*pCur = ImColor(255, 255, 0);
			pCur++;
		}
		pCur += diffVal;
	}
#endif
	
	ImGui::Text("FPS %u", _pEmu->GetVideoRenderer()->GetSystemHud()->GetFps());

	EnterCriticalSection(&gScreenViewerFrameBufferCS);

	ImGui_UpdateTextureRGBA(ScreenTexture, FrameBuffer);

	LeaveCriticalSection(&gScreenViewerFrameBufferCS);

	const FrameInfo screenSize = _pEmu->GetVideoRenderer()->GetRendererSize();

	if (screenSize.Height != FrameBufferHeight || screenSize.Width != FrameBufferWidth) 
	{
		ImGui::Text("screen size has changed");
	}
	
	ImGui::Text("Screen size: %d x %d", ScreenWidth, ScreenHeight);
	ImGui::Text("Frame buffer size: %d x %d", FrameBufferWidth, FrameBufferHeight);

	ImVec2 uv0(0, 0);
	ImVec2 uv1(1.f, 1.0f);
	ImGui::Image(ScreenTexture, ImVec2(FrameBufferWidth, FrameBufferHeight), uv0, uv1);
}
