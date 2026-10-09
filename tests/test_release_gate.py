#!/usr/bin/env python3
"""Unit tests for the release workflow's hardware evidence gate."""
from importlib.util import module_from_spec, spec_from_file_location
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[1]
SPEC = spec_from_file_location("check_release_gate", ROOT / "scripts/check_release_gate.py")
RELEASE_GATE = module_from_spec(SPEC)
SPEC.loader.exec_module(RELEASE_GATE)


def matrix_row(gate, *, result="VERIFIED_HARDWARE", model="PS5 retail model",
               firmware="13.60", controller="Steam Controller 2026 / fw 1.0",
               commit="0123456789abcdef0123456789abcdef01234567",
               evidence=None):
    evidence = evidence or f"project-memory/TEST_LOG.md#{gate.lower()}"
    return "| " + " | ".join((
        gate, "release scenario", model, firmware, controller, "USB-C",
        f"{commit}; SDK v0.43", result, evidence,
    )) + " |"


def evidence_log():
    entries = []
    for gate in RELEASE_GATE.REQUIRED_GATES:
        entries.append("\n".join((
            f"### {gate}",
            f"Test ID: {gate}",
            "PS5 model: PS5 retail model",
            "Firmware: 13.60",
            "Controller: Steam Controller 2026 / fw 1.0",
            "Transport: USB-C",
            "Payload commit: 0123456789abcdef0123456789abcdef01234567",
            "Result: VERIFIED_HARDWARE",
        )))
    return "\n\n".join(entries)


class ReleaseGateTests(unittest.TestCase):
    def valid_matrix(self):
        return "\n".join(
            matrix_row(gate) for gate in RELEASE_GATE.REQUIRED_GATES
        )

    def test_complete_evidenced_hardware_matrix_passes(self):
        self.assertEqual(RELEASE_GATE.validate_matrix(
            self.valid_matrix(), evidence_log()), [])

    def test_missing_required_gate_fails_closed(self):
        matrix = "\n".join(
            matrix_row(gate) for gate in RELEASE_GATE.REQUIRED_GATES if gate != "H-03"
        )
        self.assertTrue(any("H-03: required hardware row is missing" in error
                            for error in RELEASE_GATE.validate_matrix(
                                matrix, evidence_log())))

    def test_unverified_status_fails_closed(self):
        matrix = "\n".join(
            matrix_row(gate, result="NOT_RUN / BLOCKED_HARDWARE")
            if gate == "H-02" else matrix_row(gate)
            for gate in RELEASE_GATE.REQUIRED_GATES
        )
        self.assertTrue(any("H-02: result must be VERIFIED_HARDWARE" in error
                            for error in RELEASE_GATE.validate_matrix(
                                matrix, evidence_log())))

    def test_placeholders_and_missing_test_log_evidence_fail(self):
        rows = [matrix_row(gate) for gate in RELEASE_GATE.REQUIRED_GATES]
        rows[0] = matrix_row("H-01", model="NOT_TESTED", evidence="no evidence")
        errors = RELEASE_GATE.validate_matrix("\n".join(rows), evidence_log())
        self.assertTrue(any("H-01: PS5 model contains a placeholder" in error
                            for error in errors))
        self.assertTrue(any("H-01: evidence must link to a test-log entry" in error
                            for error in errors))

    def test_wrong_firmware_and_unrecorded_test_log_entry_fail(self):
        rows = [matrix_row(gate, firmware="13.60 target" if gate == "H-03"
                           else "13.60")
                for gate in RELEASE_GATE.REQUIRED_GATES]
        errors = RELEASE_GATE.validate_matrix("\n".join(rows), "only H-01")
        self.assertTrue(any("H-03: firmware must be exactly 13.60" in error
                            for error in errors))
        self.assertTrue(any("H-02: anchored hardware test entry is absent" in error
                            for error in errors))

    def test_test_log_must_match_matrix_values_and_release_result(self):
        log = evidence_log().replace(
            "Firmware: 13.60", "Firmware: 13.50", 1
        ).replace("Result: VERIFIED_HARDWARE", "Result: PASS", 1)
        errors = RELEASE_GATE.validate_matrix(self.valid_matrix(), log)
        self.assertTrue(any("H-01: test-log entry must record Result: VERIFIED_HARDWARE" in error
                            for error in errors))
        self.assertTrue(any("H-01: test-log entry does not record matching firmware" in error
                            for error in errors))

    def test_test_log_anchor_must_be_gate_specific(self):
        rows = [matrix_row(gate, evidence=f"project-memory/TEST_LOG.md#h-02")
                if gate == "H-01" else matrix_row(gate)
                for gate in RELEASE_GATE.REQUIRED_GATES]
        errors = RELEASE_GATE.validate_matrix("\n".join(rows), evidence_log())
        self.assertTrue(any("H-01: test-log entry does not identify the hardware gate" in error
                            for error in errors))


if __name__ == "__main__":
    unittest.main()
