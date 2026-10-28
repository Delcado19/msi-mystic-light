# Output session backend

Installed Windows mode delegates rendering/output to an automatic service; the
editor stays unelevated and closing it retains playback. Portable mode retains
the direct output lifecycle described below. See [Windows service operation,
security and validation limits](WINDOWS_SERVICE.md).

`prepareOutput(document, composedFrame, seconds)` produces one typed `OutputFrame` per
exact endpoint/profile. It uses `sampleChannels` once, concatenates physical
chains in their already-mapped wire order, converts opaque RGB to driver colors
and includes native effect commands without sampling native channels. It does
not open devices or call a driver. Scoped/custom-clock channels render locally.

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
The `OutputScheduler` below owns it on a worker thread to keep synchronous HID
calls away from the GUI.

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

## Output scheduler

`OutputScheduler` is an owner-thread QObject with one standard-library worker.
Call `start(factory)` and `stop()` on its QObject owner thread. `submit(runId,
frame)` and `status()` are thread-safe. Start returns a nonzero run generation;
submissions for an earlier generation are rejected after a restart. The host
must complete conflict resolution, explicit output confirmation and identity
selection before starting a factory. Capture immutable connection data, not
widgets or a GUI-owned ModuleRegistry. The factory creates its transport,
registry/module and connected OutputSession entirely on the worker. A session's
DriverInstance keeps the module loaded after the local registry goes away.

There is one mutex-protected pending frame, not a posted event per submission.
The worker waits 33 ms between completed iterations, takes only the latest frame,
stages and flushes it, then waits again. A slow call does not cause catch-up
bursts. There is no repeat of an unchanged software frame when no new frame was
submitted; native commands retain the session's successful-setting cache. Idle
iterations check handle ownership only, not physical endpoint presence.
Rendering and prepareOutput remain the producer's responsibility and must use
a stable document snapshot or the document's owner thread.

States are Stopped, Starting, Running, Stopping and Faulted. statusChanged asks
observers to read the current status; a queued notification may describe an
older run, so it carries no captured frame/status object. A null, disconnected
or throwing factory, an invalid frame or a driver failure closes worker-owned
resources and enters Faulted. There is no automatic retry. Restart uses a new
factory, new driver/caches and new generation.

Stop is nonblocking: it cancels pending work, wakes the worker and enters
Stopping until cleanup is complete. A factory or driver call already in progress
may finish; flush checks cancellation before starting each subsequent channel.
A single MSI software driver call may contain both arming and color reports,
so stop cannot interrupt between those internal reports. Destruction joins the
worker and can therefore wait for an in-flight OS call; no unsafe forced thread
termination is used. Transport creation, writes, close and destruction all occur
on the worker. The output window connection controls are described below.

Scheduler tests use blocked fake factories/writes and the actual MSI module.
They verify 100-frame replacement during startup, replacement during a slow
write, owner event-loop responsiveness, pacing without catch-up, stop during
I/O/startup, old-generation rejection, explicit reconnect, factory/input/write
faults and worker-thread teardown. These do not qualify physical USB timing,
a hung OS driver, unplug notification or actual device behavior.

## Automatic GUI output and presence

The app enables canvas output after project restoration and initial discovery.
Configured, detected endpoints connect automatically, without an initial report
approval checkbox. Startup process-conflict choices remain user-controlled; no
RGB process is terminated automatically. --editor skips that startup conflict dialog
but still enables configured hardware output. --inspect and --scan never write.

The host serializes initial reports through the real module using a memory-only
transport. Worker factories revalidate exact endpoint/profile identity, paired
Vulcan interfaces and serialized bytes before opening HID. Saved offline endpoints
are skipped rather than preventing output to reachable devices. MSI 0x50 includes
all 18 zones; existing unassigned zone settings are not preserved.

Software RGB payloads update live through effects, masks, layout sampling, the
preview clock and undo. Animated output starts playback if it was paused.
Topology/native-command changes stop the old workers and automatically rebuild
connections with the new validated configuration. A project load closes old
connections before replacing state, then resumes using the new assignments.

Presence is checked approximately once per second. Faults stop the affected endpoint worker;
automatic mode retries connection preparation every two seconds. Tray Stop and accepted close disable automatic retries and queued restarts.
Hardware Stop pauses selected channels while preserving their shared connection;
Resume output resumes that selection. Global stopOutput disables automatic retries. The next app launch defaults to automatic output.
Stop sends no restore/off command; devices retain their last applied state.

Tests use recording transports and verify automatic startup, scope changes, offline
subsets, changing reports, explicit Stop and queued-restart cancellation. Physical
USB behavior and color calibration still require observation on the real hardware.

### Transient layout sampling and per-channel pause

The canvas exposes temporary placement transforms without modifying the document.
Output preparation samples those placements during a gesture; release commits one
undo entry and clears the preview delta before document notifications. Cancellation
resubmits the unchanged document geometry. Rendering and per-channel time/scoped
layers retain the existing composition path.

Per-channel pauses filter commands from submitted frames, after full configuration
scope validation. An empty frame sends no commands. Pausing a header never closes
a shared controller or sends black/off/restore packets; other headers keep running.
The selected controller row applies the action to its configured channels. Pauses
are session-local, survive configuration reconnection and reset on the next launch.
A worker call already in flight can complete. New connections receive exactly
one initial submission; resuming an existing paused channel submits the current
frame without reopening its controller. Native firmware animations are not
frozen by withholding further commands. Paused state is independent of transport
state and is displayed on each channel row. Failed connections can be resumed
without closing other active controllers. Stop/Resume has no primary-button accent.

### Restored native transports

The flat endpoint/profile table is separate from Hardware's component tree. Left
inventory selection scopes Stop/Resume to a channel; clicking a technical endpoint
row selects all configured channels of that connection. The table includes exact
NVAPI targets even when unavailable, with no duplicated header rows.

The worker/session path owns the qualified NVAPI transport adapter. Module creation
and proof generation remain memory-only. The GPU lighting handshake occurs when
the worker opens that actual endpoint. Presence checks enumerate PCI without
changing lighting. IOShield uses volatile shared-color `0x50` updates, preserving
other successfully applied MSI zone settings. RAM now uses a worker-owned direct
PawnIO transport, approved by the user on 2026-10-24. GUI discovery verifies ENE
identity first; worker opening re-verifies it before volatile color output. Proof
generation remains memory-only; passive presence checks use the verified inventory
and transfer failures fault the endpoint. No RGB helper or socket is started.
See HARDWARE_MODULES.md for qualification and remaining physical validation.

Layout sampling uses LightingDocument::layoutBounds, derived from all physical
LED positions (including transient placements). The 640x400 composite is mapped
onto this growing world rectangle for both preview and output, so moved components
remain lit beyond the old edges. Global bounds are shared across endpoint and
channel-scoped renders. Changing the bounds also changes peers' sample positions.
Canvas Stop freezes animation time while devices retain/update the current frame;
per-endpoint stop/resume remains available in Hardware.
