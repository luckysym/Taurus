#pragma once

#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <errno.h>

#include "socket_utils.h"

namespace mercury {
namespace net {

class SocketIoError {};

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
    ssize_t Read(size_t limit, RuntimeError & e) {
        assert(m_pos + limit > m_size);
        ssize_t r = ::recv(m_fd, m_buffer + m_pos, limit, 0);
        if ( r > 0 )  {
            m_pos += r;
            return r;
        }
        else if ( r == 0 ) {
            std::ostringstream oss;
            oss<<"recv() failed, connection closed by peer. fd: "<<m_fd;
            e.set(-1, oss.str().c_str(), "SocketIoImpl::Read");
            return -1;
        }
        else {
            std::ostringstream oss;
            oss<<"recv() failed, "<<sockerr<<" fd: "<<m_fd;
            e.set(-1, oss.str().c_str(), "SocketIoImpl::Read");
            return -1;
        }
    }
    ssize_t Read(RuntimeError & e) { 
        size_t limit = m_size - m_pos;
        return this->Read(limit, e);
    }

    /**
     * @brief 缓存写入位置归零, 或者设置到指定位置。
     */
    void    Reset(size_t pos = 0) { assert(pos <= m_size); m_pos == pos;  }
    size_t  Position() const { return m_pos; }
    size_t  Size() const { return m_size; }
    char *  Buffer() const { return m_buffer; }

    ssize_t operator() (RuntimeError &e) { return this->Read(e); }
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
    ssize_t Write(size_t limit, RuntimeError & error) {
        assert(m_pos + limit > m_size);
        ssize_t r = ::send(m_fd, m_buffer + m_pos, limit, 0);
        if ( r >= 0 )  { m_pos += r;  return r; }  // 读取r个字节(包括0个字节)
        else {     // 读取失败。包含非阻塞无消息可读
            int en = errno;
            if ( en == EAGAIN || en == EWOULDBLOCK ) return 0;  // 非阻塞时无消息可读
            std::ostringstream oss;
            oss<<"send() failed, "<<sockerr<<" fd: "<<m_fd;
            error.set(-1, oss.str().c_str(), "SocketIoImpl::Write");   
            return -1;
        }
    }
    ssize_t Write(RuntimeError & e) {
        size_t limit = m_size - m_pos;
        return this->Write(limit, e);
    }

    /// \brief 缓存读取位置归零，或者设置到指定位置。
    void    Reset(size_t pos = 0) { assert(pos <= m_size); m_pos = pos;}
    size_t  Position() const { return m_pos; }
    size_t  Size() const { return m_size; }
    const char * Buffer() const { return m_buffer; }

    ssize_t operator() (RuntimeError &e ) { return this->Write(e); }
}; // end class SocketWriterImpl

} // end namespace net
} // end namespace mercury