#include "ScreenViewer.h"

#include <imgui.h>

#include "../ImGuiSupport/ImGuiTexture.h"
#include "Core/Shared/Emulator.h"
#include "Core/Shared/Video/VideoRenderer.h"

bool ScreenViewer::Init(void)
{
	FrameInfo screenSize = _pEmu->GetVideoRenderer()->GetRendererSize();

	// setup pixel buffer
	//const size_t pixelBufferSize = dispInfo.frame.dim.width * dispInfo.frame.dim.height;
	const size_t pixelBufferSize = screenSize.Width * screenSize.Height;
	FrameBuffer = new uint32_t[pixelBufferSize * 2];
	ScreenTexture = ImGui_CreateTextureRGBA(FrameBuffer, screenSize.Width, screenSize.Height);
	FrameBufferWidth = screenSize.Width;
	FrameBufferHeight = screenSize.Height;
	return true;
}

void ScreenViewer::Shutdown(void)
{

}

void ScreenViewer::DrawUI() 
{
	// draw test pattern
	ImU32 colr = ImColor(1.f, 0.0f, 0.f, 1.0f);
	ImU32 colg = ImColor(0.f, 1.0f, 0.f, 1.0f);
	ImU32 colb = ImColor(0.f, 0.0f, 1.f, 1.0f);
	ImU32 colm = ImColor(0.5f, 0.5f, 0.5f, 1.0f);

	for(int x = 0; x < FrameBufferWidth; x++) {
		for(int y = 0; y < FrameBufferHeight; y++) {
			const size_t quartScr = FrameBufferHeight >> 2;
			if (y < quartScr)
				FrameBuffer[y * FrameBufferWidth + x] = colr;
			else if(y < quartScr * 2)
				FrameBuffer[y * FrameBufferWidth + x] = colg;
			else if(y < quartScr * 3)
				FrameBuffer[y * FrameBufferWidth + x] = colb;
			else
				FrameBuffer[y * FrameBufferWidth + x] = colm;
		}
	}
	// convert texture to RGBA
	/*const uint8_t* pix = (const uint8_t*)disp.frame.buffer.ptr;
	const uint32_t* pal = (const uint32_t*)disp.palette.ptr;
	for(int i = 0; i < disp.frame.buffer.size; i++)
		FrameBuffer[i] = pal[pix[i]];*/

	ImGui_UpdateTextureRGBA(ScreenTexture, FrameBuffer);

	FrameInfo screenSize = _pEmu->GetVideoRenderer()->GetRendererSize();

	if(screenSize.Height != FrameBufferHeight || screenSize.Width != FrameBufferWidth) 
	{
		// todo recreate texture & framebuffer here
		ImGui::Text("screen size has changed");
	}
	
	ImGui::Text("Screen size: %d x %d", FrameBufferWidth, FrameBufferHeight);

	ImVec2 uv0(0, 0);
	ImVec2 uv1(1.f, 1.0f);
	ImGui::Image(ScreenTexture, ImVec2(FrameBufferWidth, FrameBufferHeight), uv0, uv1);
}
