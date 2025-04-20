#pragma once

#include "pch.h"

#include "IImGuiDraw.h"

class Emulator;

// Todo:
// - deal with if pd3dDevice or pDeviceContext is null
// - provide way to open analyser window if it is closed
// -

class AnalyserUI : public IImGuiDraw
{	
private:
	Emulator* _emu = nullptr;

public:
	AnalyserUI(Emulator* emu);
	~AnalyserUI();

	// IImGuiDraw
	virtual void Draw() override;
	// ~IImGuiDraw
};