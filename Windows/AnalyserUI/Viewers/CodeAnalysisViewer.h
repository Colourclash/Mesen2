#pragma once

#include "ViewerBase.h"

#include "Core/Debugger/AddressInfo.h"

class Debugger;

class CodeAnalysisViewer : public ViewerBase
{
public:
	CodeAnalysisViewer(Emulator* pEmu, AnalyserUI* pAnalyserUI) : ViewerBase(pEmu, pAnalyserUI) { _name = "Code Analysis"; }
	
	bool	Init(void) override;
	void	Shutdown(void) override;
	void	DrawUI() override;
	
	void	GotoAddress(AddressInfo address)
	{
		_GotoAddress = address;
	}

protected:
	void	DrawDebuggerButtons(Debugger* pDebugger);
	void	ProcessKeyCommands(Debugger* pDebugger);

protected:
	AddressInfo _GotoAddress;
	AddressInfo _WindowAddress;
};

