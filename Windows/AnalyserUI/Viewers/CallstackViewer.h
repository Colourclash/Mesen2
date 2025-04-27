#pragma once

#include "ViewerBase.h"

struct StackFrameInfo;

class CallstackViewer : public ViewerBase
{
public:
	CallstackViewer(Emulator* pEmu) : ViewerBase(pEmu) { _name = "Call Stack"; }
	bool	Init(void) override;
	void	Shutdown(void) override;
	void	DrawUI() override;

private:
	StackFrameInfo* _callstack = nullptr;
};

