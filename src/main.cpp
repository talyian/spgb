#if defined _WIN32
#include "platform_windows/main_limited.cpp"
#elif defined __linux__
#include "platform_linux/main.cpp"
#endif
