// Dear ImGui: standalone example application for DirectX 11

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>

#include "Utilities/FolderUtilities.h"
#include "Core/Shared/Emulator.h"
#include "Core/Shared/MessageManager.h"
#include "Core/Shared/EmuSettings.h"
#include "Core/Shared/SettingTypes.h"
#include "Core/Shared/Video/VideoRenderer.h"
#include "AnalyserUI/AnalyserUI.h"
#include "AnalyserUI/ScreenRenderer.h"

// Data
static ID3D11Device*            g_pd3dDevice = nullptr;
static ID3D11DeviceContext*     g_pd3dDeviceContext = nullptr;
static IDXGISwapChain*          g_pSwapChain = nullptr;
static bool                     g_SwapChainOccluded = false;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView*  g_mainRenderTargetView = nullptr;

ID3D11Device* GetD3dDevice()
{
	return g_pd3dDevice;
}
ID3D11DeviceContext* GetD3dDeviceContext()
{
	return g_pd3dDeviceContext;
}

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Main code
int main(int, char**)
{
    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Mesen Analyser", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, nullptr, nullptr, wc.hInstance, nullptr);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

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
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
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

    // Our state
#if 0
    bool show_demo_window = true;
    bool show_another_window = false;
#endif
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	 MessageManager::SetOsdState(false);

	 FolderUtilities::SetHomeFolder("C:\\temp\\mesenhome\\");

	 Emulator* pEmulator = new Emulator();
	 pEmulator->Initialize();

	 // Create a working PCE config, otherwise the screen is black.
	 // todo: do this in a better way.
	 PcEngineConfig pceConfig;
	 pceConfig.Overscan = { 18, 18, 3, 0};
	 constexpr uint32_t pcePalette[512] = { 0xFF000000, 0xFF000016, 0xFF040235, 0xFF01004B, 0xFF07046A, 0xFF050180, 0xFF0B069F, 0xFF0903B5, 0xFF1A0003, 0xFF200521, 0xFF1E0237, 0xFF240756, 0xFF22046C, 0xFF28098B, 0xFF2506A1, 0xFF2B0BC0, 0xFF3D080D, 0xFF3B0524, 0xFF400A42, 0xFF3E0759, 0xFF440C77, 0xFF42098D, 0xFF480EAC, 0xFF450BC2, 0xFF570910, 0xFF550526, 0xFF5B0A45, 0xFF59075B, 0xFF5E0C7A, 0xFF5C0990, 0xFF620EAF, 0xFF600BC5, 0xFF720913, 0xFF770E31, 0xFF750B48, 0xFF7B1066, 0xFF790C7C, 0xFF7F119B, 0xFF7C0EB1, 0xFF8213D0, 0xFF94111E, 0xFF920E34, 0xFF981352, 0xFF951069, 0xFF9B1587, 0xFF99129E, 0xFF9F17BC, 0xFF9D13D2, 0xFFAE1120, 0xFFAC0E36, 0xFFB21355, 0xFFB0106B, 0xFFB6158A, 0xFFB312A0, 0xFFB917BF, 0xFFB714D5, 0xFFC91123, 0xFFCF1641, 0xFFCC1358, 0xFFD21876, 0xFFD0158C, 0xFFD61AAB, 0xFFD417C1, 0xFFDA1CE0, 0xFF092408, 0xFF07211E, 0xFF0D263D, 0xFF0A2353, 0xFF102872, 0xFF0E2588, 0xFF142AA7, 0xFF1127BD, 0xFF23240B, 0xFF212121, 0xFF27263F, 0xFF252356, 0xFF2A2874, 0xFF28258B, 0xFF2E2AA9, 0xFF2C27BF, 0xFF3E240D, 0xFF43292C, 0xFF412642, 0xFF472B61, 0xFF452877, 0xFF4B2D95, 0xFF482AAC, 0xFF4E2FCA, 0xFF602C18, 0xFF5E292E, 0xFF642E4D, 0xFF612B63, 0xFF673082, 0xFF652D98, 0xFF6B32B7, 0xFF692FCD, 0xFF7A2D1B, 0xFF782931, 0xFF7E2E50, 0xFF7C2B66, 0xFF823084, 0xFF7F2D9B, 0xFF8532B9, 0xFF832FD0, 0xFF952D1D, 0xFF9B323C, 0xFF982F52, 0xFF9E3471, 0xFF9C3087, 0xFFA236A6, 0xFFA032BC, 0xFFA637DA, 0xFFB73528, 0xFFB5323E, 0xFFBB375D, 0xFFB93473, 0xFFBF3992, 0xFFBC36A8, 0xFFC23BC7, 0xFFC038DD, 0xFFD2352B, 0xFFCF3241, 0xFFD53760, 0xFFD33476, 0xFFD93994, 0xFFD736AB, 0xFFDC3BC9, 0xFFDA38E0, 0xFF0A4008, 0xFF0F4526, 0xFF0D423D, 0xFF13475B, 0xFF114471, 0xFF174990, 0xFF1445A6, 0xFF1A4AC5, 0xFF2C4813, 0xFF2A4529, 0xFF304A47, 0xFF2D475E, 0xFF334C7C, 0xFF314993, 0xFF2F46A9, 0xFF354BC7, 0xFF464815, 0xFF44452B, 0xFF4A4A4A, 0xFF484760, 0xFF4E4C7F, 0xFF4B4995, 0xFF514EB4, 0xFF4F4BCA, 0xFF614818, 0xFF674D36, 0xFF644A4D, 0xFF6A4F6B, 0xFF684C81, 0xFF6E51A0, 0xFF6C4EB6, 0xFF7253D5, 0xFF835123, 0xFF814D39, 0xFF875257, 0xFF854F6E, 0xFF8B548C, 0xFF8851A3, 0xFF864EB9, 0xFF8C53D7, 0xFF9E5125, 0xFF9B4D3C, 0xFFA1535A, 0xFF9F4F70, 0xFFA5548F, 0xFFA351A5, 0xFFA856C4, 0xFFA653DA, 0xFFB85128, 0xFFBE5646, 0xFFBC535D, 0xFFC1587B, 0xFFBF5592, 0xFFC55AB0, 0xFFC356C6, 0xFFC95CE5, 0xFFDA5933, 0xFFD85649, 0xFFDE5B68, 0xFFDC587E, 0xFFD95594, 0xFFDF5AB3, 0xFFDD57C9, 0xFFE35CE8, 0xFF126410, 0xFF106126, 0xFF166645, 0xFF14625B, 0xFF1A6779, 0xFF176490, 0xFF1D69AE, 0xFF1B66C5, 0xFF2D6412, 0xFF336931, 0xFF306647, 0xFF366B66, 0xFF34687C, 0xFF3A6D9B, 0xFF3869B1, 0xFF3E6FCF, 0xFF476415, 0xFF4D6933, 0xFF4B664A, 0xFF516B68, 0xFF4E687F, 0xFF546D9D, 0xFF526AB3, 0xFF586FD2, 0xFF6A6C20, 0xFF676936, 0xFF6D6E55, 0xFF6B6B6B, 0xFF717089, 0xFF6F6DA0, 0xFF7472BE, 0xFF726FD5, 0xFF846C22, 0xFF8A7141, 0xFF886E57, 0xFF8D7376, 0xFF8B708C, 0xFF9175AB, 0xFF8F72C1, 0xFF9577DF, 0xFF9E6C25, 0xFFA47144, 0xFFA26E5A, 0xFFA87378, 0xFFA6708F, 0xFFAB75AD, 0xFFA972C4, 0xFFAF77E2, 0xFFC17530, 0xFFBE7246, 0xFFC47765, 0xFFC2737B, 0xFFC8799A, 0xFFC675B0, 0xFFCC7ACE, 0xFFC977E5, 0xFFDB7532, 0xFFE17A51, 0xFFDF7767, 0xFFE57C86, 0xFFE2799C, 0xFFE87EBB, 0xFFE67BD1, 0xFFEC80F0, 0xFF137F0F, 0xFF19842E, 0xFF178144, 0xFF1D8663, 0xFF1A8379, 0xFF208898, 0xFF1E85AE, 0xFF248ACD, 0xFF36881A, 0xFF338531, 0xFF398A4F, 0xFF378665, 0xFF3D8C84, 0xFF3B889A, 0xFF418DB9, 0xFF3E8ACF, 0xFF50881D, 0xFF568D3B, 0xFF548A52, 0xFF598F70, 0xFF578C87, 0xFF5D91A5, 0xFF5B8EBB, 0xFF6193DA, 0xFF6A8820, 0xFF708D3E, 0xFF6E8A54, 0xFF748F73, 0xFF728C89, 0xFF7791A8, 0xFF758EBE, 0xFF7B93DD, 0xFF8D902A, 0xFF8B8D41, 0xFF90925F, 0xFF8E8F76, 0xFF949494, 0xFF9291AA, 0xFF9896C9, 0xFF9593DF, 0xFFA7902D, 0xFFAD954C, 0xFFAB9262, 0xFFB19780, 0xFFAE9497, 0xFFB499B5, 0xFFB296CC, 0xFFB89BEA, 0xFFC19030, 0xFFC7964E, 0xFFC59264, 0xFFCB9783, 0xFFC99499, 0xFFCF99B8, 0xFFCC96CE, 0xFFD29BED, 0xFFE4993A, 0xFFE29651, 0xFFE89B6F, 0xFFE59886, 0xFFEB9DA4, 0xFFE999BA, 0xFFEF9ED9, 0xFFED9BEF, 0xFF1CA317, 0xFF22A836, 0xFF20A54C, 0xFF26AA6B, 0xFF23A781, 0xFF29ACA0, 0xFF27A9B6, 0xFF25A6CC, 0xFF36A31A, 0xFF3CA939, 0xFF3AA54F, 0xFF40AA6D, 0xFF3EA784, 0xFF43ACA2, 0xFF41A9B9, 0xFF47AED7, 0xFF59AC25, 0xFF57A93B, 0xFF5CAE5A, 0xFF5AAB70, 0xFF60B08F, 0xFF5EACA5, 0xFF64B2C3, 0xFF61AEDA, 0xFF73AC28, 0xFF79B146, 0xFF77AE5C, 0xFF7DB37B, 0xFF7AB091, 0xFF78ADA8, 0xFF7EB2C6, 0xFF7CAEDC, 0xFF8DAC2A, 0xFF93B149, 0xFF91AE5F, 0xFF97B37E, 0xFF95B094, 0xFF9BB5B2, 0xFF98B2C9, 0xFF9EB7E7, 0xFFB0B435, 0xFFAEB14B, 0xFFB4B66A, 0xFFB1B380, 0xFFB7B89F, 0xFFB5B5B5, 0xFFBBBAD4, 0xFFB9B7EA, 0xFFCAB438, 0xFFD0B956, 0xFFCEB66C, 0xFFD4BB8B, 0xFFD2B8A1, 0xFFCFB5B8, 0xFFD5BAD6, 0xFFD3B7EC, 0xFFE5B53A, 0xFFEBBA59, 0xFFE8B66F, 0xFFEEBB8E, 0xFFECB8A4, 0xFFF2BDC2, 0xFFF0BAD9, 0xFFF5BFF7, 0xFF25C71F, 0xFF23C436, 0xFF28C954, 0xFF26C66B, 0xFF2CCB89, 0xFF2AC89F, 0xFF30CDBE, 0xFF2DCAD4, 0xFF3FC722, 0xFF3DC438, 0xFF43C957, 0xFF40C66D, 0xFF46CB8C, 0xFF44C8A2, 0xFF4ACDC1, 0xFF48CAD7, 0xFF59C825, 0xFF5FCD43, 0xFF5DC959, 0xFF63CF78, 0xFF61CB8E, 0xFF67D0AD, 0xFF64CDC3, 0xFF6AD2E2, 0xFF7CD02F, 0xFF7ACD46, 0xFF80D264, 0xFF7DCF7B, 0xFF83D499, 0xFF81D1AF, 0xFF87D6CE, 0xFF85D2E4, 0xFF96D032, 0xFF94CD48, 0xFF9AD267, 0xFF98CF7D, 0xFF9ED49C, 0xFF9BD1B2, 0xFFA1D6D1, 0xFF9FD3E7, 0xFFB1D035, 0xFFB7D553, 0xFFB4D26A, 0xFFBAD788, 0xFFB8D49E, 0xFFBED9BD, 0xFFBCD6D3, 0xFFC1DBF2, 0xFFD3D840, 0xFFD1D556, 0xFFD7DA74, 0xFFD5D78B, 0xFFDADCA9, 0xFFD8D9C0, 0xFFDEDEDE, 0xFFDCDBF4, 0xFFEED842, 0xFFEBD558, 0xFFF1DA77, 0xFFEFD78D, 0xFFF5DCAC, 0xFFF2D9C2, 0xFFF8DEE1, 0xFFF6DBF7, 0xFF25E31F, 0xFF2BE83E, 0xFF29E554, 0xFF2FEA73, 0xFF2DE789, 0xFF33ECA7, 0xFF30E9BE, 0xFF36EEDC, 0xFF48EB2A, 0xFF46E840, 0xFF4CED5F, 0xFF49EA75, 0xFF4FEF94, 0xFF4DECAA, 0xFF53F1C9, 0xFF51EEDF, 0xFF62EC2D, 0xFF60E843, 0xFF66ED61, 0xFF64EA78, 0xFF6AEF96, 0xFF67ECAD, 0xFF6DF1CB, 0xFF6BEEE1, 0xFF7DEC2F, 0xFF83F14E, 0xFF80EE64, 0xFF86F383, 0xFF84EF99, 0xFF8AF4B7, 0xFF88F1CE, 0xFF8DF6EC, 0xFF9FF43A, 0xFF9DF150, 0xFFA3F66F, 0xFFA1F385, 0xFFA6F8A4, 0xFFA4F5BA, 0xFFAAFAD9, 0xFFA8F6EF, 0xFFBAF43D, 0xFFB7F153, 0xFFBDF672, 0xFFBBF388, 0xFFC1F8A6, 0xFFBFF5BD, 0xFFC4FADB, 0xFFC2F7F2, 0xFFD4F43F, 0xFFDAF95E, 0xFFD7F674, 0xFFDDFB93, 0xFFDBF8A9, 0xFFE1FDC8, 0xFFDFFADE, 0xFFE5FFFC, 0xFFF6FC4A, 0xFFF4F960, 0xFFFAFE7F, 0xFFF8FB95, 0xFFFEFFB4, 0xFFFBFDCA, 0xFFFFFFE9, 0xFFFFFFFF };
	 std::copy(std::begin(pcePalette), std::end(pcePalette), std::begin(pceConfig.Palette));

	 pEmulator->GetSettings()->SetPcEngineConfig(pceConfig);
	 //pEmulator->LoadRom(VirtualFile("c:\\temp\\rabiolepus.pce"), VirtualFile());
	 pEmulator->LoadRom(VirtualFile("c:\\temp\\bonk.pce"), VirtualFile());
	 //pEmulator->LoadRom(VirtualFile("c:\\temp\\240pSuite.pce"), VirtualFile());
	 //pEmulator->LoadRom(VirtualFile("c:\\temp\\padtest.pce"), VirtualFile());

	 AnalyserUI* pAnalyserUI = new AnalyserUI(pEmulator);
	 pAnalyserUI->Init();

	 ScreenRenderer* pRenderer = new ScreenRenderer(pEmulator, pAnalyserUI);
	 pEmulator->GetVideoRenderer()->RegisterRenderingDevice(pRenderer);

    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // Handle window being minimized or screen locked
        if (g_SwapChainOccluded && g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
        {
            ::Sleep(10);
            continue;
        }
        g_SwapChainOccluded = false;

        // Handle window resize (we don't resize directly in the WM_SIZE handler)
        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
        }

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

		  pAnalyserUI->Draw();

#if 0
        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
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
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }
#endif

        // Rendering
        ImGui::Render();
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        // Present
        HRESULT hr = g_pSwapChain->Present(1, 0);   // Present with vsync
        //HRESULT hr = g_pSwapChain->Present(0, 0); // Present without vsync
        g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

// Helper functions
bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
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
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
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
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

#ifndef WM_DPICHANGED
#define WM_DPICHANGED 0x02E0 // From Windows SDK 8.1+ headers
#endif

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    case WM_DPICHANGED:
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DpiEnableScaleViewports)
        {
            //const int dpi = HIWORD(wParam);
            //printf("WM_DPICHANGED to %d (%.0f%%)\n", dpi, (float)dpi / 96.0f * 100.0f);
            const RECT* suggested_rect = (RECT*)lParam;
            ::SetWindowPos(hWnd, nullptr, suggested_rect->left, suggested_rect->top, suggested_rect->right - suggested_rect->left, suggested_rect->bottom - suggested_rect->top, SWP_NOZORDER | SWP_NOACTIVATE);
        }
        break;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
