#include "GamepadInputChecks.h"

#include <Windows.h>
#include <iostream>


#include "Controller.h"
#include "GetKeyboardInput.h"


bool cursor_mode = true;


float rest_x = 0.0f;
float rest_y = 0.0f;
const int dead_zone = 25000;
DWORD rest_cursor = 0;
void CursorNavigation()
{
	POINT cursor_pos;
	GetCursorPos(&cursor_pos);

	SHORT thumb_x = user->GetLeftAnalogX();
	SHORT thumb_y = user->GetLeftAnalogY();

	float x = cursor_pos.x + rest_x;
	float y = cursor_pos.y + rest_y;

	float dx = 0;
	float dy = 0;
	
	float lengthsq = thumb_x * thumb_x + thumb_y * thumb_y;
	if (lengthsq > dead_zone * dead_zone)
	{
		float multiplier;

		if (GetTickCount() >= rest_cursor)
		{
			multiplier = 0.0005f * ((sqrt(lengthsq) - dead_zone) / (MAXSHORT - dead_zone)) / 60;
		}
		else
		{
			multiplier = 0.0001f * ((sqrt(lengthsq) - dead_zone) / (MAXSHORT - dead_zone)) / 60;
		}

		dx = multiplier * thumb_x;
		dy = multiplier * thumb_y;
		cursor_mode = true; //If the cursor was moved enable mouse mode so that some keys will work as the mouse equivalents Enter ---> Left mouse button
	}
	else
	{
		rest_cursor = GetTickCount() + 500;
	}

	x += dx;
	rest_x = x - (float)((int)x);

	y -= dy;
	rest_y = y - (float)((int)y);

	SetCursorPos((int)x, (int)y);
}







void GamepadInputChecks()
{

	hold_key_down = false;


	if (user->IsButtonDown(GAMEPAD_LB))
	{
		to_release_hold_key = true;
		hold_key_down = true;

		// Alt key
		SimulateHoldKey(VK_MENU); 

		if (user->IsButtonJustDown(GAMEPAD_RIGHT_THUMB))
		{
			SimulateKey(VK_F4);
		}
	}




	// Release the previously held key once its no longer pressed
	if (to_release_hold_key && hold_key_down == false)
	{
		hold_key_down = false;
		to_release_hold_key = false;
		ReleaseHoldKey();
	}









	//------------------------------ Cursor exclusive inputs
	
	CursorNavigation(); //Mouse movements using the left thumb stick
	

	if (user->IsButtonJustDown(GAMEPAD_START))
	{
		SimulateKey(VK_RETURN);
	}



	if (user->IsButtonJustDown(GAMEPAD_A))
	{
		if (cursor_mode)
			SimulateMouseKey(MOUSEEVENTF_LEFTDOWN, MOUSEEVENTF_LEFTUP); // Left mouse button
		else
			SimulateKey(VK_RETURN);

	}

	

	if (user->IsButtonJustDown(GAMEPAD_B))
	{
		SimulateKey(VK_BROWSER_BACK); // Browser back -- used to go back to previous pages on Browser and File Explorer
	}

	if (user->IsButtonJustDown(GAMEPAD_Y))
	{
		if (cursor_mode)
			SimulateMouseKey(MOUSEEVENTF_RIGHTDOWN, MOUSEEVENTF_RIGHTUP); // Right mouse button
		else
			SimulateKey(VK_APPS);

	}

	if (SmoothNavigation(GAMEPAD_X))
	{
		SimulateKey(VK_BACK); // Backspace key
	}

	if (user->right_trigger > 0.0f)
		SimulateMouseWheel(-1);  // Scroll down

	if (user->left_trigger > 0.0f)
		SimulateMouseWheel(1); // Scroll up

	//------------------------------ Cursor exclusive inputs


	// CTRL Shortcuts

	if (user->IsButtonDown(GAMEPAD_RB) && user->IsButtonJustDown(GAMEPAD_RIGHT_THUMB))
	{
		SimulateHoldKey(VK_LWIN); // Left Windows key
		SimulateKey(0x44);        // D key
		ReleaseHoldKey();
		return;
	}



	if (user->IsButtonDown(GAMEPAD_RB) && user->IsButtonJustDown(GAMEPAD_DPAD_UP))
	{
		SimulateHoldKey(VK_LCONTROL); // Left CTRL key
		SimulateKey(0x54);        // T key
		ReleaseHoldKey();
		return;
	}

	if (user->IsButtonDown(GAMEPAD_RB) && user->IsButtonJustDown(GAMEPAD_DPAD_DOWN))
	{
		SimulateHoldKey(VK_LCONTROL); // Left CTRL key
		SimulateKey(0x57);        // W key
		ReleaseHoldKey();
		return;
	}

	if (user->IsButtonDown(GAMEPAD_RB) && user->IsButtonJustDown(GAMEPAD_DPAD_LEFT))
	{
		SimulateHoldKey(VK_LCONTROL); // Left CTRL key
		SimulateKey(VK_PRIOR);        // Page UP key
		ReleaseHoldKey();
		return;
	}

	if (user->IsButtonDown(GAMEPAD_RB) && user->IsButtonJustDown(GAMEPAD_DPAD_RIGHT))
	{
		SimulateHoldKey(VK_LCONTROL); // Left CTRL key
		SimulateKey(VK_NEXT);        // Page DOWN key
		ReleaseHoldKey();
		return;
	}




	// Brought from controller mode
	// Since up/down arrows seem to be used for scrolling on some websites
	// I thought it would be useful having these on on Cursor mode too as
	// it makes scrolling a whole lot easier.
	if (user->IsButtonJustDown(GAMEPAD_LEFT_THUMB))
	{
		SimulateKey(VK_TAB); // Tab key
	}




	
	if (getting_input)
	{
		return;
	}

	


	if (SmoothNavigation(GAMEPAD_DPAD_UP))
	{
		SimulateKey(VK_UP); // Up arrow
		cursor_mode = false;
	}

	if (SmoothNavigation(GAMEPAD_DPAD_DOWN))
	{
		SimulateKey(VK_DOWN); // Down arrow
		cursor_mode = false;
	}

	if (SmoothNavigation(GAMEPAD_DPAD_LEFT))
	{
		SimulateKey(VK_LEFT); // Left arrow
		cursor_mode = false;
	}

	if (SmoothNavigation(GAMEPAD_DPAD_RIGHT))
	{
		SimulateKey(VK_RIGHT); // Right arrow
		cursor_mode = false;
	}


}