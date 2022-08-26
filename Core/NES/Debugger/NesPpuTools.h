#pragma once
#include "stdafx.h"
#include "Debugger/PpuTools.h"

class Debugger;
class Emulator;
class BaseMapper;
class NesConsole;
struct NesPpuState;

class NesPpuTools final : public PpuTools
{
private:
	NesConsole* _console = nullptr;
	BaseMapper* _mapper = nullptr;
	void GetSpriteInfo(DebugSpriteInfo& sprite, uint32_t spriteIndex, GetSpritePreviewOptions& options, NesPpuState& state, uint8_t* vram, uint8_t* oamRam, uint32_t* palette);

public:
	NesPpuTools(Debugger* debugger, Emulator *emu, NesConsole* console);

	DebugTilemapInfo GetTilemap(GetTilemapOptions options, BaseState& state, uint8_t* vram, uint32_t* palette, uint32_t *outBuffer) override;
	FrameInfo GetTilemapSize(GetTilemapOptions options, BaseState& state) override;
	DebugTilemapTileInfo GetTilemapTileInfo(uint32_t x, uint32_t y, uint8_t* vram, GetTilemapOptions options, BaseState& baseState) override;

	DebugSpritePreviewInfo GetSpritePreviewInfo(GetSpritePreviewOptions options, BaseState& state) override;
	void GetSpritePreview(GetSpritePreviewOptions options, BaseState& state, uint8_t* vram, uint8_t* oamRam, uint32_t* palette, uint32_t *outBuffer) override;
	void GetSpriteList(GetSpritePreviewOptions options, BaseState& baseState, uint8_t* vram, uint8_t* oamRam, uint32_t* palette, DebugSpriteInfo outBuffer[]) override;

	DebugPaletteInfo GetPaletteInfo(GetPaletteInfoOptions options) override;
	void SetPaletteColor(int32_t colorIndex, uint32_t color) override;
};