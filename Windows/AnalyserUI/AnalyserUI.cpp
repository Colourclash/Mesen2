#include <thread>

#include "Common.h"
#include "AnalyserUI.h"
#include "Core/Shared/Emulator.h"
#include "Shared/DebuggerRequest.h"

// todo: move into imgui_windows.cpp file
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx11.h"
#include "imgui.h"

extern CRITICAL_SECTION _d3dCriticalSection;

static ID3D11Device*					g_pd3dDevice = nullptr;
static IDXGISwapChain1*				g_pSwapChain = nullptr;
//static IDXGISwapChain*				g_pSwapChain = nullptr;
static bool								g_SwapChainOccluded = false;
static UINT								g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView*	g_mainRenderTargetView = nullptr;
static ID3D11DeviceContext*		g_pd3dDeviceContext = nullptr;

bool CreateSwapChain(HWND hWnd);
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

AnalyserUI::AnalyserUI(Emulator* emu, HWND hWndParent, ID3D11Device* pd3dDevice, ID3D11DeviceContext* pDeviceContext)
{
	_emu = emu;
	_hWndParent = hWndParent;

	g_pd3dDeviceContext = pDeviceContext;
	g_pd3dDevice = pd3dDevice;
}

bool AnalyserUI::Init(/*ID3D11Device* pd3dDevice, ID3D11DeviceContext* pDeviceContext*/)
{
	EnterCriticalSection(&_d3dCriticalSection);

	/*g_pd3dDeviceContext = pDeviceContext;
	g_pd3dDevice = pd3dDevice;*/

	WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui", nullptr };
	RegisterClassExW(&wc);
	HWND hWnd = ::CreateWindowExW(WS_EX_APPWINDOW, wc.lpszClassName, L"Analyser", /*WS_EX_APPWINDOW*/  WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, _hWndParent, nullptr, wc.hInstance, nullptr);

	if(!hWnd)
		return false;

	if(!CreateSwapChain(hWnd)) 
	{
		return false;
	}

	// Show the window
	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking

	// todo: get this working
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

	// Temp
	io.FontGlobalScale = 2.0f;
	//io.ConfigViewportsNoAutoMerge = true;
	//io.ConfigViewportsNoTaskBarIcon = true;
	//io.ConfigViewportsNoDefaultParent = true;
	//io.ConfigDockingAlwaysTabBar = true;
	//io.ConfigDockingTransparentPayload = true;
	//io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleFonts;     // FIXME-DPI: Experimental. THIS CURRENTLY DOESN'T WORK AS EXPECTED. DON'T USE IN USER APP!
	//io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports; // FIXME-DPI: Experimental.

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	ImGuiStyle& style = ImGui::GetStyle();
	if(io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

	// Load Fonts
	// - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
	// - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
	// - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
	// - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
	// - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
	// - Read 'docs/FONTS.md' for more instructions and details.
	// - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
	//io.Fonts->AddFontDefault();
	//io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
	//ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
	//IM_ASSERT(font != nullptr);

	LeaveCriticalSection(&_d3dCriticalSection);

	return true;
}

AnalyserUI::~AnalyserUI()
{
	// todo clean up
}

void AnalyserUI::Draw()
{
	static bool show_demo_window = true;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	// Process windows messages.
	// This is needed for the WndProc to be called.
	// Without this, Imgui mouse clicks don't work.
	/*MSG msg;
	while(::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}*/

	// Should I be doing this here?
	// Handle window being minimized or screen locked
	if(g_SwapChainOccluded && g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED) {
		::Sleep(10);
	}
	g_SwapChainOccluded = false;

	EnterCriticalSection(&_d3dCriticalSection);

	// Handle window resize (we don't resize directly in the WM_SIZE handler)
	if(g_ResizeWidth != 0 && g_ResizeHeight != 0) {
		CleanupRenderTarget();
		g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
		g_ResizeWidth = g_ResizeHeight = 0;
		CreateRenderTarget();
	}

	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
	if(show_demo_window)
		ImGui::ShowDemoWindow(&show_demo_window);

	ImGui::Begin("Registers");

	DebuggerRequest req = _emu->GetDebugger(false);
	Debugger* dbg = req.GetDebugger();
	if(dbg) {
		// dbg is only set if debugger window is open
		ImGui::Text("PC: %x", dbg->GetProgramCounter(CpuType::Pce, true));
	}

	//ImGui::Text("counter = %d", counter);
	ImGuiIO& io = ImGui::GetIO();
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
	ImGui::End();

	// Rendering
	ImGui::Render();
	const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
	g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
	g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	// Update and Render additional Platform Windows

	// with ImGuiConfigFlags_ViewportsEnable set we get an empty/black screen for the emulator window.
	// imgui windows will be displayed up to the point the emulator is started/resumed. then they disappear.
	// the imgui windows are shown in separate windows to the emulator window.
	if(io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}

	// Present
	HRESULT hr = g_pSwapChain->Present(1, 0);   // Present with vsync
	//HRESULT hr = g_pSwapChain->Present(0, 0); // Present without vsync
	g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);

	LeaveCriticalSection(&_d3dCriticalSection);
}

void AnalyserUI::Update()
{
	// Process windows messages.
	// This is needed for the WndProc to be called.
	// Without this, Imgui mouse clicks don't work.
	MSG msg;
	while(::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}
	::Sleep(10);


	// Do I need to do this?
	// Handle window being minimized or screen locked
	/*if(g_SwapChainOccluded && g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED) {
		::Sleep(10);
	}*/
}

void AnalyserUI::StartThread()
{
	if(!_thread) {
		if(!_thread) {
			_stopFlag = false;
			_thread.reset(new std::thread(&AnalyserUI::ThreadFunc, this));
		}
	}
}

void AnalyserUI::StopThread()
{
	_stopFlag = true;
	if(_thread) {
		if(_thread) {
			_thread->join();
			_thread.reset();
		}
	}
}

void AnalyserUI::ThreadFunc()
{
	if(!Init())
		return;

	while(!_stopFlag.load()) {
		//Draw();
		Update();
	}
}

void CreateRenderTarget()
{
	ID3D11Texture2D* pBackBuffer;
	g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
	pBackBuffer->Release();
}

void CleanupRenderTarget()
{
	if(g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

bool CreateSwapChain(HWND hWnd)
{
	// Setup swap chain
	DXGI_SWAP_CHAIN_DESC1 sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.Width = 0;
	sd.Height = 0;
	sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	//sd.BufferDesc.RefreshRate.Numerator = 60;
	//sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	//sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	//sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	/*DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferWidth = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;*/

	UINT createDeviceFlags = 0;
	//createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	//D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	
	// Get factory from device
	IDXGIDevice* pDXGIDevice = nullptr;
	IDXGIAdapter* pDXGIAdapter = nullptr;
	IDXGIFactory2* pFactory = nullptr;

	if(g_pd3dDevice->QueryInterface(IID_PPV_ARGS(&pDXGIDevice)) == S_OK) {
		if(pDXGIDevice->GetParent(IID_PPV_ARGS(&pDXGIAdapter)) == S_OK) {
			if(pDXGIAdapter->GetParent(IID_PPV_ARGS(&pFactory)) != S_OK) {
				//bd->pFactory = pFactory;
				// release pdxgidevice & pdxgiadapter?
				pFactory->Release();
				return false;
			}
		}
	}
	if(pDXGIDevice) pDXGIDevice->Release();
	if(pDXGIAdapter) pDXGIAdapter->Release();

	HRESULT hr = DXGI_ERROR_UNSUPPORTED;

	hr = pFactory->CreateSwapChainForHwnd(g_pd3dDevice, hWnd, &sd, nullptr, nullptr, &g_pSwapChain);
	//hr = pFactory->CreateSwapChain(g_pd3dDevice, &sd, &g_pSwapChain);
	if(FAILED(hr))
		return false;

	// is this right to release this here?
	pFactory->Release();

	CreateRenderTarget();
	return true;
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if(ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch(msg) {
		case WM_SIZE:
			if(wParam == SIZE_MINIMIZED)
				return 0;
			g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
			g_ResizeHeight = (UINT)HIWORD(lParam);
			return 0;
		case WM_SYSCOMMAND:
			if((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
				return 0;
			break;
		case WM_DESTROY:
			//::PostQuitMessage(0);
			return 0;
		case WM_DPICHANGED:
			/*if(ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DpiEnableScaleViewports) {
				//const int dpi = HIWORD(wParam);
				//printf("WM_DPICHANGED to %d (%.0f%%)\n", dpi, (float)dpi / 96.0f * 100.0f);
				const RECT* suggested_rect = (RECT*)lParam;
				::SetWindowPos(hWnd, nullptr, suggested_rect->left, suggested_rect->top, suggested_rect->right - suggested_rect->left, suggested_rect->bottom - suggested_rect->top, SWP_NOZORDER | SWP_NOACTIVATE);
			}*/
			break;
	}
	return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}