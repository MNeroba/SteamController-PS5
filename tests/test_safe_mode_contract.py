#!/usr/bin/env python3
"""Source-level guarantees for the default runtime safe mode."""
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[1]
MAIN = (ROOT / "payload/gc_main.c").read_text()
WEBUI = (ROOT / "payload/webui.c").read_text()
POLICY = (ROOT / "payload/runtime_policy.c").read_text()


class SafeModeContract(unittest.TestCase):
    def test_optional_services_require_explicit_policy_flags(self):
        for flag, start in (
            ("enable_bridge_probe", "bridge_probe_start(g_inject_uid)"),
            ("enable_game_hooks", "game_hooks_start()"),
            ("enable_ds_handoff", "ds_handoff_start()"),
            ("enable_webui", "webui_start()"),
        ):
            self.assertIn(f"if (g_runtime_policy.{flag}", MAIN)
            self.assertIn(start, MAIN)

    def test_controller_probe_never_guesses_or_detaches_unknown_usb(self):
        start = MAIN.index("static int probe_one_path(")
        end = MAIN.index("/* ── Create VDA", start)
        body = MAIN[start:end]
        self.assertNotIn("enable_legacy_usb_probe", MAIN)
        self.assertNotIn("USB_FS_", body)
        self.assertNotIn("USB_IFACE_DRIVER_DETACH", body)
        self.assertIn("steam_usb_is_sc2_device(vid, pid)", body)

    def test_dualsense_web_control_cannot_bypass_policy(self):
        self.assertIn("if (enabled && (!g_allowed || g_stopping", 
                      (ROOT / "payload/ds_handoff.c").read_text())
        self.assertIn("ds_handoff_set_enabled(on[0] == '1') != 0", WEBUI)
        self.assertIn("403, \"DualSense hand-off is disabled by runtime policy\"", WEBUI)

    def test_policy_invalid_input_resets_to_safe_defaults(self):
        self.assertIn("runtime_policy_defaults(policy);", POLICY)
        self.assertIn("goto invalid;", POLICY)
        self.assertIn("open(path, O_RDONLY | O_NOFOLLOW)", POLICY)
        self.assertIn("runtime_policy_load_file(RUNTIME_POLICY_PATH, policy)", MAIN)

    def test_startup_does_not_call_shellui_memory_patch_entrypoints(self):
        for function in ("shellui_pad_inject(", "shellui_pad_patch(",
                         "shellui_pad_unpatch(", "shellui_pad_hook_gethandle(",
                         "shellui_pad_hook_setpriv("):
            self.assertNotIn(function, MAIN)


if __name__ == "__main__":
    unittest.main()
