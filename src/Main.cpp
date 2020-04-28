#include "imgui.h"
#include "examples/imgui_impl_sdl.h"

#include "Debug.hpp"
#include "Window.hpp"
#include "Input.hpp"

class MainCore
{
public:
  MainCore()
    : window("SDL Template", 1280, 720)
  {
    Debug::Log("Starting...");
    Debug::Log("Building fontsheet...");
    Window::ConstructFontSheet();
    ImGui_ImplSDL2_Init(window.GetSDLWindow());
    // Multi threading mode
#if MULTITHREAD_MODE
    // Start Drawing
    graphicsThread = std::thread(&MainCore::DrawLoop, this);
#endif
    Debug::Log("Started!");
    running = true;
  }

  ~MainCore()
  {
    Debug::Log("Shutting down...");
    ImGui_ImplSDL2_Shutdown();
    // Multi threading mode
#if MULTITHREAD_MODE
    // Start Drawing
    graphicsThread.join();
#endif
  }

  void Update()
  {
    static auto last = std::chrono::high_resolution_clock::now();
    static auto now = std::chrono::high_resolution_clock::now();
    static auto elapsed = now - last;
    static long long micro = 0;
    static float dt = 0;
    now = std::chrono::high_resolution_clock::now();
    elapsed = now - last;
    micro = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
    dt = micro * 0.000001f;
    last = now;
    Update(dt);
  }

  void Update(float dt)
  {
    if (!running)
      return;
    timeTillRender += dt;
    input.Update();
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0)
    {
      if (e.type == SDL_QUIT)
        running = false;
      else
      {
        window.HandleEvent(&e);
        input.HandleEvent(&e);
      }
    }
    if (!ImGui::GetIO().WantCaptureMouse)
    {
      // free to detect clicks
    }
    if (!ImGui::GetIO().WantCaptureKeyboard)
    {
      // free to detect keyboard
    }

    // Single threading mode
#if !MULTITHREAD_MODE
    if (timeTillRender >= 1.0f / 60.0f)
    {
      Draw(dt);
      timeTillRender -= 1.0f / 60.0f;
    }
#endif
  }
  
  void DrawLoop()
  {
    auto last = std::chrono::high_resolution_clock::now();
    auto now = last;
    auto elapsed = now - last;
    long long micro;
    float dt = 0;
    while (running)
    {
      while (dt <= 1.0f / 60.0f)
      {
        now = std::chrono::high_resolution_clock::now();
        elapsed = now - last;
        micro = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
        dt = micro * 0.000001f;
      }
      Draw(dt);
      last = now;
    }
  }

  void Draw(float dt)
  {
    window.Update();
    UpdateImGUI(dt);

    window.SetSDLRenderTarget(nullptr);
    window.Clear(Color::VERY_DARK_GREY);
    window.ReleaseSDLRenderer();

    if (ImGui::BeginMainMenuBar())
    {
      if (ImGui::BeginMenu("File"))
      {
        if (ImGui::MenuItem("Open"))
        {
          Debug::Log("File->Open");
        }
        ImGui::EndMenu();
      }
      if (ImGui::BeginMenu("Edit"))
      {
        if (ImGui::MenuItem("Cut"))
        {
          Debug::Log("Edit->Cut");
        }
        if (ImGui::MenuItem("Copy"))
        {
          Debug::Log("Edit->Copy");
        }
        if (ImGui::MenuItem("Paste"))
        {
          Debug::Log("Edit->Paste");
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Undo"))
        {
          Debug::Log("Edit->Undo");
        }
        if (ImGui::MenuItem("Redo"))
        {
          Debug::Log("Edit->Redo");
        }
        ImGui::EndMenu();
      }
    }
    ImGui::EndMainMenuBar();
  }
  
  void UpdateImGUI(float dt)
  {
    ImGuiIO& io = ImGui::GetIO();
    io.DeltaTime = dt;
    io.MousePos = ImVec2(static_cast<float>(input.GetMouseX()), static_cast<float>(input.GetMouseY()));
    io.MouseDown[0] = input.GetMouseButton(0).held;
    io.MouseDown[1] = input.GetMouseButton(1).held;
    io.MouseDown[2] = input.GetMouseButton(2).held;
    io.MouseWheel = static_cast<float>(input.GetMouseWheel());
  }

  bool running = false;
  Window window;
  Input input;
  float timeTillRender = 0;

#if MULTITHREAD_MODE
  std::thread graphicsThread;
#endif
};

int main(int argc, char* argv[])
{
  MainCore core;
  while (core.running)
    core.Update();
  return 0;
}

