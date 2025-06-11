/**
 * https://github.com/DLehenbauer/pico-sdcard-bootloader
 * SPDX-License-Identifier: 0BSD
 */

// Standard
#include <stdint.h>
#include <stdio.h>

// Pico SDK
#include <hardware/gpio.h>
#include <pico/stdlib.h>

// Project
#include "diag.h"

#ifndef BOOTLOADER_LED_PIN
#define BOOTLOADER_LED_PIN PICO_DEFAULT_LED_PIN
#endif

#ifndef BOOTLOADER_UART
#define BOOTLOADER_UART PICO_DEFAULT_UART
#endif

#define BOOTLOADER_UART_INSTANCE (__CONCAT(uart,BOOTLOADER_UART))

#ifndef BOOTLOADER_UART_BAUD_RATE
#define BOOTLOADER_UART_BAUD_RATE PICO_DEFAULT_UART_BAUD_RATE
#endif

#ifndef BOOTLOADER_UART_TX_PIN
#define BOOTLOADER_UART_TX_PIN PICO_DEFAULT_UART_TX_PIN
#endif

#ifndef BOOTLOADER_UART_RX_PIN
#define BOOTLOADER_UART_RX_PIN PICO_DEFAULT_UART_RX_PIN
#endif

typedef struct diag_message_s {
    const char* message;
    bool is_fatal;
    const uint8_t* pattern;
} diag_message_t;

typedef const uint8_t morse_pattern_t[];
static const morse_pattern_t morse_d = { 3, 1, 1, 0 };
static const morse_pattern_t morse_e = { 1, 0 };
static const morse_pattern_t morse_f = { 1, 1, 3, 1, 0 };
static const morse_pattern_t morse_i = { 1, 1, 0 };
static const morse_pattern_t morse_n = { 3, 1, 0 };
static const morse_pattern_t morse_s = { 3, 3, 3, 0 };
static const morse_pattern_t morse_w = { 1, 3, 3, 0 };

const diag_message_t messages[] = {
    /* DIAG_ENTERING_FIRMWARE: */           { .message = "Entering firmware", .is_fatal = false, .pattern = morse_e },
    /* FATAL_WATCHDOG_WITHOUT_FIRMWARE: */  { .message = "Watchdog bad firmware", .is_fatal = true, .pattern = morse_w },
    /* DIAG_NO_FIRMWARE: */                 { .message = "No firmware", .is_fatal = false, .pattern = morse_n },
    /* FATAL_FLASH_FAILED: */               { .message = "Flash failed", .is_fatal = true, .pattern = morse_f },
    /* FATAL_INVALID_UF2: */                { .message = "Invalid UF2", .is_fatal = true, .pattern = morse_i },
    /* DIAG_DELETE_FAILED: */               { .message = "Delete failed", .is_fatal = false, .pattern = morse_d },
    /* DIAG_SKIPPED_PROGRAMMING: */         { .message = "Skipped programming", .is_fatal = false, .pattern = morse_s },
};

void diag_init() {
    #ifdef BOOTLOADER_USE_LED
    gpio_init(BOOTLOADER_LED_PIN);
    gpio_set_dir(BOOTLOADER_LED_PIN, GPIO_OUT);
    led_off();
    #endif

    #ifdef BOOTLOADER_USE_UART
    stdio_uart_init_full(BOOTLOADER_UART_INSTANCE, BOOTLOADER_UART_BAUD_RATE, BOOTLOADER_UART_TX_PIN, BOOTLOADER_UART_RX_PIN);
    #endif
}

void led_on() {
    #ifdef BOOTLOADER_USE_LED
    gpio_put(BOOTLOADER_LED_PIN, true);
    #endif
}

void led_off() {
    #ifdef BOOTLOADER_USE_LED
    gpio_put(BOOTLOADER_LED_PIN, false);
    #endif
}

void led_toggle() {
    #ifdef BOOTLOADER_USE_LED
    gpio_put(BOOTLOADER_LED_PIN, !gpio_get_out_level(BOOTLOADER_LED_PIN));
    #endif
}

static bool is_led_on() {
    #ifdef BOOTLOADER_USE_LED
    return gpio_get_out_level(BOOTLOADER_LED_PIN) == 0;
    #else
    return false; // LED functionality is not enabled
    #endif
}

static void blink(const uint8_t* pattern) {
    const unsigned int dot = 100;

    if (is_led_on()) {
        led_off();
        sleep_ms(3 * dot);
    }

    for (int i = 0; pattern[i] != 0; i++) {
        led_on();
        sleep_ms(pattern[i] * dot);
        led_off();
        sleep_ms(dot);
    }

    sleep_ms(3 * dot);
}

static void diag_or_fatal(diag_code_t code) {
    const diag_message_t* msg = &messages[code];

    #ifdef BOOTLOADER_USE_UART
    printf("[Boot3] (%u): %s\n", code, msg->message);
    fflush(stdout);
    #endif

    #ifdef BOOTLOADER_USE_LED
    do {
        blink(msg->pattern);
    } while (msg->is_fatal);
    #endif
}

void diag(diag_code_t code) {
    assert(!messages[code].is_fatal);
    diag_or_fatal(code);
}

void fatal(diag_code_t code) {
    assert(messages[code].is_fatal);
    diag_or_fatal(code);
}
