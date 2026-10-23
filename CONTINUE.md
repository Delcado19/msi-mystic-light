# Implementation status

## Completed: first v1 increment (2026-10-23)

- Added the C++17 CMake project and a Qt6 executable scaffold.
- Implemented `BuildReport0x50` and `BuildReport0x51` as pure serializers,
  following `docs/MB_800_HID_PROTOCOL.md`; neither performs hardware I/O.
- Report `0x50` defaults to transient settings (`Store=0`). Invalid lighting
  modes, color counts, brightness, and speed throw `std::invalid_argument`.
- Report `0x51` rejects unknown area addresses and more than 240 LEDs.
  The upcoming driver must enforce each board channel's narrower limits.
- Added protocol tests and an offscreen application startup smoke test.

## Validation

Built with MSVC 19.51, Ninja, and shared Qt 6.8.3 x64 on Windows.
Both CTest entries passed. The protocol suite has five substantive test
slots covering native record boundaries, option packing, invalid native
settings, per-LED layout/padding, and packet capacity limits.
No device was opened and no USB reports were sent.
CTest verification ran outside the agent sandbox: inside it, Qt's offscreen
platform plugin was not discoverable and the app test hit its 15-second
timeout. Direct application startup also passed outside CTest.

## Next steps

Continue the v1 plan at Task 3: `IHidDevice`, fake transport, and the real
HIDAPI adapter; then Components, Channels, and the motherboard driver.
Tasks 4–12 (device model, effects, renderer, persistence, tray, autostart,
integration and hardware verification) remain unimplemented.
Lighting Studio is a subsequent increment, not part of this scaffold.

Differences from the historical implementation plan:

- HIDAPI discovery/linking is deferred until the transport actually uses it.
- The real protocol suite replaces the arithmetic-only scaffold test.
- Encoders reject invalid settings instead of silently masking their bits.
- Plan checkboxes are retained as historical instructions, not an up-to-date
  completion ledger; use this file for current progress.

Before the first real-device run, follow Task 12's safety gate: show the
exact outgoing bytes and intended zone changes, and obtain explicit user
confirmation. Passing serializer tests does not qualify hardware behavior.
