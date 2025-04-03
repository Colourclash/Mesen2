#include "Common.h"
#include "Renderer.h"
#include "DirectXTK/SpriteBatch.h"
#include "Core/Shared/Emulator.h"
#include "Core/Shared/Video/VideoDecoder.h"
#include "Core/Shared/Video/VideoRenderer.h"
#include "Core/Shared/MessageManager.h"
#include "Core/Shared/SettingTypes.h"
#include "Core/Shared/EmuSettings.h"
#include "Utilities/UTF8Util.h"

using namespace DirectX;

Renderer::Renderer(Emulator* emu, HWND hWnd)
{
	_emu = emu;
	_hWnd = hWnd;

	SetScreenSize(256, 224);
}

Renderer::~Renderer()
{
	VideoRenderer* videoRenderer = _emu->GetVideoRenderer();
	if(videoRenderer) {
		videoRenderer->UnregisterRenderingDevice(this);
	}
	CleanupDevice();
}

void Renderer::SetExclusiveFullscreenMode(bool fullscreen, void* windowHandle)
{
	if(fullscreen != _fullscreen || _hWnd != (HWND)windowHandle) {
		int counter = _resetCounter;

		_hWnd = (HWND)windowHandle;
		_monitorWidth = _emu->GetSettings()->GetVideoConfig().FullscreenResWidth;
		_monitorHeight = _emu->GetSettings()->GetVideoConfig().FullscreenResHeight;

		_newFullscreen = fullscreen;

		while(_resetCounter <= counter) {
			std::this_thread::sleep_for(std::chrono::duration<int, std::milli>(10));
		}
	}
}

DXGI_FORMAT Renderer::GetTextureFormat()
{
	return _useSrgbTextureFormat ? DXGI_FORMAT_B8G8R8A8_UNORM_SRGB : DXGI_FORMAT_B8G8R8A8_UNORM;
}

void Renderer::SetScreenSize(uint32_t width, uint32_t height)
{
	VideoConfig cfg = _emu->GetSettings()->GetVideoConfig();
	FrameInfo rendererSize = _emu->GetVideoRenderer()->GetRendererSize();
	uint32_t refreshRate = _emu->GetFps() < 55 ? cfg.ExclusiveFullscreenRefreshRatePal : cfg.ExclusiveFullscreenRefreshRateNtsc;

	auto needUpdate = [=] {
		return (
			_emuFrameHeight != height ||
			_emuFrameWidth != width ||
			_screenHeight != rendererSize.Height ||
			_screenWidth != rendererSize.Width ||
			_newFullscreen != _fullscreen ||
			_useSrgbTextureFormat != cfg.UseSrgbTextureFormat ||
			(_fullscreen && _fullscreenRefreshRate != refreshRate) ||
			(_fullscreen && (_realScreenHeight != _monitorHeight || _realScreenWidth != _monitorWidth))
		);
	};

	if(needUpdate()) {
		auto frameLock = _frameLock.AcquireSafe();
		auto textureLock = _textureLock.AcquireSafe();
		if(needUpdate()) {
			_emuFrameHeight = height;
			_emuFrameWidth = width;

			bool needReset = _fullscreen != _newFullscreen;
			bool fullscreenResizeMode = _fullscreen && _newFullscreen;

			if(_pSwapChain && _fullscreen && !_newFullscreen) {
				HRESULT hr = _pSwapChain->SetFullscreenState(FALSE, NULL);
				if(FAILED(hr)) {
					MessageManager::Log("SetFullscreenState(FALSE) failed - Error:" + std::to_string(hr));
				}
			}
			
			if(_useSrgbTextureFormat != cfg.UseSrgbTextureFormat) {
				_useSrgbTextureFormat = cfg.UseSrgbTextureFormat;
				needReset = true;
			}

			_fullscreen = _newFullscreen;
			if(_fullscreenRefreshRate != refreshRate) {
				_fullscreenRefreshRate = refreshRate;
				needReset = true;
			}

			_screenHeight = rendererSize.Height;
			_screenWidth = rendererSize.Width;

			if(_fullscreen) {
				if(_realScreenHeight != _monitorHeight) {
					_realScreenHeight = _monitorHeight;
					needReset = true;
				}

				if(_realScreenWidth != _monitorWidth) {
					_realScreenWidth = _monitorWidth;
					needReset = true;
				}

				//Ensure the screen width/height is smaller or equal to the fullscreen resolution, no matter the requested scale
				if(_monitorHeight < _screenHeight || _monitorWidth < _screenWidth) {
					double scale = (double)width / (double)height;
					_screenHeight = _monitorHeight;
					_screenWidth = (uint32_t)(scale * _screenHeight);
					if(_monitorWidth < _screenWidth) {
						_screenWidth = _monitorWidth;
						_screenHeight = (uint32_t)(_screenWidth / scale);
					}
				}
			} else {
				_realScreenHeight = _screenHeight;
				_realScreenWidth = _screenWidth;
			}

			_leftMargin = (_realScreenWidth - _screenWidth) / 2;
			_topMargin = (_realScreenHeight - _screenHeight) / 2;

			_screenBufferSize = _realScreenHeight*_realScreenWidth;

			if(!_pSwapChain || needReset) {
				Reset();
			} else {
				if(fullscreenResizeMode) {
					ResetTextureBuffers();
					CreateEmuTextureBuffers();
				} else {
					ResetTextureBuffers();
					ReleaseRenderTargetView();
					_pSwapChain->ResizeBuffers(1, _realScreenWidth, _realScreenHeight, GetTextureFormat(), 0);
					CreateRenderTargetView();
					CreateEmuTextureBuffers();
				}
			}
		}
	}
}

#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx11.h"
#include "imgui.h"

void Renderer::InitImGui()
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
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
	ImGui_ImplWin32_Init(_hWnd);
	ImGui_ImplDX11_Init(_pd3dDevice, _pDeviceContext);

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
}

void Renderer::Reset()
{
	auto lock = _frameLock.AcquireSafe();
	CleanupDevice();
	if(FAILED(InitDevice())) {
		CleanupDevice();
	} else {
		InitImGui();
		_emu->GetVideoRenderer()->RegisterRenderingDevice(this);
	}

	_resetCounter++;
}

void Renderer::CleanupDevice()
{
	ResetTextureBuffers();
	ReleaseRenderTargetView();
	if(_pSwapChain) {
		_pSwapChain->SetFullscreenState(false, nullptr);
		_pSwapChain->Release();
		_pSwapChain = nullptr;
	}
	if(_pDeviceContext) {
		_pDeviceContext->Release();
		_pDeviceContext = nullptr;
	}
	if(_pd3dDevice) {
		_pd3dDevice->Release();
		_pd3dDevice = nullptr;
	}
	if(_emuHud.Texture) {
		_emuHud.Texture->Release();
		_emuHud.Texture = nullptr;
	}
	if(_emuHud.Shader) {
		_emuHud.Shader->Release();
		_emuHud.Shader = nullptr;
	}
	if(_scriptHud.Texture) {
		_scriptHud.Texture->Release();
		_scriptHud.Texture = nullptr;
	}
	if(_scriptHud.Shader) {
		_scriptHud.Shader->Release();
		_scriptHud.Shader = nullptr;
	}
}

void Renderer::ResetTextureBuffers()
{
	if(_pTexture) {
		_pTexture->Release();
		_pTexture = nullptr;
	}
	if(_pTextureSrv) {
		_pTextureSrv->Release();
		_pTextureSrv = nullptr;
	}

	delete[] _textureBuffer[0];
	_textureBuffer[0] = nullptr;
	delete[] _textureBuffer[1];
	_textureBuffer[1] = nullptr;
}

void Renderer::ReleaseRenderTargetView()
{
	if(_pRenderTargetView) {
		_pRenderTargetView->Release();
		_pRenderTargetView = nullptr;
	}
}

HRESULT Renderer::CreateRenderTargetView()
{
	// Create a render target view
	ID3D11Texture2D* pBackBuffer = nullptr;
	HRESULT hr = _pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	if(FAILED(hr)) {
		MessageManager::Log("SwapChain::GetBuffer() failed - Error:" + std::to_string(hr));
		return hr;
	}

	hr = _pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &_pRenderTargetView);
	pBackBuffer->Release();
	if(FAILED(hr)) {
		MessageManager::Log("D3DDevice::CreateRenderTargetView() failed - Error:" + std::to_string(hr));
		return hr;
	}

	_pDeviceContext->OMSetRenderTargets(1, &_pRenderTargetView, nullptr);

	return S_OK;
}

HRESULT Renderer::CreateEmuTextureBuffers()
{
	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)_realScreenWidth;
	vp.Height = (FLOAT)_realScreenHeight;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	_pDeviceContext->RSSetViewports(1, &vp);

	_textureBuffer[0] = new uint8_t[_emuFrameWidth*_emuFrameHeight * 4];
	_textureBuffer[1] = new uint8_t[_emuFrameWidth*_emuFrameHeight * 4];
	memset(_textureBuffer[0], 0, _emuFrameWidth*_emuFrameHeight * 4);
	memset(_textureBuffer[1], 0, _emuFrameWidth*_emuFrameHeight * 4);

	_pTexture = CreateTexture(_emuFrameWidth, _emuFrameHeight);
	if(!_pTexture) {
		return S_FALSE;
	}
	_pTextureSrv = GetShaderResourceView(_pTexture);
	if(!_pTextureSrv) {
		return S_FALSE;
	}

	////////////////////////////////////////////////////////////////////////////
	_spriteBatch.reset(new SpriteBatch(_pDeviceContext));

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
HRESULT Renderer::InitDevice()
{
	HRESULT hr = S_OK;

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = _realScreenWidth;
	sd.BufferDesc.Height = _realScreenHeight;
	sd.BufferDesc.Format = GetTextureFormat();
	sd.BufferDesc.RefreshRate.Numerator = _fullscreenRefreshRate;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.Flags = _fullscreen ? DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH : 0;
	sd.OutputWindow = _hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;
	for(UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++) {
		driverType = driverTypes[driverTypeIndex];
		featureLevel = D3D_FEATURE_LEVEL_11_1;
		hr = D3D11CreateDeviceAndSwapChain(nullptr, driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels, D3D11_SDK_VERSION, &sd, &_pSwapChain, &_pd3dDevice, &featureLevel, &_pDeviceContext);

		/*if(FAILED(hr)) {
			MessageManager::Log("D3D11CreateDeviceAndSwapChain() failed - Error:" + std::to_string(hr));
		}*/

		if(hr == E_INVALIDARG) {
			// DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
			featureLevel = D3D_FEATURE_LEVEL_11_0;
			hr = D3D11CreateDeviceAndSwapChain(nullptr, driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1, D3D11_SDK_VERSION, &sd, &_pSwapChain, &_pd3dDevice, &featureLevel, &_pDeviceContext);
		}

		if(SUCCEEDED(hr)) {
			break;
		}
	}
		
	if(FAILED(hr)) {
		MessageManager::Log("D3D11CreateDeviceAndSwapChain() failed - Error:" + std::to_string(hr));
		return hr;
	}

	if(_fullscreen) {
		hr = _pSwapChain->SetFullscreenState(TRUE, NULL);
		if(FAILED(hr)) {
			MessageManager::Log("SetFullscreenState(true) failed - Error:" + std::to_string(hr));
			MessageManager::Log("Switching back to windowed mode");
			hr = _pSwapChain->SetFullscreenState(FALSE, NULL);
			if(FAILED(hr)) {
				MessageManager::Log("SetFullscreenState(false) failed - Error:" + std::to_string(hr));
				return hr;
			}
		} else {
			//Get actual monitor resolution (which might differ from the one that was requested)
			HMONITOR monitor = MonitorFromWindow(_hWnd, MONITOR_DEFAULTTOPRIMARY);
			MONITORINFO info = {};
			info.cbSize = sizeof(MONITORINFO);
			GetMonitorInfo(monitor, &info);

			uint32_t monitorWidth = info.rcMonitor.right - info.rcMonitor.left;
			uint32_t monitorHeight = info.rcMonitor.bottom - info.rcMonitor.top;

			if(_monitorHeight != monitorHeight || _monitorWidth != monitorWidth) {
				MessageManager::Log(
					"Requested resolution (" + std::to_string(_monitorWidth) + "x" + std::to_string(_monitorHeight) 
					+ ") is not available. Resetting to nearest match instead: " +
					std::to_string(monitorWidth) + "x" + std::to_string(monitorHeight)
				);
				_monitorWidth = monitorWidth;
				_monitorHeight = monitorHeight;

				//Make UI wait until this 2nd reset is over
				_resetCounter--;
			}
		}
	}

	hr = CreateRenderTargetView();
	if(FAILED(hr)) {
		return hr;
	}
	hr = CreateEmuTextureBuffers();
	if(FAILED(hr)) {
		return hr;
	}

	return S_OK;
}

ID3D11Texture2D* Renderer::CreateTexture(uint32_t width, uint32_t height)
{
	ID3D11Texture2D* texture;

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
	desc.ArraySize = 1;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.Format = GetTextureFormat();
	desc.MipLevels = 1;
	desc.MiscFlags = 0;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.Width = width;
	desc.Height = height;
	desc.MiscFlags = 0;

	HRESULT hr = _pd3dDevice->CreateTexture2D(&desc, nullptr, &texture);
	if(FAILED(hr)) {
		MessageManager::Log("D3DDevice::CreateTexture() failed - Error:" + std::to_string(hr));
		return nullptr;
	}
	return texture;
}

ID3D11ShaderResourceView* Renderer::GetShaderResourceView(ID3D11Texture2D* texture)
{
	ID3D11ShaderResourceView *shaderResourceView = nullptr;
	HRESULT hr = _pd3dDevice->CreateShaderResourceView(texture, nullptr, &shaderResourceView);
	if(FAILED(hr)) {
		MessageManager::Log("D3DDevice::CreateShaderResourceView() failed - Error:" + std::to_string(hr));
		return nullptr;
	}

	return shaderResourceView;
}

void Renderer::ClearFrame()
{
	//Clear current output and display black frame
	auto lock = _textureLock.AcquireSafe();
	if(_textureBuffer[0]) {
		//_textureBuffer[0] may be null if directx failed to initialize properly
		memset(_textureBuffer[0], 0, _emuFrameWidth * _emuFrameHeight * sizeof(uint32_t));
		_needFlip = true;
		_frameChanged = true;
	}
}

void Renderer::UpdateFrame(RenderedFrame& frame)
{
	SetScreenSize(frame.Width, frame.Height);

	auto lock = _textureLock.AcquireSafe();
	if(_textureBuffer[0]) {
		//_textureBuffer[0] may be null if directx failed to initialize properly
		memcpy(_textureBuffer[0], frame.FrameBuffer, frame.Width*frame.Height*sizeof(uint32_t));
		_needFlip = true;
		_frameChanged = true;
	}
}

void Renderer::DrawScreen()
{
	//Swap buffers - emulator always writes to _textureBuffer[0], screen always draws _textureBuffer[1]
	if(_needFlip) {
		auto lock = _textureLock.AcquireSafe();
		uint8_t* textureBuffer = _textureBuffer[0];
		_textureBuffer[0] = _textureBuffer[1];
		_textureBuffer[1] = textureBuffer;
		_needFlip = false;

		if(_frameChanged) {
			_frameChanged = false;
		}
	}

	//Copy buffer to texture
	uint32_t bpp = 4;
	uint32_t rowPitch = _emuFrameWidth * bpp;
	D3D11_MAPPED_SUBRESOURCE dd;
	HRESULT hr = _pDeviceContext->Map(_pTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &dd);
	if(FAILED(hr)) {
		MessageManager::Log("DeviceContext::Map() failed - Error:" + std::to_string(hr));
		return;
	}
	uint8_t* surfacePointer = (uint8_t*)dd.pData;
	uint8_t* videoBuffer = _textureBuffer[1];
	if(rowPitch != dd.RowPitch) {
		for(uint32_t i = 0, iMax = _emuFrameHeight; i < iMax; i++) {
			memcpy(surfacePointer, videoBuffer, rowPitch);
			videoBuffer += rowPitch;
			surfacePointer += dd.RowPitch;
		}
	} else {
		memcpy(surfacePointer, videoBuffer, rowPitch * _emuFrameHeight);
	}
	_pDeviceContext->Unmap(_pTexture, 0);

	RECT destRect;
	destRect.left = _leftMargin;
	destRect.top = _topMargin;
	destRect.right = _screenWidth+_leftMargin;
	destRect.bottom = _screenHeight+_topMargin;

	_spriteBatch->Draw(_pTextureSrv, destRect);
}

bool Renderer::CreateHudTexture(HudRenderInfo& hud, uint32_t newWidth, uint32_t newHeight)
{
	if(hud.Texture) {
		hud.Texture->Release();
		hud.Texture = nullptr;
	}
	if(hud.Shader) {
		hud.Shader->Release();
		hud.Shader = nullptr;
	}

	hud.Width = newWidth;
	hud.Height = newHeight;

	hud.Texture = CreateTexture(hud.Width, hud.Height);
	if(!hud.Texture) {
		return false;
	}
	hud.Shader = GetShaderResourceView(hud.Texture);
	if(!hud.Shader) {
		return false;
	}

	return true;
}

void Renderer::DrawHud(HudRenderInfo& hud, RenderSurfaceInfo& hudSurface)
{
	uint32_t* hudBuffer = hudSurface.Buffer;
	uint32_t newWidth = hudSurface.Width;
	uint32_t newHeight = hudSurface.Height;

	if(newWidth == 0 && newHeight == 0) {
		return;
	}

	bool needRedraw = hudSurface.IsDirty;
	if(hud.Width != newWidth || hud.Height != newHeight || !hud.Texture || !hud.Shader) {
		needRedraw = true;
		if(!CreateHudTexture(hud, newWidth, newHeight)) {
			return;
		}
	}

	if(needRedraw) {
		//Copy buffer to texture
		uint32_t rowPitch = hud.Width * sizeof(uint32_t);
		D3D11_MAPPED_SUBRESOURCE dd;
		HRESULT hr = _pDeviceContext->Map(hud.Texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &dd);
		if(FAILED(hr)) {
			MessageManager::Log("DeviceContext::Map() failed - Error:" + std::to_string(hr));
			return;
		}
		uint8_t* surfacePointer = (uint8_t*)dd.pData;
		uint8_t* videoBuffer = (uint8_t*)hudBuffer;
		if(rowPitch != dd.RowPitch) {
			for(uint32_t i = 0, iMax = hud.Height; i < iMax; i++) {
				memcpy(surfacePointer, videoBuffer, rowPitch);
				videoBuffer += rowPitch;
				surfacePointer += dd.RowPitch;
			}
		} else {
			memcpy(surfacePointer, videoBuffer, hud.Height * rowPitch);
		}
		_pDeviceContext->Unmap(hud.Texture, 0);
	}
	
	RECT destRect;
	destRect.left = _leftMargin;
	destRect.top = _topMargin;
	destRect.right = _screenWidth + _leftMargin;
	destRect.bottom = _screenHeight + _topMargin;

	_spriteBatch->Draw(hud.Shader, destRect);
}

void Renderer::RenderImGui()
{
	static bool show_demo_window = true;
	static bool show_another_window = false;

	// I don't know if it's bad to do this here...
	MSG msg;
	while(::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}

	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
	if(show_demo_window)
		ImGui::ShowDemoWindow(&show_demo_window);

	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
	{
		static float f = 0.0f;
		static int counter = 0;

		ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

		ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
		ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
		ImGui::Checkbox("Another Window", &show_another_window);

		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		//ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

		if(ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
			counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);

		//ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
		ImGui::End();
	}

	// 3. Show another simple window.
	if(show_another_window) {
		ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
		ImGui::Text("Hello from another window!");
		if(ImGui::Button("Close Me"))
			show_another_window = false;
		ImGui::End();
	}

	// Rendering
	ImGui::Render();
	//const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
	
	// ?
	//g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
	//g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	// Update and Render additional Platform Windows
	ImGuiIO& io = ImGui::GetIO();
	if(io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}

void Renderer::Render(RenderSurfaceInfo& emuHud, RenderSurfaceInfo& scriptHud)
{
	auto lock = _frameLock.AcquireSafe();
	if(_newFullscreen != _fullscreen) {
		SetScreenSize(_emuFrameWidth, _emuFrameHeight);
	}

	if(_pDeviceContext == nullptr) {
		//DirectX failed to initialize, try to init
		Reset();
		if(_pDeviceContext == nullptr) {
			//Can't init, prevent crash
			return;
		}
	}

	VideoConfig cfg = _emu->GetSettings()->GetVideoConfig();

	// Clear the back buffer 
	_pDeviceContext->ClearRenderTargetView(_pRenderTargetView, Colors::Black);

	//Draw screen
	_spriteBatch->Begin(SpriteSortMode_Immediate, cfg.UseBilinearInterpolation);
	DrawScreen();
	_spriteBatch->End();

	//Draw HUD
	_spriteBatch->Begin(SpriteSortMode_Immediate, false);
	DrawHud(_scriptHud, scriptHud);
	DrawHud(_emuHud, emuHud);
	_spriteBatch->End();

	RenderImGui();

	// Present the information rendered to the back buffer to the front buffer (the screen)
	HRESULT hr = _pSwapChain->Present(cfg.VerticalSync ? 1 : 0, 0);
	if(FAILED(hr)) {
		MessageManager::Log("SwapChain::Present() failed - Error:" + std::to_string(hr));
		if(hr == DXGI_ERROR_DEVICE_REMOVED) {
			MessageManager::Log("D3DDevice: GetDeviceRemovedReason: " + std::to_string(_pd3dDevice->GetDeviceRemovedReason()));
		}
		MessageManager::Log("Trying to reset DX...");
		Reset();
	}
}
