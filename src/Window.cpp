#define __WINDOW_CPP

#include <algorithm>
#include <string>

#include "Debug.hpp"
#include "Window.hpp"

#pragma warning(push, 0)
#include "imgui.h"
#include "imgui_sdl.h"
#pragma warning(pop)

#undef __WINDOW_CPP

std::string hex(uint64_t n, uint8_t d)
{
  std::string s(d, '0');
  for (int i = d - 1; i >= 0; i--, n >>= 4)
    s[i] = "0123456789ABCDEF"[n & 0xF];
  return s;
};

Pixel::Pixel() : r(0), g(0), b(0), a(255) {}
Pixel::Pixel(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha) : r(red), g(green), b(blue), a(alpha) {}
Pixel::Pixel(uint32_t p) : n(p) {}

bool Pixel::operator==(const Pixel& p) const
{
  return n == p.n;
}
bool Pixel::operator!=(const Pixel& p) const
{
  return n != p.n;
}

/////////////////////////////////////////////////////

Sprite::Sprite() : width(0), height(0), pColData(nullptr) {}

Sprite::Sprite(int32_t w, int32_t h)
{
  Resize(w, h);
}

Sprite::~Sprite()
{
  if (pColData) delete pColData;
}

void Sprite::Resize(int32_t w, int32_t h)
{
  width = w;
  height = h;
  pColData = new Pixel[width * height];
  for (int32_t i = 0; i < width * height; i++)
    pColData[i] = Pixel();
}

Pixel Sprite::GetPixel(int32_t x, int32_t y)
{
  if (modeSample == Sprite::Mode::NORMAL)
  {
    if (x >= 0 && x < width && y >= 0 && y < height)
      return pColData[y * width + x];
    else
      return Pixel(0, 0, 0, 0);
  }
  else
  {
    return pColData[std::abs(y % height) * width + std::abs(x % width)];
  }
}

bool  Sprite::SetPixel(int32_t x, int32_t y, Pixel p)
{
  if (x >= 0 && x < width && y >= 0 && y < height)
  {
    pColData[y*width + x] = p;
    return true;
  }
  else
    return false;
}

Pixel Sprite::Sample(float x, float y)
{
  return GetPixel(std::min((int32_t)((x * (float)width)), width - 1), std::min((int32_t)((y * (float)height)), height - 1));
}

/////////////////////////////////////////////////////

Window* Window::mainWindow = nullptr;
uint8_t Window::count = 0;
Sprite *Window::fontSprite = nullptr;
std::vector<Window*> Window::windows;

Window::Window(std::string title, unsigned width, unsigned height)
  : sdlWindow(nullptr), resX(width), resY(height), midFrame(false)
{
  if(count == 0)
  {
    if (SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO ) < 0)
    {
      Debug::LogError(std::string("SDL could not initialize! SDL_Error: ") + std::string(SDL_GetError()));
      return;
    }
  }
  sdlWindow = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, resX, resY, SDL_WINDOW_SHOWN);
  if (!SDL_GL_SetSwapInterval(0))
  {
    Debug::LogError(std::string("Could not disable vsync! SDL_Error: ") + std::string(SDL_GetError()));
  }
  if(sdlWindow == nullptr)
  {
    Debug::LogError(std::string("Window could not be created! SDL_Error: ") + std::string(SDL_GetError()));
    return;
  }
  int drivers = SDL_GetNumRenderDrivers();
  Debug::Log("Render driver count: " + std::to_string(drivers));
  sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
  if(sdlRenderer == nullptr)
  {
    Debug::LogError(std::string("Renderer could not be created! SDL_Error: ") + std::string(SDL_GetError()));
    return;
  }
  /*
  sdlTextureRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
  if (sdlTextureRenderer == nullptr)
  {
    Debug::LogError(std::string("Texture Renderer could not be created! SDL_Error: ") + std::string(SDL_GetError()));
    return;
  }
  */
  ImGui::CreateContext();
  ImGuiSDL::Initialize(sdlRenderer, resX, resY);
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.KeyMap[ImGuiKey_Tab] = SDL_SCANCODE_TAB;
  io.KeyMap[ImGuiKey_LeftArrow] = SDL_SCANCODE_LEFT;
  io.KeyMap[ImGuiKey_RightArrow] = SDL_SCANCODE_RIGHT;
  io.KeyMap[ImGuiKey_UpArrow] = SDL_SCANCODE_UP;
  io.KeyMap[ImGuiKey_DownArrow] = SDL_SCANCODE_DOWN;
  io.KeyMap[ImGuiKey_PageUp] = SDL_SCANCODE_PAGEUP;
  io.KeyMap[ImGuiKey_PageDown] = SDL_SCANCODE_PAGEDOWN;
  io.KeyMap[ImGuiKey_Home] = SDL_SCANCODE_HOME;
  io.KeyMap[ImGuiKey_End] = SDL_SCANCODE_END;
  io.KeyMap[ImGuiKey_Insert] = SDL_SCANCODE_INSERT;
  io.KeyMap[ImGuiKey_Delete] = SDL_SCANCODE_DELETE;
  io.KeyMap[ImGuiKey_Backspace] = SDL_SCANCODE_BACKSPACE;
  io.KeyMap[ImGuiKey_Space] = SDL_SCANCODE_SPACE;
  io.KeyMap[ImGuiKey_Enter] = SDL_SCANCODE_RETURN;
  io.KeyMap[ImGuiKey_Escape] = SDL_SCANCODE_ESCAPE;
  io.KeyMap[ImGuiKey_KeyPadEnter] = SDL_SCANCODE_KP_ENTER;
  io.KeyMap[ImGuiKey_A] = SDL_SCANCODE_A;
  io.KeyMap[ImGuiKey_C] = SDL_SCANCODE_C;
  io.KeyMap[ImGuiKey_V] = SDL_SCANCODE_V;
  io.KeyMap[ImGuiKey_X] = SDL_SCANCODE_X;
  io.KeyMap[ImGuiKey_Y] = SDL_SCANCODE_Y;
  io.KeyMap[ImGuiKey_Z] = SDL_SCANCODE_Z;
  SetSDLRenderTarget(nullptr);
  if (++count > windows.size())
  {
    windows.push_back(this);
    id = count - 1;
  }
  else
  {
    for (uint8_t i = 0 ; id == -1 && i < windows.size(); ++i)
    {
      if (windows[i] == nullptr)
      {
        windows[i] = this;
        id = i;
      }
    }
  }
  if (id == -1)
    Debug::LogError("Window couldn't be added to static list for some reason!");

  Debug::Log("Created window " + std::to_string(id));

  Clear();
  ReleaseSDLRenderer();
  Update();
}

Window::~Window()
{
  EndFrame();
  ImGuiSDL::Deinitialize();
  if (sdlTextureRenderer)
  {
    SDL_DestroyRenderer(sdlTextureRenderer);
    sdlTextureRenderer = nullptr;
  }
  if (sdlRenderer)
  {
    SDL_DestroyRenderer(sdlRenderer);
    sdlRenderer = nullptr;
  }
  if (sdlWindow != nullptr)
  {
    ImGui::DestroyContext();
    SDL_DestroyWindow(sdlWindow);
    sdlWindow = nullptr;
  }
  if (--count == 0)
  {
    if (fontSprite)
    {
      delete fontSprite;
      fontSprite = nullptr;
    }
    SDL_Quit();
  }
  if (id != -1)
  {
    if (windows[id] == mainWindow)
      mainWindow = nullptr;
    windows[id] = nullptr;
    id = -1;
  }
}

void Window::SetPixelMode(Pixel::Mode m)
{
  nPixelMode = m;
}

void Window::Clear(Pixel color)
{
  SDL_SetRenderDrawColor(sdlRenderer, color.r, color.g, color.b, 255);
  DrawRect({ 0, 0, resX / RESOLUTION_SCALE, resY / RESOLUTION_SCALE }, color);
  SDL_RenderClear(sdlRenderer);
  DrawRect({0, 0, resX / RESOLUTION_SCALE, resY / RESOLUTION_SCALE}, color);
}

void Window::DrawRect(SDL_Rect *rect, unsigned char r, unsigned char g, unsigned char b)
{
  SDL_SetRenderDrawColor(sdlRenderer, r, g, b, 255);
  if (!rect)
    SDL_RenderDrawRect(sdlRenderer, nullptr);
  else
  {
    SDL_Rect rect2{rect->x * RESOLUTION_SCALE, rect->y * RESOLUTION_SCALE, rect->w * RESOLUTION_SCALE,  rect->h * RESOLUTION_SCALE};
    SDL_RenderDrawRect(sdlRenderer, &rect2);
  }
}

void Window::DrawRect(SDL_Rect rect, unsigned char r, unsigned char g, unsigned char b)
{
  rect.x *= RESOLUTION_SCALE;
  rect.y *= RESOLUTION_SCALE;
  rect.w *= RESOLUTION_SCALE;
  rect.h *= RESOLUTION_SCALE;
  SDL_SetRenderDrawColor(sdlRenderer, r, g, b, 255);
  SDL_RenderDrawRect(sdlRenderer, &rect);
}

void Window::DrawRect(SDL_Rect *rect, Pixel color)
{
  SDL_SetRenderDrawColor(sdlRenderer, color.r, color.g, color.b, 255);
  if (!rect)
    SDL_RenderDrawRect(sdlRenderer, nullptr);
  else
  {
    SDL_Rect rect2{rect->x * RESOLUTION_SCALE, rect->y * RESOLUTION_SCALE, rect->w * RESOLUTION_SCALE,  rect->h * RESOLUTION_SCALE};
    SDL_RenderDrawRect(sdlRenderer, &rect2);
  }
}

void Window::DrawRect(SDL_Rect rect, Pixel color)
{
  rect.x *= RESOLUTION_SCALE;
  rect.y *= RESOLUTION_SCALE;
  rect.w *= RESOLUTION_SCALE;
  rect.h *= RESOLUTION_SCALE;
  SDL_SetRenderDrawColor(sdlRenderer, color.r, color.g, color.b, 255);
  SDL_RenderDrawRect(sdlRenderer, &rect);
}

void Window::DrawPixel(int32_t x, int32_t y, unsigned char r, unsigned char g, unsigned char b)
{
  DrawRect({x, y, 1, 1}, r, g, b);
}

void Window::DrawPixel(int32_t x, int32_t y, Pixel color)
{
  DrawRect({x, y, 1, 1}, color);
}

void Window::SwapBuffers()
{
  //SDL_UpdateWindowSurface(sdlWindow);
  SDL_RenderPresent(sdlRenderer);
}

void Window::EndFrame()
{
  if (midFrame)
  {
    ImGui::Render();
    SetSDLRenderTarget(nullptr);
    ImGuiSDL::Render(ImGui::GetDrawData());
    SwapBuffers();
    ReleaseSDLRenderer();
    midFrame = false;
  }
}

void Window::Update()
{
  EndFrame();
  ImGui::NewFrame();
  midFrame = true;
}

bool Window::HandleEvent(SDL_Event *event)
{
  return false;
}

void Window::ConstructFontSheet()
{
  std::string data;
  data += "?Q`0001oOch0o01o@F40o0<AGD4090LAGD<090@A7ch0?00O7Q`0600>00000000";
  data += "O000000nOT0063Qo4d8>?7a14Gno94AA4gno94AaOT0>o3`oO400o7QN00000400";
  data += "Of80001oOg<7O7moBGT7O7lABET024@aBEd714AiOdl717a_=TH013Q>00000000";
  data += "720D000V?V5oB3Q_HdUoE7a9@DdDE4A9@DmoE4A;Hg]oM4Aj8S4D84@`00000000";
  data += "OaPT1000Oa`^13P1@AI[?g`1@A=[OdAoHgljA4Ao?WlBA7l1710007l100000000";
  data += "ObM6000oOfMV?3QoBDD`O7a0BDDH@5A0BDD<@5A0BGeVO5ao@CQR?5Po00000000";
  data += "Oc``000?Ogij70PO2D]??0Ph2DUM@7i`2DTg@7lh2GUj?0TO0C1870T?00000000";
  data += "70<4001o?P<7?1QoHg43O;`h@GT0@:@LB@d0>:@hN@L0@?aoN@<0O7ao0000?000";
  data += "OcH0001SOglLA7mg24TnK7ln24US>0PL24U140PnOgl0>7QgOcH0K71S0000A000";
  data += "00H00000@Dm1S007@DUSg00?OdTnH7YhOfTL<7Yh@Cl0700?@Ah0300700000000";
  data += "<008001QL00ZA41a@6HnI<1i@FHLM81M@@0LG81?O`0nC?Y7?`0ZA7Y300080000";
  data += "O`082000Oh0827mo6>Hn?Wmo?6HnMb11MP08@C11H`08@FP0@@0004@000000000";
  data += "00P00001Oab00003OcKP0006@6=PMgl<@440MglH@000000`@000001P00000000";
  data += "Ob@8@@00Ob@8@Ga13R@8Mga172@8?PAo3R@827QoOb@820@0O`0007`0000007P0";
  data += "O`000P08Od400g`<3V=P0G`673IP0`@3>1`00P@6O`P00g`<O`000GP800000000";
  data += "?P9PL020O`<`N3R0@E4HC7b0@ET<ATB0@@l6C4B0O`H3N7b0?P01L3R000000020";

  if (fontSprite)
    delete fontSprite;
  fontSprite = new Sprite(128, 48);
  int px = 0, py = 0;
  for (int b = 0; b < 1024; b += 4)
  {
    uint32_t sym1 = (uint32_t)data[b + 0] - 48;
    uint32_t sym2 = (uint32_t)data[b + 1] - 48;
    uint32_t sym3 = (uint32_t)data[b + 2] - 48;
    uint32_t sym4 = (uint32_t)data[b + 3] - 48;
    uint32_t r = sym1 << 18 | sym2 << 12 | sym3 << 6 | sym4;

    for (int i = 0; i < 24; i++)
    {
      int k = r & (1 << i) ? 255 : 0;
      fontSprite->SetPixel(px, py, Pixel(k, k, k, k));
      if (++py == 48) { px++; py = 0; }
    }
  }
}
SDL_Window *Window::GetSDLWindow()
{
  return sdlWindow;
}

SDL_Renderer *Window::GetSDLRenderer()
{
  return sdlRenderer;
}

SDL_Renderer* Window::GetSDLTextureRenderer()
{
  if (sdlTextureRenderer)
    return sdlTextureRenderer;
  return sdlRenderer;
}

void Window::SetSDLRenderTarget(SDL_Texture *target)
{
  rendererLocked.lock();
  if (sdlRTarget != target)
    if (SDL_SetRenderTarget(sdlRenderer, sdlRTarget = target))
      Debug::LogError(std::string("Could not set render target! SDL_Error: ") + std::string(SDL_GetError()));
}

void Window::ReleaseSDLRenderer()
{
  rendererLocked.unlock();
}

void Window::SetSDLTextureRenderTarget(SDL_Texture* target)
{
  if (sdlTextureRenderer)
  {
    rendererTextureLocked.lock();
    if (sdlRTextureTarget != target)
      if (SDL_SetRenderTarget(sdlTextureRenderer, sdlRTextureTarget = target))
        Debug::LogError(std::string("Could not set render target! SDL_Error: ") + std::string(SDL_GetError()));
  }
  else
    SetSDLRenderTarget(target);
}

void Window::ReleaseSDLTextureRenderer()
{
  if (sdlTextureRenderer)
  {
    rendererTextureLocked.unlock();
  }
  else
    ReleaseSDLRenderer();
}

int Window::GetId()
{
  return id;
}

Window *Window::GetWindow()
{
  if (mainWindow)
    return mainWindow;
  for (uint8_t i = 0; i < windows.size(); ++i)
    if (windows[i] != nullptr)
      return mainWindow = windows[i];
  return nullptr;
}

Window *Window::GetWindow(uint8_t id)
{
  if (id < 0 || id >= windows.size())
    return nullptr;
  return windows[id];
}
