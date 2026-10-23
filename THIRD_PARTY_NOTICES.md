# Third-party runtime components

Qt is dynamically linked and remains replaceable. The WebP image format plugin
is unmodified Qt ImageFormats v6.8.3 (commit
3b6b1fb96041f1bc85a8f42593bef2ee7bcdca6c), built with its bundled libwebp.
Qt plugin copyright: Copyright (C) 2016, 2019 The Qt Company Ltd.
Qt code is used under LGPL-3.0-only; license texts and libwebp copyright/patent
notices are included in third_party/licenses and copied beside the application.

Corresponding Qt ImageFormats source (including plugin and bundled libwebp):
https://code.qt.io/cgit/qt/qtimageformats.git/tree/?id=3b6b1fb96041f1bc85a8f42593bef2ee7bcdca6c

Build/replacement instructions: tools/build-webp.ps1 and README.md. The image
plugin can be replaced in the application's imageformats directory; the
application does not enforce signatures or checksums on replacement plugins.
Qt Core/Gui/Widgets, JPEG/GIF and the Windows platform plugin are supplied by
the user's matching dynamically linked Qt SDK, under Qt's respective licenses.
HIDAPI is separately supplied by the configured HIDAPI installation; see README.
Research references do not add runtime code or change these licenses.

Optional component metadata is read from a user's existing local VortxEngine
installation for compatibility. Only validated selected geometry and LED mapping
are embedded in the user's project. No SignalRGB application code, scripts,
effects, images or binaries are distributed or executed by this integration.
Product names identify compatible hardware; no affiliation is implied.
