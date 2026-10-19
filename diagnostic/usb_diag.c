// Minimal ROM-to-flash and standard USB CDC diagnostic for Waveshare RP2040-Zero.
// Deliberately independent of our HID/NTC firmware. No GPIO pins are driven.
#include "pico/stdlib.h"
#include "pico/stdio_usb.h"
#include <stdio.h>

int main(void) {
    stdio_init_all();
    uint32_t counter = 0;
    while (true) {
        if (stdio_usb_connected()) {
            printf("RP2040-Zero flash boot OK, USB CDC OK, seconds=%lu\r\n",
                   (unsigned long)counter);
        }
        ++counter;
        sleep_ms(1000);
    }
}
