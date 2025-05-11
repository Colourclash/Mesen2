#pragma once

#include "IImGuiDraw.h"

#include "Core/Shared/SettingTypes.h"
#include "Core/Shared/CpuType.h"
#include "Core/Shared/Interfaces/INotificationListener.h"

class Emulator;
class ViewerBase;
class GlobalsViewer;
class CodeAnalysisViewer;

// Todo:
// - 

class AnalyserUI : public IImGuiDraw, public INotificationListener, public std::enable_shared_from_this<AnalyserUI>
{	
private:
	Emulator* _pEmu = nullptr;

	bool _bShowImGuiDemo = false;
	bool _bShowImPlotDemo = false;

	CodeAnalysisViewer* _pCodeViewer = nullptr;

	std::vector<ViewerBase*> _viewers;

	ConsoleType _consoleType = (ConsoleType)-1;
	CpuType _cpuType = (CpuType)0xff;

	// This is needed so we can use INotificationListener, which needs a shared_ptr to use.
	shared_ptr<AnalyserUI> _sharedSelf;

private:
	void DrawDockingView();
	void DrawUI();
	void DrawMainMenu();
	void OptionsMenu();

	void AddViewer(ViewerBase* pViewer);

public:
	AnalyserUI(Emulator* emu);
	~AnalyserUI();

	bool Init();

	// IImGuiDraw
	virtual void Draw() override;
	// ~IImGuiDraw

	// INotificationListener
	virtual void ProcessNotification(ConsoleNotificationType type, void* parameter) override;
	// ~INotificationListener

	void Reset();

	CodeAnalysisViewer* GetCodeView() const {	return _pCodeViewer;	}

	ConsoleType GetConsoleType() const { return _consoleType; }
	CpuType GetCpuType() const { return _cpuType; }
};