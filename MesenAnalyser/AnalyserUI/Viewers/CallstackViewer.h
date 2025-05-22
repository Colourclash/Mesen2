#pragma once

#include "ViewerBase.h"

struct StackFrameInfo;

class CallstackViewer : public ViewerBase
{
public:
	CallstackViewer(Emulator* pEmu, AnalyserUI* pAnalyserUI) : ViewerBase(pEmu, pAnalyserUI) { _name = "Call Stack"; }
	bool	Init(void) override;
	void	Shutdown(void) override;
	void	DrawUI() override;

private:
	bool _bShowCallstackWhenRunning = false;
	StackFrameInfo* _callstack = nullptr;
};

