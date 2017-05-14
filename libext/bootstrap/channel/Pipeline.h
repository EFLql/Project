#pragma oce
#include <libext/bootstrap/channel/HandlerContext.h>
#include <libext/io/IOBufQueue.h>
#include <libext/io/IOBuf.h>
#include <glog/logging.h>
#include <algorithm>
#include <memory>
#include <vector>
#include <type_traits>
#include <utility>

namespace libext
{
class AsyncTransport;
class PipelineManager
{
public:
    PipelineManager() = default;
};

class PipelineBase : public std::enable_shared_from_this<PipelineBase>
{
public:
    PipelineBase(): manager_(NULL) {}
    virtual ~PipelineBase() = default;
    PipelineManager* getPipelineManager() { return manager_; }
    void setPipelineManager(PipelineManager* manager) 
    {
        manager_ = manager;
    }
    /*extend setTransport...
     *
     *
     *
     */
    
    void setReadBufferSetting(uint64_t minAvailable, 
            uint64_t allocationSize);
    std::pair<uint64_t, uint64_t> getReadBufferSetting();
   
    template <class H>
    PipelineBase& addBack(std::shared_ptr<H> handler);
    template <class H>
    PipelineBase& addBack(H&& handler);
    template <class H>
    PipelineBase& addBack(H* handler);

    template <class H>
    PipelineBase& addFront(std::shared_ptr<H> handler);
    template <class H>
    PipelineBase& addFront(H&& handler);
    template <class H>
    PipelineBase& addFront(H* handler);
    
    template <class H>
    PipelineBase& remove(H* handler);
    
    template <class H>
    PipelineBase& remove();
    
    PipelineBase& removeBack();
   
    PipelineBase& removeFront();
    
    template <class H>
    typename ContextType<H>::type* getContext();

    template <class H>
    typename ContextType<H>::type* getContext(int i);
    
    template <class H>
    H* getHandler(int i);

    template <class H>
    H* getHandler();
    
    virtual void finalize() = 0;//构建链表
protected:
    std::vector<std::shared_ptr<PipelineContext>> ctxs_;
    std::vector<PipelineContext*> inCtxs_;
    std::vector<PipelineContext*> outCtxs_;

private:
    PipelineManager* manager_;

    std::pair<uint64_t, uint64_t> readBufferSetting_{2048, 2048};
    
    template <class Context>
    PipelineBase& addHelper(std::shared_ptr<Context>&& ctx, bool front);//bool front是否是增加到管道最前面

    template <class H>
    PipelineBase& removeHelper(H* handler, bool checkEqual);

    typedef std::vector<std::shared_ptr<PipelineContext>>::iterator ContextItor;
    ContextItor removeAt(const ContextItor& itr);
};

template <class R, class W>
class Pipeline : public PipelineBase
{
public:
    //using Ptr = std::shared_ptr<Pipeline>;
    typedef std::shared_ptr<Pipeline> Ptr;
    
    static Ptr create()
    {
        return std::shared_ptr<Pipeline>(new Pipeline());
    }

    ~Pipeline() = default;

    void read(R msg);
    void readEOF();

    void transportActive();
    void transportInActive();

    void write(W msg);
    void close();

    void finalize() override;

protected:
    Pipeline() {}
    explicit Pipeline(bool isStatic) : isStatic_(isStatic) {}
private:
    bool isStatic_{false};
    libext::InboundLink<R>* front_{NULL};//链表头,如果有删除操作会有bug
    libext::OutboundLink<W>* back_{NULL};//链表尾,如果有删除操作会有bug
};

template <typename Pipeline>
class PipelineFactory
{
public:
    virtual typename Pipeline::Ptr newPipeline(std::shared_ptr<AsyncTransport>) = 0;
    virtual ~PipelineFactory() = default;
};

typedef Pipeline<IOBufQueue&, std::unique_ptr<IOBuf>> DefaultPipeline;

} //libext namespace

#include <libext/bootstrap/channel/Pipeline-inl.h>
