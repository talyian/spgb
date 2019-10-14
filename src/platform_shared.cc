#include "platform.hh"

void logs::_log(u8 v) { _logx8(v); }
void logs::_log(u16 v) { _logx16(v); }
void logs::_log(u32 v) { _logx32(v); }
void logs::_log(i32 v) { _logf(v); }
void logs::_log(double f) { _logf(f); }
void logs::_log(const char * s) { _logs(s, sslen(s)); }
void logs::_log(void * s) { _logp(s); }
