#pragma once

#include "ViewerBase.h"

class ScreenViewer : public ViewerBase
{
public:
	ScreenViewer(Emulator* pEmu, AnalyserUI* pAnalyserUI) : ViewerBase(pEmu, pAnalyserUI) { _name = "Screen"; }
	bool	Init(void) override;
	void	Shutdown(void) override;
	void	DrawUI() override;

private:
};

