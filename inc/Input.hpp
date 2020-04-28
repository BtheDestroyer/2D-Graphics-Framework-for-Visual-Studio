#ifndef __INPUT_HPP
#define __INPUT_HPP
#include <cstdint>
#ifdef __WIN32
#ifndef _MSC_VER
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif
#else
#include <SDL.h>
#endif
#include <map>

struct Button
{
  bool pressed;
  bool held;
  bool released;
};

struct Mouse
{
  int x;
  int y;
  int wheel;
  int dx;
  int dy;
  Button left;
  Button right;
  Button middle;
};

class Input
{
public:
  Input();
  ~Input();

  static Button &GetKey(SDL_Keycode k);
  static Button &GetMouseButton(uint32_t b);
  static int32_t GetMouseX();
  static int32_t GetMouseY();
  static int32_t GetMouseWheel();

  void Update();
  bool HandleEvent(SDL_Event *event);

private:
  static std::map<SDL_Keycode, Button> keys;
  static Mouse mouse;
};

#endif
