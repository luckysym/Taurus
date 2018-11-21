#include <mercury/nio/buffer.h>
#include <memory.h>

namespace mercury {
namespace nio {

char ByteBuffer::get() {
    assert(Buffer::remaining() >= sizeof(char));
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
    assert(Buffer::remaining() >= sizeof(int16_t));
    const int16_t *p = Buffer::get<int16_t>(m_pos);
    m_pos += sizeof(sizeof(int16_t));
    return m_ord(*p);
}

int16_t ByteBuffer::get_int16(size_t idx) const {
    assert(Buffer::limit() - idx >= sizeof(int16_t));
    const int16_t *p = Buffer::get<int16_t>(idx);
    return m_ord(*p);
}

int32_t ByteBuffer::get_int32() {
    assert(Buffer::remaining() >= sizeof(int32_t));
    const int32_t *p = Buffer::get<int32_t>(m_pos);
    m_pos += sizeof(sizeof(int32_t));
    return m_ord(*p);
}

int32_t ByteBuffer::get_int32(size_t idx) const {
    assert(Buffer::limit() - idx >= sizeof(int32_t));
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
    assert(Buffer::limit() - idx >= sizeof(int64_t));
    const int64_t *p = Buffer::get<int64_t>(idx);
    return m_ord(*p);
}

float   ByteBuffer::get_float() {
    assert(Buffer::remaining() >= sizeof(float));
    const float *p = Buffer::get<float>(m_pos);
    m_pos += sizeof(sizeof(float));
    return m_ord(*p);
}
float   ByteBuffer::get_float(size_t idx) const {
    assert(Buffer::limit() - idx >= sizeof(float));
    const float *p = Buffer::get<float>(idx);
    return m_ord(*p);
}

double  ByteBuffer::get_double() {
    assert(Buffer::remaining() >= sizeof(double));
    const double *p = Buffer::get<double>(m_pos);
    m_pos += sizeof(sizeof(double));
    return m_ord(*p);
}

double  ByteBuffer::get_double(size_t idx) const {
    assert(Buffer::limit() - idx >= sizeof(double));
    const double *p = Buffer::get<double>(idx);
    return m_ord(*p);
}

void ByteBuffer::put(char ch) {
    assert(Buffer::remaining() >= sizeof(char));
    char * p = Buffer::get<char>(m_pos);
    ++m_pos;
    *p = ch;
}

void ByteBuffer::put(size_t idx, char ch) {
    assert(Buffer::limit() - idx >= sizeof(char));
    char *p = Buffer::get<char>(idx);
    *p = ch;
}

void ByteBuffer::put(const char *array, size_t len) {
    assert(Buffer::remaining() >= len );
    char *p = Buffer::get<char>(m_pos);
    memcpy(p, array, len);
    m_pos += len;
}

void ByteBuffer::put(size_t idx, const char *array, size_t len) {
    assert(Buffer::limit() - idx >= sizeof(char));
    char *p = Buffer::get<char>(idx);
    memcpy(p, array, len);
}

void  ByteBuffer::put_int16(int16_t value) {
    assert(Buffer::remaining() >= sizeof(int16_t));
    int16_t *p = Buffer::get<int16_t>(m_pos);
    m_pos += sizeof(sizeof(int16_t));
    *p = m_ord(value);
}

void  ByteBuffer::put_int16(size_t idx, int16_t value) {
    assert(Buffer::limit() - idx >= sizeof(int16_t));
    int16_t *p = Buffer::get<int16_t>(idx);
    *p = m_ord(value);
}

void  ByteBuffer::put_int32(int32_t value) {
    assert(Buffer::remaining() >= sizeof(int32_t));
    int32_t *p = Buffer::get<int32_t>(m_pos);
    m_pos += sizeof(sizeof(int32_t));
    *p = m_ord(value);
}

void  ByteBuffer::put_int32(size_t idx, int32_t value) {
    assert(Buffer::limit() - idx >= sizeof(int32_t));
    int32_t *p = Buffer::get<int32_t>(idx);
    *p = m_ord(value);
}

void  ByteBuffer::put_int64(int64_t value) {
    assert(Buffer::remaining() >= sizeof(int64_t));
    int64_t *p = Buffer::get<int64_t>(m_pos);
    m_pos += sizeof(sizeof(int64_t));
    *p = m_ord(value);
}
void  ByteBuffer::put_int64(size_t idx, int64_t value) {
    assert(Buffer::limit() - idx >= sizeof(int64_t));
    int64_t *p = Buffer::get<int64_t>(idx);
    *p = m_ord(value);
}

void  ByteBuffer::put_float(float value) {
    assert(Buffer::remaining() >= sizeof(float));
    float *p = Buffer::get<float>(m_pos);
    m_pos += sizeof(sizeof(float));
    *p = m_ord(value);
}

void  ByteBuffer::put_float(size_t idx, float value) {
    assert(Buffer::limit() - idx >= sizeof(float));
    float *p = Buffer::get<float>(idx);
    *p = m_ord(value);
}

void  ByteBuffer::put_double(double value) {
    assert(Buffer::remaining() >= sizeof(double));
    double *p = Buffer::get<double>(m_pos);
    m_pos += sizeof(sizeof(double));
    *p = m_ord(value);
}

void  ByteBuffer::put_double(size_t idx, double value) {
    assert(Buffer::limit() - idx >= sizeof(double));
    double *p = Buffer::get<double>(idx);
    *p = m_ord(value);
}

}} // end namespace mercury::nio