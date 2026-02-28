#include "NetAddress.h"
#include <iostream>

using namespace std;

NetAddress::NetAddress(string ip, uint16_t port) {
    memset(&_sockaddr, 0, sizeof(_sockaddr));
    _sockaddr.sin_family = AF_INET;
    _sockaddr.sin_addr = Ip2Address(ip.c_str());
    _sockaddr.sin_port = htons(port);
}

string NetAddress::GetIpAddress() {
    char buffer[100];
    // AF_INET: IPv4, src: 주소구조체 포인터, dst: 버퍼, size: 버퍼크기
    ::inet_ntop(AF_INET, &_sockaddr.sin_addr, buffer, sizeof(buffer));
    return string(buffer);
}

struct in_addr NetAddress::Ip2Address(const char* ip) {
    struct in_addr address;
    // 성공 시 1, 실패 시 0 또는 -1
    int ret = ::inet_pton(AF_INET, ip, &address);
    if (ret <= 0)
        address.s_addr = INADDR_NONE; 
    return address;
}