#include "core.h"
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#if INCLUDE_GAMEPAD_LIBRARY
#include "gamepad.h"
#endif

//////////////////////////////////////////////////////////////////////////
// Internal types
//////////////////////////////////////////////////////////////////////////

struct FontState
{
	FontId font;
	u32 size;
	float scale;
	f4 col;
};

//////////////////////////////////////////////////////////////////////////
// Internal state
//////////////////////////////////////////////////////////////////////////

// Default window settings
static int			g_window_width = 800;
static int			g_window_height = 600;
static const char*	g_window_title = "";
static int			g_window_fps = 60;
static bool			g_window_mouse_visible = true;
static f4			g_window_clear_col = f4(0, 0, 0, 1);
static bool			g_window_fullscreen = false;

// Window
static sf::RenderWindow	g_window;
static f2				g_window_scaling = f2(1);
static f2				g_window_borders = f2(0);

// Input: keyboard
static bool			g_key_down[Key::KeyCount] = { false };
static bool			g_key_click[Key::KeyCount] = { false };
static bool			g_key_unclick[Key::KeyCount] = { false };
// Input: mouse
static bool			g_mouse_click[2] = { false };
static bool			g_mouse_unclick[2] = { false };
static int			g_mouse_pos[2] = { 0 };

// Time
static sf::Clock	g_clock;
static sf::Clock	g_frameclock;
static double		g_total_time = 0;
static double		g_frame_time = 0;
static u64			g_frame_num = 0;

// Fonts
static const int	MAX_FONTS = 10;
static sf::Font		g_fonts[MAX_FONTS];
static u32			g_total_fonts = 0;

// Font styles
static const int	MAX_FONT_STACK_SIZE = 10;
static FontState	g_font_stack[MAX_FONT_STACK_SIZE];
static u32			g_font_stack_size = 0;

// Textures
static const int		MAX_TEXTURES = 100;
static sf::Texture		g_textures[MAX_TEXTURES];
static sf::Texture		g_no_texture;
static u32				g_total_textures = 0;

// Sprites
static const int		MAX_SPRITES = 8192;
static sf::Sprite		g_sprites[MAX_SPRITES];
static u32				g_total_sprites;

// Shaders
static const int		MAX_SHADERS = 10;
static sf::Texture		g_postprocess_texture;
static sf::Shader		g_shaders[MAX_SHADERS];
static u32				g_total_shaders = 0;

// Audio
static const int		MAX_SOUNDS = 30;
static sf::Sound		g_sounds[MAX_SOUNDS];
static sf::SoundBuffer	g_sound_buffers[MAX_SOUNDS];
static u32				g_total_sounds = 0;

// Random
static u64				g_random_seeds[2];

// Screenshake
static float			g_screenshake_amount = 0;

//////////////////////////////////////////////////////////////////////////
// Forward declarations as necessary.
//////////////////////////////////////////////////////////////////////////

static void SetNormalisedClipRegion(f2 top_left_px, f2 size_px);

//////////////////////////////////////////////////////////////////////////
// Internal API
//////////////////////////////////////////////////////////////////////////

sf::Color Col(f4 c) { return sf::Color(u8(c.x * 255), u8(c.y * 255), u8(c.z * 255), u8(c.w * 255)); }
f2 ScreenSize() { return f2(float(g_window_width), float(g_window_height)); }

//////////////////////////////////////////////////////////////////////////
// Game API
//////////////////////////////////////////////////////////////////////////

void CoreInit()
{
	// Open the window.
	g_window.create(sf::VideoMode(g_window_width, g_window_height), g_window_title, sf::Style::Titlebar);

	// Initialise random number generator.
	g_random_seeds[0] = 0;
	g_random_seeds[1] = 1;
	srand((unsigned int)time(NULL));
	for (int i = 0; i < 10; ++i)
		rand();
	for (int i = 0; i < 2; ++i)
		for (int j = 0; j < 64; ++j)
			g_random_seeds[i] |= (rand() % 2) << j;

	// Initialise postprocessing texture buffer.
	g_postprocess_texture.create(g_window_width, g_window_height);

	// Initialise default font.
	g_font_stack[0].size = 24;
	g_font_stack[0].col = f4(1);
	g_font_stack[0].scale = 1;
	g_font_stack[0].font = 0;

	// Set up the gamepad library.
	#if INCLUDE_GAMEPAD_LIBRARY
	GamepadInit();
	#endif
}

bool StartFrame()
{
	// Update timings.
	g_total_time = g_clock.getElapsedTime().asMicroseconds() / (1000.0 * 1000.0);
	g_frame_time = g_frameclock.restart().asMicroseconds() / (1000.0 * 1000.0);
	g_frame_time = min(g_frame_time, 1.0 / 15.0);
	++g_frame_num;
	
	// Update gamepad input.
	#if INCLUDE_GAMEPAD_LIBRARY
	GamepadStartFrame();
	#endif

	// Reset input state.
	for (int i = 0; i < Key::KeyCount; ++i)
	{
		g_key_click[i] = false;
		g_key_unclick[i] = false;
	}
	for (int i = 0; i < 2; ++i)
	{
		g_mouse_click[i] = false;
		g_mouse_unclick[i] = false;
	}

	// Event processing loop.
	sf::Event event;
	while(g_window.pollEvent(event))
	{
		if (event.type == sf::Event::Closed)
		{
			return false;
		}
		else if (event.type == sf::Event::KeyPressed)
		{
			if(event.key.code == Key::Unknown)
			{
				continue;
			}
			if (!g_key_down[event.key.code])
			{
				g_key_click[event.key.code] = true;
				g_key_down[event.key.code] = true;
			}
		}
		else if(event.type == sf::Event::KeyReleased)
		{
			if(event.key.code == Key::Unknown)
			{
				continue;
			}
			g_key_down[event.key.code] = false;
			g_key_unclick[event.key.code] = true;
		}
		else if(event.type == sf::Event::MouseButtonPressed)
		{
			if (event.mouseButton.button == sf::Mouse::Left)
				g_mouse_click[0] = true;
			else if (event.mouseButton.button == sf::Mouse::Right)
				g_mouse_click[1] = true;
		}
		else if(event.type == sf::Event::MouseButtonReleased)
		{
			if (event.mouseButton.button == sf::Mouse::Left)
				g_mouse_unclick[0] = true;
			else if (event.mouseButton.button == sf::Mouse::Right)
				g_mouse_unclick[1] = true;
		}
		else if(event.type == sf::Event::MouseMoved)
		{
			g_mouse_pos[0] = event.mouseMove.x;
			g_mouse_pos[1] = event.mouseMove.y;
		}
	}

	// Update screenshake
	sf::Vector2f cam_center	= g_window.getView().getCenter();
	f2 shake				= f2((float)RandNorm() * 2 - 1, (float)RandNorm() * 2 - 1) * g_screenshake_amount * 20;
	f2 actual_center		= f2((float)g_window_width, (float)g_window_height) * 0.5f;
	f2 target_center		= actual_center + shake;
	target_center			= lerp(f2(cam_center.x, cam_center.y), target_center, (float)GetFrameTime() * 20);

	// Apply screenshake
	sf::View v = g_window.getView();
	v.setCenter(sf::Vector2f(target_center.x, target_center.y));
	g_window.setView(v);
	g_screenshake_amount = max(g_screenshake_amount - (float)GetFrameTime(), 0.0f);

	// Default key bindings.
	{
		// F11 or Alt+Enter toggles fullscreen.
		if(KeyClicked(Key::F11) || (KeyDown(Key::LAlt) && KeyClicked(Key::Return)))
			SetWindowFullscreen(!g_window_fullscreen);

		// Escape closes the application.
		if(KeyUnclicked(Key::Escape))
			return false;
	}

	// Clear the window.
	g_window.clear(Col(g_window_clear_col));

	// Do black borders if we're in a imperfect fullscreen ratio.
	if(g_window_fullscreen)
	{
		bool xborders = g_window_borders.x > 0;
		if(xborders)
		{
			DrawQuad(f2(0), f2(g_window_borders.x, float(g_window_height)), f4(0,0,0,1));
			DrawQuad(f2(g_window_width - g_window_borders.x, 0), ScreenSize(), f4(0,0,0,1));
		}
		else
		{
			DrawQuad(f2(0), f2(float(g_window_width), g_window_borders.y), f4(0,0,0,1));
			DrawQuad(f2(0, g_window_height - g_window_borders.y), ScreenSize(), f4(0,0,0,1));
		}

		float scaling = xborders ? g_window_scaling.y : g_window_scaling.x;
		SetNormalisedClipRegion(g_window_borders / ScreenSize(), f2(scaling));
	}

	return g_window.isOpen();
}

void EndFrame()
{
	// Draw the window contents.
	g_window.display();

	// Reset clipping and coordinate regions.
	SetNormalisedClipRegion(f2(0), f2(1));
	SetWindowWorldRegion(f2(0), ScreenSize());
}

//////////////////////////////////////////////////////////////////////////
// Window API
//////////////////////////////////////////////////////////////////////////

static bool RecreateWindow(int width, int height, bool fullscreen)
{
	sf::VideoMode video_mode(width, height);
	if(fullscreen && !video_mode.isValid())
	{
		const std::vector<sf::VideoMode>& video_modes = video_mode.getFullscreenModes();
		if(video_modes.empty())
		{
			return false;
		}

		// Figure out whether we want black borders on the sides, or at the top/bottom.
		float curr_ratio = float(video_mode.width) / video_mode.height;
		float fscr_ratio = float(video_modes[0].width) / video_modes[0].height;
		g_window_scaling.x = float(video_modes[0].width) / video_mode.width;
		g_window_scaling.y = float(video_modes[0].height) / video_mode.height;
		if(curr_ratio > fscr_ratio)
		{
			g_window_borders = f2(0, (video_modes[0].height - (video_mode.height * g_window_scaling.x)) * 0.5f);
		}
		else
		{
			g_window_borders = f2((video_modes[0].width - (video_mode.width * g_window_scaling.y)) * 0.5f, 0);
		}
		video_mode = video_modes[0];
	}

	g_window_width = video_mode.width;
	g_window_height = video_mode.height;
	g_window_fullscreen = fullscreen;
	g_window.create(video_mode, g_window_title, sf::Style::Titlebar | (fullscreen ? sf::Style::Fullscreen : 0));
	g_window.setFramerateLimit(g_window_fps);
	g_window.setMouseCursorVisible(g_window_mouse_visible);
	return true;
}

static void SetNormalisedClipRegion(f2 top_left, f2 size)
{
	sf::View curr_view = g_window.getView();
	curr_view.setViewport(sf::FloatRect(top_left.x, top_left.y, size.x, size.y));
	g_window.setView(curr_view);
}

void SetWindowTitle(const char* title)
{
	g_window_title = title;
	g_window.setTitle(title);
}

void SetWindowFPSLimit(u32 fps)
{
	g_window_fps = fps;
	g_window.setFramerateLimit(fps);
}

void SetWindowMouseCursorVisible(bool b)
{
	g_window_mouse_visible = b;
	g_window.setMouseCursorVisible(b);
}

void SetWindowClearColour(f4 colour)
{
	g_window_clear_col = colour;
}

void SetWindowSize(int x, int y)
{
	if(g_window_width != x || g_window_height != y)
	{
		RecreateWindow(x, y, g_window_fullscreen);
	}
}

void SetWindowFullscreen(bool b)
{
	if(g_window_fullscreen != b)
	{
		float width = float(g_window_width);
		float height = float(g_window_height);
		if(!b)
		{
			width /= g_window_scaling.x;
			height /= g_window_scaling.y;
		}
		RecreateWindow(int(width), int(height), b);
	}
}

void SetWindowClipRegion(f2 top_left_px, f2 size_px)
{
	// Ensure the start clip position is non-negative.
	f2 max_px = ScreenSize();
	top_left_px = max(top_left_px, f2(0));

	// Deal with clipping changes when we have black borders to remain within.
	if(g_window_fullscreen)
	{
		bool xborder = g_window_borders.x > 0;
		size_px *= xborder ? g_window_scaling.y : g_window_scaling.x;
		max_px = ScreenSize() - g_window_borders;
		top_left_px += g_window_borders;
	}

	// Ensure the clipping region doesn't go outside of the screen bounds.
	size_px = min(top_left_px + size_px, max_px - top_left_px);

	// Normalise the clipping coordinates.
	top_left_px /= ScreenSize();
	size_px /= ScreenSize();
	SetNormalisedClipRegion(top_left_px, size_px);
}

void SetWindowWorldRegion(f2 top_left_world, f2 size_world)
{
	sf::View view = g_window.getView();
	view.setCenter(top_left_world.x + size_world.x * 0.5f, top_left_world.y + size_world.y * 0.5f);
	view.setSize(size_world.x, size_world.y);
	g_window.setView(view);
}

void ResetWindowClipRegion()
{
	SetWindowClipRegion(f2(0), ScreenSize());
}

void ResetWindowWorldRegion()
{
	SetWindowWorldRegion(f2(0), ScreenSize());
}

void ScreenShake(float amount)
{
	g_screenshake_amount = max(g_screenshake_amount, amount);
}

//////////////////////////////////////////////////////////////////////////
// Input API
//////////////////////////////////////////////////////////////////////////

bool KeyClicked(u32 k)
{
	if (k >= Key::KeyCount)
		return false;
	return g_key_click[k];
}

bool KeyDown(u32 k)
{
	if (k >= Key::KeyCount)
		return false;
	return g_key_down[k];
}

bool KeyUnclicked(u32 k)
{
	if (k >= Key::KeyCount)
		return false;
	return g_key_unclick[k];
}

bool LeftMouseClicked()
{
	return g_mouse_click[0];
}

bool LeftMouseUnclicked()
{
	return g_mouse_unclick[0];
}

bool RightMouseClicked()
{
	return g_mouse_click[1];
}

bool RightMouseUnclicked()
{
	return g_mouse_unclick[1];
}

int GetMouseX()
{
	return g_mouse_pos[0];
}

int GetMouseY()
{
	return g_mouse_pos[1];
}

//////////////////////////////////////////////////////////////////////////
// Time API
//////////////////////////////////////////////////////////////////////////

double GetTotalTime()
{
	return g_total_time;
}

double GetFrameTime()
{
	return g_frame_time;
}

u64 GetFrameNumber()
{
	return g_frame_num;
}

//////////////////////////////////////////////////////////////////////////
// Text API
//////////////////////////////////////////////////////////////////////////

FontId LoadFont(const char* path)
{
	if (g_total_fonts < MAX_FONTS)
	{
		if (g_fonts[g_total_fonts].loadFromFile(path))
		{
			return g_total_fonts++;
		}

		printf("[ERR]: Could not load font from %s\n", path);
		return -1;
	}

	printf("[ERR]: Out of font memory!\n");
	return -1;
}

void PushFontStyle()
{
	if(g_font_stack_size == MAX_FONT_STACK_SIZE - 1)
	{
		printf("[ERR]: Font style stack is too big to push onto. Did you forget to pop?\n");
		return;
	}

	u32 copy_from = g_font_stack_size;
	g_font_stack_size += 1;
	g_font_stack[g_font_stack_size] = g_font_stack[copy_from];
}

void PopFontStyle()
{
	if(g_font_stack_size == 0)
	{
		printf("[ERR]: Font style stack is already empty.\n");
		return;
	}

	g_font_stack_size -= 1;
}

static FontState& GetFontState()
{
	return g_font_stack[g_font_stack_size];
}

void SetFont(FontId font)
{
	if (font < g_total_fonts)
	{
		GetFontState().font = font;
	}
}

void SetFontSize(u32 size)
{
	GetFontState().size = size;
}

void SetFontColour(f4 col)
{
	GetFontState().col = col;
}

void SetFontScale(float scale)
{
	GetFontState().scale = scale;
}

void DrawText(const char* text, f2 pos, TextAlign align)
{
	FontState& font_state = GetFontState();
	DrawText(text, font_state.font, pos, font_state.size, font_state.col, align, font_state.scale);
}

void DrawText(const char* text, FontId font, f2 pos, u32 size_px, f4 col, TextAlign align, float scale)
{
	if(g_total_fonts == 0)
		return;

	if(font < 0 || font >= g_total_fonts)
	{
		printf("Font ID is not valid!\n");
		return;
	}

	sf::Text text_params;
	text_params.setFont(g_fonts[font]);
	text_params.setCharacterSize(size_px);
	text_params.setColor(Col(col));
	text_params.setString(text);
	text_params.setPosition(sf::Vector2f(pos.x, pos.y));
	text_params.setScale(sf::Vector2f(scale, scale));

	if (align == TextAlign::Centre)
	{
		sf::FloatRect rect = text_params.getLocalBounds();
		text_params.setOrigin(sf::Vector2f(rect.width * 0.5f, rect.height * 0.5f));
	}

	g_window.draw(text_params);
}

//////////////////////////////////////////////////////////////////////////
// Graphics API
//////////////////////////////////////////////////////////////////////////

void DrawQuad(f2 pos, f2 size, f4 col)
{
	sf::RectangleShape r;
	r.setPosition(sf::Vector2f(pos.x, pos.y));
	r.setSize(sf::Vector2f(size.x, size.y));
	r.setFillColor(Col(col));
	g_window.draw(r);
}

void DrawCircle(f2 pos, float radius, f4 col)
{
	sf::CircleShape c;
	c.setRadius(radius);
	c.setPosition(sf::Vector2f(pos.x, pos.y));
	c.setOrigin(sf::Vector2f(radius, radius));
	c.setFillColor(Col(col));
	g_window.draw(c);
}

TextureId LoadTexture(const char* path)
{
	if(g_total_textures < MAX_TEXTURES)
	{
		if(g_textures[g_total_textures].loadFromFile(path))
		{
			g_textures[g_total_textures].setSmooth(true);
			return g_total_textures++;
		}

		printf("[ERR]: Couldn't load texture file from %s\n", path);
		return -1;
	}

	printf("[ERR]: No remaining texture memory!\n");
	return -1;
}

SpriteId CreateSprite(TextureId texture_id, SpriteOrigin origin)
{
	if(texture_id >= g_total_textures)
	{
		printf("[ERR]: Invalid texture ID!\n");
		return -1;
	}

	u32 idx = 0;
	for (; idx < MAX_SPRITES; ++idx)
	{
		if (!g_sprites[idx].getTexture())
			break;
		
		if(idx == MAX_SPRITES - 1)
		{
			printf("[ERR]: Out of sprite memory!\n");
			return -1;
		}
	}

	sf::Sprite& sprite = g_sprites[idx];
	sprite.setTexture(g_textures[texture_id]);

	if(origin == SpriteOrigin::Centre)
	{
		sf::Vector2u size = g_textures[texture_id].getSize();
		sprite.setOrigin(sf::Vector2f(size.x * 0.5f, size.y * 0.5f));
	}

	return idx;
}

void DestroySprite(SpriteId sprite)
{
	if(sprite < MAX_SPRITES)
	{
		g_sprites[sprite] = sf::Sprite();
	}
}

void DrawSprite(SpriteId sprite, int xsegments, int xsegment)
{
	if(sprite < MAX_SPRITES && g_sprites[sprite].getTexture())
	{
		sf::Vector2u full_size = g_sprites[sprite].getTexture()->getSize();
		if (xsegments < 0)
		{
			g_sprites[sprite].setTextureRect(sf::IntRect(0, 0, full_size.x, full_size.y));
		}
		else
		{
			float s = full_size.x / (float)xsegments;
			g_sprites[sprite].setTextureRect(sf::IntRect((int)(xsegment * s), 0, (int)s, full_size.y));
		}
		g_window.draw(g_sprites[sprite]);
	}
}

static sf::Sprite* GetSFMLSprite(SpriteId sprite)
{
	if (sprite >= MAX_SPRITES || !g_sprites[sprite].getTexture())
	{
		printf("[ERR]: Invalid sprite ID!\n");
		return nullptr;
	}

	return &g_sprites[sprite];
}

f2 GetPosition(SpriteId sprite)
{
	if (sf::Sprite* sfml_sprite = GetSFMLSprite(sprite))
	{
		sf::Vector2f pos = sfml_sprite->getPosition();
		return f2(pos.x, pos.y);
	}
	return f2(0);
}

float GetRotation(SpriteId sprite)
{
	if(sf::Sprite* sfml_sprite = GetSFMLSprite(sprite))
	{
		return (float)(sfml_sprite->getRotation() * DEG_TO_RAD);
	}
	return 0;
}

f2 GetScale(SpriteId sprite)
{
	if(sf::Sprite* sfml_sprite = GetSFMLSprite(sprite))
	{
		sf::Vector2f scale = sfml_sprite->getScale();
		return f2(scale.x, scale.y);
	}
	return 1;
}

void SetPosition(SpriteId sprite, f2 pos)
{
	if(sf::Sprite* sfml_sprite = GetSFMLSprite(sprite))
	{
		sfml_sprite->setPosition(sf::Vector2f(pos.x, pos.y));
	}
}

void SetRotation(SpriteId sprite, float ang)
{
	if(sf::Sprite* sfml_sprite = GetSFMLSprite(sprite))
	{
		sfml_sprite->setRotation((float)(ang * RAD_TO_DEG));
	}
}

void SetScale(SpriteId sprite, f2 scale)
{
	if(sf::Sprite* sfml_sprite = GetSFMLSprite(sprite))
	{
		sfml_sprite->setScale(sf::Vector2f(scale.x, scale.y));
	}
}

void SetColour(SpriteId sprite, f4 col)
{
	if(sf::Sprite* sfml_sprite = GetSFMLSprite(sprite))
	{
		sfml_sprite->setColor(Col(col));
	}
}

ShaderId LoadShader(const char* path)
{
	if(!sf::Shader::isAvailable())
	{
		printf("[ERR]: Shaders not available on your platform! :(\n");
		return -1;
	}

	if(g_total_shaders < MAX_SHADERS)
	{
		if (g_shaders[g_total_shaders].loadFromFile(path, sf::Shader::Fragment))
		{
			return g_total_shaders++;
		}

		printf("[ERR]: Could not load shader from file %s\n", path);
		return -1;
	}

	printf("[ERR]: No remaining shader memory!\n");
	return -1;
}

void DrawPostEffects(ShaderId shader)
{
	if (shader >= MAX_SHADERS)
	{
		printf("[ERR]: Invalid shader ID. Perhaps shaders aren't available?\n");
		return;
	}

	// Update the postprocess texture to contain the current window contents.
	g_postprocess_texture.update(g_window);
	g_shaders[shader].setParameter("texture", g_postprocess_texture);

	// Draw the postprocess texture across the whole screen, applying the given shader.
	sf::Sprite fs_sprite;
	fs_sprite.setTexture(g_postprocess_texture);
	g_window.draw(fs_sprite, &g_shaders[shader]);
}

void SetShaderParameter(ShaderId shader, const char* name, float val)
{
	if(shader >= MAX_SHADERS)
	{
		printf("[ERR]: Invalid shader ID. Perhaps shaders aren't available?\n");
		return;
	}

	g_shaders[shader].setParameter(name, val);
}

//////////////////////////////////////////////////////////////////////////
// Sound API
//////////////////////////////////////////////////////////////////////////

SoundId LoadSound(const char* path)
{
	if(g_total_sounds < MAX_SOUNDS)
	{
		if(g_sound_buffers[g_total_sounds].loadFromFile(path))
		{
			return g_total_sounds++;
		}

		printf("[ERR]: Couldn't load sound file from %s\n", path);
		return -1;
	}

	printf("[ERR]: No remaining sound memory!\n");
	return -1;
}

SoundInstanceId PlaySound(SoundId sound, float volume, float pitch, bool loop)
{
	if (sound >= g_total_sounds)
		return -1;

	for (int i = 0; i < MAX_SOUNDS; ++i)
	{
		if(g_sounds[i].getStatus() == sf::SoundSource::Stopped)
		{
			g_sounds[i].setBuffer(g_sound_buffers[sound]);
			g_sounds[i].setVolume(volume * volume * 100.0f);
			g_sounds[i].setPitch(pitch);
			g_sounds[i].setLoop(loop);
			g_sounds[i].play();
			return i;
		}
	}

	return -1;
}

float GetSoundVolume(SoundInstanceId sound_instance)
{
	if(sound_instance >= g_total_sounds)
	{
		printf("[ERR]: Invalid sound channel ID.\n");
		return 0;
	}

	float vol = g_sounds[sound_instance].getVolume();
	return sqrt(vol / 100.0f);
}

void SetSoundVolume(SoundInstanceId sound_instance, float volume)
{
	if(sound_instance >= g_total_sounds)
	{
		printf("[ERR]: Invalid sound channel ID.\n");
		return;
	}

	g_sounds[sound_instance].setVolume(volume * volume * 100.0f);
}

void StopSound(SoundInstanceId sound)
{
	if(sound >= g_total_sounds)
	{
		printf("[ERR]: Invalid sound channel ID.\n");
		return;
	}

	if(g_sounds[sound].getStatus() != sf::SoundSource::Stopped)
		g_sounds[sound].stop();
}

void StopAllSounds()
{
	for (int i = 0; i < MAX_SOUNDS; ++i)
	{
		if (g_sounds[i].getStatus() == sf::SoundSource::Playing)
		{
			g_sounds[i].stop();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Random API
//////////////////////////////////////////////////////////////////////////

double RandNorm()
{
	// xorshift128+
	u64 s0 = g_random_seeds[1];
	u64 s1 = g_random_seeds[0];
	g_random_seeds[0] = s0;
	s1 = s1 << 23;
	g_random_seeds[1] = s1 ^ s0 ^ (s1 >> 18) ^ (s0 >> 5);
	u64 rand_u64 = g_random_seeds[1] + s0;
	return double(rand_u64) / u64(-1);
}

f4 RandPastelCol()
{
	f3 rgb((float)RandNorm(), (float)RandNorm(), (float)RandNorm());
	rgb += 1.0f;
	rgb *= 0.5f;
	return f4(rgb, 1.0f); 
}