#define __AUDIO_CPP

#include "Audio.hpp"
#include "Debug.hpp"

#undef __AUDIO_CPP

namespace NESwitch
{
Time::Time(float t)
{
  whole = int(t);
  frac = t - whole;
}

Time::operator float()
{
  return whole + frac;
}

Time& Time::operator=(float rhs)
{
  whole = int(rhs);
  frac = rhs - whole;
  return *this;
}

Time& Time::operator+=(float rhs)
{
  return operator=(*this + rhs);
}

Time& Time::operator-=(float rhs)
{
  return operator=(*this - rhs);
}

/////////////////////////////////

Audio* Audio::mainAudio = nullptr;
float Audio::avgCallbackTime = 0;
float Audio::callbackTimes[64];
std::vector<float> Audio::lastBuffer;
int Audio::lastBufferCount = 0;

Audio::Audio()
{
  if (mainAudio == nullptr)
    mainAudio = this;
  else
  {
    Debug::LogError("Can't have multiple NESwitch::Audios!");
    throw std::exception("Can't have multiple NESwitch::Audios!");
  }
}

Audio::~Audio()
{
  mainAudio = nullptr;
  callback = nullptr;
  if (setup)
  {
    SDL_PauseAudio(1);
    SDL_CloseAudio();
    setup = false;
  }
}

Audio* Audio::Get()
{
  return mainAudio;
}

void Audio::Setup(Callback callback, void* udata)
{
  if (setup)
  {
    Debug::LogError("Can't setup Audio multiple times!");
    return;
  }

  SDL_AudioSpec desiredSpec;
  desiredSpec.freq = 44100;
  desiredSpec.format = AUDIO_F32SYS;
  desiredSpec.channels = 1;
  desiredSpec.callback = Audio::CallbackBootstrap;
  desiredSpec.userdata = udata;
  SDL_OpenAudio(&desiredSpec, &audioSpec);
  if (audioSpec.format != AUDIO_F32SYS)
  {
    Debug::LogError("Could not get correct audio spec!");
    SDL_CloseAudio();
    return;
  }
  this->callback = callback;
  SDL_PauseAudio(0);
  setup = true;
}

SDL_AudioSpec Audio::GetSpec()
{
  return audioSpec;
}

float Audio::ApproximateSin(float t)
{
  t *= 0.05f * float(M_PI);
  t -= int(t);
  return 20.785f * t * (t - 0.5f) * (t - 1.0f);
}

float Audio::Triangle(float t, float p)
{
  return Saw(2 * t + float(M_PI), p) * Square(t + 0.5f * float(M_PI), 0.5f);
}

float Audio::Saw(float t, float p)
{
  t /= 2.0f * float(M_PI);
  t += p;
  return 2 * (-(t - int(t)) + 0.5f);
}

float Audio::PulseSaw(float t, float p, int harmonics)
{
  float r = 0;
  for (int i = 1; i < harmonics; ++i)
    r += ApproximateSin((t - p * 2.0f * float(M_PI)) * float(i)) / float(i);
  return r / 1.75f;
}

float Audio::Square(float t, float d)
{
  return Saw(t, 0) - Saw(t, d) + 2 * (0.5f - d);
}

float Audio::PulseSquare(float t, float d, int harmonics)
{
  if (d > 1)
    d = 1;
  else if (d < 0)
    d = 0;
  float y0 = PulseSaw(t, 0, harmonics);
  float y1 = PulseSaw(t, d, harmonics);
  return (y0 - y1) / 1.06f;
}

void Audio::CallbackBootstrap(void* udata, uint8_t* stream, int len)
{
  auto start = std::chrono::high_resolution_clock::now();
  Audio* audio = Audio::Get();
  if (!audio)
    Debug::LogError("Attempting to call audio callback without an audio being created!");
  else if (audio->callback)
    audio->callback(udata, (float*)(stream), lastBufferCount = (len / sizeof(float)));

  lastBuffer.resize(lastBufferCount);
  for (int i = 0; i < lastBufferCount; ++i)
    lastBuffer[i] = ((float*)(stream))[i];

  auto end = std::chrono::high_resolution_clock::now();
  auto elapsed = end - start;
  long long micro = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
  float dt = micro * 0.000001f;

  int totalFrames = 64;
  for (int i = 0; i < totalFrames - 1; ++i)
    callbackTimes[i] = callbackTimes[i + 1];
  callbackTimes[totalFrames - 1] = dt;
  avgCallbackTime = 0.9f * avgCallbackTime + 0.1f * dt;
}
} // namespace NESwitch
