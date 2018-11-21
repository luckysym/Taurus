#pragma once
#include <cassert>
#include <stdint.h>
#include <stdlib.h>

namespace mercury {
namespace nio {

class ByteOrder {
public:
    enum {
        LittleEndian,
        BigEndian
    };

private:
    int m_endian;
    int m_native;

public:
    static ByteOrder native_order();
    static int       native();

public:
    ByteOrder() : m_endian(native()), m_native(m_endian) {}
    ByteOrder(int order) : m_endian(order), m_native(native()) {}
    ~ByteOrder();

    int16_t operator() (int16_t value) const { 
        int16_t result = value;;
        if ( m_endian != m_native ) {
            char *p0 = (char*)&value;
            char *p1 = (char*)&result;
            p1[0] = p0[1];
            p1[1] = p0[0];
        }
        return result;
    }

    int32_t operator()(int32_t value) const {
        int32_t result = value;;
        if ( m_endian != m_native ) {
            char *p0 = (char*)&value;
            char *p1 = (char*)&result;
            p1[0] = p0[3];
            p1[1] = p0[2];
            p1[2] = p0[1];
            p1[3] = p0[0];
        }
        return result;
    }
    
    int64_t operator()(int64_t value) const {
        int64_t result = value;;
        if ( m_endian != m_native ) {
            char *p0 = (char*)&value;
            char *p1 = (char*)&result;
            p1[0] = p0[7];
            p1[1] = p0[6];
            p1[2] = p0[5];
            p1[3] = p0[4];
            p1[4] = p0[3];
            p1[5] = p0[2];
            p1[6] = p0[1];
            p1[7] = p0[0];
        }
        return result;
    }
    
    float   operator()(float value) const {
        float result = value;;
        if ( m_endian != m_native ) {
            char *p0 = (char*)&value;
            char *p1 = (char*)&result;
            p1[0] = p0[3];
            p1[1] = p0[2];
            p1[2] = p0[1];
            p1[3] = p0[0];
        }
        return result;
    }
    double  operator()(double value) const {
        double result = value;;
        if ( m_endian != m_native ) {
            char *p0 = (char*)&value;
            char *p1 = (char*)&result;
            p1[0] = p0[7];
            p1[1] = p0[6];
            p1[2] = p0[5];
            p1[3] = p0[4];
            p1[4] = p0[3];
            p1[5] = p0[2];
            p1[6] = p0[1];
            p1[7] = p0[0];
        }
        return result;
    }
}; // end class ByteOrder


class Buffer {
protected:
    void * m_buf;
    size_t m_cap;
    size_t m_pos;
    size_t m_lim;
    size_t m_mark;

protected:
    template<class T>
    const T * get(size_t index) const { assert(index < m_cap); return (const T*)m_buf + index; }

    template<class T>
    T * get(size_t index) { assert(index < m_cap); return (T*)m_buf + index; }

protected:
    Buffer() : m_buf(nullptr), m_cap(0), m_pos(0), m_lim(0), m_mark(0) {}
    
    Buffer(void *buf, size_t cap) : m_buf(buf), m_cap(cap), m_pos(0), m_lim(cap), m_mark(0) { }

public:
    virtual ~Buffer() {
        m_buf = nullptr;
        m_cap = 0;
        m_pos = 0;
        m_lim = 0;
        m_mark = 0;
    }

    size_t capacity() const { return m_cap; }
    void   clear() { m_pos = 0; m_lim = m_cap; m_mark = 0; }

    void   flip()  { m_lim = m_pos; m_pos = m_mark = 0; }
    size_t limit() const { return m_lim; }
    void   limit(size_t newlim) { assert(newlim <= m_cap); m_lim = newlim; }
    
    void   mark() { m_mark = m_pos; }

    size_t position() const { return m_pos; }
    void   position(size_t newpos ) { assert(newpos < m_lim); m_pos = newpos; }

    size_t remaining() const { return m_lim - m_pos; }
    
    void   reset() { m_pos = m_mark; }
    void   rewind() { m_pos = m_mark = 0; }
}; // end class Buffer

class ByteBuffer : public Buffer {
public:
    static ByteBuffer allocate(size_t cap);

protected:
    ByteOrder  m_ord;

public:
    ByteBuffer() {}
    ByteBuffer(const ByteOrder &order) : m_ord(order) {}
    ByteBuffer(char *buf, size_t cap) : Buffer(buf,cap) {}
    ByteBuffer(char *buf, size_t cap, const ByteOrder &order) : Buffer(buf, cap), m_ord(order) {}
    virtual ~ByteBuffer() {}

    ByteOrder    order() const { return m_ord; }
    void         order(const ByteOrder &order) { m_ord = order; }

    const char * ptr() const { return Buffer::get<char>(Buffer::position()); }
    char *       ptr() { return Buffer::get<char>(Buffer::position()); }

    const char * ptr(size_t idx) const { return Buffer::get<char>(idx); }
    char *       ptr(size_t idx) { return Buffer::get<char>(idx); }

    char         get();
    char         get(size_t idx) { return *Buffer::get<char>(idx); }
    size_t       get(char *array, size_t len);
    size_t       get(size_t idx, char *array, size_t len) const;
    
    int16_t      get_int16();
    int16_t      get_int16(size_t idx) const;
    int32_t      get_int32();
    int32_t      get_int32(size_t idx) const;
    int64_t      get_int64();
    int64_t      get_int64(size_t idx) const;
    float        get_float();
    float        get_float(size_t idx) const;
    double       get_double();
    double       get_double(size_t idx) const;

    void         put(char ch);
    void         put(size_t idx, char ch);
    void         put(const char *array, size_t len);
    void         put(size_t idx, const char *array, size_t len);

    void         put_int16(int16_t value);
    void         put_int16(size_t idx, int16_t value);
    void         put_int32(int32_t value);
    void         put_int32(size_t idx, int32_t value);
    void         put_int64(int64_t value);
    void         put_int64(size_t idx, int64_t value);
    void         put_float(float value);
    void         put_float(size_t idx, float value);
    void         put_double(double value);
    void         put_double(size_t idx, double value);

}; // end class ByteBuffer

}} // end namespace mercury::nio