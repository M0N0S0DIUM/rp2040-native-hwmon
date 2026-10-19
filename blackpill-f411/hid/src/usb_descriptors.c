// Farbwerk-compatibility HID descriptors; USB VID/PID are not owned by this project.
// FOR PERSONAL LAB RESEARCH ONLY. Do not distribute products using this ID.
#include "tusb.h"
#include <string.h>
#define USB_VENDOR_ID 0x0c70
#define USB_PRODUCT_ID 0xf00a
#define STATUS_REPORT_BYTES_WITH_ID 64
#define STATUS_PAYLOAD_BYTES 63
#define USB_HID_EP 0x81
#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN)

static tusb_desc_device_t const device_desc = {
  .bLength = sizeof(tusb_desc_device_t),
  .bDescriptorType = TUSB_DESC_DEVICE,
  .bcdUSB = 0x0200,
  .bDeviceClass = TUSB_CLASS_UNSPECIFIED,
  .bDeviceSubClass = 0,
  .bDeviceProtocol = 0,
  .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
  .idVendor = USB_VENDOR_ID,
  .idProduct = USB_PRODUCT_ID,
  .bcdDevice = 0x0100,
  .iManufacturer = 1, .iProduct = 2, .iSerialNumber = 3,
  .bNumConfigurations = 1,
};

static uint8_t const report_desc[] = {
  TUD_HID_REPORT_DESC_GENERIC_INOUT(STATUS_PAYLOAD_BYTES, HID_REPORT_ID(1))
};
static uint8_t const config_desc[] = {
  TUD_CONFIG_DESCRIPTOR(1, 1, 0, CONFIG_TOTAL_LEN, 0, 100),
  TUD_HID_DESCRIPTOR(0, 0, HID_ITF_PROTOCOL_NONE,
                     sizeof(report_desc), USB_HID_EP,
                     STATUS_REPORT_BYTES_WITH_ID, 10)
};

uint8_t const* tud_descriptor_device_cb(void) {
  return (uint8_t const*)&device_desc;
}
uint8_t const* tud_descriptor_configuration_cb(uint8_t index) {
  (void)index; return config_desc;
}
uint8_t const* tud_hid_descriptor_report_cb(uint8_t instance) {
  (void)instance; return report_desc;
}

static const char* strings[] = {
  "", "STM32F411 Experimental", "Farbwerk Temperature Monitor",
  "BLACKPILL-F411-NTC"
};
static uint16_t desc_str[64];

uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
  (void)langid;
  if (index == 0) {
    desc_str[0] = (TUSB_DESC_STRING << 8) | 4;
    desc_str[1] = 0x0409;
    return desc_str;
  }
  if (index >= 4) return NULL;
  size_t n = strlen(strings[index]);
  if (n > 63) n = 63;
  desc_str[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2*n + 2));
  for (size_t i=0; i < n; ++i) desc_str[i+1] = (uint16_t)strings[index][i];
  return desc_str;
}
