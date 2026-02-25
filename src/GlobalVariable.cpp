#include "GlobalVariable.h"

#include "IoUringWrapper.h"
#include "SocketWrapper.h"

IoUringWrapper* IORing = nullptr;
HttpIPCSession* HttpSession = nullptr;
sw::redis::Redis* pRedis = nullptr;