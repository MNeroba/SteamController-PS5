#!/usr/bin/env python3
"""Fail closed unless the first-release hardware matrix is evidenced."""
from pathlib import Path
import argparse
import re
import os
import subprocess
import sys


REQUIRED_GATES = ("H-01", "H-02", "H-03", "H-04", "H-05", "H-09", "H-12")
PLACEHOLDER = re.compile(r"NOT_TESTED|UNKNOWN|NOT_RUN|BLOCKED|TBD|N/?A|—", re.I)
COMMIT = re.compile(r"(?<![0-9a-f])[0-9a-f]{40}(?![0-9a-f])", re.I)
FIRMWARE_VERSION = re.compile(r"^\d+\.\d+(?:\.\d+)?$")
EVIDENCE = re.compile(r"^project-memory/TEST_LOG\.md#(h-\d{2})$", re.I)


def test_log_sections(text):
    lines = text.splitlines()
    sections = {}
    for index, line in enumerate(lines):
        match = re.match(r"^(#{1,6})\s+(H-\d{2})\s*$", line.strip(), re.I)
        if not match:
            continue
        level = len(match.group(1))
        end = len(lines)
        for next_index in range(index + 1, len(lines)):
            heading = re.match(r"^(#{1,6})\s+", lines[next_index].strip())
            if heading and len(heading.group(1)) <= level:
                end = next_index
                break
        key = match.group(2).lower()
        sections[key] = "\n".join(lines[index:end])
    return sections


PRODUCTION_PATHS = ("payload", "scripts", "tests", ".github/workflows")

def source_matches(tested, released, repository):
    """Only exact source or its descendant with unchanged build/test inputs."""
    if tested == released:
        return True
    def git(*args):
        return subprocess.run(["git", "-C", str(repository), *args],
                              stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    return (git("merge-base", "--is-ancestor", tested, released).returncode == 0 and
            git("diff", "--quiet", tested, released, "--", *PRODUCTION_PATHS).returncode == 0)

def validate_matrix(text, test_log_text=None, expected_sha=None, repository=None):
    rows = {}
    errors = []
    sections = test_log_sections(test_log_text or "")
    for line in text.splitlines():
        if not line.startswith("| H-"):
            continue
        columns = [part.strip() for part in line.strip().strip("|").split("|")]
        gate = columns[0] if columns else ""
        if gate in rows:
            errors.append(f"{gate}: duplicate hardware matrix row")
        rows[gate] = columns

    for gate in REQUIRED_GATES:
        columns = rows.get(gate)
        if columns is None:
            errors.append(f"{gate}: required hardware row is missing")
            continue
        if len(columns) != 9:
            errors.append(f"{gate}: expected 9 table columns, found {len(columns)}")
            continue
        if columns[7] != "VERIFIED_HARDWARE":
            errors.append(f"{gate}: result must be VERIFIED_HARDWARE")
        if not FIRMWARE_VERSION.fullmatch(columns[3]):
            errors.append(f"{gate}: firmware must be an exact numeric version such as 12.00")
        for index, label in ((2, "PS5 model"), (4, "controller model/firmware"),
                             (5, "transport"), (6, "payload commit / SDK"),
                             (8, "evidence")):
            if PLACEHOLDER.search(columns[index]):
                errors.append(f"{gate}: {label} contains a placeholder")
        if not COMMIT.search(columns[6]):
            errors.append(f"{gate}: payload commit / SDK must include a full commit SHA")
        if "SDK v0.43" not in columns[6]:
            errors.append(f"{gate}: pinned SDK v0.43 is required")
        evidence_match = EVIDENCE.fullmatch(columns[8])
        if not evidence_match:
            errors.append(f"{gate}: evidence must link to a test-log entry")
            continue
        section = sections.get(evidence_match.group(1).lower())
        if section is None:
            errors.append(f"{gate}: anchored hardware test entry is absent from TEST_LOG.md")
            continue
        if gate not in section:
            errors.append(f"{gate}: test-log entry does not identify the hardware gate")
        if "Result: VERIFIED_HARDWARE" not in section:
            errors.append(f"{gate}: test-log entry must record Result: VERIFIED_HARDWARE")
        commit_match = COMMIT.search(columns[6])
        firmware_entries = re.findall(r"^Firmware:\s*(.*?)\s*$", section, re.M)
        if firmware_entries != [columns[3]]:
            errors.append(f"{gate}: test-log entry does not record matching exact firmware")
        required_evidence = (
            (columns[2], "PS5 model"),
            (columns[4], "controller model/firmware"),
            (columns[5], "transport"),
        )
        if "SDK: v0.43" not in section.splitlines():
            errors.append(f"{gate}: test-log entry must record SDK: v0.43")
        if commit_match and expected_sha is not None:
            if not COMMIT.fullmatch(expected_sha) or not source_matches(commit_match.group(0), expected_sha, repository):
                errors.append(f"{gate}: hardware-tested source does not match released production source")
        if commit_match:
            required_evidence += ((commit_match.group(0), "payload commit SHA"),)
        for value, label in required_evidence:
            if value not in section:
                errors.append(f"{gate}: test-log entry does not record matching {label}")
    return errors


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--matrix",
        type=Path,
        default=Path(__file__).resolve().parents[1] / "project-memory/HARDWARE_MATRIX.md",
        help="hardware matrix path (used by the Actions release gate)",
    )
    parser.add_argument("--source-sha", default=os.environ.get("GITHUB_SHA"))
    args = parser.parse_args(argv)
    repository = Path(__file__).resolve().parents[1]
    expected_sha = args.source_sha
    if not expected_sha:
        result = subprocess.run(["git", "-C", str(repository), "rev-parse", "HEAD"], text=True, capture_output=True)
        if result.returncode:
            print("release gate: cannot determine release source SHA", file=sys.stderr); return 2
        expected_sha = result.stdout.strip()
    try:
        text = args.matrix.read_text(encoding="utf-8")
    except OSError as error:
        print(f"release gate: cannot read {args.matrix}: {error}", file=sys.stderr)
        return 2
    test_log = args.matrix.parent / "TEST_LOG.md"
    try:
        test_log_text = test_log.read_text(encoding="utf-8")
    except OSError as error:
        print(f"release gate: cannot read {test_log}: {error}", file=sys.stderr)
        return 2
    errors = validate_matrix(text, test_log_text, expected_sha, repository)
    if errors:
        for error in errors:
            print(f"release gate: {error}", file=sys.stderr)
        print("release gate: hardware release criteria are not satisfied", file=sys.stderr)
        return 1
    print("release gate: all required hardware rows have recorded evidence")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
