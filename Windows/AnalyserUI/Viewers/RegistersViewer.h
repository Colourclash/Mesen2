#pragma once

#include "ViewerBase.h"

class RegistersViewer : public ViewerBase
{
public:
	RegistersViewer(Emulator* pEmu) : ViewerBase(pEmu) { _name = "Registers"; }
	bool	Init(void) override;
	void	Shutdown(void) override;
	void	DrawUI() override;

private:
};

