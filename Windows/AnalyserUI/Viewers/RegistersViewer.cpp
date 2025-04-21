#include "RegistersViewer.h"

#include <imgui.h>

#include "Core/Shared/Emulator.h"
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
	DebuggerRequest req = _pEmulator->GetDebugger(true);
	Debugger* dbg = req.GetDebugger();
	if(dbg) {
		// dbg is only set if debugger window is open
		ImGui::Text("PC: %x", dbg->GetProgramCounter(CpuType::Pce, true));
	}
}
