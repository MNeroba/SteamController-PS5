#!/usr/bin/env python3
"""Source guards for the payload's ordered, owner-aware shutdown path."""

from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[1]


class ShutdownContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.sources = {
            name: (ROOT / path).read_text(encoding="utf-8")
            for name, path in {
                "gc_main": "payload/gc_main.c",
                "bridge_probe": "payload/bridge_probe.c",
                "game_hooks": "payload/game_hooks.c",
                "game_hook_safety": "payload/game_hook_safety.h",
                "ds_handoff": "payload/ds_handoff.c",
                "webui": "payload/webui.c",
                "sc2_profile": "payload/sc2_profile.c",
            }.items()
        }

    def require(self, fragment: str, source: str, message: str) -> None:
        if fragment not in source:
            self.fail(message)

    def test_services_have_joinable_stop_apis(self) -> None:
        for name, api in (
            ("bridge_probe", "bridge_probe_stop"),
            ("game_hooks", "game_hooks_stop"),
            ("ds_handoff", "ds_handoff_stop"),
            ("webui", "webui_stop"),
            ("sc2_profile", "sc2_select_stop"),
        ):
            with self.subTest(service=name):
                self.require(api, self.sources[name],
                             f"{name} must expose a coordinated stop operation")
                if "pthread_detach" in self.sources[name]:
                    self.fail(f"{name} worker must remain joinable")

    def test_main_has_signal_driven_central_shutdown(self) -> None:
        main = self.sources["gc_main"]
        for required in (
            "sigaction", "SIGTERM", "runtime_request_stop",
            "runtime_shutdown", "pthread_join", "slot_registry_destroy",
        ):
            self.require(required, main,
                         f"main runtime lacks shutdown step {required}")

    def test_usb_workers_stop_in_their_transfer_owner_and_are_joined(self) -> None:
        main = self.sources["gc_main"]
        self.require("runtime_is_stopping()", main,
                     "manager and USB loops must observe the shared stop request")
        self.require("pthread_cond_wait", main,
                     "manager must wait for reader-owned USB cleanup")
        self.require("usb_session_cleanup", main,
                     "USB resources must be stopped by the session owner")
        if "pthread_detach" in main:
            self.fail("manager, USB, and klog threads must be joinable")

    def test_closed_klog_does_not_spin(self):
        main = self.sources["gc_main"]
        start = main.index("static void *klog_capture_thread")
        stop = main.index("/* ── VDI", start)
        self.assertIn('if (len == 0) { gp_log("klog: capture stream closed', main[start:stop])
        self.assertIn('break; }', main[start:stop])

    def test_valid_input_watchdog_uses_owned_cleanup(self):
        main = self.sources["gc_main"]
        self.assertIn("usb_input_watchdog_expired(&input_watchdog", main)
        self.assertIn("usb_completed_frame_valid(&eps[0], sizeof(buf))", main)
        refresh = main.index("usb_input_watchdog_refresh(&input_watchdog")
        self.assertIn("STEAM_REPORT_INPUT", main[refresh-230:refresh])
        self.assertIn("owns_session && sc2_haptic_session_is_owner(slot_token)", main)

    def test_owned_vda_is_neutralized_before_delete(self) -> None:
        main = self.sources["gc_main"]
        self.require("neutralize_vda_handle", main,
                     "shutdown must neutralize registered VDA input")
        self.require("scePadVirtualDeviceDeleteDevice(handle)", main,
                     "the registry backend must still delete its owned VDA")
        neutral = main.index("neutralize_vda_handle")
        delete = main.index("scePadVirtualDeviceDeleteDevice(handle)")
        self.assertLess(neutral, delete,
                        "owned input must be neutralized before VDA deletion")

    def test_game_hook_shutdown_is_bounded_and_reports_remote_rollback(self) -> None:
        hooks = self.sources["game_hooks"]
        main = self.sources["gc_main"]
        self.require("GAME_HOOK_SHUTDOWN_ROLLBACK_ATTEMPTS",
                     self.sources["game_hook_safety"],
                     "game-hook shutdown must bound target rollback retries")
        self.require("GAME_HOOKS_STOP_ROLLBACK_PENDING", hooks,
                     "incomplete remote rollback must be reported after join")
        self.require("neutral", hooks,
                     "shutdown must attempt a neutral frame before remote rollback")
        self.require("target gateway remains mapped for lease fallback", main,
                     "central shutdown must preserve the documented remote-stub fallback")


if __name__ == "__main__":
    unittest.main()
