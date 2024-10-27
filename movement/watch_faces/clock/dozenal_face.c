/*
 * MIT License
 *
 * Copyright (c) 2024 flufflefam@users.noreply.github.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "dozenal_face.h"

#include "watch_private_display.h"
#include "finetune_face.h"
#include "nanosec_face.h"
#include "watch_utility.h"

static const char dozenal_digits[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'H', 'E' };

// Table at the end of: https://clocks.dozenal.ca/pdf/watch.pdf
static uint32_t dig1_sec = 2 * 60 * 60;
static uint32_t dig2_sec = 10 * 60;
static uint32_t dig3_sec = 50;
static double dig4_sec = 4 + 1/6;
//static double dig5_sec = 25 / 72;

static uint8_t dozenal_tick_frequency = 16;

void dozenal_face_setup(movement_settings_t *settings, uint8_t watch_face_index, void ** context_ptr) {
    (void) settings;
    (void) watch_face_index;
    if (*context_ptr == NULL) {
        *context_ptr = malloc(sizeof(dozenal_state_t));
        memset(*context_ptr, 0, sizeof(dozenal_state_t));
        // Do any one-time tasks in here; the inside of this conditional happens only at boot.
    }
    // Do any pin or peripheral setup here; this will be called whenever the watch wakes from deep sleep.
}

void dozenal_face_activate(movement_settings_t *settings, void *context) {
    (void) settings;
    dozenal_state_t *state = (dozenal_state_t *)context;

    // Handle any tasks related to your watch face coming on screen.
    movement_request_tick_frequency(dozenal_tick_frequency);
}

bool dozenal_face_loop(movement_event_t event, movement_settings_t *settings, void *context) {
    dozenal_state_t *state = (dozenal_state_t *)context;
    watch_date_time date_time;
    char buf[11];
    uint32_t tsec;
    double tsub;
    uint8_t dig1, dig2, dig3, dig4;

    switch (event.event_type) {
        case EVENT_ACTIVATE:
        case EVENT_TICK:
            date_time = watch_rtc_get_date_time();
            tsec = (((uint32_t)date_time.unit.hour * 60) + (uint32_t)date_time.unit.minute) * 60 + (uint32_t)date_time.unit.second;
            dig1 = tsec / dig1_sec;
            tsec = tsec % dig1_sec;
            dig2 = tsec / dig2_sec;
            tsec = tsec % dig2_sec;
            dig3 = tsec / dig3_sec;
            tsec = tsec % dig3_sec;
            // leftover subseconds
            tsub = (double)tsec + (double)event.subsecond / (double)dozenal_tick_frequency;
            dig4 = tsub / dig4_sec;
            sprintf(buf, "%c%c%c%c ", dozenal_digits[dig1], dozenal_digits[dig2], dozenal_digits[dig3], dozenal_digits[dig4]);
            watch_display_string(buf, 5);
            break;
        case EVENT_LIGHT_BUTTON_UP:
            // You can use the Light button for your own purposes. Note that by default, Movement will also
            // illuminate the LED in response to EVENT_LIGHT_BUTTON_DOWN; to suppress that behavior, add an
            // empty case for EVENT_LIGHT_BUTTON_DOWN.
            break;
        case EVENT_ALARM_BUTTON_UP:
            // Just in case you have need for another button.
            break;
        case EVENT_TIMEOUT:
            // Your watch face will receive this event after a period of inactivity. If it makes sense to resign,
            // you may uncomment this line to move back to the first watch face in the list:
            // movement_move_to_face(0);
            break;
        case EVENT_LOW_ENERGY_UPDATE:
            // If you did not resign in EVENT_TIMEOUT, you can use this event to update the display once a minute.
            // Avoid displaying fast-updating values like seconds, since the display won't update again for 60 seconds.
            // You should also consider starting the tick animation, to show the wearer that this is sleep mode:
            // watch_start_tick_animation(500);
            break;
        default:
            // Movement's default loop handler will step in for any cases you don't handle above:
            // * EVENT_LIGHT_BUTTON_DOWN lights the LED
            // * EVENT_MODE_BUTTON_UP moves to the next watch face in the list
            // * EVENT_MODE_LONG_PRESS returns to the first watch face (or skips to the secondary watch face, if configured)
            // You can override any of these behaviors by adding a case for these events to this switch statement.
            return movement_default_loop_handler(event, settings);
    }

    // return true if the watch can enter standby mode. Generally speaking, you should always return true.
    // Exceptions:
    //  * If you are displaying a color using the low-level watch_set_led_color function, you should return false.
    //  * If you are sounding the buzzer using the low-level watch_set_buzzer_on function, you should return false.
    // Note that if you are driving the LED or buzzer using Movement functions like movement_illuminate_led or
    // movement_play_alarm, you can still return true. This guidance only applies to the low-level watch_ functions.
    return true;
}

void dozenal_face_resign(movement_settings_t *settings, void *context) {
    (void) settings;
    (void) context;

    // handle any cleanup before your watch face goes off-screen.
}

