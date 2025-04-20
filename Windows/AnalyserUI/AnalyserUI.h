#pragma once

#include "pch.h"

class Emulator;

// Todo:
// - stop thread
// - shutdown imgui
// - deal with if pd3dDevice or pDeviceContext is null
// - provide way to open analyser window if it is closed
// -

class AnalyserUI final
{
private:
	Emulator* _emu = nullptr;

	atomic<bool> _stopFlag = false;
	unique_ptr<std::thread> _thread;

	atomic<bool> _initialisedFlag = false;

	// todo shutdown
	
	// Our window handle
	HWND _hWnd = nullptr;
	HWND _hWndParent = nullptr;

	ID3D11Device*				_pd3dDevice = nullptr;
	IDXGISwapChain1*			_pSwapChain = nullptr;
	bool							_SwapChainOccluded = false;
	ID3D11RenderTargetView*	_mainRenderTargetView = nullptr;
	ID3D11DeviceContext*		_pd3dDeviceContext = nullptr;

private:
	void CreateRenderTarget();
	void CleanupRenderTarget();
	bool CreateSwapChain(HWND hWnd);
	void CleanupD3D();

public:
	// todo: make this platform agnostic
	AnalyserUI(Emulator* emu, HWND hWndParent, ID3D11Device* pd3dDevice, ID3D11DeviceContext* pDeviceContext);
	~AnalyserUI();

	void Start();
	void StopThread();
	void ThreadFunc();

	// Initialise ImGui and create the window
	bool Init();

	void Draw();
	void Update();
};