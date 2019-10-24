#include "emulator.hpp"

extern "C" {
  #define EE ((emulator_t *)e)
  void * WASM_EXPORT get_emulator() { return new emulator_t {}; }

  void WASM_EXPORT button_down(void * e, u16 button) {
    EE->joypad.button_down((Buttons)button);
  }
  void WASM_EXPORT button_up(void * e, u16 button) {
    EE->joypad.button_up((Buttons)button);
  }
  void WASM_EXPORT step_frame(void * e) {
    EE->step(456 * 154); // fudge factor??
  }
  void WASM_EXPORT reset(void * e, u8 * cart, u32 len) {
    EE->load_cart(cart, len);
  }
}
