# OpenRGB controller-family worklist

Pinned-source inventory, not a device-support claim. Priorities are initial triage.
A directory may contain several protocols; exact model comparison can change grouping.

| Priority | Family | Categories | Transport hints | Current coverage | Model labels |
| --- | --- | --- | --- | --- | --- |
| EXCLUDED | DebugController | COOLER, DRAM, GAMEPAD, GPU, HEADSET, HEADSET_STAND, KEYBOARD, LEDSTRIP, LIGHT, MOTHERBOARD, MOUSE, MOUSEMAT, SPEAKER, UNKNOWN | manual review | not implemented | 1 |
| P0 | RazerController | ACCESSORY, COOLER, GPU, HEADSET, HEADSET_STAND, KEYBOARD, KEYPAD, LAPTOP, LEDSTRIP, LIGHT, MICROPHONE, MOUSE, MOUSEMAT, SPEAKER | HID | lighting.peripherals: Basilisk V3 only | 199 |
| P0 | RoccatController | HEADSET, KEYBOARD, MOUSE, MOUSEMAT | HID | lighting.peripherals: selected Vulcan/Kone models only | 28 |
| P0 | SteelSeriesController | HEADSET, KEYBOARD, MOUSE, MOUSEMAT | HID | lighting.peripherals: Apex 3 only | 73 |
| P1 | ASRockPolychromeUSBController | MOTHERBOARD | HID | asrock.polychrome-usb: native zones only, hardware unverified | 2 |
| P1 | AlienwareMonitorController | MONITOR | HID | not implemented | 2 |
| P1 | AsusAuraUSBController | COOLER, GAMEPAD, HEADSET_STAND, KEYBOARD, MONITOR, MOTHERBOARD, MOUSE, MOUSEMAT | HID | not implemented | 93 |
| P1 | AsusMonitorController | MONITOR | HID | not implemented | 7 |
| P1 | CoolerMasterController | ACCESSORY, GPU, KEYBOARD, LEDSTRIP, MONITOR, MOUSE, MOUSEMAT | HID | not implemented | 37 |
| P1 | GigabyteRGBFusion2USBController | MOTHERBOARD | HID | not implemented | 0 |
| P1 | HPOmen30LController | MOTHERBOARD | HID | hp.omen30l: seven zones, hardware unverified | 1 |
| P1 | JGINYUEInternalUSBController | MOTHERBOARD | HID | not implemented | 1 |
| P1 | JGINYUEInternalUSBV2Controller | MOTHERBOARD | HID | not implemented | 1 |
| P1 | LGMonitorController | MONITOR | HID | not implemented | 1 |
| P1 | LenovoMotherboardController | MOTHERBOARD | HID | lenovo.motherboard: native zones only, hardware unverified | 1 |
| P1 | LianLiController | COOLER, LEDSTRIP, MONITOR | HID | not implemented | 11 |
| P1 | MSIMonitorController | MONITOR | HID | not implemented | 1 |
| P1 | MSIMotherboardController | MOTHERBOARD | HID | msi.gen1: B850 profile only; other generations not covered | 124 |
| P1 | ViewSonicController | MONITOR | HID | not implemented | 3 |
| P2 | A4TechController | KEYBOARD, MOUSE, MOUSEMAT | HID | not implemented | 6 |
| P2 | AMBXController | ACCESSORY | manual review | not implemented | 1 |
| P2 | AMDWraithPrismController | COOLER | HID | not implemented | 1 |
| P2 | AOCKeyboardController | KEYBOARD | HID | not implemented | 1 |
| P2 | AOCMouseController | MOUSE | HID | not implemented | 1 |
| P2 | AOCMousematController | MOUSEMAT | HID | not implemented | 1 |
| P2 | AcerNitroHidKeyboardController | LAPTOP, LIGHT | HID | not implemented | 1 |
| P2 | AlienwareController | KEYBOARD | HID | not implemented | 1 |
| P2 | AlienwareKeyboardController | KEYBOARD | HID | not implemented | 2 |
| P2 | AnnePro2Controller | KEYBOARD | HID | not implemented | 1 |
| P2 | ArcticController | LEDSTRIP | manual review | not implemented | 1 |
| P2 | AresonController | MOUSE | HID | not implemented | 6 |
| P2 | AsusAuraCoreController | KEYBOARD, LAPTOP, LEDSTRIP, UNKNOWN | HID | not implemented | 3 |
| P2 | AsusLegacyUSBController | KEYBOARD, MOUSE | HID | not implemented | 3 |
| P2 | AsusTUFLaptopController | LAPTOP | manual review | not implemented | 1 |
| P2 | BlinkyTapeController | LEDSTRIP | serial | not implemented | 1 |
| P2 | CherryKeyboardController | KEYBOARD | HID | not implemented | 25 |
| P2 | ClevoKeyboardController | KEYBOARD | HID | not implemented | 2 |
| P2 | ClevoLightbarController | LEDSTRIP | HID | not implemented | 1 |
| P2 | CorsairCommanderCoreController | COOLER | HID | not implemented | 2 |
| P2 | CorsairController | unresolved | manual review | not implemented | 0 |
| P2 | CorsairHydro2Controller | COOLER | manual review | not implemented | 1 |
| P2 | CorsairHydroController | COOLER | manual review | not implemented | 1 |
| P2 | CorsairHydroPlatinumController | COOLER | HID | not implemented | 13 |
| P2 | CorsairICueLinkController | COOLER | HID | not implemented | 1 |
| P2 | CorsairLightingNodeController | LEDSTRIP | HID | not implemented | 7 |
| P2 | CorsairPeripheralController | HEADSET_STAND, KEYBOARD, MOUSE, MOUSEMAT, UNKNOWN | HID | not implemented | 39 |
| P2 | CorsairPeripheralV2Controller | KEYBOARD, MOUSE, MOUSEMAT | HID | not implemented | 36 |
| P2 | CougarController | KEYBOARD, MOUSE | HID | not implemented | 2 |
| P2 | CreativeController | HEADSET, SPEAKER | HID | not implemented | 2 |
| P2 | CryorigH7QuadLumiController | COOLER | HID | not implemented | 1 |
| P2 | DDPController | LEDSTRIP | manual review | not implemented | 1 |
| P2 | DMXController | LEDSTRIP | manual review | not implemented | 1 |
| P2 | DRGBController | LEDSTRIP | HID | not implemented | 32 |
| P2 | DarkProject | KEYBOARD | HID | not implemented | 1 |
| P2 | DasKeyboardController | KEYBOARD | HID | not implemented | 3 |
| P2 | DreamCheekyController | ACCESSORY | HID | not implemented | 1 |
| P2 | DuckyKeyboardController | KEYBOARD | HID | not implemented | 2 |
| P2 | DygmaRaiseController | KEYBOARD | manual review | not implemented | 1 |
| P2 | E131Controller | LEDSTRIP | manual review | not implemented | 1 |
| P2 | EKController | LEDSTRIP | HID | not implemented | 1 |
| P2 | EVGAUSBController | KEYBOARD, MOUSE | HID | not implemented | 5 |
| P2 | EVisionKeyboardController | KEYBOARD | HID | not implemented | 15 |
| P2 | EdifierHaloController | SPEAKER | HID | not implemented | 1 |
| P2 | ElgatoKeyLightController | LIGHT | manual review | not implemented | 1 |
| P2 | ElgatoLightStripController | LEDSTRIP | manual review | not implemented | 1 |
| P2 | EspurnaController | LIGHT | manual review | not implemented | 1 |
| P2 | FanBusController | COOLER | manual review | not implemented | 1 |
| P2 | FaustusController | LAPTOP | manual review | not implemented | 1 |
| P2 | FnaticStreakController | KEYBOARD | HID | not implemented | 3 |
| P2 | GaiZongGaiKeyboardController | ACCESSORY, KEYBOARD, KEYPAD, LEDSTRIP, UNKNOWN | HID | not implemented | 9 |
| P2 | GameSirController | GAMEPAD | HID | not implemented | 2 |
| P2 | GigabyteAorusCPUCoolerController | COOLER | HID | not implemented | 1 |
| P2 | GigabyteAorusLaptopController | LAPTOP | HID | not implemented | 4 |
| P2 | GigabyteAorusMouseController | MOUSE | HID | not implemented | 1 |
| P2 | GigabyteAorusPCCaseController | CASE | HID | not implemented | 1 |
| P2 | GigabyteCastor3Controller | COOLER | HID | not implemented | 1 |
| P2 | GoveeController | LIGHT | manual review | not implemented | 1 |
| P2 | HIDLampArrayController | ACCESSORY, GAMEPAD, HEADSET, KEYBOARD, MICROPHONE, MOUSE, SPEAKER, UNKNOWN | HID | not implemented | 1 |
| P2 | HPOmenLaptopController | KEYBOARD | manual review | not implemented | 1 |
| P2 | HYTEKeyboardController | KEYBOARD | HID | not implemented | 1 |
| P2 | HYTEMousematController | MOUSEMAT | serial | not implemented | 2 |
| P2 | HYTENexusController | LEDSTRIP | serial | not implemented | 2 |
| P2 | HoltekController | MOUSE, MOUSEMAT | HID | not implemented | 2 |
| P2 | HyperXKeyboardController | KEYBOARD | HID | not implemented | 13 |
| P2 | HyperXMicrophoneController | MICROPHONE | HID | not implemented | 2 |
| P2 | HyperXMicrophoneV2Controller | MICROPHONE | HID | not implemented | 1 |
| P2 | HyperXMouseController | MOUSE | HID | not implemented | 9 |
| P2 | HyperXMousematController | MOUSEMAT | HID | not implemented | 3 |
| P2 | InstantMouseController | MOUSE | HID | not implemented | 4 |
| P2 | IntelArcA770LEController | GPU | HID | not implemented | 1 |
| P2 | IonicoController | KEYBOARD, LEDSTRIP | HID | not implemented | 2 |
| P2 | KasaSmartController | LEDSTRIP, LIGHT | manual review | not implemented | 1 |
| P2 | KeychronKeyboardController | KEYBOARD | HID | not implemented | 1 |
| P2 | LEDStripController | LEDSTRIP | manual review | not implemented | 1 |
| P2 | LIFXController | LIGHT | manual review | not implemented | 1 |
| P2 | LaviewTechnologyController | MOUSE | HID | not implemented | 1 |
| P2 | LegoDimensionsToypadBaseController | LEDSTRIP | HID | not implemented | 1 |
| P2 | LenovoControllers | KEYBOARD, LAPTOP, MOUSE | HID | not implemented | 23 |
| P2 | LexipMouseController | MOUSE | HID | not implemented | 1 |
| P2 | LightSaltController | KEYBOARD, KEYPAD | HID | not implemented | 1 |
| P2 | LinuxLEDController | LEDSTRIP | manual review | not implemented | 1 |
| P2 | LogitechController | GAMEPAD, HEADSET, KEYBOARD, KEYPAD, LIGHT, MICROPHONE, MOUSE, MOUSEMAT, SPEAKER, UNKNOWN | HID | not implemented | 45 |
| P2 | LuxaforController | ACCESSORY | HID | not implemented | 1 |
| P2 | MNTKeyboardController | KEYBOARD | HID | not implemented | 2 |
| P2 | MSI3ZoneController | LAPTOP | HID | not implemented | 1 |
| P2 | MSIClutchController | MOUSE | HID | not implemented | 1 |
| P2 | MSIKeyboardController | KEYBOARD | HID | not implemented | 4 |
| P2 | MSILaptopController | KEYBOARD, LEDSTRIP | HID | not implemented | 4 |
| P2 | MSIOptixController | LEDSTRIP | HID | not implemented | 1 |
| P2 | MSIVigorController | KEYBOARD | HID | not implemented | 1 |
| P2 | MadCatzCyborgController | ACCESSORY | HID | not implemented | 1 |
| P2 | MintakaKeyboardController | KEYBOARD | HID | not implemented | 1 |
| P2 | MountainKeyboardController | KEYBOARD | HID | not implemented | 2 |
| P2 | N5312AController | MOUSE | HID | not implemented | 1 |
| P2 | NVIDIAIlluminationController | GPU | manual review | not implemented | 1 |
| P2 | NZXTHue1Controller | LEDSTRIP | HID | not implemented | 1 |
| P2 | NZXTHue2Controller | LEDSTRIP | HID | not implemented | 11 |
| P2 | NZXTHuePlusController | LEDSTRIP | serial | not implemented | 1 |
| P2 | NZXTKrakenController | COOLER | HID | not implemented | 2 |
| P2 | NZXTMouseController | MOUSE | HID | not implemented | 1 |
| P2 | NanoleafController | LIGHT | manual review | not implemented | 1 |
| P2 | NollieController | LEDSTRIP | HID | not implemented | 16 |
| P2 | NvidiaESAController | CASE | HID | not implemented | 1 |
| P2 | OKSController | KEYBOARD | HID | not implemented | 1 |
| P2 | PatriotViperMouseController | MOUSE | HID | not implemented | 1 |
| P2 | PhilipsHueController | LIGHT | network candidate | not implemented | 1 |
| P2 | PhilipsWizController | LIGHT | manual review | not implemented | 1 |
| P2 | QMKController | KEYBOARD | HID | not implemented | 5 |
| P2 | RealtekARGBController | LEDSTRIP, UNKNOWN | HID | not implemented | 1 |
| P2 | RealtekBridgeController | STORAGE, UNKNOWN | HID | not implemented | 3 |
| P2 | RedSquareKeyroxController | KEYBOARD | HID | not implemented | 2 |
| P2 | RedragonController | MOUSE | HID | not implemented | 11 |
| P2 | RobobloqLightStripController | LEDSTRIP | HID | not implemented | 1 |
| P2 | RoyuanKeyboardController | KEYBOARD | HID | not implemented | 5 |
| P2 | SRGBmodsController | LEDSTRIP | HID | not implemented | 2 |
| P2 | SayoDeviceController | KEYBOARD | HID | not implemented | 1 |
| P2 | SeagateController | STORAGE | manual review | not implemented | 1 |
| P2 | SinowealthController | KEYBOARD, MOUSE | HID | not implemented | 9 |
| P2 | SkydimoHIDController | HEADSET_STAND | HID | not implemented | 1 |
| P2 | SkydimoSerialController | LEDSTRIP | serial | not implemented | 1 |
| P2 | SkyloongController | KEYBOARD | HID | not implemented | 1 |
| P2 | SonyGamepadController | GAMEPAD | HID | not implemented | 3 |
| P2 | StreamDeckController | ACCESSORY | HID | not implemented | 1 |
| P2 | TecknetController | MOUSE | HID | not implemented | 1 |
| P2 | ThermaltakePoseidonZRGBController | KEYBOARD | HID | not implemented | 1 |
| P2 | ThermaltakeRiingController | COOLER | HID | not implemented | 51 |
| P2 | ThingMController | LEDSTRIP | HID | not implemented | 1 |
| P2 | ThrustmasterSolController | GAMEPAD | manual review | not implemented | 1 |
| P2 | TrustController | MOUSE | HID | not implemented | 2 |
| P2 | TurtleBeachKP7Controller | KEYPAD | HID | not implemented | 1 |
| P2 | ValkyrieKeyboardController | KEYBOARD | HID | not implemented | 2 |
| P2 | ValveSteamMachineController | LEDSTRIP | manual review | not implemented | 1 |
| P2 | WinbondGamingKeyboardController | KEYBOARD | HID | not implemented | 1 |
| P2 | WitmodKeyboardController | KEYBOARD | HID | not implemented | 1 |
| P2 | WootingKeyboardController | KEYBOARD | HID | not implemented | 13 |
| P2 | WushiController | ACCESSORY | HID | not implemented | 1 |
| P2 | XPGSummonerKeyboardController | KEYBOARD | HID | not implemented | 1 |
| P2 | YeelightController | LIGHT | manual review | not implemented | 1 |
| P2 | ZETKeyboardController | KEYBOARD | HID | not implemented | 1 |
| P2 | ZalmanZSyncController | LEDSTRIP | HID | not implemented | 1 |
| P3 | ASRockSMBusController | GPU, MOTHERBOARD | SMBus/I2C | not implemented | 8 |
| P3 | AsusAuraGPUController | GPU | SMBus/I2C | not implemented | 57 |
| P3 | ColorfulGPUController | GPU | SMBus/I2C | not implemented | 28 |
| P3 | ColorfulTuringGPUController | GPU | SMBus/I2C | not implemented | 1 |
| P3 | CorsairDRAMController | DRAM | SMBus/I2C | not implemented | 0 |
| P3 | CorsairVengeanceController | DRAM | SMBus/I2C | not implemented | 1 |
| P3 | CrucialController | DRAM | SMBus/I2C | not implemented | 1 |
| P3 | ENESMBusController | DRAM, GPU, MOTHERBOARD, STORAGE | SMBus/I2C | lighting.auxiliary: qualified fixed-address XPG/AMD path only | 145 |
| P3 | EVGAAmpereGPUController | GPU | SMBus/I2C | not implemented | 59 |
| P3 | EVGAGP102GPUController | GPU | SMBus/I2C | not implemented | 8 |
| P3 | EVGAPascalGPUController | GPU | SMBus/I2C | not implemented | 5 |
| P3 | EVGASMBusController | MOTHERBOARD | SMBus/I2C | not implemented | 1 |
| P3 | EVGATuringGPUController | GPU | SMBus/I2C | not implemented | 26 |
| P3 | FrameworkDesktopController | MOTHERBOARD | EC/kernel | not implemented | 1 |
| P3 | GalaxGPUController | GPU | SMBus/I2C | not implemented | 10 |
| P3 | GigabyteRGBFusion2BlackwellGPUController | GPU | SMBus/I2C | lighting.auxiliary: Aero 5080 only | 32 |
| P3 | GigabyteRGBFusion2DRAMController | DRAM | SMBus/I2C | not implemented | 1 |
| P3 | GigabyteRGBFusion2GPUController | GPU | SMBus/I2C | not implemented | 87 |
| P3 | GigabyteRGBFusion2SMBusController | MOTHERBOARD | SMBus/I2C | not implemented | 0 |
| P3 | GigabyteRGBFusionController | MOTHERBOARD | SMBus/I2C | not implemented | 1 |
| P3 | GigabyteRGBFusionGPUController | GPU | SMBus/I2C | not implemented | 75 |
| P3 | GigabyteSuperIORGBController | MOTHERBOARD | port I/O | not implemented | 1 |
| P3 | HyperXDRAMController | DRAM | SMBus/I2C | not implemented | 1 |
| P3 | KingstonFuryDRAMController | DRAM | SMBus/I2C | not implemented | 2 |
| P3 | MSIGPUController | GPU | SMBus/I2C | not implemented | 123 |
| P3 | MSIRGBController | MOTHERBOARD | port I/O | not implemented | 1 |
| P3 | ManliGPUController | GPU | SMBus/I2C | not implemented | 1 |
| P3 | PNYARGBEpicXGPUController | GPU | SMBus/I2C | not implemented | 6 |
| P3 | PNYLovelaceGPUController | GPU | SMBus/I2C | not implemented | 12 |
| P3 | PalitGPUController | GPU | SMBus/I2C | not implemented | 68 |
| P3 | PatriotViperController | DRAM | SMBus/I2C | not implemented | 0 |
| P3 | PatriotViperSteelController | DRAM | SMBus/I2C | not implemented | 0 |
| P3 | PowerColorGPUController | GPU | SMBus/I2C | not implemented | 10 |
| P3 | SapphireGPUController | GPU | SMBus/I2C | not implemented | 28 |
| P3 | TForceXtreemController | DRAM | SMBus/I2C | not implemented | 1 |
| P3 | ZotacBlackwellGPUController | GPU | SMBus/I2C | not implemented | 3 |
| P3 | ZotacTuringGPUController | GPU | SMBus/I2C | not implemented | 4 |
| P3 | ZotacV2GPUController | GPU | SMBus/I2C | not implemented | 15 |
