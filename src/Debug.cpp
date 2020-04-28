#define __DEBUG_CPP

#include <iostream>
#include "Debug.hpp"

#undef __DEBUG_CPP

std::mutex Debug::printing;

Debug::Debug()
{
  
}

Debug::~Debug()
{
  
}

void Debug::Log(std::string msg)
{
  printing.lock();
  std::cout << msg << std::endl;
  printing.unlock();
}
