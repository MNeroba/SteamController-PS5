#!/usr/bin/env python3
"""Fail closed if USB interface detachment escapes the Steam descriptor gate."""
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[1]
MAIN = (ROOT / "payload/gc_main.c").read_text()


class UsbDetachContract(unittest.TestCase):
    def test_probe_is_identity_only_and_never_initializes_usbfs(self):
        start = MAIN.index("static int probe_one_path(")
        end = MAIN.index("/* ── Create VDA", start)
        probe = MAIN[start:end]
        self.assertNotIn("enable_legacy_usb_probe", MAIN)
        self.assertNotIn("USB_FS_", probe)
        self.assertNotIn("USB_IFACE_DRIVER_DETACH", probe)
        self.assertIn("steam_usb_is_sc2_device(vid, pid)", probe)
        self.assertNotIn("PID_STEAM_WIRED", probe)

    def test_worker_rejects_non_sc2_before_usb_operations(self):
        start = MAIN.index("static void *usb_hid_thread(")
        end = MAIN.index("/* ── Controller manager thread", start)
        worker = MAIN[start:end]
        gate = worker.index("!steam_usb_is_sc2_device(vid, pid)")
        first_open = worker.index("usb_session_open(")
        self.assertLess(gate, first_open)
        self.assertIn("goto exit_slot;", worker[gate:first_open])

    def test_sc2_worker_uses_only_descriptor_mapped_endpoints_and_interfaces(self):
        start = MAIN.index("static void *usb_hid_thread(")
        end = MAIN.index("/* ── Controller manager thread", start)
        worker = MAIN[start:end]
        self.assertNotIn("0x81 + k", worker)
        self.assertNotIn("active - 0x81", worker)
        self.assertIn("detach_sc2_map_interfaces(fd, vid, pid, &sc2_map)", worker)
        self.assertIn("sc2_iface_of_in(&sc2_map, (uint8_t)active)", worker)
        self.assertIn("if (k < 0)", worker)

    def test_all_raw_detach_calls_are_in_the_checked_helper(self):
        calls = [i for i in range(len(MAIN))
                 if MAIN.startswith("USB_IFACE_DRIVER_DETACH", i)]
        self.assertEqual(len(calls), 1)
        helper_start = MAIN.index("static int detach_sc2_interface_checked(")
        helper_end = MAIN.index("\n}", helper_start)
        self.assertGreaterEqual(calls[0], helper_start)
        self.assertLess(calls[0], helper_end)
        helper = MAIN[helper_start:helper_end]
        self.assertIn("steam_usb_detach_interface_allowed", helper)
        self.assertIn("USB_IFACE_DRIVER_DETACH", helper)

    def test_unavailable_or_invalid_map_fails_before_detach(self):
        start = MAIN.index("int map_status = sc2_ep_map")
        end = MAIN.index("/* Wait for the controller to wake", start)
        setup = MAIN[start:end]
        detach = setup.index("detach_sc2_map_interfaces(fd, vid, pid, &sc2_map)")
        for status in ("SC2_USB_MAP_UNAVAILABLE", "SC2_USB_MAP_DEVICE_GONE"):
            branch = setup.index(f"map_status == {status}")
            self.assertLess(branch, detach)
            self.assertIn("goto exit_slot;", setup[branch:detach])
        self.assertIn("map_status < 0", setup[:detach])
        self.assertIn("n_eps <= 0", setup[:detach])
        self.assertNotIn("0x81 + k", setup)


if __name__ == "__main__":
    unittest.main()
