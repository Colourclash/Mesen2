#include "pch.h"
#include "SNES/InternalRegisters.h"
#include "SNES/SnesConsole.h"
#include "SNES/SnesCpu.h"
#include "SNES/SnesPpu.h"
#include "SNES/SnesControlManager.h"
#include "SNES/SnesMemoryManager.h"
#include "SNES/InternalRegisterTypes.h"
#include "Shared/MessageManager.h"
#include "Shared/Emulator.h"
#include "Utilities/Serializer.h"
#include "Utilities/HexUtilities.h"

InternalRegisters::InternalRegisters()
{
}

void InternalRegisters::Initialize(SnesConsole* console)
{
	_cpu = console->GetCpu();
	_aluMulDiv.Initialize(_cpu);
	_console = console;
	_memoryManager = console->GetMemoryManager();
	_ppu = _console->GetPpu();
	_controlManager = (SnesControlManager*)_console->GetControlManager();
	Reset();

	//Power on values
	_state = {};
	_state.HorizontalTimer = 0x1FF;
	_state.VerticalTimer = 0x1FF;
	_state.IoPortOutput = 0xFF;
}

void InternalRegisters::Reset()
{
	_state.EnableAutoJoypadRead = false;
	_state.EnableNmi = false;
	_state.EnableHorizontalIrq = false;
	_state.EnableVerticalIrq = false;
	_nmiFlag = false;
	_irqLevel = false;
	_needIrq = false;
	_irqFlag = false;
	_autoReadClockStart = 0;
	_autoReadNextClock = 0;
	_autoReadActive = false;
	_autoReadPort1Value = 0;
	_autoReadPort2Value = 0;
}

void InternalRegisters::SetAutoJoypadReadClock()
{
	//Auto-read starts at the first multiple of 256 master clocks after dot 32.5 (hclock 130)
	uint64_t rangeStart = _console->GetMasterClock() + 130;
	_autoReadClockStart = rangeStart + ((rangeStart & 0xFF) ? (256 - (rangeStart & 0xFF)) : 0) - 128;
	_autoReadNextClock = _autoReadClockStart;
}

void InternalRegisters::ProcessAutoJoypad()
{
	if(_autoReadClockStart == 0) {
		return;
	}

	uint64_t masterClock = _console->GetMasterClock();
	if(masterClock < _autoReadClockStart) {
		return;
	}

	if((masterClock - _autoReadClockStart) < 256) {
		//If EnableAutoJoypadRead changes at any point in the first 256 clocks of this process,
		//the value sent to OUT0 can be changed and immediately strobe the controllers, etc.
		_controlManager->SetAutoReadStrobe(_state.EnableAutoJoypadRead);
	}

	for(uint64_t clock = _autoReadNextClock; clock <= _console->GetMasterClock(); clock += 128) {
		_autoReadNextClock = clock + 128;
		int step = (clock - _autoReadClockStart) / 128;

		switch(step) {
			case 0:
				//Strobe starts 128 cycles before the auto-read flag is set
				_controlManager->SetAutoReadStrobe(_state.EnableAutoJoypadRead);
				break;

			case 1:
				if(!_state.EnableAutoJoypadRead) {
					//Skip auto-read for this frame if the flag is not enabled at this point
					_autoReadNextClock = 0;
					_autoReadClockStart = 0;
					_autoReadActive = false;
				} else {
					//Set the active flag, reset the registers to 0 (only if auto-read is enabled)
					_autoReadActive = true;

					_state.ControllerData[0] = 0;
					_state.ControllerData[1] = 0;
					_state.ControllerData[2] = 0;
					_state.ControllerData[3] = 0;
				}
				break;

			case 2:
				//Strobe ends after 256 clock cycles
				_controlManager->SetAutoReadStrobe(false);
				break;

			default:
				if((step & 0x01) == 1) {
					//First half of the 256-clock cycle reads the ports
					_autoReadPort1Value = _controlManager->Read(0x4016, true);
					_autoReadPort2Value = _controlManager->Read(0x4017, true);
				} else {
					//Second half shifts the data and inserts the new bit at position 0
					_state.ControllerData[0] <<= 1;
					_state.ControllerData[1] <<= 1;
					_state.ControllerData[2] <<= 1;
					_state.ControllerData[3] <<= 1;
					_state.ControllerData[0] |= (_autoReadPort1Value & 0x01);
					_state.ControllerData[1] |= (_autoReadPort2Value & 0x01);
					_state.ControllerData[2] |= (_autoReadPort1Value & 0x02) >> 1;
					_state.ControllerData[3] |= (_autoReadPort2Value & 0x02) >> 1;
				}
				break;
		}

		if(step >= 34) {
			//Last tick done, disable auto-read
			_autoReadClockStart = 0;
			_autoReadNextClock = 0;
			_autoReadActive = false;
			break;
		}
	}
}

uint8_t InternalRegisters::ReadControllerData(uint8_t port, bool getMsb)
{
	ProcessAutoJoypad();

	//When auto-poll registers are read, don't count frame as a lag frame
	_controlManager->SetInputReadFlag();

	uint8_t value;
	if(getMsb) {
		value = (uint8_t)(_state.ControllerData[port] >> 8);
	} else {
		value = (uint8_t)_state.ControllerData[port];
	}

	if(_autoReadActive) {
		//TODO add a break option for this?
		_console->GetEmulator()->DebugLog("[Input] Read input during auto-read - results may be invalid.");
	}

	return value;
}

uint8_t InternalRegisters::GetIoPortOutput()
{
	return _state.IoPortOutput;
}

void InternalRegisters::SetNmiFlag(bool nmiFlag)
{
	_nmiFlag = nmiFlag;
}

void InternalRegisters::SetIrqFlag(bool irqFlag)
{
	_irqFlag = irqFlag && (_state.EnableHorizontalIrq || _state.EnableVerticalIrq);
	if(_irqFlag) {
		_cpu->SetIrqSource(SnesIrqSource::Ppu);
	} else {
		_cpu->ClearIrqSource(SnesIrqSource::Ppu);
	}
}

uint8_t InternalRegisters::Peek(uint16_t addr)
{
	switch(addr) {
		case 0x4210: return (_nmiFlag ? 0x80 : 0) | 0x02;
		case 0x4211: return (_irqFlag ? 0x80 : 0);

		case 0x4214:
		case 0x4215:
		case 0x4216:
		case 0x4217:
			//Not completely accurate because the ALU results are only 
			//updated when the CPU actually reads the registers
			return _aluMulDiv.Peek(addr);

		case 0x4218: return (uint8_t)_state.ControllerData[0];
		case 0x4219: return (uint8_t)(_state.ControllerData[0] >> 8);

		default: return Read(addr);
	}
}

uint8_t InternalRegisters::Read(uint16_t addr)
{
	switch(addr) {
		case 0x4210: {
			constexpr uint8_t cpuRevision = 0x02;
			
			uint8_t value = (_nmiFlag ? 0x80 : 0) | cpuRevision;

			//Reading $4210 on any cycle turns the NMI signal off (except presumably on the first PPU cycle (first 4 master clocks) of the NMI scanline.)
			//i.e: reading $4210 at the same it gets set will return it as set, and will keep it set.
			//Without this, Terranigma has corrupted sprites on some frames.
			if(_memoryManager->GetHClock() >= 4 || _ppu->GetScanline() != _ppu->GetNmiScanline()) {
				SetNmiFlag(false);
			}

			return value | (_memoryManager->GetOpenBus() & 0x70);
		}

		case 0x4211: {
			uint8_t value = (_irqFlag ? 0x80 : 0);
			SetIrqFlag(false);
			return value | (_memoryManager->GetOpenBus() & 0x7F);
		}

		case 0x4212: {
			ProcessAutoJoypad();

			uint16_t hClock = _memoryManager->GetHClock();
			uint16_t scanline = _ppu->GetScanline();
			uint16_t nmiScanline = _ppu->GetNmiScanline();
			//TODO TIMING (set/clear timing)
			return (
				(scanline >= nmiScanline ? 0x80 : 0) |
				((hClock >= 1*4 && hClock <= 274*4) ? 0 : 0x40) |
				(_autoReadActive ? 0x01 : 0) | //Auto joypad read in progress
				(_memoryManager->GetOpenBus() & 0x3E)
			);
		}

		case 0x4213:
			//TODO  RDIO - Programmable I/O port (in-port)
			return 0;
						 
		case 0x4214:
		case 0x4215:
		case 0x4216:
		case 0x4217: 
			return _aluMulDiv.Read(addr);

		case 0x4218: return ReadControllerData(0, false);
		case 0x4219: return ReadControllerData(0, true);
		case 0x421A: return ReadControllerData(1, false);
		case 0x421B: return ReadControllerData(1, true);
		case 0x421C: return ReadControllerData(2, false);
		case 0x421D: return ReadControllerData(2, true);
		case 0x421E: return ReadControllerData(3, false);
		case 0x421F: return ReadControllerData(3, true);
		
		default:
			LogDebug("[Debug] Unimplemented register read: " + HexUtilities::ToHex(addr));
			return _memoryManager->GetOpenBus();
	}
}

void InternalRegisters::Write(uint16_t addr, uint8_t value)
{
	switch(addr) {
		case 0x4200: {
			//Catch up auto-read logic before modifying the auto-read flag
			bool autoRead = (value & 0x01) != 0;
			if(_state.EnableAutoJoypadRead != autoRead) {
				ProcessAutoJoypad();
			}

			_state.EnableNmi = (value & 0x80) != 0;
			_state.EnableVerticalIrq = (value & 0x20) != 0;
			_state.EnableHorizontalIrq = (value & 0x10) != 0;
			_state.EnableAutoJoypadRead = autoRead;
			
			SetNmiFlag(_nmiFlag);
			SetIrqFlag(_irqFlag);
			break;
		}

		case 0x4201:
			//TODO WRIO - Programmable I/O port (out-port)
			
			//The IO port's value affects the multitap - make sure to catch-up on the auto-read logic first to
			//ensure the reads get the correct data, otherwise it might return the data for the wrong controllers
			ProcessAutoJoypad();

			if((_state.IoPortOutput & 0x80) && !(value & 0x80)) {
				_ppu->LatchLocationValues();
			}
			_state.IoPortOutput = value;
			break;

		case 0x4202:
		case 0x4203:
		case 0x4204:
		case 0x4205:
		case 0x4206:
			_aluMulDiv.Write(addr, value);
			break;

		case 0x4207: 
			_state.HorizontalTimer = (_state.HorizontalTimer & 0x100) | value; 
			ProcessIrqCounters();
			break;

		case 0x4208: 
			_state.HorizontalTimer = (_state.HorizontalTimer & 0xFF) | ((value & 0x01) << 8); 
			ProcessIrqCounters();
			break;

		case 0x4209: 
			_state.VerticalTimer = (_state.VerticalTimer & 0x100) | value; 

			//Calling this here fixes flashing issue in "Shin Nihon Pro Wrestling Kounin - '95 Tokyo Dome Battle 7"
			//The write to change from scanline 16 to 17 occurs between both ProcessIrqCounter calls, which causes the IRQ
			//line to always be high (since the previous check is on scanline 16, and the next check on scanline 17)
			ProcessIrqCounters();
			break;

		case 0x420A: 
			_state.VerticalTimer = (_state.VerticalTimer & 0xFF) | ((value & 0x01) << 8);
			ProcessIrqCounters();
			break;

		case 0x420D: _state.EnableFastRom = (value & 0x01) != 0; break;

		default:
			LogDebug("[Debug] Unimplemented register write: " + HexUtilities::ToHex(addr) + " = " + HexUtilities::ToHex(value));
			break;
	}
}

InternalRegisterState InternalRegisters::GetState()
{
	return _state;
}

AluState InternalRegisters::GetAluState()
{
	return _aluMulDiv.GetState();
}

void InternalRegisters::Serialize(Serializer &s)
{
	SV(_state.EnableFastRom); SV(_nmiFlag); SV(_state.EnableNmi); SV(_state.EnableHorizontalIrq); SV(_state.EnableVerticalIrq); SV(_state.HorizontalTimer);
	SV(_state.VerticalTimer); SV(_state.IoPortOutput); SV(_state.ControllerData[0]); SV(_state.ControllerData[1]); SV(_state.ControllerData[2]); SV(_state.ControllerData[3]);
	SV(_irqLevel); SV(_needIrq); SV(_state.EnableAutoJoypadRead); SV(_irqFlag);

	SV(_aluMulDiv);

	SV(_autoReadClockStart);
	SV(_autoReadNextClock);
	SV(_autoReadActive);
	SV(_autoReadPort1Value);
	SV(_autoReadPort2Value);
}
