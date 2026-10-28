"""Create a portable ZIP from explicit runtime inputs, excluding local user projects."""
import argparse
import hashlib
from pathlib import Path
import zipfile

RUNTIME = (
    "msi_mystic_light.exe", "lighting_service_host.exe", "Qt6Network.dll", "hidapi.dll", "Qt6Core.dll", "Qt6Gui.dll", "Qt6Widgets.dll", "Qt6Svg.dll",
    "platforms/qwindows.dll", "imageformats/qgif.dll", "imageformats/qjpeg.dll",
    "imageformats/qwebp.dll", "modules/msi_gen1_module.dll",
    "modules/peripherals_module.dll", "modules/auxiliary_module.dll",
)


def package(root: Path, runtime: Path, archive: Path) -> str:
    files = {name: runtime / name for name in RUNTIME}
    for name in ("LICENSE", "PORTABLE_README.txt", "THIRD_PARTY_NOTICES.md",
                 "third_party/pawnio/SmbusPIIX4.bin", "third_party/pawnio/source-0.2.11.zip"):
        files[name] = root / name
    for name in ("Install.cmd", "Install-LightingStudio.ps1"):
        files["installer/" + name] = root / "installer" / name
    # Use source catalogs to avoid stale profiles and user files in a reused build folder.
    for folder, pattern in (("profiles", "*.json"), ("components", "*.json"),
                            ("third_party/licenses", "*"), ("third_party/pawnio", "*")):
        entries = [path for path in (root / folder).rglob(pattern) if path.is_file()]
        if not entries:
            raise ValueError(f"Missing release inputs: {folder}")
        files.update({path.relative_to(root).as_posix(): path for path in entries})
    for path in files.values():
        if not path.is_file():
            raise FileNotFoundError(path)
    archive.parent.mkdir(parents=True, exist_ok=True)
    # Build beside the final archive; failed writes must not replace the previous package.
    temporary = archive.with_suffix(".tmp")
    with zipfile.ZipFile(temporary, "w", zipfile.ZIP_DEFLATED) as output:
        for name, path in sorted(files.items()):
            output.write(path, "LightingStudio/" + name)
    with zipfile.ZipFile(temporary) as output:
        if output.testzip() is not None:
            raise ValueError("Release archive failed CRC verification")
    temporary.replace(archive)
    digest = hashlib.sha256(archive.read_bytes()).hexdigest()
    archive.with_suffix(archive.suffix + ".sha256").write_text(f"{digest}  {archive.name}\n", encoding="ascii")
    return digest


if __name__ == "__main__":
    root = Path(__file__).resolve().parents[1]
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--runtime", type=Path, default=root / "build/next/Release")
    parser.add_argument("--archive", type=Path,
                        default=root / "build/dist/MSI-Mystic-Light-0.1.0-windows-x64.zip")
    args = parser.parse_args()
    print(package(root, args.runtime, args.archive), args.archive)
