#pragma once
#include <libext/asyn/EventBase.h>
#include <libext/asyn/EventHandler.h>
#include <libext/lock/SpinLock.h>
#include <libext/FileUtil.h>
#include <errno.h>
#include <sys/eventfd.h>
#include <fcntl.h>
#include <queue>

namespace libext
{

template<typename MessageT>
class NotificationQueue
{
public:
    class Consumer : private EventHandler
    {
    public:
        enum : int32_t { kDefaultMaxReadAtOnce = 10};
        Consumer() : maxReadAtOnce_(kDefaultMaxReadAtOnce) {}
       ~Consumer() {}

       void init(EventBase* evb, int fd);
       void handerReady() override;
       void consumerUntilDrainned();
       virtual void messageAvailable(MessageT&& msg) = 0;
       void startConsuming(EventBase* evb, NotificationQueue* queue)
       {
           init(evb, queue);
           registHandler(READ | PERSIST);
       }
       void stopConsuming()
       {
           if(queue_ == NULL)//没有调用startconsuming直接调用stopconsuming
           {
               assert(!isHandlerRegisted());
               return;
           }
           {
               libext::SpinLockGuard g(queu_->spinlock_);
               queue_->numConsumers_ --;
               queue_->setActive(false);
           }
           assert(isHandlerRegisted());
           unregisterHandler();
           detachEventBase();
           queue_ = NULL;
       } 
    private:
       void consumeMessages(bool isDrain);
    private:
       int maxReadAtOnce_;
       NotificationQueue* queue_;
    };

    enum FdType
    {
        EVENTFD,
        PIPE,
    };
    NotificationQueue(uint32_t maxSize, FdType fdType = EVENTFD):
        eventfd_(-1),
        pid_(pid_t(getpid())),
        advisoryMaxQueueSize_(maxSize)
    {
        if(fdType == EVENTFD)
        {
            eventfd_ = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);//EFD_CLOEXEC在fork新进程时，文件描述符不会被关闭
            if(eventfd_ < 0)
            {
                if(errno ==  ENOSYS || errno == EINVAL)//ENOSYS:function not implemented; EINVAL:invalid argument
                {
                    //eventfd is not available
                }
            }
            fdType = PIPE;
        }
        if(fdType == PIPE)
        {
            if(pipe(pipefds_))//pipe生成两个，一个只读一个只可写的相互关联的管道
            {
                //LOG_ERR
            }
            do
            {
                bool flags = false;
                //设置管道为非阻塞模式
                if(fcntl(pipefds_[0], F_SETFL, O_RDONLY | O_NONBLOCK) != 0)
                {
                    //LOG_ERR
                    break;
                }
                if(fcntl(pipefds_[1], F_SETFL, O_WRONLY | O_NONBLOCK) != 0)
                {
                    //LOG_ERR
                    break;
                }
                flags = true;
            }while(0);
            if(!flags)
            {
                ::close(pipefds_[0]);
                ::close(pipefds_[1]);
            }
        }
    }
    ~NotificationQueue()
    {
        if(eventfd_ >= 0)
        {
            ::close(eventfd_);
            eventfd_ = -1;
        }
        if(pipefds_[0] >= 0)
        {
            ::close(pipefds_[0]);
            pipefds_[0] = -1;
        }
        if(pipefds_[1] >= 0)
        {
            ::close(pipefds_[1] );
            pipefds[1]_ = -1;
        }
    }
    void setMaxQueueSize(uint32_t maxSize)
    {
        advisoryMaxQueueSize_ = maxSize;
    }
    void checkPid() { CHECK_EQ(pid_, pid_t(getid)); }
    //尝试插入队列，当队列满时则不差如队列并返回false
    bool tryPutMessage(const MessageT& data)
    {
        return putMessageImpl(data, advisoryMaxQueueSize_);
    }
    bool tryPutMessage(MessageT&& data)
    {
        return putMessageImpl(data, advisoryMaxQueueSize_);
    }

    //无论当前队列是否满，强制插入队列
    bool putMessage(const MessageT& data)
    {
        return putMessageImpl(data, 0);
    }
    bool putMessage(MessageT&& data)
    {
        return putMessageImpl(data, 0);
    }
    template <typename Iterator>
    bool putMessage(Iterator first, Iterator last)
    {

    }
private:
    //forbidden copy constructor and assignment operator
    NotificationQueue(const NotificationQue& ) = delete;
    NotificationQueue& operator= (const NotificationQue& ) = delete; 
    bool checkDraining()
    {
        if(draining_)
        {
            //LOG_ERR
            std::cout<<"error"<<std::endl;
        }
        return draining_;
    }
    bool checkQuesize(uint32_t maxSize)
    {
        if(maxSize > 0 && queue_.size() > maxSize)
        {
            //LOG_ERR
            std::cout<<"error"<<std::endl;
            return false;
        }
        return true;
    }
    bool putMessageImpl(const MessageT& data, uint32_t maxSize)
    {
        checkPid();
        bool signal = false;//是否通知所有的消费者消费
        //对象成员变量访问加锁访问
        libext::SpinLockGuard g(spinLock_);//自旋锁
        if(checkDraining() || !checkQuesize(maxSize) )
        {
            return false;
        }
        if(numActiveConsumers_ < numsConsumers_)//当队列元素较多时，将所有消费者全部唤醒提高消费速度
        {
            signal = true;
        }
        queue_.emplace_back(data);
        if(signal)
        {
            ensureSignalLocked(signal);
        }
        return true;
    }
    bool putMessageImpl(MessageT&& data, uint32_t maxSize)
    {
        checkPid();
        bool signal = false;//是否通知所有的消费者消费
        //对象成员变量访问加锁访问
        libext::SpinLockGuard g(spinLock_);//自旋锁
        if(checkDraining() || checkQuesize(maxSize) )
        {
            return false;
        }
        if(numActiveConsumers_ < numsConsumers_)//当队列元素较多时，将所有消费者全部唤醒提高消费速度
        {
            signal = true;
        }
        queue_.emplace_back(std::move(data));
        if(signal)
        {
            ensureSignalLocked(signal);
        }
        return true;
    }  
   
    void ensureSignal()
    {
        libext::SpinLockGuard g(spinLock_);//自旋锁
        ensureSignalLocked();
    }
    
    void ensureSignalLocked()
    {
        if(signal_)
        {
            return;
        }
        ssize_t bytes_written;
        ssize_t bytes_expected;
        do
        {
            if(eventfd_ >= 0)
            {
                uint64_t signal = 1;
                bytes_expected = static_cast<ssize_t>(sizeof(signal));
                bytes_written = ::write(eventfd_, &signal, bytes_expected);
            }
            else if(pipefds_[1] >= 0)
            {
                uint8_t signal = 1;
                bytes_expected = static_cast<ssize_t>(sizeof(signal));
                bytes_written = ::write(pipefds_[1], &signal, bytes_expected);
            }
        }while(bytes_written == -1 && errno == EINTR)
        if(bytes_written == bytes_expected)
        {
            signal_ = true;
        }
    }

    void drainSignalsLocked()//可能在eventfd_或管道无数据的时候调用
    {
        if(eventfd_ >= 0)
        {
            uint64_t message; 
            int ret = readNoInt(eventfd_, static_cast<void*>(&message), sizeof(message));
            CHECK(ret != -1 || errno != EAGAIN);
        }
        else if(pipefds_[0] >= 0)
        {
            uint8_t message;
            int ret = readNoInt(pipefds_[0], static_cast<void*>(message), sizeof(message));
            CHECK(ret != -1 || errno != EAGAIN);
        }
        if((signal_ && ret == 0) || (signal_ && ret > 0) )
        {
            //LOG_ERR
            std::cout<<"error"<<std::endl;
        }
        signal_ = false;
    }

    void syncSignalAndQueue()
    {
        libext::SpinLockGuard g(spinLock_);
        if(queue_.empty())
        {
            drainSignalsLocked();//消费掉eventfd_上的事件，避免libevent一直通知消费者消费
        }
        else
        {
            ensureSignalLocked();//否则继续通知消费者消费,属于消费值达到maxReadAtOnce_退出消费循环的条件
        }
    }

private:
    std::dequeue<MessageT> queue_;
    pid_t pid_;//用来记录对象在哪个进程里面创建的
    int eventfd_;
    int pipefds_[2];
    uint32_t advisoryMaxQueueSize_;
    bool signal_;//唤醒信号
    bool draining_;
    uint32_t numActiveConsumers_;
    uint32_t numConsumers_;
    SpinLock spinlock_;
};

template<typename MessageT>
void NotificationQueue<MessageT>::Consumer::init(EventBase* evb, NotificationQueue* queue)
{
    //此处因该对evb进行判空操作，但是为了尽早暴露问题，此处需要assert提醒,
    //应为即使对evb进行判空也已经是偏离原有逻辑运行，所以判空操作无意义
    assert(evb);
    assert(queue);

    assert(evb->isInEventBaseThread());//此时断言是运行在对应eventbase线程里面的
    assert(queue_ == NULL);//并且此时Consumer内部队列还没有被初始化过
    assert(!isHandlerRegisted());//并且Consumer也没有注册任何事件

    queue->checkPid();
    queue_ = queue;
    
    {
        libext::SpinLockGuard g(queue_->spinLock_);
        queue_->numConsumers_ ++;
    }
    queue_->ensureSignal();
    if(queue_->eventfd_ >= 0)
    {
        initHandler(evb, queue_->eventfd_);
    }
    else if(queue_->pipefds_[0] >= 0)
    {
        initHandler(evb, queue_->pipefds_[0]);
    }
}

template<typename MessageT>
void NotificationQueue<MessageT>::Consumer::handlerReady()
{
    consumeMessages(false);
}

template<typename MessageT>
bool NotificationQueue<MessageT>::Consumer::consumeUntilDrained()
{
    if(queue_ == NULL)
    {
        return false;
    }
    
    {
        libext::SpinLockGuard g(queue_->spinLock_);
        if(queue_->draining_ == true) return false;
        queue_->draining_ = true;
    }

    consumeMessages(true); 
    
    {
        libext::SpinLockGuard g(queue_->spinLock_);
        queue_->draining_ = false;
    }
    return true;
}

template<typename MessageT>
void NotificationQueue<MessageT>::Consumer::consumeMessages(bool isDrain)
{
    uint32_t numProcessed = 0;
    while(true)
    {
        if(queue_ == NULL)
        {
            break;
        }

        {

            libext::SpinLockGuard g(queue_->spinLock_);
            if(queue_->queue_.empty())
            {
                break;
            }
            MessageT msg(queue_->queue_.front());
            queue_->queue_.pop_front();
            numProcessed ++;
            //由于消息会一直被libevent发出通知消费者，当isDrain为ture时出现总是消耗不完的情况发生
            //所以这里当回调函数执行前队列为空时为准判断队列是否消耗完全
            bool wasEmpty = queue_->queue_.empty();
        }

        messageAvailable(std::move(msg));
        
        if(!isDrain && numProcessed >= maxReadAtOnce_)
        {
            break;
        }
        if(wasEmpty)
        {
            break;
        }
    }
    //退出循环的可能有：1.queue_为空指针即中途调用了stopConsuming函数停止消费
    //2.队列元素被消费完；3.消费值达到maxReadAtOnce
    //对于第2中情况当队列为空时需要通知其他消费者停止消费
    if(queue_)
    {
        queue_->syncSignalAndQueue();
    }
}
} //libext
