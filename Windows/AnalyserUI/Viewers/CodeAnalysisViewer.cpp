#include "CodeAnalysisViewer.h"

#include <imgui.h>
#include "Core/Shared/Emulator.h"
#include "Core/Debugger/Disassembler.h"
#include "Core/Debugger/DebugTypes.h"
#include "Shared/DebuggerRequest.h"
#include "../AnalyserUI/AnalyserUI.h"

bool CodeAnalysisViewer::Init(void)
{
	return true;
}

void CodeAnalysisViewer::Shutdown(void)
{

}

void CodeAnalysisViewer::DrawDebuggerButtons(Debugger* pDebugger)
{
	const CpuType cpuType = _pAnalyserUI->GetCpuType();

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
			pDebugger->Step(cpuType, 1, StepType::Step);
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Step Over (F10)"))
	{
		pDebugger->Step(cpuType, 1, StepType::StepOver);
	}
	ImGui::SameLine();
	if (ImGui::Button("Step Into (F11)"))
	{
		pDebugger->Step(cpuType, 1, StepType::Step);
	}
	ImGui::SameLine();
	if (ImGui::Button("Step Out"))
	{
		pDebugger->Step(CpuType::Pce, 1, StepType::StepOut);
	}
	ImGui::SameLine();
	if (ImGui::Button("Step Frame (F6)"))
	{
		pDebugger->Step(cpuType, 1, StepType::PpuFrame); // does this work on pce?
	}
	ImGui::SameLine();
	if (ImGui::Button("Step Back"))
	{
		pDebugger->Step(cpuType, 1, StepType::StepBack);
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
	const CpuType cpuType = _pAnalyserUI->GetCpuType();
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
				pDebugger->Step(cpuType, 1, StepType::Step);
			}
		}
		else if (ImGui::IsKeyPressed(ImGuiKey_F10))
		{
			pDebugger->Step(cpuType, 1, StepType::StepOver);
		}
		else if (ImGui::IsKeyPressed(ImGuiKey_F11))
		{
			pDebugger->Step(cpuType, 1, StepType::Step);
		}
		else if (io.KeyShift && ImGui::IsKeyPressed(ImGuiKey_F11))
		{
			pDebugger->Step(cpuType, 1, StepType::StepOut);
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
		
		//const uint32_t pc = pDbg->GetProgramCounter(CpuType::Pce, false);
		const uint32_t pc = pDbg->GetProgramCounter(_pAnalyserUI->GetCpuType(), false);
		//pDbg->GetDisassembler()->GetDisassemblyOutput(CpuType::Pce, /*0x8000*/pc, codeLineData, nNumLines);
		pDbg->GetDisassembler()->GetDisassemblyOutput(_pAnalyserUI->GetCpuType(), /*0x8000*/pc, codeLineData, nNumLines);

		for(int i = 0; i < nNumLines; i++) {
			string flags = "";
			if(codeLineData[i].Flags & LineFlags::BlockStart) {
				flags += "blockstart ";
				ImGui::SeparatorText("Block Start");
				continue;
			}
			if(codeLineData[i].Flags & LineFlags::BlockEnd){
				//flags += "blockend ";
				ImGui::SeparatorText("Block End");
				continue;
			}
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

			ImGui::Text("%x '%s'", codeLineData[i].Address, codeLineData[i].Text);
			ImGui::Text("FLAGS: %s", flags.c_str());
			ImGui::Text("Comment '%s'", codeLineData[i].Comment);
			ImGui::Separator();
		}
	}
}
