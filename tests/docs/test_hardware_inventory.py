import importlib.util
import csv
import json
from pathlib import Path
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
SPEC = importlib.util.spec_from_file_location("inventory", ROOT / "scripts/inventory_hardware.py")
inventory = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(inventory)


class HardwareInventoryTest(unittest.TestCase):
    def test_recorded_inventory_is_consistent(self):
        folder = ROOT / "docs/hardware-inventory"
        data = json.loads((folder / "hardware-inventory.json").read_text(encoding="utf-8"))
        families = {row["family"]: row for row in data["families"]}
        self.assertEqual(len(families), len(data["families"]))
        self.assertEqual(len({row["source"] for row in data["plugins"]}), len(data["plugins"]))
        for revision in data["sources"].values():
            self.assertRegex(revision, r"^[0-9a-f]{40}$")
        for plugin in data["plugins"]:
            for family in plugin["openrgb_candidates"]:
                self.assertIsNotNone(plugin["usb_id"])
                self.assertIn(plugin["usb_id"], families[family]["usb_ids"])
            if plugin["usb_id"] is None:
                self.assertIn("VendorId/ProductId", plugin["unresolved_exports"])
        self.assertEqual(families["DebugController"]["priority"], "EXCLUDED")
        for filename, count in (
            ("openrgb-families.csv", len(families)),
            ("openrgb-registrations.csv", sum(len(row["registrations"]) for row in families.values())),
            ("signalrgb-plugins.csv", len(data["plugins"])),
            ("signalrgb-components.csv", len(data["components"])),
            ("comparison-candidates.csv", sum(len(row["openrgb_candidates"]) for row in data["plugins"])),
        ):
            with (folder / filename).open(encoding="utf-8", newline="") as file:
                self.assertEqual(len(list(csv.DictReader(file))), count, filename)

    def test_registration_arguments(self):
        source = '''// REGISTER_HID_DETECTOR("fake", fn, 1, 2);
REGISTER_HID_DETECTOR_IPU("Model, variant", fn, VID, PICK(1, 2), 0, 1, 2);
REGISTER_DETECTOR("Custom", fn);
'''
        entries = inventory.registrations(source)
        self.assertEqual(len(entries), 2)
        self.assertEqual(entries[0]["model"], "Model, variant")
        self.assertEqual(entries[0]["arguments"][3], "PICK(1, 2)")
        self.assertEqual(entries[1]["arguments"], ['"Custom"', "fn"])

    def test_unresolved_values_are_not_guessed(self):
        self.assertEqual(inventory.integer("0x1E7Du", {}), 0x1E7D)
        self.assertEqual(inventory.integer("DECIMAL", {"DECIMAL": {42}}), 42)
        self.assertIsNone(inventory.integer("AMBIGUOUS", {"AMBIGUOUS": {1, 2}}))
        self.assertIsNone(inventory.integer("PID + 1", {}))
        source = 'export function ProductId() { return getProductIds(); }'
        self.assertEqual(inventory.js_return(source, "ProductId"), "getProductIds()")
        self.assertIsNone(inventory.js_return(source, "VendorId"))

    def test_priorities_and_non_hardware(self):
        self.assertEqual(inventory.priority("RoccatController", [], [])[0], "P0")
        self.assertEqual(inventory.priority("Board", ["motherboard"], ["HID"])[0], "P1")
        self.assertEqual(inventory.priority("Board", [], ["SMBus/I2C"])[0], "P3")
        self.assertEqual(inventory.priority("DebugController", [], [])[0], "EXCLUDED")
        self.assertEqual(inventory.priority("RedragonController", [], [])[0], "P2")
        self.assertEqual(inventory.priority("AOCKeyboardController", ["keyboard"], ["HID"])[0], "P2")

    def test_export_retains_unresolved_registration(self):
        data = {"families": [{"family": "Example", "priority": "P2", "categories": [], "transports": [],
                "coverage": "not implemented", "models": [], "usb_ids": [], "reason": "review",
                "registrations": [{"source": "example.cpp", "macro": "REGISTER_DETECTOR", "model": None,
                                   "usb_id": None, "arguments": ["VARIABLE_NAME", "fn"]}]}],
                "plugins": [], "components": []}
        with tempfile.TemporaryDirectory() as folder:
            inventory.write(Path(folder), data)
            exported = (Path(folder) / "openrgb-registrations.csv").read_text()
            self.assertIn("VARIABLE_NAME", exported)
            self.assertIn("REGISTER_DETECTOR", exported)


if __name__ == "__main__":
    unittest.main()
