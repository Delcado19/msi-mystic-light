import importlib.util
from pathlib import Path
import tempfile
import unittest
import zipfile

ROOT = Path(__file__).resolve().parents[2]
SPEC = importlib.util.spec_from_file_location("release", ROOT / "scripts/package_release.py")
release = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(release)


class ReleasePackageTest(unittest.TestCase):
    def fixture(self, root):
        runtime = root / "runtime"
        for name in release.RUNTIME:
            path = runtime / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(name.encode())
        for name in ("LICENSE", "PORTABLE_README.txt", "THIRD_PARTY_NOTICES.md",
                     "profiles/example.json", "components/example.json",
                     "third_party/licenses/LICENSE", "third_party/pawnio/SmbusPIIX4.bin",
                     "third_party/pawnio/source-0.2.11.zip"):
            path = root / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(b"fixture")
        return runtime

    def test_complete_package_excludes_personal_and_stale_files(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            runtime = self.fixture(root)
            (runtime / "default.lighting.json").write_text("private project")
            (runtime / "profiles").mkdir()
            (runtime / "profiles/stale.json").write_text("stale")
            archive = root / "release.zip"
            digest = release.package(root, runtime, archive)
            with zipfile.ZipFile(archive) as output:
                self.assertIsNone(output.testzip())
                self.assertNotIn("LightingStudio/default.lighting.json", output.namelist())
                self.assertNotIn("LightingStudio/profiles/stale.json", output.namelist())
                self.assertEqual(output.read("LightingStudio/Qt6Svg.dll"), b"Qt6Svg.dll")
                self.assertEqual(output.read("LightingStudio/modules/auxiliary_module.dll"),
                                 b"modules/auxiliary_module.dll")
            self.assertEqual(archive.with_suffix(".zip.sha256").read_text(), f"{digest}  release.zip\n")

    def test_missing_runtime_preserves_previous_archive(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            runtime = self.fixture(root)
            (runtime / "hidapi.dll").unlink()
            archive = root / "release.zip"
            archive.write_bytes(b"previous package")
            with self.assertRaises(FileNotFoundError):
                release.package(root, runtime, archive)
            self.assertEqual(archive.read_bytes(), b"previous package")

    def test_empty_catalog_is_rejected_before_writing(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            runtime = self.fixture(root)
            (root / "components/example.json").unlink()
            archive = root / "release.zip"
            with self.assertRaises(ValueError):
                release.package(root, runtime, archive)
            self.assertFalse(archive.exists())

    def test_missing_pawnio_source_is_rejected(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            runtime = self.fixture(root)
            (root / "third_party/pawnio/source-0.2.11.zip").unlink()
            archive = root / "release.zip"
            with self.assertRaises(FileNotFoundError):
                release.package(root, runtime, archive)
            self.assertFalse(archive.exists())
