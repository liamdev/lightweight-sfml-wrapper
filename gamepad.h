#pragma once

#ifdef _WIN32
#include <Windows.h>
#include <Xinput.h>
typedef XINPUT_STATE GamepadState;
typedef XINPUT_VIBRATION RumbleState;
#endif

//////////////////////////////////////////////////////////////////////////
// Overview
//////////////////////////////////////////////////////////////////////////
/*
	An (in-progress) cross platform gamepad input library.

	Author: Liam de Valmency (liam.devalmency@gmail.com / @Kilo_bytes)

	TODO: 
		- Linux/OSX support.
		- Better options for rumble control.
*/

//////////////////////////////////////////////////////////////////////////
// Types
//////////////////////////////////////////////////////////////////////////

namespace PadKey
{
	enum { Up, Down, Left, Right, Cross, Square, Triangle, Circle, L1, R1, L3, R3, Start, Select, COUNT };
};

//////////////////////////////////////////////////////////////////////////
// Public gamepad API
//////////////////////////////////////////////////////////////////////////

// Status.
bool	PadIsConnected(int pad_idx);
bool	PadJustConnected(int pad_idx);
bool	PadJustDisconnected(int pad_idx);
u32		GetNumberOfPads();
u32		GetMaximumPads();

// Buttons.
bool	PadKeyClicked(int pad_idx, u32 key);
bool	PadKeyDown(int pad_idx, u32 key);
bool	PadKeyUnclicked(int pad_idx, u32 key);
 
// Triggers.
float	LeftTrigger(int pad_idx);
float	RightTrigger(int pad_idx);

// Sticks.
f2		LeftStick(int pad_idx);
f2		RightStick(int pad_idx);

// Vibration.
void	Rumble(int pad_idx, float small_motor, float large_motor);
void	StopRumble(int pad_idx);

//////////////////////////////////////////////////////////////////////////
// Internal gamepad API
//////////////////////////////////////////////////////////////////////////

void GamepadInit();
void GamepadStartFrame();


