//#include <span class="code-string">"CXBOXController.h"</span>

#include "Controller.h"
#include <iostream>


#define MAX_BUTTONS 14


XBOXController* user;
bool to_release_hold_key = false;
bool hold_key_down = false;





void SimulateKey(DWORD key)
{
    INPUT myInput;

    // Set up a generic keyboard event.
    myInput.type = INPUT_KEYBOARD;
    myInput.ki.wScan = 0; // hardware scan code for key
    myInput.ki.time = 0;
    myInput.ki.dwExtraInfo = 0;

    // Press the key
    myInput.ki.wVk = (WORD)key;
    myInput.ki.dwFlags = 0; // 0 for key press
    SendInput(1, &myInput, sizeof(INPUT));

    // Release the key
    myInput.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &myInput, sizeof(INPUT));
}




INPUT hold_input;
bool key_held_already = false;
void SimulateHoldKey(DWORD key)
{
    // Set up a keyboard event for the hold_key.
    hold_input.type = INPUT_KEYBOARD;
    hold_input.ki.wScan = 0; // hardware scan code for key
    hold_input.ki.time = 0;
    hold_input.ki.dwExtraInfo = 0;

    // Press the hold_key
    hold_input.ki.wVk = (WORD)key;
    hold_input.ki.dwFlags = 0; // 0 for key press
    SendInput(1, &hold_input, sizeof(INPUT));
}


void ReleaseHoldKey()
{
    hold_input.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
    SendInput(1, &hold_input, sizeof(INPUT));
}





void SimulateMouseKey(DWORD press, DWORD release)
{
    INPUT myInput;

    myInput.type = INPUT_MOUSE;
    myInput.mi.mouseData = 0;
    myInput.mi.time = 0;
    myInput.mi.dwExtraInfo = 0;
    
    //Press the key
    myInput.mi.dwFlags = press;
    SendInput(1, &myInput, sizeof(INPUT));

    // Release the key
    myInput.mi.dwFlags = release;
    SendInput(1, &myInput, sizeof(INPUT));
}





void SimulateMouseWheel(const int ammount)
{
    INPUT myInput;

    myInput.type = INPUT_MOUSE;
    myInput.mi.mouseData = ammount;
    myInput.mi.time = 0;
    myInput.mi.dwExtraInfo = 0;


    myInput.mi.dwFlags = MOUSEEVENTF_WHEEL;
    SendInput(1, &myInput, sizeof(INPUT));
}













struct xbox_button {
    const char* text;
    int button;
    bool down;
    bool up;
};


struct xbox_button Button[] =
{
    {"GAMEPAD_DPAD_UP", XINPUT_GAMEPAD_DPAD_UP},
    {"GAMEPAD_DPAD_DOWN", XINPUT_GAMEPAD_DPAD_DOWN},
    {"GAMEPAD_DPAD_LEFT", XINPUT_GAMEPAD_DPAD_LEFT},
    {"GAMEPAD_DPAD_RIGHT", XINPUT_GAMEPAD_DPAD_RIGHT},
    {"GAMEPAD_START", XINPUT_GAMEPAD_START},
    {"GAMEPAD_BACK", XINPUT_GAMEPAD_BACK},
    {"GAMEPAD_LEFT_THUMB", XINPUT_GAMEPAD_LEFT_THUMB},
    {"GAMEPAD_RIGHT_THUMB", XINPUT_GAMEPAD_RIGHT_THUMB},
    {"GAMEPAD_LB", XINPUT_GAMEPAD_LEFT_SHOULDER},
    {"GAMEPAD_RB", XINPUT_GAMEPAD_RIGHT_SHOULDER},
    {"GAMEPAD_A", XINPUT_GAMEPAD_A},
    {"GAMEPAD_B", XINPUT_GAMEPAD_B},
    {"GAMEPAD_X", XINPUT_GAMEPAD_X},
    {"GAMEPAD_Y", XINPUT_GAMEPAD_Y}
};








bool rest = true;
DWORD rest_time = GetTickCount() + 150;
int keypresses = 0;


void ResetKeyPressCounter() //should be called in loop
{
    if (Button[GAMEPAD_DPAD_UP].up == true && Button[GAMEPAD_DPAD_DOWN].up == true && Button[GAMEPAD_DPAD_LEFT].up == true && Button[GAMEPAD_DPAD_RIGHT].up == true)
    {
        keypresses = 0;
    }
}



bool SmoothNavigation(int controller_button, int timer1, int timer2)
{

    if (rest)
    {
        keypresses++;

        if (keypresses < 25)
            rest_time = GetTickCount() + timer1;
        else
            rest_time = GetTickCount() + timer2;

    }

    if (user->IsButtonDown(controller_button))
    {
        rest = GetTickCount() >= rest_time;
        return (GetTickCount() >= rest_time);
    }
    else
    {
        return false;
    }
}














XBOXController::XBOXController(int playerNumber)
{
    // Set the Controller Number
    controller_number = playerNumber - 1;
}

XINPUT_STATE XBOXController::GetState()
{
    // Zeroise the state
    ZeroMemory(&controller_state, sizeof(XINPUT_STATE));

    // Get the state
    XInputGetState(controller_number, &controller_state);

    return controller_state;
}

bool XBOXController::IsConnected()
{
    // Zeroise the state
    ZeroMemory(&controller_state, sizeof(XINPUT_STATE));

    // Get the state
    DWORD Result = XInputGetState(controller_number, &controller_state);

    if (Result == ERROR_SUCCESS)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void XBOXController::Vibrate(const int leftVal, const int rightVal)
{
    // Create a Vibraton State
    XINPUT_VIBRATION Vibration;

    // Zeroise the Vibration
    ZeroMemory(&Vibration, sizeof(XINPUT_VIBRATION));

    // Set the Vibration Values
    Vibration.wLeftMotorSpeed = leftVal;
    Vibration.wRightMotorSpeed = rightVal;

    // Vibrate the controller
    XInputSetState(controller_number, &Vibration);
}




SHORT XBOXController::GetLeftAnalogX()
{
    return this->GetState().Gamepad.sThumbLX;
}



SHORT XBOXController::GetLeftAnalogY()
{
    return this->GetState().Gamepad.sThumbLY;
}



SHORT XBOXController::GetRightAnalogX()
{
    return this->GetState().Gamepad.sThumbRX;
}



SHORT XBOXController::GetRightAnalogY()
{
    return this->GetState().Gamepad.sThumbRY;
}



bool XBOXController::IsButtonDown(const int button)
{
    if (this->GetState().Gamepad.wButtons & Button[button].button)
    {
        Button[button].up = false;
        return true;
    }
    else
    {
        Button[button].up = true;
        return false;
    }
}


bool XBOXController::IsButtonJustUp(const int button)
{
    if (this->GetState().Gamepad.wButtons & Button[button].button)
    {
        Button[button].down = true;
        Button[button].up = false;
    }


    if (Button[button].down == true && !(this->GetState().Gamepad.wButtons & Button[button].button))
    {
        Button[button].down = false;
        Button[button].up = true;
        return true;
    }
    return false;
}



bool XBOXController::IsButtonJustDown(const int button)
{
    if ((this->GetState().Gamepad.wButtons & Button[button].button) && Button[button].down == false)
    {
        Button[button].down = true;
        Button[button].up = false;
        return true;
    }

    if (Button[button].down == true && !(this->GetState().Gamepad.wButtons & Button[button].button))
    {
        Button[button].down = false;
        Button[button].up = true;
    }
    return false;
}