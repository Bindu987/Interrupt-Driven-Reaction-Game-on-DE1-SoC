#include "system.h"
#include "altera_avalon_pio_regs.h"
#include "altera_avalon_timer_regs.h"
#include "sys/alt_irq.h"
#include "sys/alt_stdio.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#define LED_MASK            0x0F
#define MAX_SCORE           99
#define GAME_DURATION_TICKS 600  // 60s @ 100ms = 600 ticks
#define DEBOUNCE_DELAY_MS   50

const int freq[3] = {10, 5, 3};  // pattern intervals in 100ms ticks

const alt_u8 hex_digits[10] = {
    0xC0, 0xF9, 0xA4, 0xB0, 0x99,
    0x92, 0x82, 0xF8, 0x80, 0x90
};

volatile int game_running = 0;
volatile int score = 0;
volatile int ticks = 0;
volatile int pattern_ticks = 0;
volatile int pattern_interval = 10;
volatile int current_pattern = 0;

// Delay function (approximate)
void delay_ms(int ms) {
    volatile int i;
    for (i = 0; i < ms * 10000; ++i);
}

// Display score on HEX1:HEX0
void display_score(int s) {
    int ones = s % 10;
    int tens = (s / 10) % 10;
    IOWR_ALTERA_AVALON_PIO_DATA(HEX0_BASE, hex_digits[ones]);
    IOWR_ALTERA_AVALON_PIO_DATA(HEX1_BASE, hex_digits[tens]);
    printf("Score displayed: %d\n", s);
}

// Display random LED pattern on LEDR[3:0]
void generate_pattern() {
    // Generate a new pattern that is not 0
    do {
        current_pattern = (rand() % 15) + 1;
    } while(current_pattern == 0);
    IOWR_ALTERA_AVALON_PIO_DATA(LEDS_BASE, current_pattern);
    printf("New pattern generated: 0x%X\n", current_pattern);
}

// Timer ISR
void timer_isr(void* context) {
    IOWR_ALTERA_AVALON_TIMER_STATUS(TIMER_0_BASE, 0);  // Clear IRQ

    if (!game_running) return;

    ticks++;
    pattern_ticks++;

    if (ticks >= GAME_DURATION_TICKS) {
        game_running = 0;
        alt_printf("Game Over\n");
        printf("Final Score: %d\n", score);
        IOWR_ALTERA_AVALON_PIO_DATA(LEDS_BASE, 0);
        // Display 'OFF' on HEX displays
        IOWR_ALTERA_AVALON_PIO_DATA(HEX0_BASE, 0xFF);
        IOWR_ALTERA_AVALON_PIO_DATA(HEX1_BASE, 0xFF);
        return;
    }

    if (pattern_ticks >= pattern_interval) {
        // Only apply penalty if a pattern was displayed and a key response was expected
        if (current_pattern != 0) {
            score -= 5;
            if (score < 0) score = 0;
            printf("Timeout. Penalty applied. Score: %d\n", score);
            display_score(score);
        }

        generate_pattern();
        pattern_ticks = 0;
    }
}

// KEY[0:3] ISR
void key_isr(void* context) {
    // Clear edge capture to acknowledge the interrupt
    int edge_capture = IORD_ALTERA_AVALON_PIO_EDGE_CAP(KEYS_BASE);
    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(KEYS_BASE, 0);

    // Ignore if game is not running
    if (!game_running) {
        return;
    }

    // Debounce to prevent multiple triggers
    delay_ms(DEBOUNCE_DELAY_MS);

    // Read the current state of all keys
    int key_state = IORD_ALTERA_AVALON_PIO_DATA(KEYS_BASE);

    // Convert active-low key state to active-high pattern
    // The keys are at bit positions 0-3, so we check for those bits.
    int pressed_pattern = (~key_state) & 0x0F;

    // If no key is currently pressed, ignore this interrupt.
    if (pressed_pattern == 0) {
        return;
    }

    printf("Key press detected. Pressed pattern: 0x%X, Expected pattern: 0x%X\n", pressed_pattern, current_pattern);

    // Only process a key press if a pattern is currently displayed
    if (current_pattern == 0) {
        printf("No pattern to match. Ignoring key press.\n");
        return;
    }

    // Check if the pressed pattern matches the current LED pattern
    if (pressed_pattern == current_pattern) {
        score += 10;
        if (score > MAX_SCORE) score = MAX_SCORE;
        printf("Correct! Score: %d\n", score);
    } else {
        score -= 5;
        if (score < 0) score = 0;
        printf("Incorrect! Penalty. Score: %d\n", score);
    }

    display_score(score);

    // Clear the LEDs to indicate a response was registered
    IOWR_ALTERA_AVALON_PIO_DATA(LEDS_BASE, 0);
    current_pattern = 0;
    pattern_ticks = 0; // Reset timer for next pattern
}

// SW1 ISR to start or restart game
void switch_isr(void* context) {
    int edge = IORD_ALTERA_AVALON_PIO_EDGE_CAP(SWITCHES_BASE);
    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(SWITCHES_BASE, 0);  // Clear edge

    if (edge & 0x02) {  // SW1
        game_running = 1;
        ticks = 0;
        score = 0;
        pattern_ticks = 0;

        int difficulty_switch = IORD_ALTERA_AVALON_PIO_DATA(SWITCHES_BASE) & 0x01;
        pattern_interval = freq[rand() % 3];

        if (difficulty_switch) {
            pattern_interval /= 2;
            if (pattern_interval == 0) pattern_interval = 1;
        }

        printf("\n=== GAME STARTED ===\n");
        printf("Selected Difficulty (SW0): %d (0=Easy, 1=Hard)\n", difficulty_switch);
        printf("Pattern Interval set to %d ticks\n", pattern_interval);

        generate_pattern();
        display_score(score);
    }
}

int main() {
    srand(IORD_ALTERA_AVALON_TIMER_SNAPL(TIMER_0_BASE));

    IOWR_ALTERA_AVALON_PIO_DATA(LEDS_BASE, 0);
    // Initialize HEX displays to '00'
    display_score(0);

    // Timer Config (100ms)
    IOWR_ALTERA_AVALON_TIMER_PERIODL(TIMER_0_BASE, 0x4240);
    IOWR_ALTERA_AVALON_TIMER_PERIODH(TIMER_0_BASE, 0x0098);
    IOWR_ALTERA_AVALON_TIMER_CONTROL(TIMER_0_BASE, 0x7);

    alt_ic_isr_register(TIMER_0_IRQ_INTERRUPT_CONTROLLER_ID, TIMER_0_IRQ, timer_isr, NULL, NULL);

    // KEY[0:3] Interrupt setup
    IOWR_ALTERA_AVALON_PIO_IRQ_MASK(KEYS_BASE, 0x0F);
    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(KEYS_BASE, 0);
    alt_ic_isr_register(KEYS_IRQ_INTERRUPT_CONTROLLER_ID, KEYS_IRQ, key_isr, NULL, NULL);

    // SW1 Interrupt setup
    IOWR_ALTERA_AVALON_PIO_IRQ_MASK(SWITCHES_BASE, 0x02);
    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(SWITCHES_BASE, 0);
    alt_ic_isr_register(SWITCHES_IRQ_INTERRUPT_CONTROLLER_ID, SWITCHES_IRQ, switch_isr, NULL, NULL);

    printf("System Initialized. Press SW1 to start the game.\n");

    while (1);
}
