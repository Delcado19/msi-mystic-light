import importlib.util
import json
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SPEC = importlib.util.spec_from_file_location("screenshots", ROOT / "scripts/update_doc_screenshots.py")
screenshots = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(screenshots)


class ScreenshotPromotionTest(unittest.TestCase):
    def test_rejected_render_never_replaces_existing_image(self):
        for metadata in ({"platform": "offscreen", "latinGlyphsAvailable": True},
                         {"platform": "windows", "latinGlyphsAvailable": False},
                         {"platform": "windows", "latinGlyphsAvailable": "true"}, {}):
            with self.subTest(metadata=metadata), tempfile.TemporaryDirectory() as folder:
                source = Path(folder) / "source"
                destination = Path(folder) / "docs"
                source.mkdir()
                destination.mkdir()
                (source / "rendering.json").write_text(json.dumps(metadata))
                image = destination / "studio-workspace.png"
                image.write_bytes(b"previous approved image")
                with self.assertRaises(ValueError):
                    screenshots.promote(source, destination)
                self.assertEqual(image.read_bytes(), b"previous approved image")

    def test_incomplete_native_set_leaves_docs_untouched(self):
        with tempfile.TemporaryDirectory() as folder:
            source = Path(folder) / "source"
            destination = Path(folder) / "docs"
            source.mkdir()
            (source / "rendering.json").write_text(json.dumps({"platform": "windows", "latinGlyphsAvailable": True}))
            (source / "studio-workspace.png").write_bytes(b"new screenshot")
            with self.assertRaises(FileNotFoundError):
                screenshots.promote(source, destination)
            self.assertFalse(destination.exists())

    def test_native_set_copies_exact_bytes(self):
        with tempfile.TemporaryDirectory() as folder:
            source = Path(folder) / "source"
            destination = Path(folder) / "docs"
            source.mkdir()
            (source / "rendering.json").write_text(json.dumps({"platform": "windows", "latinGlyphsAvailable": True}))
            for filename in screenshots.IMAGES:
                (source / filename).write_bytes(filename.encode())
            screenshots.promote(source, destination)
            self.assertEqual((destination / "studio-workspace.png").read_bytes(), b"studio-workspace.png")
            self.assertEqual((destination / "hardware-workspace.png").read_bytes(), b"restored-hardware.png")


if __name__ == "__main__":
    unittest.main()
