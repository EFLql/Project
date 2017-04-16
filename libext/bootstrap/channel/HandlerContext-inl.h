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
class PipelineContext
{
public:
    virtual ~PipelineContext() = default;

    virtual void attachPipeline() = 0;
    virtual void detachPipeline() = 0;

    //attachContext...

    virtual void setNextIn(PipelineContext* ctx) = 0;
    virtual void setNextOut(PipelineContext* ctx) = 0;

    virtual HandlerDir getDirection() = 0;
};

template <class In>
class InboundLink
{
public:
    virtual ~InboundLink() = default;

    virtual void read(In msg) = 0;
    virtual void readEOF() = 0;
    virtual void transportActive() = 0;
    virtual void transportInActive() = 0;
};

template <class Out>
class OutboundLink
{
public:
    virtual ~OutboundLink() = default;

    virtual void write(Out msg) = 0;
    virtual void close() = 0;
};

template <class H, class Context>
class ContextImplBase : public PipelineContext
{
public:
    virtual ~ContextImplBase() = default;
    H* getHandler()
    {
       return handler_.get();
    }
    void initialize(std::weak_ptr<PipelineBase> pipelineWeak,
                    std::shared_ptr<H> handler)
    {
        pipelineWeak_ = pipelineWeak;
        handler_ = std::move(handler);
        pipelineRaw_ = pipelineWeak.lock().get();
    }

    void attachPipeline() override
    {
    }
    void detachPipeline() override
    {
    }
    void setNextIn(PipelineContext* ctx)
    {
        if(!ctx)
        {
            nextIn_ = NULL;
            return;
        }
        //转换原则:能用static_cast则不能使用dynamic_cast,会有轻微的性能损失
        auto nextIn = dynamic_cast<InboundLink<typename H::rout>*>(ctx);//sidecast,只能用dynamic_cast才能转换
        if(nextIn)
        {
            nextIn_ = nextIn;
        }
        else
        {
            nextIn_ = NULL;
            throw std::invalid_argument("参数错误，不能转换为InboundLink指针");
        }
    }

    void setNextOut(PipelineContext* ctx)
    {
        if(!ctx)
        {
            nextOut_ = NULL;
            return;
        }
        auto nextOut = dynamic_cast<OutboundLink<typename H::wout>*>(ctx);
        if(nextOut)
        {
            nextOut_ = nextOut;
        }
        else
        {
            nextOut_ = NULL;
            throw std::invalid_argument("参数错误，不能转换为OutboundLink指针");//sidecast,只能用dynamic_cast才能转换
        }
    }
    HandlerDir getDirection() override
    {
        return H::dir;
    }

protected:
    Context* impl_;
    std::shared_ptr<H> handler_;
    std::weak_ptr<PipelineBase> pipelineWeak_;
    PipelineBase* pipelineRaw_{NULL};

    InboundLink<typename H::rout>* nextIn_{NULL};
    OutboundLink<typename H::wout>* nextOut_{NULL};
};

template <class H>
class ContextImpl 
    : public HandlerContext<typename H::rout, typename H::wout>,//注意和Inboundlink构造模板类型参数不一致
      public InboundLink<typename H::rin>,//注意和HandlerContext构造模板参数类型不一致
      public OutboundLink<typename H::win>,
      public ContextImplBase<H, HandlerContext<typename H::rout, 
                                               typename H::wout>>
{
public:
    typedef typename H::rout Rout;
    typedef typename H::rin Rin;
    typedef typename H::wout Wout;
    typedef typename H::win Win;
    static const HandlerDir dir = HandlerDir::BOTH;
    
    explicit ContextImpl(std::weak_ptr<PipelineBase> pipeline,
                        std::shared_ptr<H> handler)
    {
        this->impl_ = this;
        this->initialize(pipeline, std::move(handler));
    }
    
    //for staticPipeline
    ContextImpl()
    {
       this->impl_ = this;
    }
    
    ~ContextImpl() = default;

    //HandlerContext override
    void fireRead(Rout msg) override
    {
        if(this->nextIn_)
        {
            this->nextIn_->read(std::forward<Rout>(msg));
        }
        else
        {
            //LOG-INFO
            std::cout<<"read reached end of pipeline"<<std::endl;
        }
    }
    void fireReadEOF() override
    {
        if(this->nextIn_)
        {
            this->nextIn_->readEOF();
        }
        else
        {
            //LOG-INFO
            std::cout<<"read reached end of pipeline"<<std::endl;
        }
    }
    void fireTransportActive() override
    {
        if(this->nextIn_)
        {
            this->nextIn_->transportActive();
        }
    } 
    void fireTransportInActive() override
    {
        if(this->nextIn_)
        {
            this->nextIn_->transportInActive();
        }
    } 
    void fireWrite(Wout msg) override
    {
        if(this->nextOut_)
        {
            this->nextOut_->write(std::forward<Wout>(msg));
        }
        else
        {
            //LOG-INFO
            std::cout<<"Write reached end of pipeline"<<std::endl;
        }
    }
    void fireClose() override
    {
        if(this->nextOut_)
        {
            this->nextOut_->close();
        }
        else
        {
            //LOG-INFO
            std::cout<<"close reached end of pipeline"<<std::endl;
        }
    }
    PipelineBase* getPipeline() override
    {
        return this->pipelineRaw_;
    }

    //Inboundlink override
    void read(Rin msg) override
    {
        this->handler_->read(this, std::forward<Rin>(msg));
    }
    void readEOF() override
    {
        this->handler_->readEOF(this);
    }
    void transportActive() override
    {
        this->handler_->transportActive(this);
    }
    void transportInActive() override
    {
        this->handler_->transportInActive(this);
    }

    //outboundlink override
    void write(Win msg) override
    {
        this->handler_->write(this, std::forward<Win>(msg));
    }
    void close() override
    {
        this->handler_->close(this);
    }

};

template <class H>
class InboundContextImpl
        :public InboundHandlerContext<typename H::rout>,
        public InboundLink<typename H::rin>,
        public ContextImplBase<H, InboundHandlerContext<typename H::rout>>
{
public:
    typedef typename H::rout Rout;
    typedef typename H::rin Rin;
    static const HandlerDir dir = HandlerDir::IN;
    
    explicit InboundContextImpl(
            std::weak_ptr<PipelineBase> pipeline,
            std::shared_ptr<H> handler)
    {
        this->impl_ = this;
        this->initialize(pipeline, handler);
    }
    ~InboundContextImpl() = default;

    //InboundhandlerContext override
    void fireRead(Rout msg) override
    {
        if(this->nextIn_)
        {
            this->nextIn_->read(std::forward<Rout>(msg));
        }
        else
        {
            //lql LOG-INFO
            std::cout<<"read reached end of pipeline"<<std::endl;
        }
    }
    void fireReadEOF() override
    {
        if(this->nextIn_)
        {
            this->nextIn_->readEOF();
        }
        else
        {
            //lql LOG-INFO
            std::cout<<"readEOF reached end of pipeline"<<std::endl;
        }
    }
    void fireTransportActive() override
    {
        if(this->nextIn_)
        {
            this->nextIn_->transportActive();
        }
    }
    void fireTransportInActive() override
    {
        if(this->nextIn_)
        {
            this->nextIn_->transportInActive();
        }
    }
    PipelineBase* getPipeline() override
    {
        return this->pipelineRaw_;
    }
    
    //InboundLink override
    void read(Rin msg) override
    {
        this->handler_->read(this, std::forward<Rin>(msg));
    }
    void readEOF() override
    {
        this->handler_->readEOF(this);
    }
    void transportActive() override
    {
        this->handler_->transportActive(this);
    }
    void transportInActive() override
    {
        this->handler_->transportInActive(this);
    }
};


template <class H>
class OutboundContextImpl
        :public OutboundHandlerContext<typename H::wout>,
        public OutboundLink<typename H::win>,
        public ContextImplBase<H, OutboundHandlerContext<typename H::wout>>
{
public:
    typedef typename H::wout Wout;
    typedef typename H::win Win;
    static const HandlerDir dir = HandlerDir::OUT;
    
    explicit OutboundContextImpl(
            std::weak_ptr<PipelineBase> pipeline,
            std::shared_ptr<H> handler)
    {
        this->impl_ = this;
        this->initialize(pipeline, handler);
    }
    ~OutboundContextImpl() = default;

    //InboundhandlerContext override
    void fireWrite(Wout msg) override
    {
        if(this->nextIn_)
        {
            this->nextOut_->write(std::forward<Wout>(msg));
        }
        else
        {
            //lql LOG-INFO
            std::cout<<"read reached end of pipeline"<<std::endl;
        }
    }
    void fireClose() override
    {
        if(this->nextOut_)
        {
            this->nextOut_->close();
        }
        else
        {
            //lql LOG-INFO
            std::cout<<"readEOF reached end of pipeline"<<std::endl;
        }
    }
    PipelineBase* getPipeline() override
    {
        return this->pipelineRaw_;
    }
    
    //OutboundLink override
    void write(Win msg) override
    {
        this->handler_->write(this, std::forward<Win>(msg));
    }
    void close() override
    {
        this->handler_->close(this);
    }
};

//根据handler的类型确定Context的类型
template <class Handler>
struct ContextType
{
    typedef typename std::conditional<
        Handler::dir == HandlerDir::BOTH,
        ContextImpl<Handler>,
        typename std::conditional<
        Handler::dir == HandlerDir::IN,
        InboundContextImpl<Handler>,
        OutboundContextImpl<Handler>>::type>::type type;
};

} //libext
