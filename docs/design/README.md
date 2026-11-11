# Ice workspace (GS-06)

The owner supplied the ice-mountain/aurora panorama and requested its palette and
image in the native UI. Deep navy surfaces, ice-white text and cyan interaction
marks supersede Stealth red. The unchanged image now has a dedicated larger
responsive strip. Since the 2026-11-11 reliability correction, it fits the full
image without cropping or an overlay fade; unused space uses the native background.
Navigation below uses plain chrome; Help / About also shows the full image.
The compact LS wordmark, dock arrangement, system font and keyboard access remain.

## Native captures

- `../images/reliability-header.png`: current native startup with the complete header artwork.

The earlier captures below document the preceding GS-06 presentation.

- `../images/refined-editor.png`: wide editor with the larger artwork strip.
- `../images/refined-compact.png`: responsive compact editor.
- `../images/refined-legacy-toolbar.png`: restored legacy toolbar layout.
- `../images/refined-about.png`: full owner-supplied image inside About.
- `../images/refined-plasma.png`: Plasma spatial controls.
- `../images/refined-kaleidoscope.png`: Kaleidoscope controls.
- `../images/refined-dna.png`: independent DNA Wave width.

Captures show isolated test projects, not physical lighting output. Green Play /
red Stop remain semantic controls; the theme does not change rendered effect
colors, geometry, preview time or device output. The earlier Glacier concept and
Stealth captures remain as historical references, not the current palette.

## Direction and native adaptation

Applied design-taste-frontend app-screen direction `data-terminal`, seed
1783922600, with palette/type retained from the owner reference and Windows.
Question: what lighting composition is being edited, and how does it change?
Face: the owner's ice panorama. Signature: actual component/effect canvas and
aligned source/motion controls. No fabricated counters, sample customers or charts.
User instructions keep the existing native layout; the dealt web shell and chart
patterns do not replace Qt docks. Native image branding, navy surfaces, accessible
control borders and the existing functional effect gradients are intentional
adaptations to the explicit reference and a desktop graphics editor.

The HTML browser inspector cannot inspect Qt widgets. Native Windows captures,
pixel assertions, contrast measurements and Qt scaling checks provide the visual
checks instead; no browser compliance score is claimed.

## Native critique

Compared with the retained Stealth wide/compact captures: the component/effect
canvas remains the working area, while navy/cyan matches the supplied image.
The larger artwork strip trades 180 DIP vertically for visible branding, reducing
to 100 DIP in compact/short windows. Inspector and navigator retain their widths. Numeric fields remain
right-aligned and share one width. Selection/focus use cyan; green/red playback
and error states keep their meaning. No counters or repeated data were introduced.
Zero speed still freezes, negative rates reverse, and missing hardware stays an
offline state rather than a fictitious device. Native compact mode collapses the
inspector and keeps canvas access; phone/browser layout is outside this Qt app.

Measured sRGB contrast: primary text on panels 15.58:1; secondary text on panels
9.59:1; cyan focus on inputs 11.49:1; control borders on inputs 4.08:1. These are
color-pair measurements, not screen-reader or Windows contrast-theme certification.
The owner image is the only new picture; no external stock assets or sample data
need replacing. Source provenance is in `../../assets/branding/README.md`.

## GS-06 interaction and effect review

Gestures keep the current Play/Stop state; press-time geometry and live render time
are separate. Source tabs reopen at their primary controls, including particle
count/rate/size above palette details. Selection has one row marker rather than
repeated cyan cell seams. Plasma/Kaleidoscope add frequency, contrast and distortion;
DNA adds horizontal Wave width and Visor supports a wider band. The source canvas,
undo and saved numeric values remain functional rather than decorative examples.

Lightning/Fireworks received the explicitly authorized visual changes documented
in [the real-image comparison](../EFFECT_VISUAL_REVIEW.md). Their old visual
character is intentionally superseded; the theme itself still does not recolor
effect palettes. Native captures are checked after Qt's layout events settle.

Pre-artwork Qt window states can crash during the first show. Window-state
version 5 deliberately rebuilds those old dock/toolbar arrangements once while
retaining geometry, explicit panel visibility and projects. A sanitized recording
of the owner's failing state tests the complete AppSession startup path, visible
full-width artwork and first window exposure. Current version-5 layouts persist.
The drag test fits the camera before exercising particle
grips, and asserts that a real effect gesture began; transparent centers and
minimap/camera interactions do not count as effect drags.

Final native checks: 147/147 editor cases, 6/6 each at 1.5x and 2x, plus settled
capture checks and legacy-toolbar restoration. Both Release and Debug CTest pass
21/21. The qualified GS-06 startup-fix package and exact hashes are recorded in CONTINUE.md.
Physical lighting output and browser compliance are not claimed.
