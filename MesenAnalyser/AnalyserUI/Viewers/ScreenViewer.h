#pragma once

#include "ViewerBase.h"
#include "imgui.h"

class ScreenViewer : public ViewerBase
{
public:
	ScreenViewer(Emulator* pEmu, AnalyserUI* pAnalyserUI) : ViewerBase(pEmu, pAnalyserUI) { _name = "Screen"; }
	bool	Init(void) override;
	void	Shutdown(void) override;
	void	DrawUI() override;

	void SetFrameData(uint32_t* pBuffer, uint32_t width, uint32_t height);

private:
	void ResizeFrameBuffer(int width, int height);
	void DestroyFrameBuffer();

private:
	uint32_t* FrameBuffer = nullptr;	// pixel buffer to store emu output
	ImTextureID		ScreenTexture = 0;		// texture
	
	uint32_t FrameBufferWidth = 0;
	uint32_t FrameBufferHeight = 0;
	uint32_t ScreenWidth = 0;
	uint32_t ScreenHeight = 0;
};

