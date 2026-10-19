# STM32F411CEU6 Black Pill — 4-channel native Linux USB temperature monitor

The original 2-channel firmware was physically verified on STM32F411CEU6 with 25 MHz oscillator: USB HID bound to Linux's existing \`aquacomputer_d5next\` driver and Sensor 1/2 measured realistic temperatures. This revision adds PA3 and PA4 as ADC inputs without altering the PA1/PA2 wiring or Linux USB identity.

## Pinout (3.3 V ONLY)

Each channel needs: **3V3 → 10 kΩ fixed resistor → ADC pin → 10 kΩ NTC thermistor → GND**.

| Linux sensor | Board silkscreen | STM32 ADC channel | Purpose |
| --- | --- | --- | --- |
| Sensor 1 | A1 / PA1 | ADC1_IN1 | Alphacool coolant |
| Sensor 2 | A2 / PA2 | ADC1_IN2 | Valorup case air |
| Sensor 3 | A3 / PA3 | ADC1_IN3 | Optional |
| Sensor 4 | A4 / PA4 | ADC1_IN4 | Optional |

**IMPORTANT — optional channels:** PA3/PA4 are **floating ADC inputs** without resistors; a floating channel can show a random plausible temperature. To allow later sensors without reflashing, **install the 10 kΩ pull-ups on PA3 and PA4 now**. With a pull-up and no NTC installed, the input is near 3.3 V and firmware reports invalid \`0x7fff\` (Linux displays \`N/A\`). When ready, add a 10 kΩ NTC from the pin to GND. Without the pull-up, **N/A is NOT guaranteed**. Internal STM32 pull-ups are disabled in ADC analog mode.

The TinyUSB board support uses PA2 and PA3 as UART TX/RX at startup. Our firmware reconfigures PA1/PA2/PA3/PA4 for ADC **after** \`board_init()\`, intentionally disabling USART2 use of PA2/PA3.

All channels assume 10 kΩ NTC at 25 °C, beta **3950 K**. Adjust individual beta constants if additional probes have different specifications. Open/short circuits show N/A with correctly installed dividers; valid displayed range is currently 0–100 °C.

## Firmware and flashing

The GitHub Actions workflow retains its board-specific **HSE_VALUE=25000000** oscillator fix, required for this physical WeAct board, and builds diagnostic + native HID firmware.

With BOOT0 held, press/release NRST then release BOOT0. Check \`lsusb -d 0483:df11\`. From the binary's directory:

\`\`\`sh
sudo dfu-util -d 0483:df11 -a 0 -s 0x08000000:leave -D blackpill_f411_hwmon_4ch_25mhz.bin
\`\`\`

Reconnect without BOOT0. Confirm:

\`\`\`sh
lsusb -d 0c70:f00a
sudo modprobe aquacomputer_d5next
sensors | grep -A 5 '^farbwerk-'
\`\`\`

Reports use Farbwerk-compatible 64-byte HID status report ID 1; four BE16 centidegree readings at offsets 0x2f, 0x31, 0x33, 0x35. Failed channels report 0x7fff. This is for private Linux interoperability: the USB VID/PID belongs to Aqua Computer; do not sell hardware using it.

## Testing and safety

Keep an independent, known-safe motherboard fan configuration. Check that the existing coolant/air sensors have not swapped, verify each optional channel by warming its probe, and ensure disconnected sensors report N/A with their pull-ups installed. Also check USB removal and machine restart. This four-channel revision is CI-built and **not yet physically tested**.
