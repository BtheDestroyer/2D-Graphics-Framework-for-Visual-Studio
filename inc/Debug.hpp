#ifndef __DEBUG_HPP
#define __DEBUG_HPP
#include <string>
#include <sstream>
#include <mutex>

class Debug
{
public:
  static void Log(std::string msg);
#define LogError(msg) Log("[ERR:" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "] " + std::string(msg))

private:
  Debug();
  ~Debug();

  static std::mutex printing;
};

#endif
