#pragma once
/*-inl.h文件介绍
 *在c++中的模板编程中，由于模板类型需要在
 *编译器期间决定，所以模板函数或类的实现
 *需要写在.h文件里面,否则编译会报错
 *但是如果所有的模板函数或类都放在.h文件里面
 *实现的话将导致代码可读性下降，所以这里引用
 *一种新的文件-inl.h文件代替实现.cpp文件专门
 *用来实现模板函数或类，达到声明和实现分离
 */

namespace libext
{
template <class H>
PipelineBase& PipelineBase::addFront(std::shared_ptr<H> handler)
{
    typedef typename ContextType<H>::type Context;
    return addHelper(std::make_shared<Context>(shared_from_this(), std::move(handler)), true); 
}

template <class H>
PipelineBase& PipelineBase::addFront(H&& handler)
{
    typedef typename ContextType<H>::type Context;
    return addHelper(std::make_shared<Context>(shared_from_this(), handler), true); 
}

template <class H>
PipelineBase& PipelineBase::addFront(H* handler)
{
    typedef typename ContextType<H>::type Context;
    return addFront(std::shared_ptr<H>(handler));
}

template <class H>
PipelineBase& PipelineBase::addBack(std::shared_ptr<H> handler)
{
    typedef typename ContextType<H>::type Context;
    return addHelper(std::make_shared<Context>(shared_from_this(), std::move(handler)), false); 
}

template <class H>
PipelineBase& PipelineBase::addBack(H&& handler)
{
    typedef typename ContextType<H>::type Context;
    return addHelper(std::make_shared<Context>(shared_from_this(), handler), false); 
}

template <class H>
PipelineBase& PipelineBase::addBack(H* handler)
{
    typedef typename ContextType<H>::type Context;
    return addBack(std::shared_ptr<H>(handler));
}

template <class Context>
PipelineBase& PipelineBase::addHelper(std::shared_ptr<Context>&& ctx, bool front)
{
    ctxs_.insert(front ? ctxs_.begin() : ctxs_.end(), ctx);
    if(Context::dir == HandlerDir::BOTH || Context::dir == HandlerDir::IN)
    {
        inCtxs_.insert(front ? inCtxs_.begin() : inCtxs_.end(), ctx.get());
    }
    else if(Context::dir == HandlerDir::BOTH || Context::dir == HandlerDir::OUT)
    {
        outCtxs_.insert(front ? outCtxs_.begin() : outCtxs_.end(), ctx.get());
    }

    return *this;
}

template <class H>
PipelineBase& PipelineBase::remove()
{
    return removeHelper<H>(NULL, false);
}

template <class H>
PipelineBase& PipelineBase::remove(H* handler)
{
    return removeHelper<H>(handler, true);
}

template <class H>
PipelineBase& PipelineBase::removeHelper(H* handler, bool checkEqual)
{
    bool removed = false;
    typedef typename ContextType<H>::type Context;
    //ctxs_中删除
    std::vector<std::shared_ptr<PipelineContext>>::iterator itr;
    for(itr = ctxs_.begin(); itr != ctxs_.end(); )
    {
        Context* ctx = dynamic_cast<Context*>(*itr);
        if(ctx && (!checkEqual || ctx->getHandler() == handler))
        {
            itr = removeAt(itr);
            removed = true;
            if(itr == ctxs_.end())
            {
                break;
            }
        }
        else
        {
            ++itr;
        }
    }

    if(!removed)
    {
        throw std::invalid_argument("No such handler in pipeline");
    }

    return *this;
}

template <class H>
H* PipelineBase::getHandler()
{
    return getContext<H>()->getHandler();
}

template <class H>
H* PipelineBase::getHandler(int i)
{
    return getContext<H>(i)->getHandler();
}

template <class H>
typename ContextType<H>::type* PipelineBase::getContext()
{
    typedef typename ContextType<H>::type Context;
    for(auto& pipelineCtx : ctxs_)
    {
        auto ctx = dynamic_cast<Context*>(pipelineCtx);
        if(ctx)//默认找第一个合法的context返回
        {
            return ctx;
        }
    }
    return NULL;
}

template <class H>
typename ContextType<H>::type* PipelineBase::getContext(int i)
{
    if(i < 0 || i >= ctxs_.size() )
    {
        throw std::invalid_argument("argument is invalid");
    }
    typedef typename ContextType<H>::type Context;
    auto ctx = dynamic_cast<Context*>(ctxs_[i].get());
    CHECK(ctx);//因为context是类内部封装上去的，不存在参数错误，如果这个地方ctx==null直接abort
    return ctx;
}

//建立处理链表，同一类型的ctx链接到一起
template <class R, class W>
void Pipeline<R, W>::finalize()
{
    front_ = NULL;
    //inCtxs_建立
    if(!inCtxs_.empty())
    {
        front_ = dynamic_cast<InboundLink<R>*>(inCtxs_[0]);
        for(int i = 0; i < inCtxs_.size() - 1; i ++)
        {
            inCtxs_[i]->setNextIn(inCtxs_[i + 1]);
        }
        inCtxs_[inCtxs_.size() - 1]->setNextIn(NULL);
    }
    
    back_ = NULL;
    if(!outCtxs_.empty())
    {    
        back_ = dynamic_cast<OutboundLink<W>*>(outCtxs_[outCtxs_.size() - 1]);
        for(int i = outCtxs_.size() - 1; i > 0 ; i ++)//方向应该和inCtxs相反
        {
            outCtxs_[i]->setNextOut(outCtxs_[i - 1]);
        }
        outCtxs_[0]->setNextOut(NULL);
    }

    if(!front_)
    {
        //LOG-WARN
        std::cout<<"No inbound handler in pipeline, inbound operation will throw \
            std::invalid_argument"<<std::endl;
            
    }

    if(!back_)
    {
        //LOG-WARN
        std::cout<<"No inbound handler in pipeline, inbound operation will throw \
            std::invalid_argument"<<std::endl;
    }
}

template <class R, class W>
void Pipeline<R, W>::read(R msg)
{
    if(!front_)
    {
        throw std::invalid_argument("no inbound handler in pipeline");
    }
    front_->read(std::forward<R>(msg));
}

template <class R, class W>
void Pipeline<R, W>::readEOF()
{
    if(!front_)
    {
        throw std::invalid_argument("no inbound handler in pipeline");
    }
    front_->readEOF();
}

template <class R, class W>
void Pipeline<R, W>::transportActive()
{
    if(!front_)
    {
        throw std::invalid_argument("no inbound handler in pipeline");
    }
    front_->transportActive();
}

template <class R, class W>
void Pipeline<R, W>::transportInActive()
{
    if(!front_)
    {
        throw std::invalid_argument("no inbound handler in pipeline");
    }
    front_->transportInActive();
}

template <class R, class W>
void Pipeline<R, W>::write(W msg)
{
    if(!back_)
    {
        throw std::invalid_argument("no outbound handler in pipeline");
    }
    back_->write(std::forward<W>(msg));
}

template <class R, class W>
void Pipeline<R, W>::close()
{
    if(!back_)
    {
        throw std::invalid_argument("no outbound handler in pipeline");
    }
    back_->close();
}

} //libext
