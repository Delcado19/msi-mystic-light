# Branding assets

Current GS-06-R2 assets are supplied by the owner and embedded without pixel edits.

| Asset | Source | Dimensions | SHA256 |
| --- | --- | --- | --- |
| lighting-studio-header.png | Lightning Studio Turbinenlogo.png | 2172x724 RGBA | cc342c47cca3583e334a324ccbcf2e4aceb6068e290fa4b2126b0586ec15df40 |
| lighting-studio-background.png | background.png | 3840x2160 RGBA | e0376d6d3b3ad55cee47107d63a71c26a2b6f7d484617305132a4c265aa6675f |

The logo is fitted proportionally to 360x120 DIP at the right of the branding
row; its supplied transparency and full bounds are preserved. The photo covers
the window with centered proportional cropping and a dark overlay. Scenery may
crop as the aspect ratio changes; the separate logo does not.

branding.qrc also retains the earlier conceptual banners for About/history.
They are not the window background or the command-row logo. dropdown-arrow.svg
is a code-native UI chevron, not a replacement brand asset. Its QPixmap decoding
requires the shipped Qt SVG image-format plugin.

See docs/HEADER_MASK_CLARITY.md for native captures and validation boundaries.
