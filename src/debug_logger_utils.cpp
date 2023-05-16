#include "debug_logger_utils.h"

//static member variables must be declared outside of the class in order to be defined. Without this line, the linker will complain. 
//When static members are declared inside the class, they are only declared, not defined. This means they donot have memory assigned to it yet.
//This is because static members are shared by all objects of the class. So, the memory is allocated only once when the static member is defined.
//This is done in the .cpp file, which is here.
std::shared_ptr<spdlog::logger> IVRDebugLogger::Logger_;