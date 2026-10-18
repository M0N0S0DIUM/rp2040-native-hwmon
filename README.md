# RP2040-Zero native Linux hwmon NTC monitor (experimental)

Read **Safety** before flashing. This is a research prototype, not verified cooling hardware.

## Goal

Expose two 10 kΩ NTC thermistors as native Linux `hwmon` temperature inputs, suitable for use as inputs to CoolerControl. **The RP2040 never drives fans**; it only reports temperatures. Existing motherboard/BIOS fan controls must remain effective if the USB device disappears.

The firmware experimentally imitates an Aqua Computer **Farbwerk** USB HID temperature-reporting device so the Linux `aquacomputer_d5next` kernel driver can provide four `temp*_input` nodes. Only inputs 1 and 2 are wired; 3 and 4 report unavailable.

This compatibility technique uses Aqua Computer's USB VID/PID for **personal testing only**. Do not distribute hardware using someone else's allocated USB VID/PID.

## Connections

| Waveshare RP2040-Zero | Function |
| --- | --- |
| GPIO26 / ADC0 | Water thermistor (Alphacool Eiszapfen) |
| GPIO27 / ADC1 | Case-air thermistor (Valorup 10 kΩ flat) |
| 3V3 | One independent 10 kΩ fixed pull-up to *each* ADC input |
| GND | Other terminal of *each* NTC |
| USB-C | Power and USB HID reporting to Linux PC |

Thermistor divider, each channel: `3V3 -- 10k fixed resistor -- ADC input -- 10k NTC -- GND`. **Do not connect these GPIO inputs to motherboard fan headers, 5 V, or 12 V.** GPIO26 and GPIO27 are small pads on the Zero; verify the particular board pinout before soldering.

## Build

GitHub Actions runs automatically on `main` changes and can also be launched via **Actions → RP2040 native hwmon firmware → Run workflow**.

* Workflow clones Raspberry Pi Pico SDK **2.1.1**, installs the ARM compiler on Ubuntu, and generates `rp2040_native_hwmon.uf2`.
* Download from the successful run's **Artifacts → rp2040-native-hwmon-uf2** (ZIP), then extract the UF2.
* To flash, hold **BOOT** while connecting the RP2040 over USB, copy the UF2 onto the **RPI-RP2** drive, and allow it to reboot.

Local build (with `PICO_SDK_PATH` and toolchain already installed):

```sh
cmake -S . -B build -G Ninja -DPICO_BOARD=pico
cmake --build build
```

Using `PICO_BOARD=pico` is intentional: both target the RP2040, and this project only needs ADC and USB, not Waveshare-specific board features.

## Linux verification (Arch)

```sh
sudo modprobe aquacomputer_d5next
lsusb -d 0c70:f00a
dmesg | tail -n 70
sensors
for f in /sys/class/hwmon/hwmon*/name; do
  if grep -qx farbwerk "$f"; then
    d="${f%/name}"; echo "Found $d"; cat "$d"/temp{1,2,3,4}_input 2>/dev/null
  fi
done
```

If the device is detected, confirm that temp1 follows the water sensor and temp2 follows the air sensor. Use **sane and stable temperatures** before connecting them to CoolerControl curves. There is no custom Python daemon or serial bridge.

## Limitations and safety

- **Not hardware validated.** Successful compilation does not prove Linux USB HID binding, temperature accuracy, or robust hot unplug/replug.
- Both beta values currently default to **3950 K**, *provisional*. Check the thermistor data curves against a known reference at multiple temperatures, especially if water and air thermistors differ.
- The Linux driver exposes labels corresponding to Farbwerk's four sensor inputs; the first two are assigned to water and air by this wiring convention.
- Broken/disconnected/shorted NTCs should report an invalid sensor value, but the behavior must be demonstrated on hardware, including USB unplug and stalled data.
- **Do not disable BIOS/firmware fan fail-safes.** Test failure behavior before letting host software control cooling.
- This project does **not** implement direct PWM/RPM fan control, and is **not** Aqua Computer-compatible in every respect.

Source of compatibility offsets: [Linux kernel `aquacomputer_d5next.c`](https://github.com/torvalds/linux/blob/master/drivers/hwmon/aquacomputer_d5next.c) (Farbwerk temperatures start at report offset `0x2f`, two bytes each, big endian; report ID `1`).
