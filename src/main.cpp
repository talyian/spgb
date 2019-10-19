#if defined _WIN32
#include "win32/main.cpp"
#elif defined __linux__
#include "platform_linux/main.cpp"
#endif
