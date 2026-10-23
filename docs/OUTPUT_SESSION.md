# Output session backend

`prepareOutput(document, composedFrame)` produces one typed `OutputFrame` per
exact endpoint/profile. It uses `sampleChannels` once, concatenates physical
chains in their already-mapped wire order, converts opaque RGB to driver colors
and includes native effect commands without sampling native channels. It does
not render again, open devices or call a driver.

`OutputSession` is single-thread owned and synchronous. Its host must resolve
process conflicts, present/obtain the required real-device output confirmation,
revalidate discovery identity and explicitly open the selected exact path before
transferring a transport to `connect`. The session does not enumerate, open or
choose an endpoint. It verifies profile/identity through `ModuleRegistry`, owns
the transport and driver, and keeps plugin code loaded through teardown.
Creating a connection sends no reports; `Connected` means an owned open handle
and driver, not proof that a physically present device will accept output.

`stage(frame)` validates the entire bundle before any writes and replaces the
single pending frame. Wrong endpoint/profile, unknown or duplicate channels,
unsupported software/native mode, empty/oversized software data, fixed-count
mismatches and invalid native options are rejected. An invalid replacement also
clears older pending work so a later flush cannot transmit stale data. Empty
bundles are allowed and send nothing; they do not restore firmware state.

`flush()` consumes at most that latest bundle. Software colors are passed to the
module; native settings are sent only when changed since their last successful
application. Cache entries include mode, colors, brightness, speed and direction.
A successful software command invalidates that channel's native cache. The
module retains responsibility for protocol arming and report serialization.
No timer, background worker, queue or automatic retry is hidden in this class.
A future scheduler must call it from its owning worker thread and avoid blocking
the GUI with synchronous HID calls.

A failed/throwing driver write or a closed transport discards remaining commands,
destroys the driver before closing/destroying its transport and sets `Faulted`.
Later stage/flush calls cannot write and retain the original fault explanation.
Explicit reconnect uses a fresh transport/driver and empty caches, so software
arming and native settings are reapplied. Disconnect/destruction cancels pending
work and closes ownership without sending off/restore reports. A successful
write to one channel cannot be rolled back if a later channel fails.

Physical unplug detection is limited: `HidApiDevice::isOpen()` reports handle
ownership. A software write failure can reveal an unplug, but an idle native-only
session needs host enumeration/notification handling; no polling is added here.
The MSI module writes complete zone-state `0x50` reports when configuring or
arming a zone. It does not read existing firmware state to preserve unassigned
zones. Real-device confirmation must account for those complete reports, not
just a selected channel label. No live output UI is enabled by this backend.

## Verification

The output test target loads the actual MSI module with recording fake HID
transports, never HidApiDevice. It verifies zero construction reports, newest
frame replacement, exact 0x51 bytes, native caching/change detection and
Native/Software transitions. It checks malformed bundles before any write,
count/capability rejection, pending cancellation, partial-write failures,
exceptions, closed transports, explicit reconnect/rearming and plugin lifetime.
Document preparation covers separate endpoints, chain permutations, alpha over
black and exclusion of sampled colors for native commands. This is protocol and
lifecycle evidence, not physical color, timing, reconnect or hardware proof.
