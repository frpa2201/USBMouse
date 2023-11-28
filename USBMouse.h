#include "usbhid.h"

#ifndef MBED_USBMOUSE_H
#define MBED_USBMOUSE_H

/* Class: USBMouse
 * Emulate a USB Mouse HID device
 *
 * Example:
 * > #include "mbed.h"
 * > #include "USBMouse.h"
 * > 
 * > USBMouse mouse;
 * >
 * > int main() {
 * >     while(1) {
 * >         mouse.move(10, 0);
 * >         wait(2);
 * >     }
 * > }
 */
class USBMouse : private usbhid {
public:
    /* Constructor: USBMouse
     * Create a USB Mouse using the mbed USB Device interface
     */
    USBMouse();
    
    /* Function: move
     * Move the mouse
     *
     * Variables:
     *  x - Distance to move in x-axis 
     *  y - Distance to move in y-axis
     */
    void move(int x, int y);
    
    /* Function: scroll
     * Scroll the scroll wheel
     *
     * Variables:
     *  z - Distance to scroll scroll wheel
     */
    void scroll(int z);
    
    /* Function: buttons
     * Set the state of the buttons
     *
     * Variables:
     *  left - set the left button as down (1) or up (0)
     *  middle - set the middle button as down (1) or up (0)
     *  right - set the right button as down (1) or up (0)
     */
    void buttons(int left, int middle, int right);    
    
private:
    int _buttons;
};

#endif
