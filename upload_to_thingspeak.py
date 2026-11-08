"""
Send scenario readings to ThingSpeak using the SAME request format as the ESP32 sketch.

The class for each reading is computed by the exported decision tree (aq_model.h),
so the values match what the ESP32-S3 would compute on the device.

IMPORTANT: this runs on your laptop. It emulates the device's upload step; it is NOT
the ESP32 itself. Say so clearly in your report.

Usage (from the project folder, with the .venv active):
    python upload_to_thingspeak.py --dry-run     # prints what it would send
    python upload_to_thingspeak.py               # asks for your Write API Key and uploads

The key is typed at a hidden prompt, so it is never saved in a file or in Git.
"""
import argparse
import getpass
import re
import sys
import time
import urllib.error
import urllib.parse
import urllib.request
from pathlib import Path

# name, CO, NO2, SO2, O3, number of uploads to send for this scenario
SCENARIOS = [
    ("Clean air",     150, 10,  1,  60, 2),
    ("Moderate",      300, 25, 12,  50, 2),
    ("Moderate (2)",  250, 20,  9,  90, 2),
    ("Polluted",     2000, 55, 70, 110, 2),
]
CLASS_NAMES = ["Good", "Moderate", "Poor"]
INTERVAL_S = 20          # free ThingSpeak accounts allow 1 update per 15 s


def load_tree(header_path):
    """Translate the generated C if/else tree into a Python function."""
    out = []
    for line in Path(header_path).read_text().splitlines():
        s = line.strip()
        if not s or s.startswith("//") or s.startswith("#pragma"):
            continue
        sp = " " * (len(line) - len(line.lstrip()))
        if s.startswith("static inline"):
            out.append("def aq_predict(co, no2, so2, o3):")
        elif s == "}":
            continue
        elif s == "} else {":
            out.append(sp + "else:")
        elif m := re.match(r"if \((\w+) <= ([\d.eE+-]+)f\) \{", s):
            out.append(f"{sp}if {m.group(1)} <= {m.group(2)}:")
        elif m := re.match(r"return (\d);", s):
            out.append(f"{sp}return {m.group(1)}")
        else:
            raise ValueError("unexpected line in header: " + line)
    ns = {}
    exec("\n".join(out), ns)
    return ns["aq_predict"]


def find_header():
    for p in ("aq_model.h", "firmware/aq_model.h"):
        if Path(p).exists():
            return p
    sys.exit("Could not find aq_model.h in this folder or in firmware/.")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--dry-run", action="store_true", help="print the requests without sending")
    args = ap.parse_args()

    predict = load_tree(find_header())
    key = "DRYRUN" if args.dry_run else getpass.getpass("Write API Key (hidden): ")
    if not args.dry_run:
        key = "".join(ch for ch in key if ch.isalnum())     # drop spaces / hidden characters
        print(f"Key length: {len(key)} (a ThingSpeak Write API Key has 16 characters)")
        if len(key) != 16:
            sys.exit("The key was not pasted correctly. Right-click the terminal to paste, then try again.")

    first = True
    for name, co, no2, so2, o3, repeats in SCENARIOS:
        cls = predict(co, no2, so2, o3)
        for _ in range(repeats):
            if not first and not args.dry_run:
                time.sleep(INTERVAL_S)
            first = False
            query = urllib.parse.urlencode({
                "api_key": key, "field1": f"{co:.0f}", "field2": f"{no2:.1f}",
                "field3": f"{so2:.1f}", "field4": f"{o3:.1f}", "field5": cls})
            url = "https://api.thingspeak.com/update?" + query
            label = f"{name:13s} CO={co} NO2={no2} SO2={so2} O3={o3} -> {CLASS_NAMES[cls]}"
            if args.dry_run:
                print(label, "|", url.replace(key, "<key>"))
                continue
            try:
                with urllib.request.urlopen(url, timeout=20) as r:
                    print(label, "| HTTP", r.status, "entry id", r.read().decode().strip())
            except urllib.error.HTTPError as e:
                body = e.read().decode(errors="replace").strip()[:60]
                print(label, "| HTTP", e.code, "response:", repr(body), "(0 = key not accepted)")
            except Exception as e:
                print(label, "| FAILED:", e)


if __name__ == "__main__":
    main()