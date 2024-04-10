#include "GetKeyboardInput.h"
#include <iostream>
#include <Windows.h>
#include "Controller.h"
#include "Controller.h"
#include <vector>


using std::cout;
using std::endl;



const char NoCapQWERTY[] =
{
    '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', 
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', '-', 
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 
};


const char CapQWERTY[] =
{
    '!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P',
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', '_',      //'\"',
    'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?',
};


const DWORD DWORDKeys[] = 
{
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
    0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0xBA, 0xBB,
    0xBC, 0xBD, 0xBE, 0xBF, 0xC0, 0xDB, 0xDC, 0xDD, 0xDE, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
    0x6A, 0x6D, 0x6F, 0x6E
};



//QWERTY variables
int current_button = 15;
bool caps_lock = false;
std::string temp_input = "";








HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
// Prints with the specified color and then resets the color back to white (15)
void PrintText(const char* text, WORD color)
{
    SetConsoleTextAttribute(hConsole, color);
    std::cout << text;
    SetConsoleTextAttribute(hConsole, 15); // Reset text color to white
}



void UpdateKeyboardMenu()
{
    return;
    system("cls");

    // Create Screen Buffer
    /*wchar_t* screen = new wchar_t[50 * 50];
    for (int i = 0; i < 50 * 50; i++) screen[i] = L' ';
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole);
    DWORD dwBytesWritten = 0;*/


    

    
    cout << endl << endl << endl;
    cout << "   ";
    for (int i = 0; i < 40; i++)
    {
        if (i == 10 || i == 20 || i == 30 || i == 40)
            cout << endl << endl << "   ";

        if (!caps_lock)
        {
            if (current_button == i)
            {
                std::string temp;
                temp[0] = NoCapQWERTY[i];
                //WriteConsoleOutputCharacter(hConsole, screen, 50 * 50, { 0,0 }, &dwBytesWritten);
                PrintText(temp.c_str(), 12); cout << "  ";
            }
            else
            {
                cout << NoCapQWERTY[i] << "  ";
            }
                

        }
        else
        {
            if (current_button == i)
            {
                std::string temp;
                temp[0] = CapQWERTY[i];
                PrintText(temp.c_str(), 12); cout << "  ";
            }
            else
            {
                cout << CapQWERTY[i] << "  ";
            }

        }
    }

    cout << endl << endl << "Space: Triangle/Y   Caps Lock: L1/LB\nDelete: Square/X   Confirm input: Start" << endl;
    cout << "---------------------------------------------------------------------------------" << endl;
    PrintText("Your input: ", 12); cout << temp_input << "|" << endl;
}






bool getting_input = false;
void VirtualKeyboard::SendKeyboardInput()
{
    this->show();

    


    


    temp_input = "";
    current_button = 15;

    //Keep the window in which the input will be "sent"
    HWND foreground_window = GetForegroundWindow();

    // Get virtual keyboard window to start typing
    HWND qt_keyboard_window = FindWindow(NULL, L"Virtual Keyboard");
    
    SetWindowPos(qt_keyboard_window, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    ShowWindow(qt_keyboard_window, SW_MAXIMIZE);

   
    



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

        temp_input = virtual_input->text().toStdString();
        std::cout << "Keyboard input finished with: " << temp_input << std::endl;

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
    }


    return;
    



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
}