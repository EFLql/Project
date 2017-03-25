#pragma once
#include <libext/asyn/AsyncServerSocket.h>
#include <libext/asyn/EventBase.h>
#include <libext/EventBaseManger.h>

namespace libext
{
class Acceptor : public AsyncServerSocket::AcceptCallback
{
public:
    Acceptor() {}
    ~Acceptor() {}
    void init(EventBase* evb)
    {
        CHECK(base_ == NULL || evb == base_);
        base_ = evb;
    }

    EventBase* getEventBase() const
    {
        return base_;
    }

private:
    EventBase* base_;
};


class AcceptorFactory
{
public:
    AcceptorFactory() {}
    ~AcceptorFactory() {}

    std::shared_ptr<Acceptor> newAcceptor(EventBase* evb)
    {
        std::shared_ptr<Acceptor> acceptor = std::make_shared<Acceptor>();
        acceptor->init(evb);
        return acceptor;
    } 
};

} //libext
