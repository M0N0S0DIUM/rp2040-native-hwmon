import pathlib, unittest
ROOT=pathlib.Path(__file__).resolve().parents[1]
class FirmwareContract(unittest.TestCase):
    def test_adc_channels(self):
        text=(ROOT/"hid/src/main.c").read_text()
        self.assertIn("ntc_centidegrees(1u, WATER_BETA)",text)
        self.assertIn("ntc_centidegrees(2u, AIR_BETA)",text)
        self.assertIn("GPIOA->MODER",text)
    def test_hid_identity(self):
        text=(ROOT/"hid/src/usb_descriptors.c").read_text()
        self.assertIn("0x0c70",text)
        self.assertIn("0xf00a",text)
        self.assertIn("HID_REPORT_ID(1)",text)
    def test_report_shape(self):
        import struct
        arr=bytearray(64)
        arr[0]=1
        for index,value in enumerate([3125,2460,0x7fff,0x7fff]):
            struct.pack_into(">H",arr,0x2f+index*2,value)
        self.assertEqual(arr[0],1)
        self.assertEqual(struct.unpack_from(">H",arr,0x2f)[0]*10,31250)
        self.assertEqual(struct.unpack_from(">H",arr,0x31)[0]*10,24600)
        self.assertEqual(arr[0x33:0x37],bytes.fromhex("7fff7fff"))
    def test_build_tooling(self):
        assert "stm32f411blackpill" in (ROOT.parent/".github/workflows/blackpill-f411.yml").read_text()
if __name__ == "__main__": unittest.main()
