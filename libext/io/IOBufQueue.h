#pragma once

namespace libext
{
/*
 * IOBufQueue 封装(encapsulate)IObuf链表，提供一系列方便
 * 的方法以供外部从链表中插入或删除数据
 */
class IOBufQueue
{
public:
    ~IOBufQueue() = default;
    //预分配内存空间
    std::pair<uint64_t, uint64_t> preallocate(
            uint64_t minAvailable,
            uint64_t allocationSize);
    //实际使用的内存空间,空余的内存空间供下次使用
    void postallocate(uint64_t size);
};

} //libext
