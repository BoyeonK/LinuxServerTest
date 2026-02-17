#include "S2HPacketHandler.h"

using namespace std;

bool Handle_Invalid(unsigned char* buffer, int32_t len) {
#ifdef _DEBUG
    cout << "Something goes wrong, Client sent invalid packet" << endl;
#endif
    return false;
}