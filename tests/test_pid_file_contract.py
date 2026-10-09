#!/usr/bin/env python3
"""Protect startup/shutdown from PID reuse and unsafe previous-instance signals."""
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[1]
MAIN = (ROOT / "payload/gc_main.c").read_text()


class PidFileContract(unittest.TestCase):
    def test_startup_never_signals_a_pid_loaded_from_disk(self):
        self.assertNotIn("kill(old, SIGTERM)", MAIN)
        self.assertNotIn("Killing prev pid=", MAIN)
        self.assertIn("runtime_instance_acquire(PID_PATH", MAIN)

    def test_instance_lock_is_held_and_released_by_its_owner(self):
        instance = (ROOT / "payload/runtime_instance.c").read_text()
        self.assertIn("flock(fd, LOCK_EX | LOCK_NB)", instance)
        self.assertIn("kill(existing_pid, 0)", instance)
        self.assertNotIn("SIGTERM", instance)
        release_start = MAIN.index("static int remove_pid_file_if_owned(void) {")
        release_end = MAIN.index("\n}", release_start)
        self.assertIn("runtime_instance_release(PID_PATH, fd)",
                      MAIN[release_start:release_end])

    def test_pid_lock_is_acquired_before_privileged_runtime_setup(self):
        acquire = MAIN.index("runtime_instance_acquire(PID_PATH")
        user_service = MAIN.index("sceUserServiceInitialize(NULL)")
        privilege = MAIN.index("elevate_credentials()")
        self.assertLess(acquire, user_service)
        self.assertLess(acquire, privilege)


if __name__ == "__main__":
    unittest.main()
