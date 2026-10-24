# Test review: recent lighting changes (2026-10-24)

Scope: the uncommitted component, RAM, layout/navigation and related output tests.
This is a source/test review, not a physical hardware qualification or an exhaustive
review of every old test in the repository.

## Independent expected results

- Replaced the expanding-layout test's self-derived sample coordinates with a
  fixed fixture: LED centers (-200,500), (-180,500), (-160,500), a 12-pixel margin,
  expected world (-212,0,852,512), source pixels (9,390), (24,390), (39,390), and
  expected wire colors [blue, green, red]. The draft case independently pins world
  (0,-112,952,512) and pixels (605,9), (618,9), (631,9).
- Channel samples are compared to the explicit color vector, not to the component
  sampler's output. A shared mistake cannot make both assertions pass unnoticed.
- Scroll tests now exercise native mouse dragging and verify that the scrollbars
  are enabled and actually receive pointer hits. Camera extremes must align with
  the independently specified world edges within one screen pixel.
- Added exact-fit, one-pixel overflow, horizontal-only, vertical-only and coupled
  scrollbar visibility cases. Neither a broad sign check nor thumb visibility is
  enough to establish usable navigation.
- RAM goldens use distinct LEDs and 0/255/mixed channel values with literal R-B-G
  blocks and register selections. Every one of the 20 writes is failed in turn;
  output must stop at the failed operation, for both qualified RAM addresses.
- Invalid RAM addresses and 0/1/7/9/65535-color payloads must issue zero transfers.
- Component checks cover 1 and 65535 entries, invalid counts/types/slot indices,
  NaN/infinity positions and the 1-MiB file limit (exactly allowed, one byte over
  rejected). A malformed single-position test fixture was corrected to contain a
  nested JSON array; its intended expected behavior was not relaxed.

## Defects exposed before production changes

1. Zero, negative and non-finite zoom factors changed or poisoned the view.
   Invalid factors now leave it unchanged. Positive zoom still clamps to .001..4.
2. Overflowing component transforms and non-finite draft positions were accepted.
   Shared placement-to-world validation now rejects them before document commit
   or sampling; rejected edits preserve document/history. Coordinate headroom
   protects world spans, marker margins and the supported 4x navigation zoom.
3. A scrollbar could be at its maximum while the camera was still beyond that
   endpoint. Setting the same value emits no signal, so the camera did not move.
   Idle camera positions now clamp to the actual scrollable extent before thumb
   synchronization. Clamping is deferred during gestures to preserve pointer
   tracking as the world grows.

Failure evidence before fixes:

- build/next/audit-red-before-fixes.log
- build/next/audit-red-native.log (10 failing input cases)
- build/next/audit-scroll-endpoint-red.log (camera endpoint mismatch)

The Debug run also exposed a dangling reference in a test: iterating a vector
obtained through operator[] on a temporary QMap outlived that map in C++17. The
sample vector is now stored before iteration; every assertion is retained. This
was a test lifetime bug, not evidence of a fullscreen product crash.

## Mutation checks

Two deliberately incorrect ENE implementations were compiled temporarily:
removing volatile Apply, and swapping green/blue encoding. The fixed golden tests
rejected both. Source bytes were restored and the normal target rebuilt afterward.
Evidence: build/next/audit-mutation-summary.json and per-mutation logs.
No mutated application was packaged or started.

## Previously adjusted expectations

The old gallery tests were removed because the user explicitly removed that UI;
renderer fixtures remain tested through the document API. Sampling's legacy
within-frame fixture was moved inside the new growing world while retaining its
wire-order/alpha assertions; independently pinned negative/outside cases now
cover the changed contract. IOShield is now a valid software channel, so the old
negative case targets native-only JRGB1; positive IOShield encoding/capacity checks
remain in the module/protocol tests. These changes follow requirements, not merely
current test outcomes.

## Limits

Passing these tests cannot prove the user's entire hardware setup or every mouse,
DPI and window-layout combination. Native Windows dragging, pointer targeting and
one-pixel extremes are exercised. The user's broader scrollbar report must not be
claimed fully reproduced solely because one endpoint defect was found.

## Final validation

Release and Debug CTest each pass 16/16. Native Windows editor: 58/58; output UI:
14/14. Both intentional RAM mutations were detected and restored. The corrected
portable folder and ZIP were rebuilt and checked for CRC and matching app/profile/
module bytes. No physical hardware output or SignPath application was performed.
