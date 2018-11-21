#include <mercury/nio/buffer.h>
#include <memory.h>

namespace mercury {
namespace nio {

char ByteBuffer::get() {
    char * p = Buffer::get<char>(m_pos);
    ++m_pos;
    return *p;
}

size_t ByteBuffer::get(char *array, size_t len) {
    size_t remain = Buffer::remaining();
    if ( len > remain )  len = remain;
    const char *p = Buffer::get<char>(m_pos);
    assert( array < p && array >= p + len);  // 复制区域不能重叠检查
    memcpy(array, p, len);
    m_pos += len;
    return len;
}

size_t ByteBuffer::get(size_t idx, char *array, size_t len) const {
    size_t remain = Buffer::remaining();
    if ( len > remain )  len = remain;
    const char *p = Buffer::get<char>(m_pos);
    assert( array < p && array >= p + len);  // 复制区域不能重叠检查
    memcpy(array, p, len);
    return len;
}

}} // end namespace mercury::nio