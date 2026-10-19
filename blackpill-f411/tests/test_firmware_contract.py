import pathlib
import re
import struct
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]

class FirmwareContract(unittest.TestCase):
    def setUp(self):
        self.src = (ROOT / "hid/src/main.c").read_text()

    def test_four_physical_adc_channels_in_exact_report_order(self):
        channels = re.findall(
            r'store_be16\(&report\[SENSOR_OFFSET(?: \+ (\d+))?\], '
            r'ntc_centidegrees\((\d+)u, ([A-Z0-9_]+)\)\);',
            self.src
        )
        self.assertEqual(channels, [
            ("", "1", "WATER_BETA"),
            ("2", "2", "AIR_BETA"),
            ("4", "3", "CHANNEL3_BETA"),
            ("6", "4", "CHANNEL4_BETA"),
        ])

    def test_adc_gpio_analog_setup_covers_pa1_to_pa4(self):
        self.assertIn("for (unsigned channel = 1u; channel <= 4u; ++channel)", self.src)
        self.assertIn("GPIOA->MODER =", self.src)
        self.assertIn("GPIOA->PUPDR &= ~mode_mask;", self.src)
        self.assertIn("ADC1->SMPR2 =", self.src)
        self.assertIn("ADC1->SQR3 = channel;", self.src)

    def test_open_short_and_adc_failure_returns_na(self):
        self.assertIn("#define INVALID_TEMPERATURE 0x7fffu", self.src)
        self.assertIn("frac <= 0.005f || frac >= 0.995f", self.src)
        self.assertIn("if (!adc_one(adc_channel, &raw)) return INVALID_TEMPERATURE;", self.src)

    def test_hid_identity(self):
        descriptor = (ROOT / "hid/src/usb_descriptors.c").read_text()
        self.assertIn("0x0c70", descriptor)
        self.assertIn("0xf00a", descriptor)
        self.assertIn("HID_REPORT_ID(1)", descriptor)

    def test_report_shape(self):
        for values in ((3125, 2460, 3180, 2250),
                       (3125, 2460, 0x7fff, 0x7fff)):
            buf = bytearray(64)
            buf[0] = 1
            for index, value in enumerate(values):
                struct.pack_into(">H", buf, 0x2f + index * 2, value)
            self.assertEqual(buf[0], 1)
            self.assertEqual(len(buf), 64)
            self.assertEqual(tuple(struct.unpack_from(">H", buf, 0x2f + i*2)[0] for i in range(4)), values)
        self.assertEqual(buf[0x33:0x37], bytes.fromhex("7fff7fff"))

    def test_workflow_preserves_25mhz_fix(self):
        workflow = (ROOT.parent / ".github/workflows/blackpill-f411.yml").read_text()
        self.assertIn("stm32f411blackpill", workflow)
        self.assertIn("HSE_VALUE=25000000", workflow)
        self.assertIn("blackpill_f411_hwmon_4ch_25mhz.bin", workflow)

if __name__ == "__main__":
    unittest.main()
