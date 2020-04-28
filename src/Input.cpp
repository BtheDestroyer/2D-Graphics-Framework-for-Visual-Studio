#define __INPUT_CPP

#include "Input.hpp"
#include "imgui.h"
#include "examples/imgui_impl_sdl.h"

#undef __INPUT_CPP

std::map<SDL_Keycode, Button> Input::keys;
Mouse Input::mouse;

Input::Input()
{
}

Input::~Input()
{
}

Button &Input::GetKey(SDL_Keycode k)
{
  if (keys.find(k) == keys.end())
    keys[k] = {false, false, false};
  return keys[k];
}

Button &Input::GetMouseButton(uint32_t b)
{
  switch (b)
  {
  default:
  case 0:
    return mouse.left;
  case 1:
    return mouse.right;
  case 2:
    return mouse.middle;
  }
}

int32_t Input::GetMouseX()
{
  return mouse.x;
}

int32_t Input::GetMouseY()
{
  return mouse.y;
}

int32_t Input::GetMouseWheel()
{
  return mouse.wheel;
}

void Input::Update()
{
  // Update key info
  for (auto &it : keys)
  {
    it.second.pressed = false;
    it.second.released = false;
  }
  // Update mouse info
  mouse.wheel = 0;
  int mouseX, mouseY;
  const int buttons = SDL_GetMouseState(&mouseX, &mouseY);
  mouse.dx = mouse.x - mouseX;
  mouse.dy = mouse.y - mouseY;
  mouse.x = mouseX;
  mouse.y = mouseY;
  mouse.left.pressed = false;
  if (!mouse.left.held && buttons & SDL_BUTTON(SDL_BUTTON_LEFT))
  {
    mouse.left.held = true;
    mouse.left.pressed = true;
  }
  else if (mouse.left.held && !(buttons & SDL_BUTTON(SDL_BUTTON_LEFT)))
  {
    mouse.left.held = false;
    mouse.left.released = true;
  }
  mouse.right.pressed = false;
  if (!mouse.right.held && buttons & SDL_BUTTON(SDL_BUTTON_RIGHT))
  {
    mouse.right.held = true;
    mouse.right.pressed = true;
  }
  else if (mouse.right.held && !(buttons & SDL_BUTTON(SDL_BUTTON_RIGHT)))
  {
    mouse.right.held = false;
    mouse.right.released = true;
  }
  mouse.middle.pressed = false;
  if (!mouse.middle.held && buttons & SDL_BUTTON(SDL_BUTTON_MIDDLE))
  {
    mouse.middle.held = true;
    mouse.middle.pressed = true;
  }
  else if (mouse.middle.held && !(buttons & SDL_BUTTON(SDL_BUTTON_MIDDLE)))
  {
    mouse.middle.held = false;
    mouse.middle.released = true;
  }
}

bool Input::HandleEvent(SDL_Event *event)
{
  if (event)
  {
    ImGui_ImplSDL2_ProcessEvent(event);
    if (event->type == SDL_KEYDOWN || event->type == SDL_KEYUP)
    {
      Button &b = GetKey(event->key.keysym.sym);

      if (event->type == SDL_KEYDOWN && !b.held)
      {
        b.pressed = b.held = true;
      }
      else if (event->type == SDL_KEYUP && b.held)
      {
        b.held = false;
        b.released = true;
      }
      return true;
    }
    else if (event->type == SDL_MOUSEWHEEL)
    {
      mouse.wheel = event->wheel.y;
      return true;
    }
  }
  return false;
}
