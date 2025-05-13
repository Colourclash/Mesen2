#include "AnalyserUI.h"

#include <thread>
#include "imgui.h"
//#include "Common.h"

#include "Core/Shared/Emulator.h"
#include "Core/Shared/NotificationManager.h"

// Viewers
#include "Viewers/CallstackViewer.h"
#include "Viewers/CodeAnalysisViewer.h"
#include "Viewers/GlobalsViewer.h"
#include "Viewers/RegistersViewer.h"
#include "Viewers/ScreenViewer.h"

AnalyserUI::AnalyserUI(Emulator* emu)
{
	_pEmu = emu;

	_pCodeViewer = new CodeAnalysisViewer(_pEmu, this);
	AddViewer(_pCodeViewer);

	AddViewer(new GlobalsViewer(_pEmu, this));
	AddViewer(new RegistersViewer(_pEmu, this));
	AddViewer(new CallstackViewer(_pEmu, this));
	AddViewer(new ScreenViewer(_pEmu, this));

	_sharedSelf.reset(this);
}

AnalyserUI::~AnalyserUI()
{
	// crash here destroying the list of viewers
}

bool AnalyserUI::Init()
{
	_pEmu->GetNotificationManager()->RegisterNotificationListener(shared_from_this());
	return true;
}

void AnalyserUI::Reset()
{
	_consoleType = _pEmu->GetConsoleType();
	_cpuType = _pEmu->GetCpuTypes()[0];
}

// Note: this is called on a different thread to the main Draw() function
void AnalyserUI::ProcessNotification(ConsoleNotificationType type, void* parameter)
{
	if(type == ConsoleNotificationType::GameLoaded) {
		Reset();
		/*shared_ptr<IConsole> console = _emu->GetConsole();
		if(console) {
		}*/
	}
}

void AnalyserUI::Draw()
{
	DrawDockingView();
}


void AnalyserUI::DrawUI()
{
	if(_bShowImGuiDemo)
		ImGui::ShowDemoWindow(&_bShowImGuiDemo);

	if(!_pEmu->GetConsole().get())
		return;
	
	if (_consoleType != _pEmu->GetConsoleType())
	{
		Reset();
	}
	
	/*if(_bShowImPlotDemo)
		ImPlot::ShowDemoWindow(&_bShowImPlotDemo);*/

	// Draw registered viewers
	for(auto viewer : _viewers) {
		if(viewer->_bOpen) {
			if(viewer->_bCreateImGuiWindow) {
				if(ImGui::Begin(viewer->GetName(), &viewer->_bOpen))
					viewer->DrawUI();
				ImGui::End();
			} else {
				viewer->DrawUI();
			}

		}
	}
}

void AnalyserUI::DrawMainMenu()
{
	if(ImGui::BeginMainMenuBar()) {
		if(ImGui::BeginMenu("Options")) {
			OptionsMenu();
			ImGui::EndMenu();
		}

		if(ImGui::BeginMenu("Actions")) {
			//ActionsMenu();
			ImGui::EndMenu();
		}

		/*if(ImGui::BeginMenu("Windows")) {
			WindowsMenu();
			ImGui::EndMenu();
		}*/

		// Draw fps and frame time
		ImGui::SameLine(ImGui::GetWindowWidth() - 300);
		ImGuiIO& io = ImGui::GetIO();
		ImGui::Text("%.1f ms (%.1f fps)", 1000.0f / io.Framerate, io.Framerate);

		ImGui::EndMainMenuBar();
	}
}

void AnalyserUI::OptionsMenu()
{
	/*if(ImGui::BeginMenu("Number Mode")) {
		bool bClearCode = false;
		if(ImGui::MenuItem("Decimal", 0, GetNumberDisplayMode() == ENumberDisplayMode::Decimal)) {
			SetNumberDisplayMode(ENumberDisplayMode::Decimal);
			CodeAnalysis.SetAllBanksDirty();
			bClearCode = true;
		}
		if(ImGui::MenuItem("Hex - FEh", 0, GetNumberDisplayMode() == ENumberDisplayMode::HexAitch)) {
			SetNumberDisplayMode(ENumberDisplayMode::HexAitch);
			SetHexNumberDisplayMode(ENumberDisplayMode::HexAitch);
			CodeAnalysis.SetAllBanksDirty();
			bClearCode = true;
		}
		if(ImGui::MenuItem("Hex - $FE", 0, GetNumberDisplayMode() == ENumberDisplayMode::HexDollar)) {
			SetNumberDisplayMode(ENumberDisplayMode::HexDollar);
			SetHexNumberDisplayMode(ENumberDisplayMode::HexDollar);
			CodeAnalysis.SetAllBanksDirty();
			bClearCode = true;
		}

		// clear code text so it can be written again
		// TODO: this needs to work for banks
		if(bClearCode) {
			for(int i = 0; i < 1 << 16; i++) {
				FCodeInfo* pCodeInfo = CodeAnalysis.GetCodeInfoForPhysicalAddress(i);
				if(pCodeInfo && pCodeInfo->Text.empty() == false)
					pCodeInfo->Text.clear();

			}
		}

		ImGui::EndMenu();
	}*/
	/*if(ImGui::MenuItem("Edit Mode", 0, &CodeAnalysis.bAllowEditing)) {
		if(CodeAnalysis.bAllowEditing)
			OnEnterEditMode();
		else
			OnExitEditMode();

	}*/
	
	//ImGui::MenuItem("Show Opcode Values", 0, &CodeAnalysis.pGlobalConfig->bShowOpcodeValues);
	/*if(ImGui::BeginMenu("Image Scale")) {
		for(int i = 0; i < 4; i++) {
			char numStr[4];
			snprintf(numStr, 4, "%dx", i + 1);
			if(ImGui::MenuItem(numStr, 0, CodeAnalysis.pGlobalConfig->ImageScale == i + 1))
				CodeAnalysis.pGlobalConfig->ImageScale = i + 1;
		}
		ImGui::EndMenu();
	}*/
	/*if(ImGui::BeginMenu("Font")) {
		if(const ImFont* pCurFont = ImGui::GetFont()) {
			FGlobalConfig* pConfig = CodeAnalysis.pGlobalConfig;
			const bool bConfigFontIsLoaded = !strncmp(pConfig->Font.c_str(), pCurFont->GetDebugName(), pConfig->Font.length());

			ImGui::Checkbox("Use Built-In Font", &pConfig->bBuiltInFont);

			ImGui::Text("Current Font: %s", pCurFont->GetDebugName());
			if(!pConfig->bBuiltInFont) {
				ImGui::Separator();

				if(!pConfig->Font.empty()) {
					if(bConfigFontIsLoaded) {
						if(ImGui::Button("Decrease Font Size")) {
							pConfig->FontSizePts--;
						}
						ImGui::SameLine();
						ImGui::Dummy(ImGui::CalcTextSize("FF"));
						ImGui::SameLine();
						if(ImGui::Button("Increase Font Size")) {
							pConfig->FontSizePts++;
						}

						ImGui::InputInt("Size (Pt)", &pConfig->FontSizePts, 0, 0, ImGuiInputTextFlags_EnterReturnsTrue);
						pConfig->FontSizePts = std::min(std::max(pConfig->FontSizePts, 8), 72);
					} else {
						ImGui::Text("Font '%s' could not be loaded.", pConfig->Font.c_str());
					}
				} else {
					ImGui::Text("No font defined in GlobalConfig.json.");
				}
			}
		}
		ImGui::EndMenu();
	}*/

#ifndef NDEBUG
	ImGui::MenuItem("ImGui Demo", 0, &_bShowImGuiDemo);
	//ImGui::MenuItem("ImPlot Demo", 0, &_bShowImPlotDemo);
#endif // NDEBUG
}

void AnalyserUI::AddViewer(ViewerBase* pViewer)
{
	pViewer->Init();
	_viewers.push_back(pViewer);
}

void AnalyserUI::DrawDockingView()
{
	//static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
	bool bOpen = false;
	ImGuiDockNodeFlags dockFlags = 0;

	// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
	// because it would be confusing to have two docking targets within each others.
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

	// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
	if(dockFlags & ImGuiDockNodeFlags_PassthruCentralNode)
		window_flags |= ImGuiWindowFlags_NoBackground;

	// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
	// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive, 
	// all active windows docked into it will lose their parent and become undocked.
	// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise 
	// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	if(ImGui::Begin("DockSpace Demo", &bOpen, window_flags)) {
		ImGui::PopStyleVar();
		ImGui::PopStyleVar(2);

		// DockSpace
		ImGuiIO& io = ImGui::GetIO();
		if(io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
			const ImGuiID dockspaceId = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), dockFlags);
		}

		DrawMainMenu();
		DrawUI();

		ImGui::End();
	} else {
		ImGui::PopStyleVar();
	}
}