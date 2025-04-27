#include "CodeAnalysisViewer.h"

#include <imgui.h>
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

void CodeAnalysisViewer::DrawDebuggerButtons(Debugger* pDebugger)
{
	if (pDebugger->IsPaused())
	{
		if (ImGui::Button("Continue (F5)"))
		{
			pDebugger->Run();
		}
	}
	else
	{
		if (ImGui::Button("Break (F5)"))
		{
			pDebugger->Step(CpuType::Pce, 1, StepType::Step);
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Step Over (F10)"))
	{
		pDebugger->Step(CpuType::Pce, 1, StepType::StepOver);
	}
	ImGui::SameLine();
	if (ImGui::Button("Step Into (F11)"))
	{
		pDebugger->Step(CpuType::Pce, 1, StepType::Step);
	}
	ImGui::SameLine();
	if (ImGui::Button("Step Out"))
	{
		pDebugger->Step(CpuType::Pce, 1, StepType::StepOut);
	}
	ImGui::SameLine();
	if (ImGui::Button("Step Frame (F6)"))
	{
		pDebugger->Step(CpuType::Pce, 1, StepType::PpuFrame); // does this work on pce?
	}
	ImGui::SameLine();
	if (ImGui::Button("Step Back"))
	{
		pDebugger->Step(CpuType::Pce, 1, StepType::StepBack);
	}
	/*ImGui::SameLine();
	if (ImGui::Button("Step Screen Write (F7)"))
	{
		state.Debugger.StepScreenWrite();
	}*/
	/*ImGui::SameLine();
	if (ImGui::Button("<<< Trace"))
	{
		state.Debugger.TraceBack(viewState);
	}
	ImGui::SameLine();
	if (ImGui::Button("Trace >>>"))
	{
		state.Debugger.TraceForward(viewState);
	}*/
	
	//ImGui::Checkbox("Jump to PC on break", &bJumpToPCOnBreak);
}

void CodeAnalysisViewer::ProcessKeyCommands(Debugger* pDebugger)
{
	ImGuiIO& io = ImGui::GetIO();
	//if (ImGui::IsWindowFocused())
	{
		if (ImGui::IsKeyPressed(ImGuiKey_F5))
		{
			if (pDebugger->IsPaused())
			{
				pDebugger->Run();
			}
			else
			{
				pDebugger->Step(CpuType::Pce, 1, StepType::Step);
			}
		}
		else if (ImGui::IsKeyPressed(ImGuiKey_F10))
		{
			pDebugger->Step(CpuType::Pce, 1, StepType::StepOver);
		}
		else if (ImGui::IsKeyPressed(ImGuiKey_F11))
		{
			pDebugger->Step(CpuType::Pce, 1, StepType::Step);
		}
		else if (io.KeyShift && ImGui::IsKeyPressed(ImGuiKey_F11))
		{
			pDebugger->Step(CpuType::Pce, 1, StepType::StepOut);
		}
	}
}

void CodeAnalysisViewer::DrawUI() 
{
	DebuggerRequest req = _pEmu->GetDebugger(true);
	if(Debugger* pDbg = req.GetDebugger()) {

		DrawDebuggerButtons(pDbg);
		ProcessKeyCommands(pDbg);

		const int nNumLines = 10;
		CodeLineData codeLineData[nNumLines];
		pDbg->GetDisassembler()->GetDisassemblyOutput(CpuType::Pce, 0x8000, codeLineData, nNumLines);

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
