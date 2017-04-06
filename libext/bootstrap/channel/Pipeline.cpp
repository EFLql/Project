#include <libext/bootstrap/channel/Pipeline.h>

namespace libext
{
PipelineBase& PipelineBase::removeFront()
{
    if(!ctxs_.empty())
    {
        throw std::invalid_argument("pipeline is empty");
    }
    removeAt(ctxs_.begin());
    return *this;
}

PipelineBase& PipelineBase::removeBack()
{
    if(!ctxs_.empty())
    {
        throw std::invalid_argument("pipeline is empty");
    }
    removeAt(--ctxs_.end());
    return *this;
}

//removeAt需要在ctxs_、InCtxs_, outCtxs_中同步删除掉itr指向的元素
PipelineBase::ContextItor PipelineBase::removeAt(const PipelineBase::ContextItor& itr)
{
    //inCtxs_删除
    HandlerDir dir = (*itr)->getDirection();    
    if(dir == HandlerDir::BOTH || dir == HandlerDir::IN)
    {
       auto itr2 = std::find(inCtxs_.begin(), inCtxs_.end(), (itr)->get());
       CHECK(itr2 != inCtxs_.end());
       inCtxs_.erase(itr2);
    }
    //outCtxs_删除
    if(dir == HandlerDir::BOTH || dir == HandlerDir::OUT)
    {
       auto itr2 = std::find(outCtxs_.begin(), outCtxs_.end(), (itr)->get());
       CHECK(itr2 != outCtxs_.end());
       outCtxs_.erase(itr2);
    }
    
    //因为inCtxs_和outCtxs_是引用ctxs_中元素所以ctxs_最后删除
    return ctxs_.erase(itr);
}

} //libext
