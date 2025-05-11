#pragma once

#include "ViewerBase.h"
//#include "../CodeAnalyserTypes.h"

//class FCodeAnalysisState;
//class FEmuBase;

struct LabelListFilter
{
	std::string		FilterText;
	uint16_t		MinAddress = 0x0000;
	uint16_t		MaxAddress = 0xffff;
	//bool			bNoMachineRoms = true;
	//EDataTypeFilter DataType = EDataTypeFilter::All;
};

class GlobalsViewer : public ViewerBase
{
public:
	GlobalsViewer(Emulator* pEmulator, AnalyserUI* pAnalyserUI) : ViewerBase(pEmulator, pAnalyserUI) { _name = "Globals"; }
	bool	Init(void) override;
	void	Shutdown(void) override;
	void	DrawUI() override;

	void	ToggleRebuild()
	{
		bRebuildFilteredGlobalDataItems = true;
		bRebuildFilteredGlobalFunctions = true;
	}
	void	Reset()
	{
		//ShowROMLabels = false;
		FilterText.clear();
		//DataTypeFilter = EDataTypeFilter::All;
		//GlobalDataItemsFilter.Reset();
		//GlobalFunctionsFilter.Reset();
		bRebuildFilteredGlobalDataItems = true;
		bRebuildFilteredGlobalFunctions = true;
		//DataSortMode = EDataSortMode::Location;
		//FunctionSortMode = EFunctionSortMode::Location;
	}
private:
	void	DrawGlobals();

	// for global Filters
	std::string					FilterText;
	//EDataTypeFilter						DataTypeFilter = EDataTypeFilter::All;
	//FLabelListFilter			GlobalDataItemsFilter;
	//std::vector<FCodeAnalysisItem>	FilteredGlobalDataItems;
	bool										bRebuildFilteredGlobalDataItems = true;
	//EDataSortMode				DataSortMode = EDataSortMode::Location;
	LabelListFilter				GlobalFunctionsFilter;
	//std::vector<FCodeAnalysisItem>	FilteredGlobalFunctions;
	bool										bRebuildFilteredGlobalFunctions = true;
	//EFunctionSortMode				FunctionSortMode = EFunctionSortMode::Location;
};

