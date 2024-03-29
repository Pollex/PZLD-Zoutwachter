#include "board.h"
#include "pcal6524.h"
#include "pcal6524_params.h"
#include "periph/i2c.h"
#include "ztimer.h"
#include <stdio.h>

pcal6524_t pcal;
pcal6524_pins_t pins_to_activate;

// Define the mapping of input pins to output pins as per Field_pin order.
// The number indicates the Field_pin number.
// Assuming there is a way to write a specific pin, e.g., pcal6524_write_pin function
int field_pin_order[] = {13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

int main(void) {
    puts("Zoutwachter Switch getting ready...");
    i2c_init(I2C_DEV(1));

    if (pcal6524_init(&pcal, pcal6524_params) < 0) {
        puts("Failed to initialize PCAL6524...");
        while (1);
    }

    while (1) {
        for (int step = 0; step < 3; step++) {
            pins_to_activate.raw = 0; // Reset the pin activation bitmask

            int pin_indices[4]; // To hold the indices of pins to be activated

            // Calculate the index of the first pin of the current field pin to activate
            int start_pin_index = step * 4; // 0 for step 1, 4 for step 2, 8 for step 3

            // Assign the indices of the pins to be activated
            for (int i = 0; i < 4; i++) {
                // Calculate pin index with wrapping
                int pin_index = (start_pin_index + i * 5) % 24;
                pin_indices[i] = field_pin_order[pin_index];
            }

            // Print the set of pins to be activated for this step
            printf("Step %d activated pins: ", step + 1);
            // Create the activation bitmask based on the calculated indices
            for (int i = 0; i < 4; i++) {
                // Shift 1 to the left by the pin number, and OR it into the bitmask
                pins_to_activate.raw |= (1 << (pin_indices[i] - 1));
                printf("%d ", pin_indices[i]);
            }
            printf("\n");

            // Write the pin activation bitmask to the PCAL6524
            pcal6524_write(&pcal, pins_to_activate);

            // Wait 2 seconds before the next step
            ztimer_sleep(ZTIMER_MSEC, 2000);
        }
    }
    return 0;
}
