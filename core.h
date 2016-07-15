#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <stdio.h>
#include <cstdint>
#include "maths.h"

//////////////////////////////////////////////////////////////////////////
// Overview
//////////////////////////////////////////////////////////////////////////
/*

	This library acts as a lightweight C-style API to wrap the functionality
	of SFML in a more lightweight manner. It has been written and tested
	against SFML version 2.3.2. Support of alternate versions is not
	guaranteed.

	Author: Liam de Valmency (liam.devalmency@gmail.com / @Kilo_bytes)

	TODO: 
		- Ability to set window icon.
		- An internal 2D physics system would be nice.
		- Rework the sprite API to be less cumbersome.
		- Rework the font API to be less cumbersome (add single-call API in addition to stateful one; add push/pop to stateful API).
		- Some helper functionality for quickly creating particle systems/effects?
		- Fix fullscreen switch requiring an alt-tab out, followed by an alt-tab back in, before the window shows its contents properly.
		- Fix game logic continuing to run while the window is unfocused.
		- Fix fullscreen black border + screen shake combination giving undesirable results.
		- Fix negative coordinate clipping region not playing well with modified world region.

	TODO-Optional:
		- Option to change whether Y=0 refers to bottom or top of screen.
*/

//////////////////////////////////////////////////////////////////////////
// Configuration options.
//////////////////////////////////////////////////////////////////////////

#define INCLUDE_GAMEPAD_LIBRARY 0

//////////////////////////////////////////////////////////////////////////
// Typedefs
//////////////////////////////////////////////////////////////////////////

// SFML types
typedef sf::Keyboard Key;
typedef sf::Sprite Sprite;

// Types
typedef uint8_t		u8;
typedef uint16_t	u16;
typedef uint32_t	u32;
typedef uint64_t	u64;
typedef u32			FontId;
typedef u32			ShaderId;
typedef u32			SoundId;
typedef u32			SoundInstanceId;
typedef u32			SpriteId;
typedef u32			TextureId;

enum class SpriteOrigin { TopLeft, Centre };
enum class TextAlign { Left, Centre };

//////////////////////////////////////////////////////////////////////////
// Game API
//////////////////////////////////////////////////////////////////////////

void CoreInit();
bool StartFrame();
void EndFrame();

//////////////////////////////////////////////////////////////////////////
// Window API
//////////////////////////////////////////////////////////////////////////

// General window parameters.
void SetWindowTitle(const char* title);
void SetWindowFPSLimit(u32 fps);
void SetWindowMouseCursorVisible(bool b);
void SetWindowClearColour(f4 colour);

// Window sizing / clipping / coordinates.
void SetWindowSize(int x, int y);
void SetWindowFullscreen(bool b);
void SetWindowClipRegion(f2 top_left_px, f2 size_px);
void SetWindowWorldRegion(f2 top_left_world, f2 size_world);
void ResetWindowClipRegion();
void ResetWindowWorldRegion();

// Screen shake!
void ScreenShake(float amount);

//////////////////////////////////////////////////////////////////////////
// Input API
//////////////////////////////////////////////////////////////////////////

// Keyboard
bool KeyClicked(u32 key);
bool KeyDown(u32 key);
bool KeyUnclicked(u32 key);

// Mouse
bool LeftMouseClicked();
bool LeftMouseUnclicked();
bool RightMouseClicked();
bool RightMouseUnclicked();
int GetMouseX();
int GetMouseY();

//////////////////////////////////////////////////////////////////////////
// Time API (in seconds)
//////////////////////////////////////////////////////////////////////////

double	GetTotalTime();
double	GetFrameTime();
u64		GetFrameNumber();

//////////////////////////////////////////////////////////////////////////
// Text API
//////////////////////////////////////////////////////////////////////////

FontId	LoadFont(const char* path);

// Stateful font API.
void	PushFontStyle();
void	PopFontStyle();
void	SetFont(FontId font);
void	SetFontSize(u32 size);
void	SetFontColour(f4 col);
void	SetFontScale(float scale);
void	DrawText(const char* text, f2 pos, TextAlign align = TextAlign::Centre); // Draws text with the current state settings.

// Immediate font API.
void	DrawText(const char* text, FontId font, f2 pos, u32 size_px, f4 col, TextAlign align = TextAlign::Centre, float scale = 1);

//////////////////////////////////////////////////////////////////////////
// Graphics API
//////////////////////////////////////////////////////////////////////////

// Geometry library.
void		DrawQuad(f2 pos, f2 size, f4 col);
void		DrawCircle(f2 pos, float radius, f4 col);

// Sprite library.
TextureId	LoadTexture(const char* path);
SpriteId	CreateSprite(TextureId texture, SpriteOrigin origin = SpriteOrigin::TopLeft);
void		DestroySprite(SpriteId sprite);
void		DrawSprite(SpriteId sprite, int xsegments = -1, int xsegment = 0);
f2			GetPosition(SpriteId sprite);
float		GetRotation(SpriteId sprite);
f2			GetScale(SpriteId sprite);
void		SetPosition(SpriteId sprite, f2 pos);
void		SetRotation(SpriteId sprite, float ang);
void		SetScale(SpriteId sprite, f2 scale);
void		SetColour(SpriteId sprite, f4 col);

// Shader library.
ShaderId	LoadShader(const char* path);
void		DrawPostEffects(ShaderId shader);
void		SetShaderParameter(ShaderId shader, const char* name, float val);
TextureId	LoadTexture(const char* path);

//////////////////////////////////////////////////////////////////////////
// Sound API
//////////////////////////////////////////////////////////////////////////

SoundId			LoadSound(const char* path);
SoundInstanceId	PlaySound(SoundId sound, float volume = 1, float pitch = 1, bool loop = false);
float			GetSoundVolume(SoundInstanceId sound_instance);
void			StopSound(SoundInstanceId sound);
void			SetSoundVolume(SoundInstanceId sound_instance, float volume);
void			StopAllSounds();

//////////////////////////////////////////////////////////////////////////
// Random API
//////////////////////////////////////////////////////////////////////////

// Random numbers.
double	RandNorm();

// Random colours.
f4		RandPastelCol();

//////////////////////////////////////////////////////////////////////////
// Debug API
//////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
#define ASSERT(check, msg) _ASSERT(check)
#else
#define ASSERT(check,msg) do {} while(0)
#endif

//////////////////////////////////////////////////////////////////////////
// INTERNAL API, PLEASE IGNORE
//////////////////////////////////////////////////////////////////////////

// Windows SDK stomps all over our audio API. Undo this villainy.
#ifdef UNICODE
#define PlaySoundW PlaySound
#else
#define PlaySoundA PlaySound
#endif