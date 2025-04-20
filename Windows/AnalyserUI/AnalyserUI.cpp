#include <thread>

#include "imgui.h"

#include "Common.h"
#include "AnalyserUI.h"
#include "Core/Shared/Emulator.h"
#include "Shared/DebuggerRequest.h"

AnalyserUI::AnalyserUI(Emulator* emu)
{
	_emu = emu;
}

AnalyserUI::~AnalyserUI()
{
}

void AnalyserUI::Draw()
{
	static bool show_demo_window = true;

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
}