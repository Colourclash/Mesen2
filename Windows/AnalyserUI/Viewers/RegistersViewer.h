#pragma once

#include "ViewerBase.h"

class RegistersViewer : public ViewerBase
{
public:
	RegistersViewer(Emulator* pEmu, AnalyserUI* pAnalyserUI) : ViewerBase(pEmu, pAnalyserUI) { _name = "Registers"; }
	bool	Init(void) override;
	void	Shutdown(void) override;
	void	DrawUI() override;

private:
};

