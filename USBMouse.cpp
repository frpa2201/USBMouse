#include "USBMouse.h"

USBMouse::USBMouse() {
    _buttons = 0;
}

void USBMouse::move(int x, int y) {
    while(x > 127) {
        mouse(127, 0, _buttons, 0);
        x = x - 127;
    }
    while(x < -128) {
        mouse(-128, 0, _buttons, 0);
        x = x + 128;
    }
    while(y > 127) {
        mouse(0, 127, _buttons, 0);
        y = y - 127;
    }
    while(y < -128) {
        mouse(0, -128, _buttons, 0);
        y = y + 128;
    }
    mouse(x, y, _buttons, 0);
}

void USBMouse::scroll(int z) {
    while(z > 127) {
        mouse(0, 0, _buttons, 127);
        z = z - 127;
    }
    while(z < -128) {
        mouse(0, 0, _buttons, -128);
        z = z + 128;
    }
    mouse(0, 0, _buttons, z);
}

void USBMouse::buttons(int left, int middle, int right) {
    int _buttons = 0;
    if(left) {
        _buttons |= MOUSE_L;
    }
    if(middle) {
        _buttons |= MOUSE_M;
    }
    if(right) {
        _buttons |= MOUSE_R;
    }
    mouse(0,0, _buttons, 0);    
}    
