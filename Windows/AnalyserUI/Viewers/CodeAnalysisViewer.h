#pragma once

#include "ViewerBase.h"

class CodeAnalysisViewer : public ViewerBase
{
public:
	CodeAnalysisViewer(Emulator* pEmu) : ViewerBase(pEmu) { _name = "Code Analysis"; }
	bool	Init(void) override;
	void	Shutdown(void) override;
	void	DrawUI() override;
};

