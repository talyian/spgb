#if defined _WIN32
#include "win32/main.cpp"
#elif defined WASM
#include "host_wasm.cpp"
#elif defined __linux__
#include "linux/main.cpp"
#endif
