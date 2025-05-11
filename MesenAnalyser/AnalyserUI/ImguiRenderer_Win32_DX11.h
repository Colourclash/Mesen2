#pragma once

#include <atomic>

#include "IImGuiRenderer.h"

// Todo:
// - deal with if pd3dDevice or pDeviceContext is null
// - provide way to open analyser window if it is closed
// -

class IImGuiDraw;

class ImGuiRenderer_Win32_DX11 : public IImGuiRenderer
{
private:
	std::atomic<bool> _stopFlag = false;
	unique_ptr<std::thread> _thread;
	std::atomic<bool> _initialisedFlag = false;

	// Our window handle
	HWND _hWnd = nullptr;
	HWND _hWndParent = nullptr;

	ID3D11Device*				_pd3dDevice = nullptr;
	IDXGISwapChain1*			_pSwapChain = nullptr;
	bool							_SwapChainOccluded = false;
	ID3D11RenderTargetView*	_mainRenderTargetView = nullptr;
	ID3D11DeviceContext*		_pd3dDeviceContext = nullptr;

	IImGuiDraw* _imguiDraw = nullptr;
private:
	// Initialise ImGui and create the window
	bool Init();

	void ThreadFunc();
	void StopThread();

	void CreateRenderTarget();
	void CleanupRenderTarget();
	bool CreateSwapChain(HWND hWnd);
	void CleanupD3D();

public:
	ImGuiRenderer_Win32_DX11(IImGuiDraw* pImGuiDraw, HWND hWndParent, ID3D11Device* pd3dDevice, ID3D11DeviceContext* pDeviceContext);
	~ImGuiRenderer_Win32_DX11();

	// IImGuiRenderer
	virtual void Start() override; // Init and start thread
	virtual void Draw() override;
	// ~IImGuiRenderer
	
	void Update();
};