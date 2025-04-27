#include "CallstackViewer.h"

#include <imgui.h>

#include "Core/Shared/Emulator.h"
#include "Core/Debugger/CallstackManager.h"
#include "Shared/DebuggerRequest.h"

bool CallstackViewer::Init(void)
{
	// Allocate a callstack array so we can copy the one from the debugger each frame.
	// We could access the callstack manager's callstack directly as an optimisation but it's not currently public.
	_callstack = new StackFrameInfo[512];

	return true;
}

void CallstackViewer::Shutdown(void)
{

}

void CallstackViewer::DrawUI() 
{
	DebuggerRequest req = _pEmu->GetDebugger(true);
	Debugger* pDbg = req.GetDebugger();
	if(pDbg) {
		uint32_t callstackSize = 0;
		pDbg->GetCallstackManager(CpuType::Pce)->GetCallstack(_callstack, callstackSize);
		
		// Top of stack
		const uint32_t pc = pDbg->GetProgramCounter(CpuType::Pce, false);
		ImGui::Text("[top] %x %x", pc, pDbg->GetAbsoluteAddress( {(int32_t)pc, MemoryType::PceMemory }));

		if (callstackSize > 0) {
			for (int i = callstackSize-1; i >= 0; i--) {
				const bool isMapped = pDbg->GetRelativeAddress(_callstack[i].AbsSource, CpuType::Pce).Address >= 0;
				const int32_t functionAddr = i == 0 ? -1 : _callstack[i -1].Target;
				string context = "      ";
				if (i > 0) {
					if (_callstack[i - 1].Flags == StackFrameFlags::Irq)
						context = "[IRQ] ";
					else if (_callstack[i - 1].Flags == StackFrameFlags::Nmi)
						context = "[NMI] ";
				}
				// Source is PC Address. AbsSource is ROM address
				ImGui::Text("%s Function:%x PC: %x ROM Addr: %x [mapped=%d]", context.c_str(), functionAddr, _callstack[i].Source, _callstack[i].AbsSource, isMapped);
			}
		}
	}
}
