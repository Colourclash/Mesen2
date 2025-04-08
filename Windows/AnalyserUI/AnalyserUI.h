#pragma once

#include "Core/Shared/Interfaces/IAnalyserUI.h"

class Emulator;

class AnalyserUI final : public IAnalyserUI
{
private:
	Emulator* _emu = nullptr;

	// todo shutdown

public:
	AnalyserUI(Emulator* emu);
	~AnalyserUI();

	// todo: make this platform agnostic
	void Init(HWND hWnd, ID3D11Device* _pd3dDevice, ID3D11DeviceContext* _pDeviceContext);

	// IAnalyserUI
	virtual void Draw() override;
	// ~IAnalyserUI
};