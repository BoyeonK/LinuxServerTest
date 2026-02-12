#pragma once
#include <string>
#include <cstring>
#include <arpa/inet.h>

class NetAddress {
public:
    NetAddress() {
        memset(&_sockaddr, 0, sizeof(_sockaddr));
    }

    NetAddress(struct sockaddr_in sockaddr) : _sockaddr(sockaddr) {}
    NetAddress(std::string ip, uint16_t port);

    struct sockaddr_in& GetSockAddr() { return _sockaddr; }
    std::string GetIpAddress();

    uint16_t GetPort() { return ntohs(_sockaddr.sin_port); }

    static struct in_addr Ip2Address(const char* ip);

private:
    struct sockaddr_in _sockaddr = {};
};
