#pragma once

#include "Core/Shared/Interfaces/IAnalyserUI.h"

class Emulator;

class AnalyserUI final : public IAnalyserUI
{
private:
	Emulator* _emu = nullptr;

	atomic<bool> _stopFlag;
	unique_ptr<std::thread> _thread;

	// todo shutdown
	
	HWND _hWndParent = nullptr;
public:
	// todo: make this platform agnostic
	AnalyserUI(Emulator* emu, HWND hWndParent, ID3D11Device* pd3dDevice, ID3D11DeviceContext* pDeviceContext);
	~AnalyserUI();

	void StartThread();
	void StopThread();

	void ThreadFunc();

	bool Init(/*ID3D11Device* pd3dDevice, ID3D11DeviceContext* pDeviceContext*/);

	// IAnalyserUI
	virtual void Draw() override;
	// ~IAnalyserUI

	void Update();

};