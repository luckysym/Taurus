#pragma once
#include <string>
#include <list>
#include <sstream>

namespace taurus {

class ErrorInfo {
    int                    m_code;
    std::string            m_message;
    std::string            m_hint;
    std::list<std::string> m_stack;

public:
    ErrorInfo();
    ErrorInfo(const ErrorInfo &e);
    ErrorInfo(ErrorInfo && e);
    ~ErrorInfo();

    ErrorInfo & operator= (const ErrorInfo &e);
    ErrorInfo & operator= (ErrorInfo &&e);

    int          Code() const throw() { return m_code; }
    const char * Message() const throw() { return m_message.c_str(); }
    const char * Hint() const throw() { return m_hint.c_str();}
    
    void Set(int code, const char *message, const char * hint = nullptr) throw();
    void Push(const char * stackinfo) throw();
    void Merge(ErrorInfo &errinfo) throw();
    void Clear() throw();

    std::string ToString() const throw();
    template <class OutStream>
    void PrintStack(OutStream & os);
}; // end class ErrorInfo

inline ErrorInfo::ErrorInfo() : m_code(0) {}
inline ErrorInfo::~ErrorInfo() { m_code = 0; }

inline ErrorInfo::ErrorInfo(const ErrorInfo &e) 
    : m_code(e.m_code), m_message(e.m_message)
    , m_hint(e.m_hint), m_stack(e.m_stack) { }

inline ErrorInfo::ErrorInfo(ErrorInfo && e) 
    : m_code(e.m_code)
    , m_message(std::move(e.m_message))
    , m_hint(std::move(e.m_hint))
    , m_stack( std::move(e.m_stack ) ) { e.m_code = 0; }

inline ErrorInfo & ErrorInfo::operator=(const ErrorInfo &e) {
    if ( this == &e) return *this;
    m_code = e.m_code;
    m_message = e.m_message;
    m_hint = e.m_hint;
    m_stack = e.m_stack;
    return *this;
}

inline ErrorInfo & ErrorInfo::operator= ( ErrorInfo && e) {
    if ( this == &e) return *this;
    m_code = e.m_code; e.m_code = 0;
    m_message = std::move(e.m_message);
    m_hint = std::move(e.m_hint);
    m_stack = std::move(e.m_stack);
    return *this;
}

inline void ErrorInfo::Set(int code, const char *message, const char * hint) throw() {
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

inline void ErrorInfo::Push(const char * errinfo) throw() {
    m_stack.push_back(errinfo);
}

inline void ErrorInfo::Merge(ErrorInfo &errinfo) throw() {
    m_code = errinfo.m_code; errinfo.m_code = 0;
    m_message = std::move(errinfo.m_message);
    m_hint = std::move(errinfo.m_hint);
    m_stack.splice(m_stack.begin(), errinfo.m_stack);
}

inline void ErrorInfo::Clear() throw() {
    m_code = 0;
    m_message.clear();
    m_hint.clear();
    m_stack.clear();
}

inline std::string ErrorInfo::ToString() const throw() {
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
inline void ErrorInfo::PrintStack(OutStream & os) {
    os<<"error code: "<<m_code<<", message: "<<m_message<<std::endl;
    os<<"stack: "<<std::endl;
    auto it = m_stack.begin();
    for( ; it != m_stack.end(); ++it) {
        os<<"  > "<<*it<<std::endl;
    }
}



} // end namespace taurus