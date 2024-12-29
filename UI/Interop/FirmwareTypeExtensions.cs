﻿using System;
using System.Collections.Generic;

namespace Mesen.Interop;

public static class FirmwareTypeExtensions
{
	public static FirmwareFiles GetFirmwareInfo(this FirmwareType type)
	{
		switch(type) {
			case FirmwareType.DSP1: return new("dsp1.rom") { new(0x2000, "91E87D11E1C30D172556BED2211CCE2EFA94BA595F58C5D264809EF4D363A97B") };
			case FirmwareType.DSP1B: return new("dsp1b.rom") { new(0x2000, "D789CB3C36B05C0B23B6C6F23BE7AA37C6E78B6EE9CEAC8D2D2AA9D8C4D35FA9") };
			case FirmwareType.DSP2: return new("dsp2.rom") { new(0x2000, "03EF4EF26C9F701346708CB5D07847B5203CF1B0818BF2930ACD34510FFDD717") };
			case FirmwareType.DSP3: return new("dsp3.rom") { new(0x2000, "0971B08F396C32E61989D1067DDDF8E4B14649D548B2188F7C541B03D7C69E4E") };
			case FirmwareType.DSP4: return new("dsp4.rom") { new(0x2000, "752D03B2D74441E430B7F713001FA241F8BBCFC1A0D890ED4143F174DBE031DA") };
			
			case FirmwareType.ST010: return new("st010.rom") { new(0xD000, "FA9BCED838FEDEA11C6F6ACE33D1878024BDD0D02CC9485899D0BDD4015EC24C") };
			case FirmwareType.ST011: return new("st011.rom") { new(0xD000, "8B2B3F3F3E6E29F4D21D8BC736B400BC988B7D2214EBEE15643F01C1FEE2F364") };
			case FirmwareType.ST018: return new("st018.rom") { new(0x28000, "6DF209AB5D2524D1839C038BE400AE5EB20DAFC14A3771A3239CD9E8ACD53806") };
			
			case FirmwareType.Satellaview: return new("BS-X.bin") {
				new(1024 * 1024,
					"27CFDB99F7E4252BF3740D420147B63C4C88616883BC5E7FE43F2F30BF8C8CBB", //Japan, no DRM
					"A49827B45FF9AC9CF5B4658190E1428E59251BC82D8A63D8E9E0F71E439F008F", //English, no DRM
					"3CE321496EDC5D77038DE2034EB3FB354D7724AFD0BC7FD0319F3EB5D57B984D", //Japan, original
					"77D94D64D745014BF8B51280A4204056CDEB9D41EA30EAE80DBC006675BEBEF8" //English, DRM
				)
			};

			case FirmwareType.Gameboy: return new("dmg_boot.bin", "gb_bios.bin") { new(256, "CF053ECCB4CCAFFF9E67339D4E78E98DCE7D1ED59BE819D2A1BA2232C6FCE1C7", "26E71CF01E301E5DC40E987CD2ECBF6D0276245890AC829DB2A25323DA86818E") };
			case FirmwareType.GameboyColor: return new("cgb_boot.bin", "gbc_bios.bin") { new(2304, "B4F2E416A35EEF52CBA161B159C7C8523A92594FACB924B3EDE0D722867C50C7", "3A307A41689BEE99A9A32EA021BF45136906C86B2E4F06C806738398E4F92E45") };
			case FirmwareType.GameboyAdvance: return new("gba_bios.bin") { new(0x4000, "FD2547724B505F487E6DCB29EC2ECFF3AF35A841A77AB2E85FD87350ABD36570") };
			
			case FirmwareType.Sgb1GameboyCpu: return new("sgb_boot.bin") { new(256, "0E4DDFF32FC9D1EEAAE812A157DD246459B00C9E14F2F61751F661F32361E360") };
			case FirmwareType.Sgb2GameboyCpu: return new("sgb2_boot.bin") { new(256, "FD243C4FB27008986316CE3DF29E9CFBCDC0CD52704970555A8BB76EDBEC3988") };
			
			case FirmwareType.SGB1: return new("SGB1.sfc") { new(0x40000,
				"BBA9C269273BEDB9B38BD5EB23BFAA6E509B8DECC7CB80BB5513905AF04F4CEB", //Rev 0 (Japan)
				"C6C4DAAB5C899B69900C460787DE6089EDABE94B760F96D9F583D30CC0A5BB30", //Rev 1 (Japan+USA)
				"A75160F7B89B1F0E20FD2F6441BB86285C7378DB5035EF6885485EAFF6059376" //Rev 2 (World)
			) };

			case FirmwareType.SGB2: return new("SGB2.sfc") { new(0x80000, "C172498A23D1176672931BAB33B629C7D28F914A43DCA9E540B8AF1B37CCF2C6") }; //SGB2 (Japan)
			
			case FirmwareType.FDS: return new("disksys.rom", "FdsBios.bin") { new(0x2000,
				"99C18490ED9002D9C6D999B9D8D15BE5C051BDFA7CC7E73318053C9A994B0178",
				"A0A9D57CBACE21BF9C85C2B85E86656317F0768D7772ACC90C7411AB1DBFF2BF"
			) };

			case FirmwareType.StudyBox: return new("StudyBox.bin") { new(0x40000, "365F84C86F7F7C3AAA2042D78494D41448E998EC5A89AC1B5FECB452951D514C") };

			case FirmwareType.PceSuperCd: return new("[BIOS] Super CD-ROM System (Japan) (v3.0).pce", "syscard3.pce") { new(0x40000, "E11527B3B96CE112A037138988CA72FD117A6B0779C2480D9E03EAEBECE3D9CE")
			};

			case FirmwareType.PceGamesExpress: return new("[BIOS] Games Express CD Card (Japan).pce", "gecard.pce") {
				new(0x8000, "4B86BB96A48A4CA8375FC0109631D0B1D64F255A03B01DE70594D40788BA6C3D"),
				new(0x4000, "DA173B20694C2B52087B099B8C44E471D3EE08A666C90D4AFD997F8E1382ADD8")
			};

			case FirmwareType.ColecoVision: return new("bios.col") { new(0x2000, "990BF1956F10207D8781B619EB74F89B00D921C8D45C95C334C16C8CCECA09AD", "FB4A898EB93B19B36773D87A6C70EB28F981B2686BEBDD2D431B05DCDF9CFFD4") };
			case FirmwareType.SmsBootRom:
				return new("bios.sms") {
					new(0x2000, "477617917A12A30F9F43844909DC2DE6E6A617430F5C9A36306C86414A670D50", "1EB16FC06CEAF05651D7CFD69D1713E6DDCCC4349A511F6FD17E037D71F9D85B", "67846E26764BD862F19179294347F7353A4166B62AC4198A5EC32933B7DA486E", "CB01DE3464797A77B52E82F5F9B05398540DF8C1151D31128D49584028589062"),
					new(0x4000, "6F4E6DE10F1D07BDA695C629C67457BB0248EEB8FFA0C61A2742B8529274DE30"),
					new(0x20000, "0C08E17B0ECC3967E1094F12841FDC97FDCBBF986E3BF9A06B35B69E7D95E4CD", "1A73BC6E1A9973B98432774A1480B3AC1259027A97590D5E67D52745159ABBBC", "E9EB04EC80DED8658F765C4ECA75CDBE23CBC7D8FED73F9C3DB7A868DBF649FF", "13C0639CC037167F53E6151DBDEA013758C3D09A76B064A894BF15C39C0A3897", "AAF39841BE10F9CD6908A95A7451C3A0C8AC77125EB412B7A41CAD260C888AB5", "57ABF3C00C82A8E042EBEB2BFE412ADAC9BA005E9009845236390DE3EE645248"),
					new(0x40000, "C51EC80B7A609C5EE3D203CEEE2B602AE4DFB4479CAF15DC1D030C40C0364E93"),
				};

			case FirmwareType.GgBootRom: return new("bios.gg") { new(0x400, "8C8A21335038285CFA03DC076100C1F0BFADF3E4FF70796F11F3DFAAAB60EEE2") };
			case FirmwareType.WonderSwan: return new("bootrom.ws") { new(0x1000, "BF4480DBEA1C47C8B54CE7BE9382BC1006148F431FBE4277E136351FA74F635E") };
			case FirmwareType.WonderSwanColor: return new("bootrom.wsc") { new(0x2000, "F5A5C044D84CE1681F94E9EF74287CB989784497BE5BD5108DF17908DFA55DB2") };
			case FirmwareType.SwanCrystal: return new("bootrom_sc.wsc") { new(0x2000, "82E96ADDF5AB1CE09A84B6EEDAA904E4CA432756851F7E0CC0649006C183834D") };
			case FirmwareType.Ymf288AdpcmRom: return new("ymf288_adpcm_rom.bin") { new(0x2000, "53AFD0FA9C62EDA3E2BE939E23F3ADF48A2AF8AD37BB1640261726C5D5ADEBA8") };
		}

		throw new Exception("Unsupported firmware type");
	}
}

public class FirmwareFiles : List<FirmwareFileInfo>
{
	public string[] Names;

	public FirmwareFiles(string filename) : this(new string[] { filename })
	{
	}

	public FirmwareFiles(params string[] filename)
	{
		Names = filename;
	}
}

public record FirmwareFileInfo
{
	public int Size;
	public string[] Hashes;

	public FirmwareFileInfo(int size, params string[] hashes)
	{
		Size = size;
		Hashes = hashes;
	}
}
