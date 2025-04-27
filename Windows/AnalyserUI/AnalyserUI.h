#pragma once

#include "pch.h"

#include "IImGuiDraw.h"

#include "Core\Shared\SettingTypes.h"
#include "Core\Shared\CpuType.h"

class Emulator;
class ViewerBase;
class GlobalsViewer;

// Todo:
// - 

class AnalyserUI : public IImGuiDraw
{	
private:
	Emulator* _pEmu = nullptr;

	bool _bShowImGuiDemo = false;
	bool _bShowImPlotDemo = false;

	std::vector<ViewerBase*> _viewers;

	ConsoleType _consoleType = (ConsoleType)-1;
	CpuType _cpuType = (CpuType)0xff;

private:
	void DrawDockingView();
	void DrawUI();
	void DrawMainMenu();
	void OptionsMenu();

	void AddViewer(ViewerBase* pViewer);

public:
	AnalyserUI(Emulator* emu);
	~AnalyserUI();

	// IImGuiDraw
	virtual void Draw() override;
	// ~IImGuiDraw

	void OnRomLoaded();
	void Reset();

	ConsoleType GetConsoleType() const { return _consoleType; }
	CpuType GetCpuType() const { return _cpuType; }
};