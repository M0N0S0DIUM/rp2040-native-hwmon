#pragma once
// The Pico SDK defines CFG_TUSB_MCU and CFG_TUSB_OS in compiler flags.
// Configure TinyUSB as a full-speed USB device on the RP2040's port 0.
#define CFG_TUSB_RHPORT0_MODE (OPT_MODE_DEVICE | OPT_MODE_FULL_SPEED)
#define CFG_TUD_ENABLED 1
#define CFG_TUD_ENDPOINT0_SIZE 64
#define CFG_TUD_HID 1
#define CFG_TUD_HID_EP_BUFSIZE 64
