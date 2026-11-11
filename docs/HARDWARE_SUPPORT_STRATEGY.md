# Hardware support strategy

See [the migration inventory and priorities](hardware-inventory/README.md) for
the pinned reference-family/model index, unresolved comparisons and installed
manufacturer source metadata. Inventory entries are not support claims.

Owner decision: 2026-10-30. This is an implementation and research policy, not
a claim that the listed capabilities have been implemented or tested.

## Independent hardware modules

Lighting Studio will use its own hardware modules. OpenRGB and SignalRGB are
reference sources, not required installed applications or background servers.
An OpenRGB SDK server is not the default dependency for Windows or Linux.
Any future optional bridge requires a separate decision.

Keep protocol-family drivers separate from device profiles and component data.
Reuse a driver for compatible models; do not create a DLL for every product or
collect all hardware implementations into one monolithic driver. Device profiles
describe identity, capabilities and protocol parameters. Component definitions
describe LED geometry and wiring. A profile cannot replace a missing protocol.

Linux is a target for this strategy, but native Windows transports do not imply
Linux support. Record platform dependencies and qualification independently.

## Mandatory comparison of reference implementations

For every exact hardware model supported by both OpenRGB and SignalRGB, inspect
both implementations before deciding how to implement or extend our support.
This applies to motherboards, fans, controllers, displays and peripherals, not
only the initial motherboard work.

Pin source revisions and identify the exact model, hardware revision, USB IDs,
interfaces and firmware scope. Do not treat a similar name or shared vendor ID
as proof that two implementations address the same device variant.

Record differences in detection, initialization, transport, packet formats,
checksums, read-back, LED/channel topology, color order, limits, native modes,
frame timing, persistence, shutdown and known firmware workarounds. State which
behavior we select and why. Neither upstream implementation wins automatically;
unresolved differences remain explicit research questions.

If only one source supports a model, document that fact and the remaining
uncertainty. Review the license of each source before copying or distributing
code. Accessible source is not automatically permission to reuse it. Keep
proprietary binaries and decompiled vendor code out of the repository.

## Installed manufacturer software and peripheral settings

The owner identifies Roccat, Razer and SteelSeries as currently active peripheral
families. Inventory their installed manufacturer software as another source for
additional models, capability tables, layouts, device identifiers, configuration
formats and protocol behavior. Record exact application, driver and module
versions. Installed files may reveal support beyond the devices physically
available, but an entry in a catalog is not proof of working hardware support.

Owner-supplied manufacturer entry points (recorded 2026-10-30; do not launch
these applications as part of inventory without an explicit execution task):

| Application | Executable | Launch arguments |
| --- | --- | --- |
| Roccat Swarm | `C:\Program Files (x86)\ROCCAT\ROCCAT SWARM\ROCCAT_Swarm.exe` | None supplied |
| Razer Synapse via App Engine | `C:\Program Files\Razer\RazerAppEngine\RazerAppEngine.exe` | `--url-params=apps=synapse` |
| SteelSeries GG | `C:\Program Files\SteelSeries\GG\SteelSeriesGGEZ.exe` | `-dataPath="C:\ProgramData\SteelSeries\GG" -dbEnv=production` |

These are application entry points, not necessarily the modules or services that
implement device control. Locate the actual device packages and relevant service
boundaries during the analysis; preserve installed configuration and databases.

The owner additionally wants THX Spatial Audio to work. Track this as an audio
integration/compatibility investigation, not an assumed hardware-module feature.
Identify the actual headset/output device, installed THX/Synapse components,
entitlement requirements and audio-processing path before proposing support.
Distinguish preserving operation of an existing licensed installation from
controlling exposed settings or implementing an equivalent audio processor.
Do not promise standalone THX functionality or redistribution of proprietary
audio components. This request records the desired outcome; it does not authorize
audio-driver installation, routing changes or replacement of the audio stack.

The owner also reports an AGON monitor with LED lighting and a small set of
presets, controlled by G-Menu installed at `C:\G-Menu`. Include this installation
as a research source. Identify the exact monitor model, firmware, connection and
control transport before selecting a driver approach. Inventory its lighting
presets, their parameters and the remaining exposed monitor settings; distinguish
standard monitor controls from vendor-specific lighting commands. The reported
presets do not establish per-LED streaming support. No monitor capability or
protocol has been verified by recording this installation path.

Research the complete settings surface of each inspected product, including:

- Motion and acceleration, sensitivity, DPI stages and separate axes.
- Mouse button assignments, keyboard mappings and alternate layers/profiles.
- Distance Control Unit, lift-off/landing distance and surface calibration.
- Angle snapping and polling rate.
- Macro assignment, recording/playback timing and repeat behavior.
- Character repeat delay/rate and blink rate, with the original UI label and unit
  recorded to avoid assuming whether blink refers to lighting or a host cursor.
- Lighting, onboard profiles and every other setting exposed by the inspected
  product's software, including settings not listed above.

For every setting, distinguish device firmware functionality from manufacturer
software behavior, operating-system preferences and application-specific logic.
Record scope, units, allowed values, defaults, read/write commands, volatile vs
onboard persistence, reconnect behavior and platform availability. Do not assume
that a vendor utility feature can be sent to hardware or works without that
utility. A feature shared by one model does not establish support across a brand.

## Evidence and qualification

Use static source/binary analysis and controlled vendor-application captures as
appropriate. Capture a baseline and one setting change at a time; retain exact
inputs, firmware/software versions and observed outcomes. Unknown commands must
not be replayed blindly. Firmware updates, reset commands and persistent writes
are separate operations, not routine discovery steps.

Automated protocol tests and captured report fixtures support development
without owning every device. Actual device owners provide physical qualification.
Track implemented, capture-validated, physically verified and unsupported states
separately. Never present a protocol test or an upstream support entry as proof
of physical operation in Lighting Studio.

A guided diagnostic/capture assistant is a candidate follow-up, not an approved
implementation in this decision. Its role would be evidence collection and
export, not automatic probing of arbitrary commands.

## Execution boundary

This decision records the intended architecture and research scope only. It
does not itself start vendor-software analysis, new driver implementation,
hardware writes, device-setting changes or a JavaScript plugin host.

The owner subsequently authorized starting migration on 2026-10-30. The first
implemented increment is documented in HP_OMEN_MIGRATION.md. This does not imply
completion of all hardware families or manufacturer configuration features.
