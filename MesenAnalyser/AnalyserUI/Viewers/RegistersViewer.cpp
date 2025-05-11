#include "RegistersViewer.h"

#include <imgui.h>

#include "Core/Shared/Emulator.h"
#include "Core/PCE/PceTypes.h"
#include "Core/PCE/PceConsole.h"
#include "Core/PCE/PceCpu.h"
#include "Shared/DebuggerRequest.h"

bool RegistersViewer::Init(void)
{
	return true;
}

void RegistersViewer::Shutdown(void)
{

}

void RegistersViewer::DrawUI() 
{
	shared_ptr<IConsole> consolePtr = _pEmu->GetConsole();
	if(PceConsole* pConsole = (PceConsole*)consolePtr.get()) {
		PceCpuState& cpuState = pConsole->GetCpu()->GetState();
		ImGui::Text("A: %x", cpuState.A);
		ImGui::Text("X: %x", cpuState.X);
		ImGui::Text("Y: %x", cpuState.Y);
		ImGui::Text("SP: %x", cpuState.SP);
		ImGui::Text("PS: %x", cpuState.PS);
		ImGui::Text("PC: %x", cpuState.PC);
	}

	/*DebuggerRequest req = _pEmulator->GetDebugger(true);
	Debugger* dbg = req.GetDebugger();
	if(dbg) {
		// dbg is only set if debugger window is open
		// todo dont crash if wrong machine is active
		ImGui::Text("PC: %x", dbg->GetProgramCounter(CpuType::Pce, true));
	}*/
}
