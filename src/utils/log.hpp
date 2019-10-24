#pragma once

#include "../base.hpp"
#include "../gb/lib_gb.hpp"
#include "str.hpp"

namespace logs {
void _log(u8 v);
void _log(u16 v);
void _log(u32 v);
void _log(i32 v);
void _log(double f);
void _log(const char * s);
void _log(str s);
void _log(void* s);

template<class T>
void log(T x) { _log(x); spgb_showlog (); }
template<class T, class ... TS>
void log(T x, TS ... xs) { _log(x); log(xs...); }
}
using logs::log;
using logs::_log;
