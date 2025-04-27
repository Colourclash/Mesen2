#pragma once

#include "ViewerBase.h"

class Debugger;

class CodeAnalysisViewer : public ViewerBase
{
public:
	CodeAnalysisViewer(Emulator* pEmu, AnalyserUI* pAnalyserUI) : ViewerBase(pEmu, pAnalyserUI) { _name = "Code Analysis"; }
	bool	Init(void) override;
	void	Shutdown(void) override;
	void	DrawUI() override;

protected:
	void	DrawDebuggerButtons(Debugger* pDebugger);
	void	ProcessKeyCommands(Debugger* pDebugger);
};

