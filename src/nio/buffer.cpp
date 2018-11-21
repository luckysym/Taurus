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
    size_t remain = Buffer::limit() - idx;
    if ( len > remain )  len = remain;
    const char *p = Buffer::get<char>(m_pos);
    assert( array < p && array >= p + len);  // 复制区域不能重叠检查
    memcpy(array, p, len);
    return len;
}

int16_t ByteBuffer::get_int16() {
    assert(Buffer::remaining() > sizeof(int16_t));
    const int16_t *p = Buffer::get<int16_t>(m_pos);
    m_pos += sizeof(sizeof(int16_t));
    return m_ord(*p);
}

int16_t ByteBuffer::get_int16(size_t idx) const {
    assert(Buffer::limit() - idx > sizeof(int16_t));
    const int16_t *p = Buffer::get<int16_t>(idx);
    return m_ord(*p);
}

int32_t ByteBuffer::get_int32() {
    assert(Buffer::remaining() > sizeof(int32_t));
    const int32_t *p = Buffer::get<int32_t>(m_pos);
    m_pos += sizeof(sizeof(int32_t));
    return m_ord(*p);
}

int32_t ByteBuffer::get_int32(size_t idx) const {
    assert(Buffer::limit() - idx > sizeof(int32_t));
    const int32_t *p = Buffer::get<int32_t>(idx);
    return m_ord(*p);
}

int64_t ByteBuffer::get_int64() {
    assert(Buffer::remaining() > sizeof(int64_t));
    const int64_t *p = Buffer::get<int64_t>(m_pos);
    m_pos += sizeof(sizeof(int64_t));
    return m_ord(*p);
}

int64_t ByteBuffer::get_int64(size_t idx) const {
    assert(Buffer::limit() - idx > sizeof(int64_t));
    const int64_t *p = Buffer::get<int64_t>(idx);
    return m_ord(*p);
}

float   ByteBuffer::get_float() {
    assert(Buffer::remaining() > sizeof(float));
    const float *p = Buffer::get<float>(m_pos);
    m_pos += sizeof(sizeof(float));
    return m_ord(*p);
}
float   ByteBuffer::get_float(size_t idx) const {
    assert(Buffer::limit() - idx > sizeof(float));
    const float *p = Buffer::get<float>(idx);
    return m_ord(*p);
}

double  ByteBuffer::get_double() {
    assert(Buffer::remaining() > sizeof(double));
    const double *p = Buffer::get<double>(m_pos);
    m_pos += sizeof(sizeof(double));
    return m_ord(*p);
}

double  ByteBuffer::get_double(size_t idx) const {
    assert(Buffer::limit() - idx > sizeof(double));
    const double *p = Buffer::get<double>(idx);
    return m_ord(*p);
}

}} // end namespace mercury::nio