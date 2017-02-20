#pragma once
#include "typedef.h"

#include <string>
#include <socket.h>

namespace libext
{
class SocketAddr
{
public:
    SocketAddr();
    ~SocketAddr();
    SocketAddr(const char* host, uint16_t port); 
    SocketAddr(const std::string& host, uint16_t port);
    SocketAddr(const SocketAddr& addr);
    SocketAddr(const SocketAddr&& addr);
    SocketAddr& operator=(const SocketAddr& addr);

public:
    void setFromIpAddr(const char* ip, uint16_t port);
    inline void setFromIpAddr(const std::string& ip, uint16_t port)
    {
        setFromIpAddr(ip.c_str(), port);
    }
    
    void setFromLocalPort(uint16_t port);

    inline struct socketaddr_in getSocketAddr() const
    {
        return addr_;
    }

private:
    struct socketaddr_in addr_;
};

}//libext
