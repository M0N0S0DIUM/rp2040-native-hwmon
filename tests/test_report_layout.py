"""Protocol layout regression test (does not compile or execute the RP2040 firmware)."""
from pathlib import Path
import re
import unittest

ROOT = Path(__file__).resolve().parents[1]

class ReportLayoutSpec(unittest.TestCase):
    def test_kernel_farbwerk_offsets(self):
        text = (ROOT / "main.c").read_text()
        self.assertRegex(text, r"#define\s+SENSOR_OFFSET\s+0x2f")
        self.assertIn("tud_hid_report(1,report+1,63)", text)

    def test_usb_identity_matches_farbwerk(self):
        text = (ROOT / "usb_descriptors.c").read_text()
        self.assertRegex(text, r"#define\s+VID\s+0x0c70")
        self.assertRegex(text, r"#define\s+PID\s+0xf00a")
        self.assertIn("HID_REPORT_ID(1)", text)

    def test_farbwerk_report_bytes(self):
        report = bytearray(64)
        report[0] = 1
        for index, raw in enumerate([3125, 2460, 0x7fff, 0x7fff]):
            off = 0x2f + 2 * index
            report[off:off+2] = raw.to_bytes(2, "big")
        self.assertEqual(report[0], 1)
        self.assertEqual(int.from_bytes(report[0x2f:0x31], "big") * 10, 31250)
        self.assertEqual(int.from_bytes(report[0x31:0x33], "big") * 10, 24600)
        self.assertEqual(report[0x33:0x37], bytes.fromhex("7fff7fff"))
        self.assertEqual(len(report), 64)

if __name__ == "__main__":
    unittest.main()
