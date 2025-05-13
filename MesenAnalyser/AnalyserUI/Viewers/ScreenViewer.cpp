#include "ScreenViewer.h"

#include <imgui.h>

#include "Core/Shared/Emulator.h"

bool ScreenViewer::Init(void)
{
	return true;
}

void ScreenViewer::Shutdown(void)
{

}

void ScreenViewer::DrawUI() 
{
	ImGui::Text("screen goes here");
}
