#include "pch.h"
#include "Shared/Interfaces/IRenderingDevice.h"
#include "Utilities/SimpleLock.h"

class Emulator;
class AnalyserUI;

class ScreenRenderer : public IRenderingDevice
{
private:
	Emulator* _emu = nullptr;
	AnalyserUI* _analyserUI = nullptr;

	SimpleLock _frameLock;
	SimpleLock _textureLock;
	
	uint32_t _frameWidth = 0;
	uint32_t _frameHeight = 0;
	
	uint32_t* _textureBuffer[2] = { nullptr, nullptr };

	atomic<bool> _needSwap = false;

	void SetScreenSize(uint32_t width, uint32_t height);

public:
	ScreenRenderer(Emulator* emu, AnalyserUI* pAnalyser);
	~ScreenRenderer();

	void UpdateFrame(RenderedFrame& frame) override;
	void ClearFrame() override;
	void Render(RenderSurfaceInfo& emuHud, RenderSurfaceInfo& scriptHud) override;
	void Reset() override;
	void SetExclusiveFullscreenMode(bool fullscreen, void* windowHandle) override;
};