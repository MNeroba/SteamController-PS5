#!/usr/bin/env python3
"""Source-level guards for destructive VDA ownership regressions."""

from pathlib import Path
import re
import unittest


ROOT = Path(__file__).resolve().parents[1]


def function_body(source: str, signature: str, end_signature: str) -> str:
    start = source.index(signature)
    end = source.index(end_signature, start)
    return source[start:end]


class VdaOwnershipTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.gc_main = (ROOT / "payload/gc_main.c").read_text(encoding="utf-8")
        cls.shellui = (ROOT / "payload/shellui_pad.c").read_text(encoding="utf-8")

    def test_startup_never_deletes_anonymous_handle_range(self) -> None:
        self.assertNotRegex(
            self.gc_main,
            r"scePadVirtualDeviceDeleteDevice\s*\(\s*dh\s*\)",
            "startup must not delete anonymous VDA IDs",
        )
        self.assertNotRegex(
            self.gc_main,
            r"for\s*\(\s*int\s+dh\s*=\s*0\s*;[^)]*<\s*64",
            "startup must not sweep the global handle range",
        )
        self.assertEqual(len(re.findall(
            r"scePadVirtualDeviceDeleteDevice\s*\(\s*handle\s*\)",
            self.gc_main,
        )), 1, "the payload delete API must only be called by the registry backend")

    def test_create_path_never_promotes_scanned_or_klog_ids_to_owned_handle(self) -> None:
        body = function_body(self.gc_main, "static int32_t create_vda_for_slot(",
                             "static int sc2_pad_in_use(")
        self.assertNotIn("scePadGetHandle(", body,
                         "a looked-up controller handle is not owned by AddDevice")
        self.assertNotIn("klog_dequeue_ms", body,
                         "asynchronous klog events are not ownership receipts")
        self.assertNotRegex(body, r"handle\s*=\s*\(int32_t\)\s*\(\s*dev_id",
                            "device IDs must not be reinterpreted as handles")
        self.assertIn("(ret > 0) ? ret : -1", body,
                      "only the positive AddDevice return is accepted as owned")

    def test_shellui_stubs_delete_only_the_handle_returned_by_their_vda_call(self) -> None:
        for signature, end_signature in (
            ("void shellui_stub(void *arg)\n{", "void shellui_stub_end(void)"),
            ("void shellui_stub_force_vda(void *arg)\n{",
             "void shellui_stub_force_vda_end(void)"),
        ):
            body = function_body(self.shellui, signature, end_signature)
            self.assertIn("created_handle", body)
            self.assertEqual(body.count("created_handle = vda_ret;"),
                             body.count("if (vda_ret > 0)"),
                             "only direct positive VDA results can establish ownership")
            direct_return_blocks = re.findall(
                r"if\s*\(\s*vda_ret\s*>\s*0\s*\)\s*\{([^{}]*)\}",
                body,
                re.DOTALL,
            )
            self.assertEqual(
                sum("created_handle = vda_ret;" in block
                    for block in direct_return_blocks),
                body.count("created_handle = vda_ret;"),
                "each ownership mark must be inside a positive-return branch",
            )
            guarded_delete = re.compile(
                r"if\s*\(\s*!use_insert\s*&&\s*a->fp_del\s*&&\s*"
                r"created_handle\s*>=\s*0\s*&&\s*"
                r"vda_handle\s*==\s*created_handle\s*\)\s*\{\s*"
                r"a->fp_del\s*\(\s*created_handle\s*\)\s*;\s*\}",
                re.DOTALL,
            )
            self.assertEqual(len(guarded_delete.findall(body)), 1,
                             "the stub must guard its delete call with exact ownership")
        direct_delete_calls = re.findall(r"a->fp_del\s*\(", self.shellui)
        self.assertEqual(len(direct_delete_calls), 2)


if __name__ == "__main__":
    unittest.main()
