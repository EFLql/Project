#include "SocketAddr.h"

using libext::SocketAddr;

SocketAddr::SocketAddr()
{
} 

SocketAddr::SocketAddr(const char* ip, uint16_t port)
{
    setFromIpAddr(ip, port);
}

SocketAddr::SocketAddr(const std::string& ip, uint16_t port)
{
    setFromIpAddr(ip, port);
}

SocketAddr::SocketAddr(const SocketAddr& addr)
{
    addr_.sin_family = addr.addr_.sin_family;
    addr_.sin_port = addr.addr_.sin_port;
    addr_.sin_addr = addr.addr_.sin_addr;
}

SocketAddr::SocketAddr(const SocketAddr&& addr)
{
    addr_.sin_family = addr.addr_.sin_family;
    addr_.sin_port = addr.addr_.sin_port;
    addr_.sin_addr = addr.addr_.sin_addr;
}

SocketAddr& SocketAddr::operator=(const SocketAddr& addr)
{
    if(this != &addr)
    {
        addr_.sin_family = addr.addr_.sin_family;
        addr_.sin_port = addr.addr_.sin_port;
        addr_.sin_addr = addr.addr_.sin_addr;
    }
    return *this;
}

void SocketAddr::setFromIpAddr(const char* ip, uint16_t port)
{
    addr_.sin_family = AF_INET;
    addr_.sin_port = hton(port);
    addr_.sin_addr = inet_addr(ip);
    addr_.sin_zero[0] = '\0'; 
}

void SocketAddr::setFromLocalPort(uint16_t port)
{
    addr_.sin_family = AF_INET;
    addr_.sin_port = hton(port);
    addr_.sin_addr = INADDR_ANY;
    addr_.sin_zero[0] = '\0';
}

