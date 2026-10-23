# SignalRGB repository research

Checked on 2026-10-23 for the standalone app's extension architecture.
This is source/documentation research, not an installation or runtime test.
Third-party source snapshots are only in ignored `build/research/`; no vendor
implementation or component library has been imported into project code.

## Result

SignalRGB separates data-only component descriptions, executable JavaScript
device plugins, and installable repository add-ons. A repository add-on can
carry plugins, QML UI, and component JSON. Native DLLs are not needed for
each new fan/strip definition or each supported USB device in its ecosystem.
The reviewed public projects expose payloads and installation entry points;
they do not expose the app's complete repository downloader/installer.

## Inventory and pinned revisions

The public group API returned 16 projects with `include_subgroups=true`;
the public subgroups endpoint returned no subgroups. All available default
branch root trees and README files were inspected. Selected JS/QML payloads,
component examples, types, validators, CI and UI translation sources were
inspected more deeply. A repository root inspection is not a full code audit.

| Project | Inspected revision | Role / relevant limitation |
|---|---|---|
| [signal-plugins](https://gitlab.com/signalrgb/signal-plugins) | master `95031635526054e267124fb32cf04509375f34ea`; Development `2343f7f7c305d592132a380a7ecc6821154a26c2`; beta `9511979fa21a0a00a35a236d0aac8c438179094f` | Main component/plugin repository, types, validators and CI |
| [hotfix](https://gitlab.com/signalrgb/hotfix) | `40f69ec4ff9b463e881e2148bac52b74b5985c16` | Repository-installed fixes between app patches; contains both `.js` and `.json` |
| [msi](https://gitlab.com/signalrgb/msi) | `179777bb401bb0ae3f1b44be26068f13c9992544` | JS/QML bridge through LEDKeeper2/Mystic Light; not a standalone direct-HID replacement |
| [lianli](https://gitlab.com/signalrgb/lianli) | `d792e9f1a55a6b5af814297cad69d03f7d8c165d` | Colordrop bridge through L-Connect 3; README says no detection |
| [Govee](https://gitlab.com/signalrgb/Govee) | `439d7bf69459867d3d4a8da26e1716b6038e6bdc` | LAN device discovery/control with JS/QML |
| [Twinkly](https://gitlab.com/signalrgb/Twinkly) | `e1b3e5d527a6a2983277ae1350d4074fc3073d7b` | Network plugin plus `Components/` JSON files |
| [yeelight](https://gitlab.com/signalrgb/yeelight) | `948d1eda0a19847b9e6dd70f3f304a19c1138cd1` | UDP plugin plus `Components/`; README explicitly says cube topology/count cannot currently be fetched |
| [nanoleaf](https://gitlab.com/signalrgb/nanoleaf) | `f4b8ee878421189ae047da22b23da4af432f97e6` | Network add-on; README notes detection limitations |
| [philipshue](https://gitlab.com/signalrgb/philipshue) | `24b08efada5902aff205df5dc9d739c934de6a7b` | Hue Bridge integration, not Bluetooth |
| [cololight](https://gitlab.com/signalrgb/cololight) | `330f0d03c062e1e791828f56c0041e5e9e6937c0` | Network add-on with product/count limitations |
| [magichome](https://gitlab.com/signalrgb/magichome) | `72d518203fde8bd260cc9345e9bd64df7f482fd7` | Network add-on; README marks it unfinished |
| [LeetDesk](https://gitlab.com/signalrgb/LeetDesk) | `cd08acd825c36b305515dcd34bff452045b90403` | Wi-Fi integration with JS/QML |
| [signal-translations](https://gitlab.com/signalrgb/signal-translations) | `ad19bffb71ff84eeb0af6084d0f311433caa12c9` | Qt translation sources reveal add-on settings UI, not downloader implementation |
| [qmk-firmware](https://gitlab.com/signalrgb/qmk-firmware) | `d2ff967becf7a9afb733e036b001ee1549540870` | README redirects to SRGBmods/QMK on GitHub |
| [sonix-qmk-firmware](https://gitlab.com/signalrgb/sonix-qmk-firmware) | `e4bfbd6bee767f454b44b2990bec089b340a8dac` | README redirects to SRGBmods/SonixQMK on GitHub |
| [streamdeck](https://gitlab.com/signalrgb/streamdeck) | no readable default-branch commit | Project listed publicly; `main` commit lookup returned HTTP 404 |

Branch selection matters: master's inspected head dates to 2023-08-30,
Development to 2025-09-06, and beta to 2026-01-26. Beta was the newest head
among the branch listings returned during this check. These are repository
commit dates, not proof of which code a running SignalRGB release uses.

## Components are data, not hardware drivers

The inspected Development tree contains 200 component JSON files. Beta has
218. Their common fields are `ProductName`, `DisplayName`, `Brand`, `Type`,
`LedCount`, `Width`, `Height`, `LedMapping`, `LedCoordinates`, and `LedNames`.
Beta has 216 `ImageUrl` fields and two embedded `Image` fields. The inspected
Yeelight and Twinkly component samples use the same basic layout format.

The mapping separates logical canvas positions from physical LED order.
Controller plugins register channels and consume their assembled color
buffers. They need not know which fan brands are attached to a header.
See the official [component model](https://docs.signalrgb.com/developer/plugins/components/),
[schema](https://docs.signalrgb.com/developer/plugins/component-structure/),
and [channel API](https://docs.signalrgb.com/developer/plugins/using-components/).

The documented lookup layers are app-bundled components and persistent
user-defined components. User files override bundled files with matching
`ProductName`. Product name is thus also a persistence key, not just a label.
Our own catalog should use stable IDs plus source/version metadata so a
display-name change cannot silently detach a saved component selection.

The `.cursor/rules/component-standards.mdc` guidance in the inspected tree
describes a different lowercase/3D shape from the real files and documented
schema. It is not the source of truth for an importer. Actual files, the
official schema and consumers take precedence.

## Repository installation and update evidence

The [hotfix README](https://gitlab.com/signalrgb/hotfix/-/blob/40f69ec4ff9b463e881e2148bac52b74b5985c16/README.md)
contains an install action targeting `signalrgb://addon/install` with the
GitLab repository URL as its `url` argument. The payload includes several
controller plugins and standalone Arctic/Montech component definitions.
This is concrete evidence that repository add-ons are not restricted to
compiled code or to full app releases. Other add-on READMEs use the same
installation entry point.

The [translation source](https://gitlab.com/signalrgb/signal-translations/-/blob/ad19bffb71ff84eeb0af6084d0f311433caa12c9/Translation-Files/SignalRgb_en.ts)
has an `AddonSettings` context with a clone/repository URL input, a trust
confirmation, duplicate/unsafe URL errors, branch selection, latest change,
startup enable/disable, source viewing, deletion, and restart prompts.
These strings establish intended UI controls; they do not establish the
implementation of URL validation, update scheduling, signature checks,
transactional installation or rollback. Those internals remain unverified.

The plugin repository's CI separately publishes plugin and component
artifacts. The beta configuration also has a TypeScript build artifact job.
Plugin/component validation jobs permit failure and publish artifacts with
`when: always`. This is source-level CI configuration evidence, not an
assessment of a live pipeline run or of a signed production release feed.

The official docs describe [bundled/user plugin precedence](https://docs.signalrgb.com/developer/plugins/how-is-a-plugin-loaded-/)
and [plugin rollback](https://docs.signalrgb.com/troubleshooting/advanced-troubleshooting/plugin-rollback/).
Local plugin overrides are identified by VID/PID. Component overrides are
identified by product name. Neither should be confused with automatic
physical product detection.

## Device plugins and gaps in our current contract

The official [plugin documentation](https://docs.signalrgb.com/developer/plugins/)
and repository types describe JavaScript exports for VID/PID, endpoint
validation, device layout, user parameters, competing process names, and
Initialize/Render/Shutdown. `Validate` may select multiple endpoints; plugins
can switch between already-open endpoints. Conflicts delay initialization
until resolved or bypassed by the user.

Network add-ons additionally export discovery services and use host-provided
network modules. The [module documentation](https://docs.signalrgb.com/developer/plugins/modules-and-features/)
also describes system-information and permission services. Their JS files
depend on the SignalRGB host API; they are not directly runnable in our
Qt native-plugin loader.

Our current interface handles one host-owned feature-report transport. Before
declaring a stable public SDK, add explicit startup/shutdown semantics and
declarative conflict requirements, then design grouped endpoints and required
I/O operations against concrete devices. Do not force network integrations
through `IHidDevice` or assume feature reports cover normal HID writes/reads.
Keep speculative APIs out until the next supported device requires them.

For easy community contributions, data-only catalog updates should be the
first download path. A constrained script-driver host is a separate option
for future protocols; native Qt plugins can remain available for cases that
need native implementation. Selecting a JS runtime and compatibility model
is a future design decision, not implemented by this research.

## Exact MSI B850 findings

The relevant direct-HID file is
[`Plugins/Msi/185 Byte Motherboard Controller/MSI_185Byte_Controller_0x0DB0.js`](https://gitlab.com/signalrgb/signal-plugins/-/blob/9511979fa21a0a00a35a236d0aac8c438179094f/Plugins/Msi/185%20Byte%20Motherboard%20Controller/MSI_185Byte_Controller_0x0DB0.js).
Do not substitute the separate `signalrgb/msi` bridge add-on for it.

- The plugin uses system motherboard/BIOS information and a board-model
  table. Missing motherboard identity or an unmapped model stops that path.
- The B850 Edge TI entry lists three ARGB headers, EZ Connector, an IO Shield
  entry with 11 LEDs, and JRGB1 with one LED. Header capacities are table-driven.
- `Validate` admits interfaces 0 or 2; this is a source selector to investigate,
  not a live verification of our board's current descriptor/interface layout.
- The new-controller setup reads `0x50` zone state, then uses the explicit
  profile table to construct the topology. It does not determine fan models
  or actual attached LED counts from that readback.
- `readDeviceZones` has an internal contradiction: it notes the read does not
  yield LED counts, but logs the last record byte as a count. Our vendor-based
  protocol notes identify that byte as `Cycle_Num`. The log label is not
  sufficient evidence of LED detection.
- Older-controller code in this same file contains Gen2-related helpers,
  including a port strip-count request and a feature-report LED-count read.
  `detectGen2Devices` has a definition but no call in this file. The reviewed
  new-controller branch does not invoke it for B850. Existence of the helper
  is a research lead, not qualification of those packets for this board.

No command was sent to hardware. Actual B850 attached-topology discovery
remains Unknown. Research should next trace the exact vendor controller's
request/readback implementation and saved configuration before proposing a
hardware probe. Maintain the existing explicit confirmation gate for any
new/unqualified command.

## Import validation and provenance

All 200 Development component files parsed and had per-LED array lengths
matching `LedCount` in a lightweight local check. In beta, one of 218 files,
`Components/Jonsbo/JONSBO_ZB240_DUAL.json`, declares 52 LEDs but has 53 mapping
entries and 53 coordinates with 52 names; its mapping also includes address
52. This check is narrower than full schema or hardware validation.

Consequently our importer must validate counts, permutation/address ranges,
coordinate bounds, duplicate IDs and size limits, and preserve the last valid
installed version if a candidate fails. Repo presence or upstream CI alone
is not a guarantee of usable component data. Do not silently truncate an
invalid mapping or invent missing LEDs.

Public readable source is not evidence of a redistribution license. The
inspected plugin branch trees had no LICENSE/COPYING path, and GitLab's
project license metadata was null. This is a recorded search result, not a
legal determination. Keep our implementation independent and establish
permission/provenance before bundling external definitions, images or code.

## Recommended next implementation increment

1. Implement the Component/Channel model with a documented data-only schema
   and an explicit import boundary for external component formats.
2. Add an app-owned versioned catalog for components and board profiles:
   stable IDs, source revision, compatible schema, package digest, origin
   authentication and license/provenance metadata. Keep downloads as data.
3. Use bundled, managed-downloaded and user-override layers, with visible
   origins and preserved user edits. Install candidates in staging, validate,
   activate atomically and retain the previous valid version.
4. Extend the driver lifecycle/conflict contract before adding another real
   controller. Evaluate script drivers separately; no direct SignalRGB JS
   compatibility is implied.
5. Continue the exact-board LED detection investigation independently of
   the repository installer. A larger component library does not establish
   what is physically connected.

No production code was changed, no vendor add-on was installed or executed,
and no commit/push was performed as part of this research.
