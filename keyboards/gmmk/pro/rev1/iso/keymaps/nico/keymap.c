/* Copyright 2021 Glorious, LLC <salman@pcgamingrace.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include QMK_KEYBOARD_H
#include "paddlegame.h"
#include "rgb_matrix_map.h"

// clang-format off
enum custom_keycodes {
#ifdef VIA_ENABLE
    KC_MISSION_CONTROL = USER00,
#else
    KC_MISSION_CONTROL = SAFE_RANGE,
#endif
    KC_LAUNCHPAD
};


#define KC_MCTL KC_MISSION_CONTROL
#define KC_LPAD KC_LAUNCHPAD

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT(
        KC_ESC,  KC_BRID, KC_BRIU, KC_MCTL, KC_LPAD, RGB_VAD, RGB_VAI, KC_MPRV, KC_MPLY, KC_MNXT, KC_MUTE, KC_VOLD, KC_VOLU, KC_DEL,           KC_MUTE,
        KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, KC_EQL,  KC_BSPC,          KC_PGUP,
        KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC,                   KC_PGDN,
        KC_CAPS, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT, KC_NUHS, KC_ENT,           KC_HOME,
        KC_LSFT, KC_NUBS, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH,          KC_RSFT, KC_UP,   KC_END,
        KC_LCTL, KC_LALT, KC_LGUI,                            KC_SPC,                             KC_RGUI, KC_RALT, MO(1),   KC_LEFT, KC_DOWN, KC_RGHT
    ),

    [1] = LAYOUT(
        KC_F13,  RGB_SAD, RGB_SAI, _______, _______, RGB_HUD, RGB_HUI, _______, _______, _______, _______, _______, _______, RGB_TOG,          QK_BOOT,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, TO(2), _______, _______,                   _______,
        _______, _______, RGB_VAD, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______,
        _______, _______, _______, RGB_HUI, _______, _______, _______, NK_TOGG, _______, _______, _______, _______,          _______, RGB_MOD, _______,
        _______, _______, _______,                            _______,                            _______, _______, _______, RGB_SPD, RGB_RMOD, RGB_SPI
    ),

    [2] = LAYOUT(
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,                   _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______, _______, _______,
        _______, _______, _______,                            _______,                            _______, _______, TG(2), _______, _______, _______
    ),
};
// clang-format on

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case KC_MISSION_CONTROL:
            if (record->event.pressed) {
                host_consumer_send(0x29F);
            } else {
                host_consumer_send(0);
            }
            return false;  // Skip all further processing of this key
        case KC_LAUNCHPAD:
            if (record->event.pressed) {
                host_consumer_send(0x2A0);
            } else {
                host_consumer_send(0);
            }
            return false;  // Skip all further processing of this key
        default:
            return true;  // Process all other keycodes normally
    }
}


#ifdef ENCODER_ENABLE
bool encoder_update_user(uint8_t index, bool clockwise) {
    uint8_t mods_state = get_mods();
    if (mods_state & MOD_BIT(KC_LSFT)) { // If holding L shift, encoder changes screen brightness
        unregister_mods(MOD_BIT(KC_LSFT));
        clockwise ? tap_code16(KC_BRIU) : tap_code16(KC_BRID);
        register_mods(MOD_BIT(KC_LSFT));
    } else if (mods_state & MOD_BIT(KC_LGUI)) { // If holding L Cmd, encoder navigates through history
        clockwise ? tap_code16(LGUI(LSFT(KC_W))) : tap_code16(LGUI(KC_W));
    } else {
        switch (get_highest_layer(layer_state)) {
        case 2:
            // Game: Paddle movement
            if (damage_count == 0) {
                if (clockwise) {
                    if (paddle_pos_full < 15) ++paddle_pos_full;
                } else {
                    if (paddle_pos_full > 0) --paddle_pos_full;
                }
            }
            break;
        default:
            clockwise ? tap_code(KC_VOLU) : tap_code(KC_VOLD);
            break;
        }

    }
    return false;
}
#endif

void init_ball(uint8_t i) {
    i &= 1;
    ball[i].on = true;
    ball[i].up = false;
    ball[i].y = 0;
    ball[i].x = rand() % 16;
    
    // Set initial ball state
    if (ball[i].x < 8) {
        ball[i].left = false;
    } else {
        ball[i].x -= 4;
        ball[i].left = true;
    }
    
    // 1/4 chance of being an enemy ball after level 6
    if (level_number > 3) {
        ball[i].enemy = ((rand() % 4) == 0);
    } else {
        ball[i].enemy = false;
    }
}

void hurt_paddle(void) {
    if (paddle_lives > 0) {
        --paddle_lives;
    }
    damage_timer = timer_read();
    damage_count = 10;
    
    // Reset board
    init_ball(0);
    ball[1].on = false;
}

void rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    switch(get_highest_layer(layer_state)) {        
        case 2:
        // Paddle game
        if (!game_start) {
            srand((unsigned int)timer_read());
            
            // Store user light settings
            last_hsv = rgb_matrix_get_hsv();
            rgb_matrix_sethsv_noeeprom(0, 0, 0);

            rgb_value.r = 0xff;
            rgb_value.g = 0x00;
            rgb_value.b = 0x00;
            
            paddle_pos_full = 8;
            paddle_lives = 4;
            bounce_count = 0;
            level_number = 0;
            damage_count = 0;

            init_ball(0);
            ball[1].on = false;
            ball_timer = timer_read();

            game_start = true;
        }
        
        // Set level indicator
        if (level_number < 12) {
            rgb_matrix_set_color(GAME_R0[level_number], RGB_BLUE);
        }
        
        // Set life bar
        for (uint8_t i=0; i < paddle_lives ; i++) {
            rgb_matrix_set_color(GAME_LIVES[i], RGB_GREEN);
        }
        
        uint8_t paddle_pos = paddle_pos_full >> 1;

        if (damage_count > 0) {
            // Flash paddle when receiving damage
            if (timer_elapsed(damage_timer) > 500) {
                --damage_count;
                damage_timer = timer_read();
            }
            if ((damage_count & 1) == 0) {
                for (uint8_t i=0; i < 3 ; i++) {
                    rgb_matrix_set_color(GAME_PADDLE[paddle_pos + i], RGB_RED);
                }
            }
            if (damage_count == 0) {
                ball_timer = timer_read();
            }
            

        } else if (paddle_lives == 0) {
            // Game over
            for (uint8_t i=0; i<sizeof(LED_GAME_OVER)/sizeof(LED_GAME_OVER[0]); i++) {
                rgb_matrix_set_color(LED_GAME_OVER[i], RGB_RED);
            }
        
        } else if (level_number >= 12) {
            // You win
            if (rgb_value.r == 0xff && rgb_value.g < 0xff) {
                if (rgb_value.b > 0) { --rgb_value.b; }
                else { ++rgb_value.g; }
            } else if (rgb_value.g == 0xff && rgb_value.b < 0xff) {
                if (rgb_value.r > 0) { --rgb_value.r; }
                else { ++rgb_value.b; }
            } else if (rgb_value.b == 0xff && rgb_value.r < 0xff) {
                if (rgb_value.g > 0) { --rgb_value.g; }
                else { ++rgb_value.r; }
            }
            
            for (uint8_t i=0; i < 3 ; i++) {
                rgb_matrix_set_color(GAME_PADDLE[paddle_pos + i], rgb_value.r, rgb_value.g, rgb_value.b);
            }
            rgb_matrix_set_color(GAME_SMILE1[paddle_pos], rgb_value.r, rgb_value.g, rgb_value.b);
            rgb_matrix_set_color(GAME_SMILE1[paddle_pos + 3], rgb_value.r, rgb_value.g, rgb_value.b);
            rgb_matrix_set_color(GAME_SMILE2[paddle_pos], rgb_value.r, rgb_value.g, rgb_value.b);
            rgb_matrix_set_color(GAME_SMILE2[paddle_pos + 3], rgb_value.r, rgb_value.g, rgb_value.b);
        
        } else {
            // normal game loop
            
            // Set paddle position
            for (uint8_t i=0; i < 3 ; i++) {
                rgb_matrix_set_color(GAME_PADDLE[paddle_pos + i], RGB_GREEN);
            }
            
            // Ball movement logic happens at intervals
            if (timer_elapsed(ball_timer) > GAME_TIMER[level_number]) {
                for (int i=0; i<2; ++i) {
                    if (ball[i].on) {
                        // Ball movement
                        if (ball[i].up) {
                            if (ball[i].y > 0) {
                                --ball[i].y;
                                if (!ball[i].left) ++ball[i].x;
                            } else {
                                // Count reflections. If > 10, increase level
                                ++bounce_count;
                                if (bounce_count >= 10) {
                                    bounce_count = 0;
                                    ++level_number;
                                }
                                ball[i].on = false;
                            }
                        } else {
                            ++ball[i].y;
                            if (ball[i].left) --ball[i].x;
                            if (ball[i].y > 4) {
                                // Remove a life if ball isn't returned and isn't enemy
                                if (!ball[i].enemy) {
                                    hurt_paddle(); 
                                    i = 2;
                                } else {
                                    ball[i].on = false;
                                }
                            }
                        }
                    }
                }
                if (ball[0].y == 4 && !ball[1].on) {
                    init_ball(1);
                }
                if (ball[1].y == 4 && !ball[0].on) {
                    init_ball(0);
                }
                if (!ball[0].on && !ball[1].on) {
                    init_ball(0);
                }
                ball_timer = timer_read();
            }
            
            // Other ball stuff
            for (int i=0; i<2; ++i) {
                if (ball[i].on) {
                    // Ball deflection logic
                    if (!ball[i].up && ball[i].y == 4 && (ball[i].x == paddle_pos || ball[i].x == paddle_pos - 1 || ball[i].x == paddle_pos + 1)) {
                        if (!ball[i].enemy) {
                            --ball[i].y;
                            if (!ball[i].left) { ++ball[i].x; }
                            ball[i].up = true;
                        } else {
                            hurt_paddle();
                            i = 2;
                        }
                    }

                    // Ball display
                    switch (ball[i].y) {
                        case 0:
                        if (ball[i].enemy) {
                            rgb_matrix_set_color(GAME_R0[ball[i].x], RGB_RED);
                        } else {
                            rgb_matrix_set_color(GAME_R0[ball[i].x], RGB_WHITE);
                        }
                        break;

                        case 1:
                        if (ball[i].enemy) {
                            rgb_matrix_set_color(GAME_R1[ball[i].x], RGB_RED);
                        } else {
                            rgb_matrix_set_color(GAME_R1[ball[i].x], RGB_WHITE);
                        }
                        break;

                        case 2:
                        if (ball[i].enemy) {
                            rgb_matrix_set_color(GAME_R2[ball[i].x], RGB_RED);
                        } else {
                            rgb_matrix_set_color(GAME_R2[ball[i].x], RGB_WHITE);
                        }
                        break;

                        case 3:
                        if (ball[i].enemy) {
                            rgb_matrix_set_color(GAME_R3[ball[i].x], RGB_RED);
                        } else {
                            rgb_matrix_set_color(GAME_R3[ball[i].x], RGB_WHITE);
                        }
                        break;

                        case 4:
                        if (ball[i].enemy) {
                            rgb_matrix_set_color(GAME_R4[ball[i].x], RGB_RED);
                        } else {
                            rgb_matrix_set_color(GAME_R4[ball[i].x], RGB_WHITE);
                        }
                        break;
                    }
                }
            }				
        }

        
        break;
        
        default:
        fn_active = false;
        if (game_start) {
            // Reset lighting settings
            game_start = false;
            rgb_matrix_sethsv_noeeprom(last_hsv.h, last_hsv.s, last_hsv.v);
        }
        break;			
    }

    if (IS_HOST_LED_ON(USB_LED_CAPS_LOCK)) {
        if (!caps_active) {
            caps_active = true;
            caps_flash_on = true;
            caps_flasher = timer_read();
        }
        if (timer_elapsed(caps_flasher) > 500) {
            caps_flasher = timer_read();
            caps_flash_on = !caps_flash_on;
        }

        rgb_matrix_set_color(LED_CAPS, RGB_WHITE);
        if (caps_flash_on) {
            for (uint8_t i=0; i<sizeof(LED_SIDE_LEFT)/sizeof(LED_SIDE_LEFT[0]); i++) {
                rgb_matrix_set_color(LED_SIDE_LEFT[i], RGB_RED);
                rgb_matrix_set_color(LED_SIDE_RIGHT[i], RGB_RED);
            }
        } else {
            for (uint8_t i=0; i<sizeof(LED_SIDE_LEFT)/sizeof(LED_SIDE_LEFT[0]); i++) {
                rgb_matrix_set_color(LED_SIDE_LEFT[i], 0, 0, 0);
                rgb_matrix_set_color(LED_SIDE_RIGHT[i], 0, 0, 0);
            }
        }
    } else {
        caps_active = false;
    }
    if (IS_HOST_LED_ON(USB_LED_SCROLL_LOCK)) {
        rgb_matrix_set_color(LED_F10,  RGB_WHITE);
    }
    if (keymap_config.no_gui) {
        rgb_matrix_set_color(LED_LWIN, RGB_RED);  //light up Win key when disabled
    }
}
