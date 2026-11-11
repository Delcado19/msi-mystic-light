"""Inventory pinned reference trees; candidate matches never imply protocol compatibility."""
import argparse
import csv
import io
import json
from pathlib import Path
import re
import subprocess
import tarfile

ROOT = Path(__file__).resolve().parents[1]


def git(repo, *args):
    return subprocess.run(["git", "-C", str(repo), *args], check=True, capture_output=True).stdout


def snapshot(repo, folder):
    revision = git(repo, "rev-parse", "HEAD").decode().strip()
    # Read Git objects, not a sparse working tree. Missing promisor objects must fail visibly.
    archive = git(repo, "archive", revision, folder)
    files = {}
    with tarfile.open(fileobj=io.BytesIO(archive)) as source:
        for item in source.getmembers():
            if item.isfile() and item.name.endswith((".cpp", ".h", ".js", ".json")):
                files[item.name] = source.extractfile(item).read().decode("utf-8-sig", errors="replace")
    return revision, files


def arguments(text):
    """Split macro arguments without splitting nested calls or quoted commas."""
    result, start, depth, quote, escaped = [], 1, 0, None, False
    for index, char in enumerate(text):
        if escaped:
            escaped = False
        elif quote:
            if char == "\\":
                escaped = True
            elif char == quote:
                quote = None
        elif char in "\"'":
            quote = char
        elif char == "(":
            depth += 1
        elif char == ")":
            depth -= 1
            if depth == 0:
                result.append(text[start:index].strip())
                return result
        elif char == "," and depth == 1:
            result.append(text[start:index].strip())
            start = index + 1
    raise ValueError("Unterminated registration macro")


def registrations(text):
    # Comments can contain example detectors; exclude them while retaining line numbers.
    clean = re.sub(r"/\*.*?\*/|//[^\n]*", lambda match: "\n" * match[0].count("\n"), text, flags=re.S)
    result = []
    for match in re.finditer(r"\b(REGISTER_[A-Z0-9_]*DETECTOR[A-Z0-9_]*)\s*\(", clean):
        args = arguments("(" + clean[match.end():])
        result.append({"macro": match[1], "arguments": args,
                       "model": args[0][1:-1] if args and re.fullmatch(r'"[^"\n]*"', args[0]) else None})
    return result


def integer(token, definitions):
    token = token.strip()
    if re.fullmatch(r"(?:0[xX][0-9a-fA-F]+|\d+)[uUlL]*", token):
        return int(re.sub(r"[uUlL]+$", "", token), 16 if token.lower().startswith("0x") else 10)
    values = definitions.get(token, set())
    return next(iter(values)) if len(values) == 1 else None


def js_return(text, name):
    match = re.search(r"export\s+function\s+" + name + r"\s*\([^)]*\)\s*\{\s*return\s+([^;\n}]+)", text)
    return match[1].strip() if match else None


def priority(name, categories, transports):
    if name == "DebugController":
        return "EXCLUDED", "Synthetic controller, not physical hardware"
    if re.search(r"Roccat|Razer|SteelSeries", name, re.I):
        return "P0", "Extend active peripheral families; compare vendor settings and both references"
    if "monitor" in categories or re.search(r"\bAGON\b|Monitor|LCD|Display", name, re.I):
        return "P1", "Display/lighting investigation, including owner AGON; exact model match required"
    if "motherboard" in categories and "HID" in transports:
        return "P1", "Next motherboard HID protocol comparison"
    if any(item in transports for item in ("SMBus/I2C", "port I/O", "EC/kernel")):
        return "P3", "Platform transport qualification before protocol port"
    return "P2", "Broader device coverage after active-device and motherboard priorities"


IMPLEMENTED = {
    "MSIMotherboardController": "msi.gen1: B850 profile only; other generations not covered",
    "ASRockPolychromeUSBController": "asrock.polychrome-usb: native zones only, hardware unverified",
    "LenovoMotherboardController": "lenovo.motherboard: native zones only, hardware unverified",
    "HPOmen30LController": "hp.omen30l: seven zones, hardware unverified",
    "RoccatController": "lighting.peripherals: selected Vulcan/Kone models only",
    "RazerController": "lighting.peripherals: Basilisk V3 only",
    "SteelSeriesController": "lighting.peripherals: Apex 3 only",
    "ENESMBusController": "lighting.auxiliary: qualified fixed-address XPG/AMD path only",
    "GigabyteRGBFusion2BlackwellGPUController": "lighting.auxiliary: Aero 5080 only",
}


def build(openrgb, signalrgb):
    orgb_revision, orgb_files = snapshot(openrgb, "Controllers")
    signal_revision, signal_files = snapshot(signalrgb, "Plugins")
    families = []
    folders = sorted({Path(path).parts[1] for path in orgb_files})
    for folder in folders:
        files = {path: value for path, value in orgb_files.items() if Path(path).parts[1] == folder}
        combined = "\n".join(files.values())
        definitions = {}
        for match in re.finditer(r"^\s*#define\s+(\w+)\s+(0[xX][0-9a-fA-F]+|\d+)\b", combined, re.M):
            definitions.setdefault(match[1], set()).add(integer(match[2], {}))
        detectors = []
        for path, content in files.items():
            for entry in registrations(content):
                entry["source"] = path
                args = entry["arguments"]
                entry["usb_id"] = None
                if entry["macro"].startswith("REGISTER_HID_") and len(args) >= 4:
                    vendor, product = integer(args[2], definitions), integer(args[3], definitions)
                    if vendor is not None and product is not None:
                        entry["usb_id"] = f"{vendor:04X}:{product:04X}"
                detectors.append(entry)
        categories = sorted(set(re.findall(r"\bDEVICE_TYPE_([A-Z_]+)", combined)))
        transports = []
        if any("HID" in entry["macro"] for entry in detectors) or re.search(r"\bhid_(?:write|send_feature_report|open)", combined):
            transports.append("HID")
        if any("I2C" in entry["macro"] for entry in detectors):
            transports.append("SMBus/I2C")
        if re.search(r"\b(?:outb|inb|InpOut|superio|super_io)\b", combined, re.I):
            transports.append("port I/O")
        if re.search(r"\bcros_ec\b", combined):
            transports.append("EC/kernel")
        if any("SERIAL" in entry["macro"] for entry in detectors):
            transports.append("serial")
        if re.search(r"\b(?:sendto|recvfrom|connect|SOCK_DGRAM|SOCK_STREAM)\s*\(", combined):
            transports.append("network candidate")
        rank, reason = priority(folder, [item.lower() for item in categories], transports)
        families.append({"family": folder, "categories": categories, "transports": transports or ["manual review"],
                         "priority": rank, "reason": reason, "coverage": IMPLEMENTED.get(folder, "not implemented"),
                         "sources": sorted(files), "registrations": detectors,
                         "motherboard_sources": sorted(path for path, text in files.items() if "DEVICE_TYPE_MOTHERBOARD" in text),
                         "models": sorted({entry["model"] for entry in detectors if entry["model"]}),
                         "usb_ids": sorted({entry["usb_id"] for entry in detectors if entry["usb_id"]})})
    plugins = []
    for path, text in sorted(signal_files.items()):
        if not path.endswith(".js"):
            continue
        vendor, product = js_return(text, "VendorId"), js_return(text, "ProductId")
        vendor = integer(vendor, {}) if vendor else None
        product = integer(product, {}) if product else None
        usb_id = f"{vendor:04X}:{product:04X}" if vendor is not None and product is not None else None
        name = js_return(text, "Name")
        name = name[1:-1] if name and re.fullmatch(r'''["'][^"']*["']''', name) else None
        matches = [item["family"] for item in families if usb_id and usb_id in item["usb_ids"]]
        rank, reason = priority(path, [], [])
        plugins.append({"source": path, "name": name, "usb_id": usb_id,
                        "transport_expression": js_return(text, "Type"),
                        "endpoint_validation_present": bool(re.search(r"export\s+function\s+Validate\b", text)),
                        "device_type_expression": js_return(text, "DeviceType"),
                        "subdevices_present": bool(re.search(r"export\s+function\s+Subdevices\b", text)),
                        "priority": rank, "reason": reason, "openrgb_candidates": matches,
                        "match_status": "USB identity candidate; exact model/interface/firmware comparison required" if matches else "unmatched or unresolved; manual review",
                        "unresolved_exports": [key for key, value in (("Name", name), ("VendorId/ProductId", usb_id)) if value is None]})
    component_revision, component_files = snapshot(signalrgb, "Components")
    if component_revision != signal_revision:
        raise ValueError("SignalRGB revision changed during inventory")
    components = []
    for path, text in sorted(component_files.items()):
        if not path.endswith(".json"):
            continue
        try:
            item = json.loads(text)
        except ValueError as error:
            raise ValueError(f"Malformed component {path}: {error}") from error
        components.append({"source": path, "product_name": item.get("ProductName"),
                           "brand": item.get("Brand"), "type": item.get("Type"), "led_count": item.get("LedCount"),
                           "status": "data conversion candidate; not a device driver"})
    return {"schemaVersion": 1, "sources": {"openrgb": orgb_revision, "signalrgb": signal_revision},
            "families": families, "plugins": plugins, "components": components}


def write(output, data):
    output.mkdir(parents=True, exist_ok=True)
    (output / "hardware-inventory.json").write_text(json.dumps(data, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    for name, fields, rows in (
        ("openrgb-families.csv", ["family", "priority", "categories", "transports", "coverage", "models", "usb_ids", "reason"], data["families"]),
        ("signalrgb-plugins.csv", ["source", "name", "usb_id", "priority", "device_type_expression", "transport_expression", "endpoint_validation_present", "subdevices_present", "openrgb_candidates", "match_status", "unresolved_exports"], data["plugins"]),
        ("signalrgb-components.csv", ["source", "product_name", "brand", "type", "led_count", "status"], data["components"]),
        ("openrgb-registrations.csv", ["family", "source", "macro", "model", "usb_id", "arguments"],
         [dict(row, family=family["family"]) for family in data["families"] for row in family["registrations"]]),
        ("comparison-candidates.csv", ["signalrgb_source", "signalrgb_name", "usb_id", "openrgb_family", "status"],
         [{"signalrgb_source": plugin["source"], "signalrgb_name": plugin["name"], "usb_id": plugin["usb_id"],
           "openrgb_family": family, "status": "USB pair only; exact model/interface/firmware and protocol comparison pending"}
          for plugin in data["plugins"] for family in plugin["openrgb_candidates"]]),
    ):
        with (output / name).open("w", newline="", encoding="utf-8") as file:
            writer = csv.DictWriter(file, fieldnames=fields, extrasaction="ignore")
            writer.writeheader()
            for row in rows:
                writer.writerow({key: " | ".join(row[key]) if isinstance(row[key], list) else row[key] for key in fields})
    print(json.dumps({"families": len(data["families"]), "plugin_files": len(data["plugins"]),
                      "component_files": len(data["components"]),
                      "model_registration_labels": sum(len(row["models"]) for row in data["families"]),
                      "resolved_signal_usb_ids": sum(bool(row["usb_id"]) for row in data["plugins"]),
                      "usb_candidate_plugins": sum(bool(row["openrgb_candidates"]) for row in data["plugins"]),
                      "unresolved_signal_exports": sum(bool(row["unresolved_exports"]) for row in data["plugins"])}, indent=2))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--openrgb", type=Path, default=ROOT / "build/research/openrgb")
    parser.add_argument("--signalrgb", type=Path, default=ROOT / "build/research/signalrgb-plugins")
    parser.add_argument("--output", type=Path, default=ROOT / "docs/hardware-inventory")
    args = parser.parse_args()
    write(args.output, build(args.openrgb, args.signalrgb))
