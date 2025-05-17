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

private:
	uint32_t* FrameBuffer;	// pixel buffer to store emu output
	ImTextureID		ScreenTexture;		// texture
	size_t FrameBufferWidth = 0;
	size_t FrameBufferHeight = 0;
};

