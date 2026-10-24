"""Promote visually reviewed native Windows test screenshots, never offscreen renders."""
import json
import shutil
from pathlib import Path

IMAGES = {
    "studio-workspace.png": "studio-workspace.png",
    "hardware-picker.png": "hardware-picker.png",
    "header-layout.png": "header-layout.png",
    "restored-hardware.png": "hardware-workspace.png",
    "editor-motion.png": "lighting-properties.png",
}


def promote(source: Path, destination: Path) -> None:
    metadata = json.loads((source / "rendering.json").read_text())
    if metadata.get("platform") != "windows" or metadata.get("latinGlyphsAvailable") is not True:
        raise ValueError("Documentation screenshots require native Windows rendering and available text glyphs")
    # Validate the complete set before replacing any existing documentation image.
    for filename in IMAGES:
        if not (source / filename).is_file():
            raise FileNotFoundError(source / filename)
    destination.mkdir(parents=True, exist_ok=True)
    for filename, target in IMAGES.items():
        shutil.copyfile(source / filename, destination / target)


if __name__ == "__main__":
    root = Path(__file__).resolve().parents[1]
    promote(root / "build/next/editor-artifacts/windows", root / "docs/images")
    print("Native documentation screenshots updated; inspect images before publishing.")
