#!/usr/bin/env python3
"""Compare packed CE-E (hex) and CE-H (binary) pTTs without ROOT dependencies."""
import argparse
from pathlib import Path
import re
import sys

PATTERN = re.compile(r"TowerSums_Sector_([0-2])_Board_(\d+)_CE_([EH])_Event_(\d+)\.txt")


def index(directory):
    result = {}
    for path in Path(directory).glob("TowerSums_*.txt"):
        match = PATTERN.fullmatch(path.name)
        if not match or not 0 <= int(match[2]) < 14:
            raise ValueError("Unexpected tower output filename: " + path.name)
        result[path.name] = path
    if not result:
        raise ValueError("No tower files in " + str(directory))
    events = {PATTERN.fullmatch(name)[4] for name in result}
    for event in events:
        expected = {
            "TowerSums_Sector_{}_Board_{}_CE_{}_Event_{}.txt".format(s, b, region, event)
            for s in range(3) for b in range(14) for region in ("E", "H")
        }
        missing = expected - result.keys()
        if missing:
            raise ValueError("Missing output files: " + ", ".join(sorted(missing)[:5]))
    return result


def read(path):
    region = PATTERN.fullmatch(path.name)[3]
    values = [int(line, 16 if region == "E" else 2) for line in path.read_text().splitlines()]
    if len(values) != 480 or any(not 0 <= value < 256 for value in values):
        raise ValueError("Expected 480 packed 8-bit pTTs in " + str(path))
    return region, values


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("python_outputs")
    parser.add_argument("cpp_outputs")
    args = parser.parse_args()
    reference, cpp = index(args.python_outputs), index(args.cpp_outputs)
    if reference.keys() != cpp.keys():
        raise ValueError("Python and C++ output file sets differ")
    counts = {"E": [0, 0], "H": [0, 0]}
    examples = []
    for name in sorted(reference):
        region, expected = read(reference[name])
        _, actual = read(cpp[name])
        for tower, (before, after) in enumerate(zip(expected, actual)):
            counts[region][0] += 1
            if before != after:
                counts[region][1] += 1
                if len(examples) < 10:
                    examples.append((name, tower, before, after))
    for region in ("E", "H"):
        print("CE-{}: {} pTTs, {} differences".format(region, *counts[region]))
    for name, tower, before, after in examples:
        print("{}: pTT {}, Python={}, C++={}".format(name, tower, before, after))
    return int(any(result[1] for result in counts.values()))


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (ValueError, OSError) as error:
        print("Comparison failed:", error, file=sys.stderr)
        sys.exit(2)
