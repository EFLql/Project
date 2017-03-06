#pragma once
#include <libext/typedef.h>

#include <string>
#include <netinet/in.h>
#include <arpa/inet.h>
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

    inline struct sockaddr_in getSocketAddr() const
    {
        return addr_;
    }

private:
    struct sockaddr_in addr_;
};

}//libext
