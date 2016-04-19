# Lightweight SFML Wrapper
A C-style API which wraps the functionality of SFML v2.3.2 in a more lightweight interface.

###Overview:

This API is currently primarily for personal use, and so is only somewhat documented, and is based on certain decisions that would be questionable were this to be used externally, such as its reliance on an included maths library (which provides basic 2D/3D/4D vector functionality, as well as some mathematical helpers).

###Build Info:

Must include the SFML Include directory, and link `sfml-system.lib`, `sfml-window.lib`, `sfml-graphics.lib`, and `sfml-audio.lib`.

If gamepad support is not desired, set the `INCLUDE_GAMEPAD_LIBRARY` flag in the configuration section at the top of core.h to 0. The `gamepad.h` and `gamepad.cpp` files can be safely excluded from your project if this flag is set to 0.

If gamepad support is desired (Windows only), then also link `Xinput9_1_0.lib`.

###Basic usage:

The core update of the application (window display, input polling, timer updates etc.) is handled by calling corresponding pairs of `StartFrame()` and `EndFrame()`:

```c++
#include "core.h"

int main()
{
	CoreInit();

	while(StartFrame())
	{
		// Game loop goes here

		EndFrame();
	}

	return 0;
}
```

### Input API

Input state is automatically polled and updated in the core library loop, and exposed through a few simple functions:

- `KeyClicked(Key k)`, `KeyUnclicked(Key k)`, and `KeyDown(Key k)` check the state/events for the given key. Makes use of the `Key` type (wraps the sf::Keyboard enum) for keyboard input.
- `LeftMouseClicked()`, `LeftMouseUnclicked()`, `RightMouseClicked()`, `RightMouseUnclicked()` for checking mouse click events.
- `GetMouseX()` and `GetMouseY()` for getting the cursor position within the window. Coordinates are 

### Graphics, Text, and Sound APIs

The text, graphics, and sound APIs work via an ID system, in which requests for a new asset (Font, Texture, Sprite, Sound) are fulfilled by returning an ID for the requested asset. All operations using those assets take the ID, rather than an object reference or pointer.

## Graphics example
```c++
TextureId my_texture = LoadTexture("assets/textures/path.png");

// Textures can be used for multiple sprites.
SpriteId player = CreateSprite(my_texture, SpriteOrigin::Centre);
SpriteId enemy1 = CreateSprite(enemy);
SpriteId enemy2 = CreateSprite(enemy);

// Use SpriteIds to alter the sprites.
SetPosition(player, f2(100,100));
SetColour(enemy1, f4(1,0,0,1)); // Red enemy
SetColour(enemy2, f4(0,1,0,1)); // Green enemy
SetRotation(enemy1, GetTotalTime()); // Spinning enemy.

// Destroy sprites when done with them.
DestroySprite(player);
```

## Shader example
```c++
ShaderId my_shader = LoadShader("assets/shaders/vignette.glsl");

// Set uniform variables.
SetShaderParameter(my_shader, "radius", 1.5f);

// Apply post-process effects.
DrawPostEffects(my_shader);
```

## Text example
```c++
FontId my_font = LoadFont("assets/fonts/font.ttf");

// Set font parameters. These apply to all DrawText() calls from this point onwards.
SetFontSize(48);

// Get some text on screen.
DrawText("Game Jam 2016", f2(400,300)); // Centres by default.

// Change some more settings.
SetFontColour(f4(1,0,0,1)); 
DrawText("Subheading", f2(0,500), TextAlign::Left); // Align left, in red.
SetFontSize(64);
DrawText("Bigger subheading", f2(0,600)); // Align centre, in red, 64 pixels high.
```

## Sound example
```c++
SoundId my_sound = LoadSound("assets/audio/sound.wav");

// Play the sound a few times.
SoundInstanceId sound1 = PlaySound(my_sound, volume_mult, pitch_mult, looping_bool); // Only first argument is required.
SoundInstanceId sound2 = PlaySound(my_sound);

// Alter the volume of a sound in progress.
SetSoundVolume(sound1, 2.0f);

// Silence!
StopAllSounds();
```

## Geometry example
```c++
// Draws a 100x100 pixel red quad, with its top-left at 50,50.
DrawQuad(f2(50,50), f2(100,100), f4(1,0,0,1));

// Draws a blue 25-pixel radius circle, centred at 60,60.
DrawCircle(f2(60,60), 25, f4(0,0,1,0));
```

### Time API

- `GetTotalTime()`: returns the time since the application started.
- `GetFrameTime()`: returns the total time taken for the previous frame. Clamped to a maximum of 1/15th of a second.
- `GetFrameNumber()`: total frames since the application started.

### Window API

The majority of the window settings API should be self-explanatory. The only noteworthy addition is:

- `ScreenShake()`: applies screen shake to the camera, with duration and intensity proportional to the value passed in.

### Random API

For convenience, the wrapper comes with a function for generating random numbers using xorshift128+ (automatically seeded on initialising the wrapper):

- `RandNorm()`: returns a random double in the range [0,1].

### Gamepad API

Runs using XInput, and so currently only supports Windows platforms.

- `PadIsConnected(u32 pad_id)`, `PadJustConnected(u32 pad_id)`, and `PadJustDisconnected(u32 pad_id)` check the current connection state of a gamepad, and allow detection of connect/disconnect events.
- `GetNumberOfPads()` and `GetMaximumPads()` query the total pad count, and the maximum supported pad count.
- `PadKeyClicked(int pad_id, PadKey k)`, `PadKeyDown(int pad_id, PadKey k)`, and `PadKeyUnclicked(int pad_id, PadKey k)` check the state/events for the given gamepad button. Makes use of the `PadKey` type.
- `LeftTrigger(int pad_id)` and `RightTrigger(int pad_id)` return the current trigger values for the specified gamepad.
- `LeftStick(int pad_id)` and `RightStick(int pad_id)` return the current thumbstick input for the specified gamepad.
- `Rumble(int pad_id, float small_motor, float large_motor, float duration)` allows a given gamepad to be vibrated. Motor values range from 0 to 1.
- `StopRumble(int pad_id)` cancels the rumble on a given gamepad.

###Application defaults:

There are some settings within the wrapper that are not yet exposed, but added as useful defaults. For now, disabling/altering these defaults will require altering `core.cpp`:

- `F11` toggles fullscreen on and off.
- `Escape` closes the application.
- The window style defaults to a cleaner style, lacking a close button.
