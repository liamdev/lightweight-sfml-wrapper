#include "core.h"
#include "gamepad.h"
#include "stdio.h"

//////////////////////////////////////////////////////////////////////////
// Internal state
//////////////////////////////////////////////////////////////////////////

static const int	MAX_GAMEPADS = 4;
static bool			g_connected[MAX_GAMEPADS];
static f2			g_thumb[MAX_GAMEPADS][2];
static float		g_trigger[MAX_GAMEPADS][2];
static float		g_thumb_deadzone[MAX_GAMEPADS][2];
static float		g_trigger_deadzone[MAX_GAMEPADS][2];
static bool			g_buttons_curr[MAX_GAMEPADS][PadKey::COUNT];
static bool			g_buttons_prev[MAX_GAMEPADS][PadKey::COUNT];
static bool			g_just_connected[MAX_GAMEPADS];
static bool			g_just_disconnected[MAX_GAMEPADS];
static float		g_rumble_timers[MAX_GAMEPADS];

//////////////////////////////////////////////////////////////////////////
// Platform-specific gamepad API
//////////////////////////////////////////////////////////////////////////

static void GamepadStartFrameWin32()
{
	#ifndef _WIN32
	ASSERT(false, "Win32 functionality called on non-Windows platform?");
	return;
	#else
	#if INCLUDE_GAMEPAD_LIBRARY

	const float THUMB_RANGE = 0x7FFF;
	const float TRIGGER_RANGE = 0xFF;

	GamepadState gamepad_state;
	for(int i = 0; i < MAX_GAMEPADS; ++i)
	{
		// Fetch the pad state.
		ZeroMemory(&gamepad_state, sizeof(GamepadState));
		DWORD result = XInputGetState(i, &gamepad_state);

		// Status.
		g_connected[i] = (result == ERROR_SUCCESS);

		// Triggers.
		g_trigger[i][0] = gamepad_state.Gamepad.bLeftTrigger / TRIGGER_RANGE;
		g_trigger[i][1] = gamepad_state.Gamepad.bRightTrigger / TRIGGER_RANGE;
		g_trigger_deadzone[i][0] = XINPUT_GAMEPAD_TRIGGER_THRESHOLD / TRIGGER_RANGE;
		g_trigger_deadzone[i][1] = XINPUT_GAMEPAD_TRIGGER_THRESHOLD / TRIGGER_RANGE;

		// Sticks.
		g_thumb[i][0] = f2(gamepad_state.Gamepad.sThumbLX, gamepad_state.Gamepad.sThumbLY) / THUMB_RANGE;
		g_thumb[i][1] = f2(gamepad_state.Gamepad.sThumbRX, gamepad_state.Gamepad.sThumbRY) / THUMB_RANGE;
		g_thumb_deadzone[i][0] = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE / THUMB_RANGE;
		g_thumb_deadzone[i][1] = XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE / THUMB_RANGE;
		
		// Buttons.
		g_buttons_curr[i][PadKey::Up]		= (gamepad_state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) > 0;
		g_buttons_curr[i][PadKey::Down]		= (gamepad_state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) > 0;
		g_buttons_curr[i][PadKey::Left]		= (gamepad_state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) > 0;
		g_buttons_curr[i][PadKey::Right]	= (gamepad_state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) > 0;
		g_buttons_curr[i][PadKey::Cross]	= (gamepad_state.Gamepad.wButtons & XINPUT_GAMEPAD_A) > 0;
		g_buttons_curr[i][PadKey::Square]	= (gamepad_state.Gamepad.wButtons & XINPUT_GAMEPAD_X) > 0;
		g_buttons_curr[i][PadKey::Triangle]	= (gamepad_state.Gamepad.wButtons & XINPUT_GAMEPAD_Y) > 0;
		g_buttons_curr[i][PadKey::Circle]	= (gamepad_state.Gamepad.wButtons & XINPUT_GAMEPAD_B) > 0;
		g_buttons_curr[i][PadKey::L1]		= (gamepad_state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) > 0;
		g_buttons_curr[i][PadKey::R1]		= (gamepad_state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) > 0;
		g_buttons_curr[i][PadKey::Start]	= (gamepad_state.Gamepad.wButtons & XINPUT_GAMEPAD_START) > 0;
		g_buttons_curr[i][PadKey::Select]	= (gamepad_state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) > 0;
		g_buttons_curr[i][PadKey::L3]		= (gamepad_state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) > 0;
		g_buttons_curr[i][PadKey::R3]		= (gamepad_state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) > 0;

		// Positive Y values should come from holding the stick up.
		for(int j = 0; j < 2; ++j)
		{
			g_thumb[i][j].y *= -1;
		}
	}

	#endif
	#endif
}

static void ResetControllerState(int idx)
{
	for(int i = 0; i < MAX_GAMEPADS; ++i)
	{
		// Status.
		g_just_connected[idx]		= false;
		g_connected[idx]			= false;
		
		// Triggers.
		g_trigger[idx][0]			= 0;
		g_trigger[idx][1]			= 0;
		g_trigger_deadzone[idx][0]	= 0;
		g_trigger_deadzone[idx][1]	= 0;

		// Sticks.
		g_thumb[idx][0]				= f2(0);
		g_thumb[idx][1]				= f2(0);
		g_thumb_deadzone[idx][0]	= 0;
		g_thumb_deadzone[idx][1]	= 0;

		// Buttons.
		for(int j = 0; j < PadKey::COUNT; ++j)
		{
			g_buttons_curr[idx][j] = false;
			g_buttons_prev[idx][j] = false;
		}
	}
}

static void ApplyDeadzone(int idx)
{
	// Triggers
	for(int i = 0; i < 2; ++i)
	{
		// Sanity check, clamp trigger input to [0,1];
		g_trigger[idx][i] = clamp(g_trigger[idx][i], 0.0f, 1.0f);

		float val = g_trigger[idx][i];
		if(val < g_trigger_deadzone[idx][i])
		{
			// Apply deadzone.
			g_trigger[idx][i] = 0;
		}
		else
		{
			// Scale the input value from the deadzone to maximum.
			float minval = g_trigger_deadzone[idx][i];
			float maxval = 1.0f;
			float newval = (val - minval) / (maxval - minval);
			g_trigger[idx][i] = newval;
		}
	}

	// Sticks
	for(int i = 0; i < 2; ++i)
	{
		// Sanity check, clamp the thumbstick input to [-1,1].
		if(abs(g_thumb[idx][i].x) > 1)
			g_thumb[idx][i].x = (float)sign(g_thumb[idx][i].x);
		if(abs(g_thumb[idx][i].y) > 1)
			g_thumb[idx][i].y = (float)sign(g_thumb[idx][i].y);

		float len = length(g_thumb[idx][i]);
		if(len > 0)
		{
			if(len < g_thumb_deadzone[idx][i])
			{
				// Within the deadzone, zero the output.
				g_thumb[idx][i] = 0;
				len = 0;
			}
			else
			{
				// Scale the given value from deadzone to maximum.
				float minval = g_thumb_deadzone[idx][i];
				float maxval = 1.0f;
				float newlen = (len - minval) / (maxval - minval);
				g_thumb[idx][i] *= (newlen / len);
				len = newlen;
			}
		}

		// And finally normalise to give us circular input, rather than the bizarre mess we get otherwise.
		if(len > 1)
		{
			g_thumb[idx][i] /= len;
		}
	}
}

static bool IsValidController(int idx)
{
	if(idx < 0 || idx >= MAX_GAMEPADS)
	{
		ASSERT(false, "Invalid controller index!");
		return false;
	}
	return true;
}

static bool IsValidButton(int key)
{
	if(key < 0 || key >= PadKey::COUNT)
	{
		ASSERT(false, "Invalid button index!");
		return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
// Public gamepad API
//////////////////////////////////////////////////////////////////////////

bool PadIsConnected(int idx)
{
	if(!IsValidController(idx))
		return false;
	return g_connected[idx];
}

bool PadJustConnected(int idx)
{
	if(!IsValidController(idx))
		return false;
	return g_just_connected[idx];
}

bool PadJustDisconnected(int idx)
{
	if(!IsValidController(idx))
		return false;
	return g_just_disconnected[idx];
}

u32 GetNumberOfPads()
{
	int count = 0;
	for(int i = 0; i < MAX_GAMEPADS; ++i)
		if(g_connected[i])
			count += 1;
	return count;
}

u32 GetMaximumPads()
{
	return MAX_GAMEPADS;
}

bool PadKeyClicked(int idx, u32 k)
{
	if(!IsValidController(idx) || !IsValidButton(k))
		return false;
	return g_buttons_curr[idx][k] && !g_buttons_prev[idx][k];
}

bool PadKeyDown(int idx, u32 k)
{
	if(!IsValidController(idx) || !IsValidButton(k))
		return false;
	return g_buttons_curr[idx][k];
}

bool PadKeyUnclicked(int idx, u32 k)
{
	if(!IsValidController(idx) || !IsValidButton(k))
		return false;
	return !g_buttons_curr[idx][k] && g_buttons_prev[idx][k];
}

float LeftTrigger(int idx)
{
	if(!IsValidController(idx))
		return 0;
	return g_trigger[idx][0];
}

float RightTrigger(int idx)
{
	if(!IsValidController(idx))
		return 0;
	return g_trigger[idx][1];
}

f2 LeftStick(int idx)
{
	if(!IsValidController(idx))
		return 0;
	return g_thumb[idx][0];
}

f2 RightStick(int idx)
{
	if(!IsValidController(idx))
		return 0;
	return g_thumb[idx][1];
}

void RumbleWin32(int idx, float small_motor, float large_motor)
{
	#ifndef _WIN32
	ASSERT(false, "Win32 functionality called on non-Windows platform?");
	return;
	#else
	#if INCLUDE_GAMEPAD_LIBRARY

	const double RUMBLE_RANGE = 0xFFFF;
	RumbleState rumble_state;
	rumble_state.wLeftMotorSpeed = int(small_motor * RUMBLE_RANGE);
	rumble_state.wRightMotorSpeed = int(large_motor * RUMBLE_RANGE);
	XInputSetState(idx, &rumble_state);

	#endif
	#endif
}

void Rumble(int idx, float small_motor, float large_motor, float duration)
{
	if(!IsValidController(idx) || !g_connected[idx])
		return;

	small_motor = clamp(small_motor, 0.0f, 1.0f);
	large_motor = clamp(large_motor, 0.0f, 1.0f);
	g_rumble_timers[idx] = duration;

	#ifdef _WIN32
	RumbleWin32(idx, small_motor, large_motor);
	#else
	ASSERT(false, "Rumble not yet supported on non-Windows platforms!");
	#endif
}

void StopRumble(int idx)
{
	if(!IsValidController(idx))
		return;

	#ifdef _WIN32
	RumbleWin32(idx, 0, 0);
	#else
	ASSERT(false, "Rumble not yet supported on non-Windows platforms!");
	#endif
}

//////////////////////////////////////////////////////////////////////////
// Core gamepad API
//////////////////////////////////////////////////////////////////////////

void GamepadInit()
{
	for(int i = 0; i < MAX_GAMEPADS; ++i)
	{
		ResetControllerState(i);
	}
}

void GamepadStartFrame()
{
	// Track which controllers were connected last frame, to detect connect/disconnect events.
	bool was_connected[4];

	// Convert current button state to previous button state.
	for(int i = 0; i < MAX_GAMEPADS; ++i)
	{
		was_connected[i] = g_connected[i];

		for(int j = 0; j < PadKey::COUNT; ++j)
		{
			g_buttons_prev[i][j] = g_buttons_curr[i][j];
			g_buttons_curr[i][j] = false;
		}

		if(g_connected[i])
		{
			g_rumble_timers[i] = max(g_rumble_timers[i] - float(GetFrameTime()), 0.0f);
			if(g_rumble_timers[i] == 0)
				StopRumble(i);
		}
	}

	// Get controller data.
	#ifdef _WIN32
	GamepadStartFrameWin32();
	#endif

	for(int i = 0; i < MAX_GAMEPADS; ++i)
	{
		g_just_connected[i] = g_connected[i] && !was_connected[i];
		g_just_disconnected[i] = !g_connected[i] && was_connected[i];

		// Check disconnections.
		if(g_just_disconnected[i])
			ResetControllerState(i);

		// Massage controller inputs.
		ApplyDeadzone(i);
	}
}
