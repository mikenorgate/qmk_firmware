/* Copyright 2021 Glorious, LLC <salman@pcgamingrace.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include QMK_KEYBOARD_H

enum my_keycodes {
  RGB_TOGI = SAFE_RANGE, //Toggle RGB but keep indicators
};

static HSV m_hsv;
static bool m_enabled;
static int m_mode;

// Each layer gets a name for readability, which is then used in the keymap matrix below.
// The underscores don't mean anything - you can have a layer called STUFF or any other name.
#define _BL 0
#define _FL 1

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  /* Keymap _BL: Base Layer (Default Layer)
   */
[_BL] = LAYOUT(
  KC_GESC,  KC_F1,    KC_F2,    KC_F3,    KC_F4,    KC_F5,    KC_F6,    KC_F7,    KC_F8,    KC_F9,    KC_F10,   KC_F11,   KC_F12,   KC_PSCR,  KC_DEL,   KC_INS,   KC_HOME,  KC_END,
  KC_GRV,   KC_1,     KC_2,     KC_3,     KC_4,     KC_5,     KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_MINS,  KC_EQL,   KC_BSPC,  KC_NLCK,  KC_PSLS,  KC_PAST,  KC_PMNS,
  KC_TAB,   KC_Q,     KC_W,     KC_E,     KC_R,     KC_T,     KC_Y,     KC_U,     KC_I,     KC_O,     KC_P,     KC_LBRC,  KC_RBRC,  KC_ENT,   KC_P7,    KC_P8,    KC_P9,    KC_PPLS,
  KC_CAPS,  KC_A,     KC_S,     KC_D,     KC_F,     KC_G,     KC_H,     KC_J,     KC_K,     KC_L,     KC_SCLN,  KC_QUOT,  KC_NUHS,            KC_P4,    KC_P5,    KC_P6,
  KC_LSFT,  KC_NUBS,  KC_Z,     KC_X,     KC_C,     KC_V,     KC_B,     KC_N,     KC_M,     KC_COMM,  KC_DOT,   KC_SLSH,  KC_RSFT,  KC_UP,    KC_P1,    KC_P2,    KC_P3,    KC_PENT,
  KC_LCTL,  KC_LGUI,  KC_LALT,                      KC_SPC,                                 KC_RALT,  MO(_FL),  KC_RCTL,  KC_LEFT,  KC_DOWN,  KC_RGHT,  KC_P0,    KC_PDOT),

  /* Keymap _FL: Function Layer
   */
[_FL] = LAYOUT(
    RESET,  _______,  _______,  _______,  _______,  KC_MSEL,  KC_MPRV,  KC_MPLY,  KC_MNXT,  _______,  KC_MUTE,  KC_VOLD,  KC_VOLU,   _______,   _______,  _______,  _______,  _______,
  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,   _______,  _______,  _______,  _______,  _______,
  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,   _______,  _______,  _______,  _______,  _______,
  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,             _______,  _______,  _______,
  _______,  RGB_TOGI, RGB_SAI,  RGB_SAD,  RGB_HUI,  RGB_HUD,  _______,  _______,  _______,  _______,  _______,  _______,  _______,   RGB_VAI,  _______,  _______,  _______,  _______,
  _______,  UC_M_WI,  RGB_M_P,                      _______,                                KC_LOCK,  _______,  _______, RGB_RMOD,   RGB_VAD,  RGB_MOD,  _______,  _______)
};

void rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
  // Only light keys available on the current layer 
  if (get_highest_layer(layer_state) > 0) {
      uint8_t layer = get_highest_layer(layer_state);

      for (uint8_t row = 0; row < MATRIX_ROWS; ++row) {
          for (uint8_t col = 0; col < MATRIX_COLS; ++col) {
              uint8_t index = g_led_config.matrix_co[row][col];

              if (index >= led_min && index <= led_max && index != NO_LED &&
              keymap_key_to_keycode(layer, (keypos_t){col,row}) <= KC_TRNS) {
                RGB_MATRIX_INDICATOR_SET_COLOR(index, 0, 0, 0);
              }
          }
      }
  }

  HSV hsv = rgb_matrix_get_hsv();
  RGB rgb = hsv_to_rgb(hsv);

  // Set the underglow to be the same as other keys
  for (uint8_t i = led_min; i <= led_max; i++) {
      if (g_led_config.flags[i] & LED_FLAG_UNDERGLOW) {
          RGB_MATRIX_INDICATOR_SET_COLOR(i, rgb.r, rgb.g, rgb.b);
      }
  }

  // Set caps and num lock indicators to have oposite saturation
  if(!m_enabled)
    hsv = m_hsv; // If not enabled the current HSV will be black, so use HSV from before disable
  if(hsv.s > 128)
    hsv.s = 0;
  else
    hsv.s = 255;
  if(hsv.v < 128)
    hsv.v = 128;
  rgb = hsv_to_rgb(hsv); 

  led_t host_state = host_keyboard_led_state();
  if (host_state.caps_lock) {
      RGB_MATRIX_INDICATOR_SET_COLOR(54, rgb.r, rgb.g, rgb.b);
  }

  if (host_state.num_lock) {
      RGB_MATRIX_INDICATOR_SET_COLOR(32, rgb.r, rgb.g, rgb.b);
  }
}

void keyboard_post_init_user(void) {
  m_hsv = rgb_matrix_get_hsv();
  m_enabled = rgb_matrix_is_enabled();
  m_mode = rgb_matrix_get_mode();
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  switch (keycode) {
    case RGB_TOGI:
      if (record->event.pressed) {
        if(m_enabled){
          m_hsv = rgb_matrix_get_hsv();
          m_mode = rgb_matrix_get_mode();
          rgb_matrix_mode_noeeprom(RGB_MATRIX_SOLID_COLOR);
          rgb_matrix_sethsv_noeeprom(HSV_OFF);
          m_enabled = false;
        } else{
          rgb_matrix_mode_noeeprom(m_mode);
          rgb_matrix_sethsv_noeeprom(m_hsv.h, m_hsv.s, m_hsv.v);
          m_enabled = true;
        }
      }
      return false; // Skip all further processing of this key
    //Don't process other RGB keys if not enabled
    case RGB_MODE_FORWARD:
    case RGB_MODE_REVERSE:
    case RGB_HUI:
    case RGB_HUD:
    case RGB_SAI:
    case RGB_SAD:
    case RGB_VAI:
    case RGB_VAD:
    case RGB_MODE_PLAIN:
    case RGB_MODE_BREATHE:
    case RGB_MODE_RAINBOW:
    case RGB_MODE_SWIRL:
    case RGB_MODE_SNAKE:
    case RGB_MODE_KNIGHT:
    case RGB_MODE_XMAS:
    case RGB_MODE_GRADIENT:
    case RGB_MODE_RGBTEST:
      return m_enabled;
    default:
      return true; // Process all other keycodes normally
  }
}