#pragma once
#include <exception>
#include <string.h>
#include <errno.h>
#include <stdio.h>

namespace libext
{
class AsyncSocketException : public std::runtime_error
{
public:
    enum AsyncSocketExceptionType
    {
        UNKNOWN = 0,
        NOT_OPEN = 1,
        ALREADY_OPEN = 2,
        TIME_OUT = 3,
        END_OF_FILE = 4,
        INTERRUPTED = 5,
        BAD_ARGS = 6,
        CURRUPTED_DATA = 7,
        INTERNAL_ERROR = 8,
        NOT_SUPPORTED = 9,
        INVALID_STATE = 10,
        SSL_ERROR = 12,
        COULD_NOT_BIND = 13,
        SASL_HANDSHAKE_TIMEOUT = 14,
        NETWORK_ERROR = 15
    };

    AsyncSocketException(AsyncSocketExceptionType type, 
                         const std::string& message,
                         int errno_copy = 0)
        :std::runtime_error(
            AsyncSocketException::getMessage(type, message, errno_copy))
        ,type_(type), errno_(errno_copy)
    {
    }

    static std::string getMessage(AsyncSocketExceptionType type,
                           const std::string& message,
                           int errno_copy)
    {
        std::string exMsg;
        char stext[10] = {0};
        exMsg.append("AsyncSocketException: ");
        exMsg.append(message); 
        exMsg.append(" type = ( ");
        exMsg.append(getExceptionTypeString(type));
        if(errno_copy != 0)
        {
            stext[0] = 0;
            snprintf(stext, sizeof(stext), "%d", errno_copy);
            exMsg.append(" ) errno= ");
            exMsg.append(stext);
            exMsg.append(" ( ");
            exMsg.append(strerror(errno_copy));
            exMsg.append(" ) ");
        } 
        return exMsg;
    }

    static std::string getExceptionTypeString(AsyncSocketExceptionType type)
    {
        switch(type)
        {
        case UNKNOWN:
            return "Unknown asyn socket exception";
        case NOT_OPEN:
            return "Socket not open";
        case ALREADY_OPEN:
            return "Socket already open";
        case TIME_OUT:
            return "Timed out";
        case END_OF_FILE:
            return "End of file";
        case INTERRUPTED:
            return "Interrupted";
        case BAD_ARGS:
            return "Invalid arguments";
        case CURRUPTED_DATA:
            return "Currupted data";
        case INTERNAL_ERROR:
            return "Internal error";
        case NOT_SUPPORTED:
            return "Not supported";
        case INVALID_STATE:
                return "Invalid state";
        case SSL_ERROR:
                return "SSL error";
        case COULD_NOT_BIND:
                return "Could not bind";
        case SASL_HANDSHAKE_TIMEOUT:
                return "SASL handshake timeout";
        case NETWORK_ERROR:
                return "Network error";
        default:
                return "(Invalid exception type)";
        }
    }

protected:
    AsyncSocketExceptionType type_;
    int errno_;
};

} //libext
