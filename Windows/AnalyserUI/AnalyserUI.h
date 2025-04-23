#pragma once

#include "pch.h"

#include "IImGuiDraw.h"

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
};