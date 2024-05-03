#include <Windows.h>
#include <vector>

#include "GetKeyboardInput.h"
#include "Controller.h"
#include "Controller.h"













bool getting_input = false;
bool input_init = true;


void VirtualKeyboard::SendKeyboardInput()
{
    

    //current_button = 15;
    //Keep the window in which the input will be "sent"
    HWND foreground_window = GetForegroundWindow();

    // Get virtual keyboard window to start typing
    HWND qt_keyboard_window = FindWindow(NULL, L"Virtual Keyboard");

    if (input_init)
    {
        

        SetWindowPos(qt_keyboard_window, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        ShowWindow(qt_keyboard_window, SW_MAXIMIZE);

        input_init = false;
    }
    


    // Space character
    if (user->IsButtonJustDown(GAMEPAD_A))
    {
        SimulateKey(VK_SPACE);
    }

    // Space character
    if (user->IsButtonJustDown(GAMEPAD_Y)) 
    {
        virtual_input->insert(" ");
    }


    // Backspace
    if (SmoothNavigation(GAMEPAD_X, 90, 90)) 
    {
        virtual_input->backspace();
    }

    // Select backward
    if (SmoothNavigation(GAMEPAD_LB, 90, 90))
    {
        virtual_input->cursorBackward(true, 1);
    }

    // Select forward
    if (SmoothNavigation(GAMEPAD_RB, 90, 90))
    {
        virtual_input->cursorForward(true, 1);
    }


    //Caps lock
    if (user->IsButtonJustDown(GAMEPAD_LEFT_THUMB))
    {
        caps_lock = !caps_lock;
        CreateKeyboardUI();
        
    }



    //Changing font size because the virtual keyboard looks too small on the default font
    /*
    HANDLE Hout = ::GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_FONT_INFOEX Font = { sizeof(Font) };
    ::GetCurrentConsoleFontEx(Hout, FALSE, &Font);
    Font.dwFontSize = { 38, 38 };
    SetCurrentConsoleFontEx(Hout, 0, &Font);
    */

    if (user->IsButtonJustDown(GAMEPAD_START)) //Input finished
    {
        

        getting_input = false;
        input_init = true;
        std::string temp_input = virtual_input->text().toStdString();
        
        //std::cout << "Keyboard input finished with: " << temp_input << std::endl;

        SetWindowPos(qt_keyboard_window, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        SetWindowPos(foreground_window, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        SetForegroundWindow(foreground_window);
        SetWindowPos(foreground_window, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);


        //https://stackoverflow.com/questions/49503240/passing-a-string-into-sendinput-c
        std::wstring msg(temp_input.begin(), temp_input.end());

        std::vector<INPUT> vec;
        for (auto ch : msg)
        {
            INPUT input = { 0 };
            input.type = INPUT_KEYBOARD;
            input.ki.dwFlags = KEYEVENTF_UNICODE;
            input.ki.wScan = ch;
            vec.push_back(input);

            input.ki.dwFlags |= KEYEVENTF_KEYUP;
            vec.push_back(input);
        }


        SetForegroundWindow(foreground_window);
        this->hide();
        SendInput(vec.size(), vec.data(), sizeof(INPUT));
        virtual_input->clear();
    }


    return;
    


    /*
    UpdateKeyboardMenu();
    while (getting_input)
    {



        for (int i = 0; i < 40; i++)
        {


            if (user->IsButtonJustDown(GAMEPAD_A))
            {

                if (caps_lock)
                    temp_input = temp_input + CapQWERTY[current_button];
                else
                    temp_input = temp_input + NoCapQWERTY[current_button];

                UpdateKeyboardMenu();
            }


            if (SmoothNavigation(GAMEPAD_DPAD_UP, 90, 90))
            {
                if (current_button >= 10)
                    current_button -= 10;
                else
                    current_button += 30;

                UpdateKeyboardMenu();
            }

            if (SmoothNavigation(GAMEPAD_DPAD_DOWN, 90, 90))
            {
                if (current_button < 30)
                    current_button += 10;
                else
                    current_button -= 30;

                UpdateKeyboardMenu();
            }

            if (SmoothNavigation(GAMEPAD_DPAD_LEFT, 90, 90))
            {
                if ((current_button % 10) == 0)
                    current_button = current_button + 9;
                else
                    current_button -= 1;

                UpdateKeyboardMenu();
            }

            if (SmoothNavigation(GAMEPAD_DPAD_RIGHT, 90, 90))
            {
                if ((current_button % 10) == 9)
                    current_button = current_button - (current_button % 10);
                else
                    current_button += 1;

                UpdateKeyboardMenu();
            }
        }


        if (user->IsButtonJustDown(GAMEPAD_Y)) // Space
        {
            temp_input = temp_input + " ";
            UpdateKeyboardMenu();
        }

        if (!temp_input.empty() && SmoothNavigation(GAMEPAD_X, 90, 90)) //Backspace
        {
            temp_input.resize(temp_input.size() - 1);
            UpdateKeyboardMenu();
        }

        if (user->IsButtonJustDown(GAMEPAD_LB)) //Caps lock
        {
            caps_lock = !caps_lock;
            UpdateKeyboardMenu();
        }




        if (user->IsButtonJustDown(GAMEPAD_START)) //Input finished
        {
            getting_input = false;
            SetWindowPos(GetConsoleWindow(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
            SetWindowPos(foreground_window, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
            SetForegroundWindow(foreground_window);
            SetWindowPos(foreground_window, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);


            //https://stackoverflow.com/questions/49503240/passing-a-string-into-sendinput-c
            std::wstring msg(temp_input.begin(), temp_input.end());
            
            std::vector<INPUT> vec;
            for (auto ch : msg)
            {
                INPUT input = { 0 };
                input.type = INPUT_KEYBOARD;
                input.ki.dwFlags = KEYEVENTF_UNICODE;
                input.ki.wScan = ch;
                vec.push_back(input);

                input.ki.dwFlags |= KEYEVENTF_KEYUP;
                vec.push_back(input);
            }


            SetForegroundWindow(foreground_window);
            SendInput(vec.size(), vec.data(), sizeof(INPUT));
        }
    }


    // Resetting font to default
    //Font.dwFontSize = {8, 16};
    //SetCurrentConsoleFontEx(Hout, 0, &Font);


    */
}