#if defined WASM
#include "host_wasm.cpp"
#elif defined _WIN32
#include "win32/main.cpp"
#elif defined __linux__
#include "linux/main.cpp"
#endif
