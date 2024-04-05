#pragma once
#include "Windows.h"





extern HANDLE hConsole;

void SendKeyboardInput();
void PrintText(const char* text, WORD color = 15);

