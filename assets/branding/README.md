# Branding assets

Current GS-06-R2 assets are supplied by the owner and embedded without pixel edits.

| Asset | Source | Dimensions | SHA256 |
| --- | --- | --- | --- |
| lighting-studio-header.png | Lighting-Studio-Logo.png (2026-11-15, corrected "Lighting" spelling) | 2172x724 RGBA | 8bd21d1934937c7ed6972c5e12c28e9a843c6ecf84e3ab822c81b647b2df64b4 |
| lighting-studio-background.png | background.png | 3840x2160 RGBA | e0376d6d3b3ad55cee47107d63a71c26a2b6f7d484617305132a4c265aa6675f |
| lighting-studio.ico | Lighting-Studio-Icon.png (2026-11-15), padded to square, 16-256 px | ICO, 9 sizes | 9768f65774e42ca08743c239497970606c7289b548d5123003a1f08503d8b2f6 |

The logo is fitted proportionally to 360x120 DIP at the right of the branding
row; its supplied transparency and full bounds are preserved. The photo covers
the window with centered proportional cropping and a dark overlay. Scenery may
crop as the aspect ratio changes; the separate logo does not.

branding.qrc also retains the earlier conceptual banners for About/history.
They are not the window background or the command-row logo. dropdown-arrow.svg
is a code-native UI chevron, not a replacement brand asset. Its QPixmap decoding
requires the shipped Qt SVG image-format plugin.

See docs/HEADER_MASK_CLARITY.md for native captures and validation boundaries.

lighting-studio.ico is the executable icon (src/app.rc). Explorer, the taskbar and the
installer's Start menu shortcut read it from the executable. It was generated from the supplied
PNG with Pillow (transparent padding to square, then sizes 16, 20, 24, 32, 40, 48, 64, 128, 256);
the supplied single-entry .ico held the same pixels but only one mislabeled 398x388 entry.
