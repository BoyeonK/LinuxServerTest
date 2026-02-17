#include "GlobalVariable.h"

#include "IoUringWrapper.h"
#include "SocketWrapper.h"

IoUringWrapper* IORing = new IoUringWrapper();
HttpIPCSession* HttpSession = nullptr;