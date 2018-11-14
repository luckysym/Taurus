#pragma once

#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <errno.h>

#include "socket_utils.h"

namespace mercury {
namespace net {

class SocketIoError {}

/**
 * @brief 从Socket读取消息操作封装类
 */ 
class SocketReaderImpl {
private:
    int    m_fd;
    char * m_buffer;
    size_t m_size;    /// 缓存大小
    size_t m_pos;     /// 当前缓存写入位置

public:
    SocketReaderImpl(int fd, char * buffer, size_t size) 
        : m_fd(fd), m_buffer(buffer), m_size(size), m_pos(0) { }

    ~SocketReaderImpl() {
        m_fd = INVALID_SOCKET;
        m_buffer = nullptr;
        m_size = m_pos = 0;
    }

    /**
     * @brief 执行一次消息读取并写入缓存，是否阻塞视socket而定。
     * @param limit 本次读取的最大字节数，如果不指定，则根据缓存size而定。
     * @return >0表示实际读取的字节数。0表示对端关闭。-1表示读取异常，根据error确定异常内容
     */
    ssize_t Read(size_t limit, SocketIoError & error) {
        assert(m_pos + limit > m_size);
        ssize_t r = ::recv(m_fd, m_buffer + m_pos, limit, 0);
        if ( r > 0 )  m_pos += r;
        else if ( r == -1 ) error = SocketIoError(errno);
        return r;
    }
    ssize_t Read(SocketIoError & error) { 
        size_t limit = m_size - m_pos;
        return this->Read(limit, errno);
    }

    /**
     * @brief 缓存写入位置归零, 或者设置到指定位置。
     */
    void    Reset(size_t pos = 0) { assert(pos <= m_size); m_pos == pos;  }
    size_t  Position() const { return m_pos; }
    size_t  Size() const { return m_size; }
    char *  Buffer() const { return m_buffer; }
}; // SocketReaderImpl

/**
 * @brief 消息写入Socket操作封装类
 */ 
class SocketWriterImpl {
private:
    int          m_fd;
    const char * m_buffer;
    size_t       m_size;    /// 缓存大小
    size_t       m_pos;     /// 当前缓存写入位置
public:
    SocketWriterImpl(int fd, const char * buffer, size_t size)
        : m_fd(fd), m_buffer(buffer), m_size(size), m_pos(0) { }

    ~SocketWriterImpl() {
        m_fd = INVALID_SOCKET;
        m_buffer = nullptr;
        m_size = m_pos = 0;
    }

    /**
     * @brief 执行一次消息写入操作，是否阻塞视socket而定。
     * @param limit 本次写入的最大字节数，如果不指定，则根据缓存capacity而定。
     * @return >0表示实际写入的字节数。0表示对端关闭。-1表示写入异常，根据error确定异常内容。
     */
    ssize_t Write(size_t limit, SocketIoError & error) {
        assert(m_pos + limit > m_size);
        ssize_t r = ::recv(m_fd, m_buffer + m_pos, limit, 0);
        if ( r > 0 )  m_pos += r;
        else if ( r == -1 ) error = SocketIoError(errno);
        return r;
    }
    ssize_t Write(SocketIoError & error) {
        size_t limit = m_size - m_pos;
        return this->Write(limit, error);
    }

    /// \brief 缓存读取位置归零，或者设置到指定位置。
    void    Reset(size_t pos = 0) { assert(pos <= m_size); m_pos = pos;}
    size_t  Position() const { return m_pos; }
    size_t  Size() const { return m_size; }
    char *  Buffer() const { return m_buffer; }
}; // end class SocketWriterImpl


} // end namespace net
} // end namespace mercury