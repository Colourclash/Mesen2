#include "ScreenRenderer.h"
#include "Shared/Video/VideoRenderer.h"
#include "Shared/RenderedFrame.h"
#include "Shared/Emulator.h"
#include "Shared/NotificationManager.h"
#include "AnalyserUI.h"
#include "Viewers/ScreenViewer.h"

ScreenRenderer::ScreenRenderer(Emulator* emu, AnalyserUI* analyserUI)
{
	_emu = emu;
	_analyserUI = analyserUI;

	SetScreenSize(256, 240);
}

ScreenRenderer::~ScreenRenderer()
{
	delete[] _textureBuffer[0];
	delete[] _textureBuffer[1];
}

void ScreenRenderer::SetScreenSize(uint32_t width, uint32_t height)
{
	_frameWidth = width;
	_frameHeight = height;

	delete[] _textureBuffer[0];
	delete[] _textureBuffer[1];
	_textureBuffer[0] = new uint32_t[width * height];
	_textureBuffer[1] = new uint32_t[width * height];
	memset(_textureBuffer[0], 0, width * height * sizeof(uint32_t));
	memset(_textureBuffer[1], 0, width * height * sizeof(uint32_t));
}

void ScreenRenderer::UpdateFrame(RenderedFrame& frame)
{
	if(_frameWidth != frame.Width || _frameHeight != frame.Height) {
		auto lock = _frameLock.AcquireSafe();
		if(_frameWidth != frame.Width || _frameHeight != frame.Height) {
			SetScreenSize(frame.Width, frame.Height);
		}
	}

	auto lock = _textureLock.AcquireSafe();
	memcpy(_textureBuffer[0], frame.FrameBuffer, frame.Width * frame.Height * sizeof(uint32_t));
	_needSwap = true;
}

void ScreenRenderer::ClearFrame()
{
	//Clear current output and display black frame
	auto lock = _textureLock.AcquireSafe();
	memset(_textureBuffer[0], 0, _frameWidth * _frameHeight * sizeof(uint32_t));
	_needSwap = true;
}

void ScreenRenderer::Render(RenderSurfaceInfo& emuHud, RenderSurfaceInfo& scriptHud)
{
	auto lock = _frameLock.AcquireSafe();
	
	if(_needSwap) {
		_needSwap = false;
		auto textureLock = _textureLock.AcquireSafe();
		std::swap(_textureBuffer[0], _textureBuffer[1]);
	}

	//_textureBuffer[0] is the buffer the emulator is rendering into
	//_textureBuffer[1] is the buffer to be rendered

	_analyserUI->GetScreenView()->SetFrameData(_textureBuffer[1], _frameWidth, _frameHeight);
}

void ScreenRenderer::Reset()
{
}

void ScreenRenderer::SetExclusiveFullscreenMode(bool fullscreen, void* windowHandle)
{
	//not supported
}
