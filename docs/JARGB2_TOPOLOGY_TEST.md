# JARGB2 physical response tests - 2026-11-13 / 2026-11-14

Board/profile: MSI MPG B850 EDGE TI WIFI, JARGB2. Owner-described setup:
four fans with six color zones each and a nominal 15-LED diffused case strip.
Three side fans are physically linked; hidden cabling prevents direct inspection
of all branches. These tests characterize visible response, not the hidden circuit.

## Approved tests and observations

| Pattern | Transmitted positions (one-based) | Owner observation |
| --- | --- | --- |
| 1 | 1 red; 7 green; 13 blue; 25 dim white | All four fans show the same red zone. Strip shows red/green/blue. No unambiguous white marker identified. |
| 2 | 7 green; all others black | All four fans dark; one green region on strip. |
| 3 | 6 blue; all others black | All four fans show the same blue zone; one blue region on strip. |
| 4 | 16 green; all others black | Owner reports one green region at the very end of the strip. |
| 5 | 17 blue; all others black | Owner reports everything dark. |

Every color packet was a 727-byte 0x51 report for area 0x0401, containing a
120-position frame. Non-marker positions were black. Each successful pattern
was sent once, with a full-length HID return value and subsequent owner observation.
Red/green/blue markers used channel value 64; pattern-1 white used 24 per channel.

## Supported interpretation

The fully sequential assumption (fan blocks 1..6, 7..12, 13..18, 19..24,
strip 25..39) does not match the response. The fans share at least positions 1
and 6; position 7 reaches the strip but none of the fans. With the owner's
six-zone description, this supports a shared 1..6 prefix for all four fans.
The subsequent continuous position-1..6 sweep confirmed synchronized changes
across all four fans. The owner describes each fan zone as two LEDs mirrored
about the vertical center axis. These observations establish
mirrored output behavior; a passive splitter, internal pass-through or controller
mirroring cannot be distinguished from these tests alone.

The strip shares the tested prefix and responds beyond it (positions 7, 13 and
16). Position 16 produces visible output at its end; position 17 produces no
visible output. The observed usable strip boundary is 16 in the tested mode,
not the nominal 15. This does not physically count LED packages or rule out an
unlit/nonfunctional segment. Intermediate positions were not all tested singly.

The practical routing model (one-based positions) is:

| Visible component | Shared output positions |
| --- | --- |
| Side fan, top | 1..6 |
| Side fan, middle | 1..6 |
| Side fan, bottom | 1..6 |
| Bottom fan | 1..6 |
| Case strip | 1..16 |

Thus there are 16 output color slots, with 1..6 shared by all five component
representations and 7..16 used by the strip only. Four six-zone fans plus the
16-position strip represent 40 visible zones but not 40 independent output slots.

## Software implications

Current sampling concatenates component colors. Four 6-position components and
one 15-position component therefore produce 39 independent sequential entries.
Both the serial concatenation and the nominal 15-position strip are unsuitable for the measured response. A future topology-aware model
needs explicit shared positions and a visible source/conflict policy for effects
sampled at different component locations. Merely displaying four fans does not
create independent electrical addresses. No production code or saved layout has
been changed by this test session.

## Sleep/resume event

After the owner's computer slept, JARGB2's read status changed from Advanced (9)
to mode 0. The first pattern-2 attempt aborted before any write. At inspection,
SignalRGB and Mystic Light services were stopped; MSI_Case_Service was running.
No causal assignment to that service or firmware is established.

With separate owner approval, a 290-byte 0x50 status-derived report changed only
byte 17 from 0 to 9, retaining Store=0 and all other bytes. Readback reported 9;
pattern 2 then transferred successfully. No services were stopped or reconfigured,
and no flash-store command was issued. Local detailed packet/log/photo evidence
is under build/aurora/hardware-probe and is not included with this document.

## Pattern 6 - sequential fan-prefix qualification

Owner approved two sweeps of positions 1..6, one red (64,0,0) position
per frame, held for three seconds. All other positions were black. The final
frame was all black. All 13 reports returned 727 bytes. Initial observation
confirmed red changes but did not resolve every zone. The owner subsequently
requested continuous operation until feedback. No mode change was performed.
The initial sandbox attempt failed at process enumeration before opening HID;
the authorized run outside the sandbox completed with the same safeguards.

## Follow-up observations - 2026-11-14

The continuous three-second sweep of positions 1..6 was observed by the owner:
all four fans changed synchronously through their zones; each zone consists
of two LEDs mirrored about the vertical fan center axis. Red also advanced
through the initial portion of the strip, not its full length. The sweep was
stopped before the subsequent static boundary pattern.

The owner disputed the earlier interpretation of the position-16 test, so the
strip endpoint was qualified again with explicit simultaneous colors:
13 red, 14 green, 15 blue, 16 yellow, 17 magenta; all others black. A subsequent
owner-requested continuous blink alternated positions 16 yellow and 17 magenta
with an all-black frame, one second per phase. The owner explicitly confirmed
yellow as the last visible LED. This confirms visible response at position 16
and no visible magenta response at position 17 in this test. It does not count
physical LED packages. Positions 7..15 were not all individually qualified.

At this handoff the blink test is still running, until owner instruction.
Local process session: 99522; script: apply-blink16-17.py; bounded recent
transfer log: blink16-17-result.json under build/aurora/hardware-probe.
Do not start another hardware writer before stopping this test.

SignalRGB was explicitly authorized to be stopped for these follow-up tests.
An initial service-stop attempt failed due to permissions; later inspection
confirmed the service stopped. SignalRGB GUI/launcher processes reappeared
and blocked one test before HID writes. On a later authorized retry, remaining
SignalRGB processes were stopped and the test passed its process guard.
No service startup configuration or scheduled task configuration was changed.

Required software work remains unimplemented: explicit shared address ranges
per component, a 16-position strip instance, a defined source/conflict policy
for overlapping addresses, matching preview/help, backward-compatible saved
layout handling, and tests for bounds, overlap and unchanged serial layouts.
The current personal layout and production code remain unchanged.
