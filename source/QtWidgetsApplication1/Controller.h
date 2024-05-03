#ifndef _XBOX_CONTROLLER_H_
#define _XBOX_CONTROLLER_H_

// No MFC
#define WIN32_LEAN_AND_MEAN

// We need the Windows Header and the XInput Header
//#include <span class="code-keyword"><windows.h></span>
//#include <span class="code-keyword"><XInput.h></span>

#include <Windows.h>
#include <Xinput.h>

// Now, the XInput Library
// NOTE: COMMENT THIS OUT IF YOU ARE NOT USING
// A COMPILER THAT SUPPORTS THIS METHOD OF LINKING LIBRARIES
#pragma comment(lib, "XInput.lib")


enum pad_button {
    GAMEPAD_DPAD_UP,
    GAMEPAD_DPAD_DOWN,
    GAMEPAD_DPAD_LEFT,
    GAMEPAD_DPAD_RIGHT,
    GAMEPAD_START,
    GAMEPAD_BACK,
    GAMEPAD_LEFT_THUMB,
    GAMEPAD_RIGHT_THUMB,
    GAMEPAD_LB,
    GAMEPAD_RB,
    GAMEPAD_A,
    GAMEPAD_B,
    GAMEPAD_X,
    GAMEPAD_Y,
};






// XBOX Controller Class Definition
class XBOXController
{
private:
    XINPUT_STATE controller_state;
    int controller_number;
public:
    float left_trigger = 0.0f;
    float right_trigger = 0.0f;

    XBOXController(int playerNumber);
    XINPUT_STATE GetState();
    bool IsConnected();
    void Vibrate(const int leftVal = 0, const int rightVal = 0);
    SHORT GetLeftAnalogX();
    SHORT GetLeftAnalogY();
    SHORT GetRightAnalogX();
    SHORT GetRightAnalogY();
    bool IsButtonDown(const int button);
    bool IsButtonJustUp(const int button);
    bool IsButtonJustDown(const int button);
};


void ResetKeyPressCounter();
bool SmoothNavigation(int controller_button, int timer1 = 90, int timer2 = 40);



extern XBOXController* user;
extern bool to_release_hold_key;
extern bool hold_key_down;


void SimulateKey(DWORD key);
void SimulateMouseKey(DWORD press, DWORD release);
void SimulateMouseWheel(const int ammount);
void SimulateHoldKey(DWORD key);
void ReleaseHoldKey();

#endif