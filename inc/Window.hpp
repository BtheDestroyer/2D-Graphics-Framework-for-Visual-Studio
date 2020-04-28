#ifndef __WINDOW_HPP
#define __WINDOW_HPP
#include <string>
#include <vector>
#include <imgui.h>
#include <mutex>
#ifdef __WIN32
#ifndef _MSC_VER
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif
#else
#include <SDL.h>
#endif

std::string hex(uint64_t n, uint8_t d);

static const int SCREEN_WIDTH = 256;
static const int SCREEN_HEIGHT_STD = 240;
static const int SCREEN_HEIGHT_4_3 = 341;


static const float SCREEN_STRETCH_STD = 1;
static const float SCREEN_STRETCH_4_3 = float(SCREEN_HEIGHT_4_3) / float(SCREEN_HEIGHT_STD);

static const int SCREEN_HEIGHT = SCREEN_HEIGHT_STD;
static const float SCREEN_STRETCH = SCREEN_STRETCH_STD;

static const float SCREEN_ASPECT = float(SCREEN_WIDTH) / float(SCREEN_HEIGHT);

static const int RESOLUTION_SCALE = 2;

struct Pixel
{
  union
  {
    uint32_t n = 0xFF000000;
    struct
    {
      uint8_t r;  uint8_t g;  uint8_t b;  uint8_t a;
    };
  };

  Pixel();
  Pixel(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255);
  Pixel(uint32_t p);

  enum Mode { NORMAL, MASK, ALPHA, CUSTOM };
  bool operator==(const Pixel& p) const;
  bool operator!=(const Pixel& p) const;

  operator ImVec4() const
  {
    return ImVec4(float(r) / 0xFF, float(g) / 0xFF, float(b) / 0xFF, float(a) / 0xFF);
  }
  operator uint32_t() const
  {
    return n;
  }
};

namespace Color {
static const Pixel
  WHITE(255, 255, 255),
  GREY(192, 192, 192), DARK_GREY(128, 128, 128), VERY_DARK_GREY(64, 64, 64),
  RED(255, 0, 0), DARK_RED(128, 0, 0), VERY_DARK_RED(64, 0, 0),
  YELLOW(255, 255, 0), DARK_YELLOW(128, 128, 0), VERY_DARK_YELLOW(64, 64, 0),
  GREEN(0, 255, 0), DARK_GREEN(0, 128, 0), VERY_DARK_GREEN(0, 64, 0),
  CYAN(0, 255, 255), DARK_CYAN(0, 128, 128), VERY_DARK_CYAN(0, 64, 64),
  BLUE(0, 0, 255), DARK_BLUE(0, 0, 128), VERY_DARK_BLUE(0, 0, 64),
  MAGENTA(255, 0, 255), DARK_MAGENTA(128, 0, 128), VERY_DARK_MAGENTA(64, 0, 64),
  BLACK(0, 0, 0),
  BLANK(0, 0, 0, 0);
} // /Namespace Color

class Sprite
{
public:
  Sprite();
  Sprite(int32_t w, int32_t h);
  ~Sprite();

  void Resize(int32_t w, int32_t h);
  Pixel GetPixel(int32_t x, int32_t y);
  bool  SetPixel(int32_t x, int32_t y, Pixel p);
  Pixel Sample(float x, float y);

  enum Mode { NORMAL, PERIODIC };
private:
  int32_t width = 0;
  int32_t height = 0;
  Pixel *pColData = nullptr;
  Mode modeSample = Mode::NORMAL;
};

class Window
{
public:
  Window(std::string title, unsigned width, unsigned height);
  ~Window();

  void SetPixelMode(Pixel::Mode m);
  void Clear(Pixel color = Color::WHITE);
  void DrawRect(SDL_Rect *rect, unsigned char r, unsigned char g, unsigned char b);
  void DrawRect(SDL_Rect rect, unsigned char r, unsigned char g, unsigned char b);
  void DrawRect(SDL_Rect *rect, Pixel color);
  void DrawRect(SDL_Rect rect, Pixel color);
  void DrawPixel(int32_t x, int32_t y, unsigned char r, unsigned char g, unsigned char b);
  void DrawPixel(int32_t x, int32_t y, Pixel color);

  void SwapBuffers();

  void EndFrame();
  void Update();
  bool HandleEvent(SDL_Event *event);

  static void ConstructFontSheet();

  SDL_Window *GetSDLWindow();
  SDL_Renderer* GetSDLRenderer();
  SDL_Renderer *GetSDLTextureRenderer();
  void SetSDLRenderTarget(SDL_Texture* target);
  void ReleaseSDLRenderer();
  void SetSDLTextureRenderTarget(SDL_Texture *target);
  void ReleaseSDLTextureRenderer();

  int GetId();
  static Window *GetWindow();
  static Window *GetWindow(uint8_t id);

  const int resX, resY;

private:
  static Window* mainWindow;
  SDL_Window *sdlWindow = nullptr;
  SDL_Renderer* sdlRenderer = nullptr;
  SDL_Renderer *sdlTextureRenderer = nullptr;
  SDL_Texture* sdlRTarget = nullptr;
  SDL_Texture *sdlRTextureTarget = nullptr;
  static uint8_t count;
  static Sprite *fontSprite;
  bool midFrame;
  int id = -1;

  Pixel::Mode nPixelMode;

  static std::vector<Window*> windows;

  std::mutex rendererLocked;
  std::mutex rendererTextureLocked;
};

#endif
