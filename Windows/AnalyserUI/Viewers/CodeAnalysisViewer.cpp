#include "CodeAnalysisViewer.h"

#include <imgui.h>
//#include "misc/cpp/imgui_stdlib.h"
#include "Core/Shared/Emulator.h"
#include "Core/Debugger/Disassembler.h"
#include "Core/Debugger/DebugTypes.h"
#include "Shared/DebuggerRequest.h"

bool CodeAnalysisViewer::Init(void)
{
	return true;
}

void CodeAnalysisViewer::Shutdown(void)
{

}

void CodeAnalysisViewer::DrawUI() 
{
	DebuggerRequest req = _pEmu->GetDebugger(true);
	if(Debugger* dbg = req.GetDebugger()) {
		const int nNumLines = 10;
		CodeLineData codeLineData[nNumLines];
		dbg->GetDisassembler()->GetDisassemblyOutput(CpuType::Pce, 0x8000, codeLineData, nNumLines);

		for(int i = 0; i < nNumLines; i++) {
			string flags = "";
			if(codeLineData[i].Flags & LineFlags::BlockStart)
				flags += "blockstart ";
			if(codeLineData[i].Flags & LineFlags::BlockEnd)
				flags += "blockend ";
			if(codeLineData[i].Flags & LineFlags::SubStart) {
				ImGui::SeparatorText(codeLineData[i].Text);
				//flags += "substart ";
				continue;
			}
			if(codeLineData[i].Flags & LineFlags::Label) {
				ImGui::Text("     %s", codeLineData[i].Text);
				//flags += "label ";
				continue;
			}
			if(codeLineData[i].Flags & LineFlags::Comment)
				flags += "comment ";
			if(codeLineData[i].Flags & LineFlags::Empty)
				flags += "empty ";
			if(codeLineData[i].Flags & LineFlags::VerifiedCode)
				flags += "verifiedcode ";
			if(codeLineData[i].Flags & LineFlags::VerifiedData)
				flags += "verifieddata ";

			ImGui::Text("%x %s FLAGS: %s %s", codeLineData[i].Address, codeLineData[i].Text, flags.c_str(), codeLineData[i].Comment);
		}
	}
}
