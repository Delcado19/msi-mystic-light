# Installed lighting hardware

Inventory source: the user-provided July 2026 Caseking system invoice. No personal details, serial numbers or invoice contents are reproduced here. The list describes model evidence, not automatic cable detection.

| Component | Quantity | Lighting model / evidence | App support |
| --- | --- | --- | --- |
| Phanteks M25 Gen2 120 white | 1 standalone + 3 supplied with AIO | Manufacturer: 12 physical LEDs. Six paired address groups follow the user's hardware observation. | Fan catalog templates, six addresses / two physical LEDs each |
| Phanteks M25 Gen2 120 Reverse white | 4 | Same 12 physical LEDs / six user-observed groups | Fan catalog template |
| Phanteks Glacier One 360M25 Gen2 | 1 | Pump D-RGB via Linq6; three M25 G2 fans. Pump address count unverified for invoice revision. | Separate pump template; user must enter actual address count. Add fans separately. |
| Phanteks NV5 MK2 white | 1 | Integrated PSU-shroud D-RGB, motherboard synchronization. Address count unverified. | Case template; actual address count required |
| ADATA XPG Lancer Blade RGB DDR5-6000 CL36 | 2 x 24 GB | Two matching AX5U6000C3624G-DTLABRBK modules identified locally; each ENE AUDA0-E6K5-0101 controller reports eight addresses at 0x71 / 0x73 | Two Canvas devices via a dedicated RAM-only OpenRGB SDK helper; not attachable to an ARGB header |
| MSI MPG B850 Edge TI WiFi | 1 | Existing USB 0DB0:0076 controller. I/O cover zone 4 responds to 0x50; 0x51 per-LED tests failed. | ARGB headers plus I/O cover whole-zone Canvas sampling via volatile Steady color updates, limited to 10 Hz |
| Gigabyte RTX 5080 AERO OC SFF 16G | 1 | PCI 10DE:2C02 / 1458:4179 observed locally. OpenRGB Blackwell detector uses I2C 0x75, one logo color zone. | NVAPI RGB port 1, exact PCI match and controller signature required. Direct colors at <=30 Hz; no flash-save command |
| AMD Ryzen 9 9950X | 1 | No separately controlled RGB on the processor | Not an RGB catalog item |
| Corsair RM1000e white | 1 | No RGB lighting identified for this model | Not an RGB catalog item |
| Kingston NV3 2 TB | 1 | No RGB lighting identified for this model | Not an RGB catalog item |

Daisy-chaining fan frames does not prove independent addressing across fans: parallel RGB wiring mirrors the same addresses. Cable-to-header assignments must remain user configured. No existing profile wiring is rewritten from the invoice.

## Sources

- [Phanteks M25 Gen2 specifications](https://phanteks.com/product/m25g2-120-black-d-rgb/): physical LED count and regular/reverse variants.
- [Phanteks Glacier One 360M25 G2](https://phanteks.com/product/glacier-one-360m25g2-white/): D-RGB connection and included fans. Current manufacturer page identifies DWT02; the invoice does not establish this revision, so its pump count is not assumed.
- [Phanteks NV5 MK2](https://phanteks.com/product/nv5-mk2-black/): built-in D-RGB and motherboard synchronization.
- [XPG Lancer Blade RGB datasheet](https://webapi3.adata.com/storage/downloadfile/datasheet_xpg_lancer_blade_rgb_ddr5_memory_20250318.pdf): RGB control and module capacities.
- [Gigabyte RTX 5080 AERO](https://www.gigabyte.com/se/Graphics-Card/GV-N5080AERO-OC-16GD): RGB lighting.
- Local OpenRGB source: `Controllers/GigabyteRGBFusion2BlackwellGPUController/`, `i2c_smbus/Windows/i2c_smbus_nvapi.cpp`, `dependencies/NVFC/nvapi.h`. Protocol/ABI adaptation is GPL-2.0-or-later, as identified in `src/device/GigabyteAeroDriver.h`.

## Layout editor

The dialog opens maximized with standard Windows controls. Snap to grid is optional, starts disabled, and uses a visible grid with adjustable 1-100 scene-unit spacing. Components and individual LEDs snap in scene coordinates even inside rotated/scaled components. Saving bakes coordinates; cancel retains existing hardware mappings. Native keyboard key units can use a 1-unit grid.

## Validation limits

Packet and UI tests establish formatting, zone preservation, snap transforms and capability gating. They do not prove physical color output. GPU support is exposed only after its exact PCI match and RGB-controller handshake succeed. RAM color transport uses the separately running administrator-level OpenRGB/PawnIO helper on loopback port 6745. The app reconnects to an existing helper or starts the installed OpenRGB with its isolated bundled detector configuration and a Windows UAC prompt. The helper stays running when the Studio closes. It neither loads the user's normal OpenRGB profile nor enables motherboard, GPU or peripheral detection. I/O cover is a shared color zone, not eleven independently verified LEDs.

## Layout visibility and model labels

Each device row has an eye toggle. Visibility is saved per profile and affects only drawing and pointer hits; hidden devices keep receiving physical lighting. Single-zone GPU and I/O artwork uses the entire placement footprint, rather than a zero-size LED extent. RAM light-bar artwork also retains a readable height. Existing locally configured six-region paired fans were relabeled as Phanteks M25 Gen2 without changing their LED mappings, coordinates or port order; regular/reverse templates remain separate choices in the catalog.

## Runtime confirmation

The user confirmed that I/O cover colors follow the canvas and that editing/saving the RAM LED arrangement works after preserving standalone device footprints. GPU physical illumination remains unconfirmed; a successful controller handshake or accepted color packet alone is not visual output proof.
