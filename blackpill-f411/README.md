# STM32F411 Black Pill (WeAct) temperature monitor

This directory targets the **STM32F411CEU6**, confirmed by physical chip marking. The previous RP2040-Zero firmware does not run on this board.

**Two stages are compiled by GitHub Actions:**
1. **USB diagnostic** — upstream TinyUSB \`cdc_msc\` example for the exact \`stm32f411blackpill\` board. Check first that Linux sees a USB serial and a tiny demo mass-storage device. This has NO sensor functionality.
2. **Experimental HID NTC monitor** — reads two analog sensors and sends 64-byte Farbwerk-compatible HID report ID 1 every second. Intended to bind to Linux \`aquacomputer_d5next\` for native kernel \`hwmon\` inputs. No desktop serial bridge or Python service. Not yet tested on physical Black Pill.

## Wiring (3.3 V only)

For each 10k NTC: \`3V3 -- 10k fixed pull-up -- ADC input -- 10k thermistor -- GND\`.

| Pin as marked on PCB | Purpose |
| --- | --- |
| A1 / PA1 (ADC1_IN1) | Alphacool Eiszapfen coolant NTC |
| A2 / PA2 (ADC1_IN2) | Valorup case-air NTC |
| 3V3 | Each sensor's 10k fixed pull-up |
| G / GND | Other sensor leads |
| USB-C | Communication and power |

If the earlier pin damage included A1/A2, check continuity and no shorts before soldering. TinyUSB's board initialization also uses PA2 for the USART2 TX function; our main firmware reconfigures PA2 for ADC after board initialization.

Water and air both currently assume nominal 10k at 25 °C and beta **3950 K**; validate using datasheets/reference thermometer before use.

## Flashing via STM32 factory USB DFU

**This does not support UF2 drag-and-drop.** Both artifacts are raw \`.bin\` intended for writing to **0x08000000**.

With board USB-C attached, hold **BOOT0**, tap/release **NRST**, then release BOOT0. Verify that STM32 DFU is present:

\`\`\`sh
lsusb | grep -i -E '0483:df11|stm'
sudo pacman -S --needed dfu-util
dfu-util -l
\`\`\`

Download/extract the **blackpill-f411-usb-diagnostic** GitHub artifact. In DFU mode, program:

\`\`\`sh
sudo dfu-util -a 0 -s 0x08000000:leave -D blackpill_f411_usb_diagnostic.bin
\`\`\`

If USB DFU works, Linux should now show an experimental TinyUSB CDC serial + demo mass-storage device (NOT the STM32 ROM DFU ID). The diagnostic's tiny USB mass-storage volume is expected.

Only after successful diagnostic, switch to DFU mode again and flash **blackpill_f411_hwmon.bin** using the same DFU command with the filename changed. Verify:

\`\`\`sh
sudo modprobe aquacomputer_d5next
lsusb -d 0c70:f00a
sensors
\`\`\`

A detected HID device is not enough: verify water/air thermistor temperatures, reversed/open sensor behavior, removal, reconnection, and stalled-report recovery before allowing CoolerControl to control fans.

## Safety

- Keep existing motherboard cooling controls enabled; do not assume BIOS safeguards resume automatically after CoolerControl changes PWM settings.
- This is experimental; USB enumeration, kernel binding and temperature calibration are unverified on hardware.
- Never power the analog inputs from 5 V. The F411 GPIO/ADC inputs must stay within supply limits.
- The USB VID/PID \`0c70:f00a\` belongs to Aqua Computer; used here strictly for private interoperability experiments. Never sell hardware with this USB identity.

TinyUSB version 0.18.0 supplies the STM32F411 Black Pill's board startup and clock setup; source is built in GitHub Actions. No user-provided proprietary code is included.
