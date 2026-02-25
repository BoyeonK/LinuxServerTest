#pragma once

#include <sw/redis++/redis++.h>

class IoUringWrapper;
class HttpIPCSession;

extern IoUringWrapper* IORing;
extern HttpIPCSession* HttpSession;
extern sw::redis::Redis* pRedis;