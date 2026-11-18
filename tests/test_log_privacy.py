from pathlib import Path
import unittest
ROOT=Path(__file__).resolve().parents[1]
class LogPrivacy(unittest.TestCase):
    def test_sanitize_before_any_raw_sink(self):
        s=(ROOT/'payload/gc_main.c').read_text();body=s[s.index('void ghostpad_status_log(const'):s.index('#define gp_log')]
        self.assertIn('diagnostics_sanitize_raw_log',body)
        self.assertLess(body.index('diagnostics_sanitize_raw_log'),body.index('klog_printf'))
        self.assertLess(body.index('diagnostics_sanitize_raw_log'),body.index('write(g_log_fd'))
    def test_safe_policy_guards_klog_thread(self):
        s=(ROOT/'payload/gc_main.c').read_text();p=s.index('pthread_create(&g_klog_tid')
        self.assertIn('g_runtime_policy.enable_ds_handoff',s[p-150:p])
    def test_disabled_handoff_does_not_observe(self):
        s=(ROOT/'payload/ds_handoff.c').read_text();a=s.index('void ds_handoff_klog_line(');b=s.index('int added',a)
        self.assertIn('!g_allowed',s[a:b]);self.assertIn('!g_enabled',s[a:b])
    def test_optional_bridge_reports_are_sanitized(self):
        s=(ROOT/'payload/bridge/wireless_ds4.c').read_text();start=s.index('report_printf(int fd, const char *format, ...)\n{')
        body=s[start:s.index('static int\nremote_pad_identity',start)]
        self.assertLess(body.index('diagnostics_sanitize_raw_log'),body.index('write(fd'))
        self.assertNotIn('"Source User ID:   0x%08x',s)
        self.assertIn('(uint32_t)handle, matches_user, inferred_index',s)
if __name__=='__main__':unittest.main()
