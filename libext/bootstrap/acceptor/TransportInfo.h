#pragma once

namespace libext
{
struct TransportInfo
{
    //连接握手完成时的时间搓(timestamp)
    std::chrono::steady_clock::time_point acceptTime{};

    //连接RTT(Round-trip time)
    std::chrono::microsends rtt{0};

    bool secure;
    //lql-need add......
};

}
