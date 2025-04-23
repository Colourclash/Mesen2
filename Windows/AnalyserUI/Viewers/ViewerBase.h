#pragma once

#include <string>

class Emulator;

// Base class for all viewers
class ViewerBase
{
public:
	ViewerBase(Emulator* pEmu) : _pEmu(pEmu) {}
	virtual bool	Init() = 0;
	virtual void	ResetForGame() {};
	virtual void	Shutdown() = 0;
	virtual void	DrawUI() = 0;

	const char* GetName() const { return _name.c_str(); }
protected:
	Emulator* _pEmu = nullptr;
	std::string _name;
public:
	bool _bOpen = true;
	bool _bCreateImGuiWindow = true;
};
