#pragma once
#include <string>
#include <list>
#include <sstream>

namespace taurus {

class RuntimeError {
    int                    m_code;
    std::string            m_message;
    std::string            m_hint;
    std::list<std::string> m_stack;

public:
    RuntimeError();
    RuntimeError(const RuntimeError &e);
    RuntimeError(RuntimeError && e);
    ~RuntimeError();

    RuntimeError & operator= (const RuntimeError &e);
    RuntimeError & operator= (RuntimeError &&e);

    operator bool () const { return m_code != 0;  }  // 是否包含错误
    bool operator !() const { return m_code == 0; }  // 是否正常

    int          code() const throw() { return m_code; }
    const char * message() const throw() { return m_message.c_str(); }
    const char * hint() const throw() { return m_hint.c_str();}
    
    void set(int code, const char *message, const char * hint = nullptr) throw();
    void push(const char * stackinfo) throw();
    void merge(RuntimeError &errinfo) throw();
    void clear() throw();

    std::string toString() const throw();
    template <class OutStream>
    void printStack(OutStream & os);
}; // end class RuntimeError

inline RuntimeError::RuntimeError() : m_code(0) {}
inline RuntimeError::~RuntimeError() { m_code = 0; }

inline RuntimeError::RuntimeError(const RuntimeError &e) 
    : m_code(e.m_code), m_message(e.m_message)
    , m_hint(e.m_hint), m_stack(e.m_stack) { }

inline RuntimeError::RuntimeError(RuntimeError && e) 
    : m_code(e.m_code)
    , m_message(std::move(e.m_message))
    , m_hint(std::move(e.m_hint))
    , m_stack( std::move(e.m_stack ) ) { e.m_code = 0; }

inline RuntimeError & RuntimeError::operator=(const RuntimeError &e) {
    if ( this == &e) return *this;
    m_code = e.m_code;
    m_message = e.m_message;
    m_hint = e.m_hint;
    m_stack = e.m_stack;
    return *this;
}

inline RuntimeError & RuntimeError::operator= ( RuntimeError && e) {
    if ( this == &e) return *this;
    m_code = e.m_code; e.m_code = 0;
    m_message = std::move(e.m_message);
    m_hint = std::move(e.m_hint);
    m_stack = std::move(e.m_stack);
    return *this;
}

inline void RuntimeError::set(int code, const char *message, const char * hint) throw() {
    m_code = code;
    m_message.assign(message);
    if ( hint ) m_hint.assign(hint);
    if ( m_hint.empty() ) m_stack.push_back(m_message);
    else {
        std::string err;
        err.reserve(m_hint.length() + m_message.length() + 8);
        err.append(m_hint).append(": ").append(m_message);
        m_stack.push_back(err);
    }
}

inline void RuntimeError::push(const char * errinfo) throw() {
    m_stack.push_back(errinfo);
}

inline void RuntimeError::merge(RuntimeError &errinfo) throw() {
    m_code = errinfo.m_code; errinfo.m_code = 0;
    m_message = std::move(errinfo.m_message);
    m_hint = std::move(errinfo.m_hint);
    m_stack.splice(m_stack.begin(), errinfo.m_stack);
}

inline void RuntimeError::clear() throw() {
    m_code = 0;
    m_message.clear();
    m_hint.clear();
    m_stack.clear();
}

inline std::string RuntimeError::toString() const throw() {
    std::ostringstream oss;
    oss<<"error code: "<<m_code<<", message: "<<m_message<<std::endl;
    oss<<"stack: "<<std::endl;
    auto it = m_stack.begin();
    for( ; it != m_stack.end(); ++it) {
        oss<<"  > "<<*it<<std::endl;
    }
    return oss.str();
}

template <class OutStream>
inline void RuntimeError::printStack(OutStream & os) {
    os<<"error code: "<<m_code<<", message: "<<m_message<<std::endl;
    os<<"stack: "<<std::endl;
    auto it = m_stack.begin();
    for( ; it != m_stack.end(); ++it) {
        os<<"  > "<<*it<<std::endl;
    }
}

} // end namespace taurus