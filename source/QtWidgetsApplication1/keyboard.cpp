#include "keyboard.h"


#define MAX_KEYS 255




struct Keyboard {
	bool down;
	bool up;
};

Keyboard keys[MAX_KEYS];







bool IsKeyDown(DWORD key)
{
	if (GetAsyncKeyState(key)  /*GetKeyState(key) & 0x8000*/)
	{
		keys[key].up = false;
		return true;
	}
	else
	{
		keys[key].up = true;
		return false;
	}
}



bool IsKeyJustDown(DWORD key)
{
	if ((GetKeyState(key) & 0x8000) && keys[key].down == false)
	{
		keys[key].down = true;
		keys[key].up = false;
		return true;
	}

	if (keys[key].down == true && !(GetKeyState(key) & 0x8000))
	{
		keys[key].down = false;
		keys[key].up = true;
	}
	return false;
}
