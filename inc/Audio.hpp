#ifndef __AUDIO_HPP
#define __AUDIO_HPP

#include <vector>
#include <cstdint>
#include <SDL_audio.h>

/// \brief NESwitch namespace
namespace NESwitch
{
  class Time
  {
  public:
    Time(float t = 0);

    operator float();
    Time& operator=(float rhs);
    Time& operator+=(float rhs);
    Time& operator-=(float rhs);

  private:
    int whole;
    float frac;
  };

class Audio
{
public:
  Audio();
  ~Audio();

  typedef void (*Callback)(void* udata, float* stream, int len);

  static Audio* Get();

  void Setup(Callback callback, void* udata);
  SDL_AudioSpec GetSpec();

  static float ApproximateSin(float t);
  static float Triangle(float t, float p = 0);
  static float Saw(float t, float p = 0);
  static float PulseSaw(float t, float p = 0, int harmonics = 10);
  static float Square(float t, float p = 0);
  static float PulseSquare(float t, float d = 0.5f, int harmonics = 10);

  static float avgCallbackTime;
  static float callbackTimes[64];
  static std::vector<float> lastBuffer;
  static int lastBufferCount;

private:
  static void CallbackBootstrap(void* udata, uint8_t* stream, int len);
  static Audio* mainAudio;

  SDL_AudioSpec audioSpec{};
  Callback callback = nullptr;
  bool setup = false;
  bool running = false;
};
} // namespace NESwitch

#endif
