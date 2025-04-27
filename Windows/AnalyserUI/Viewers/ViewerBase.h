#pragma once

#include <string>

class Emulator;
class AnalyserUI;

// Base class for all viewers
class ViewerBase
{
public:
	ViewerBase(Emulator* pEmu, AnalyserUI* pAnalyserUI) 
		: _pEmu(pEmu) 
		, _pAnalyserUI(pAnalyserUI){}
	virtual bool	Init() = 0;
	virtual void	ResetForGame() {};
	virtual void	Shutdown() = 0;
	virtual void	DrawUI() = 0;

	const char* GetName() const { return _name.c_str(); }
protected:
	Emulator* _pEmu = nullptr;
	AnalyserUI* _pAnalyserUI = nullptr;

	std::string _name;
public:
	bool _bOpen = true;
	bool _bCreateImGuiWindow = true;
};
