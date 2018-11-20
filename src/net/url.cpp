#include <mercury/net/network.h>
#include <regex>

namespace mercury {
namespace net {

class URLParserImpl;

struct URL::URL_Impl {
    std::string m_schema;
    std::string m_host;
    int         m_port;
    mutable std::string m_string;

    static const int ParseState_Init = 0;
    static const int ParseState_Schema = 1;


    bool is_opaque() const { return true; }
    void define_string() const;
    std::string to_string(const char *schema, const char *host, int port);
    bool parse(const char *str, RuntimeError &e);

private:
    ssize_t parse_schema(const char *str, size_t pos, RuntimeError &e);
    ssize_t parse_host(const char *str, size_t pos, RuntimeError &e);
    ssize_t parse_port(const char *str, size_t pos, RuntimeError &e);
}; // end struct URL::URL_Impl;

URL::URL() : m_pImpl (new URL_Impl) {}
URL::~URL()  { delete m_pImpl; m_pImpl = nullptr; }

URL::URL(const URL &other) : URL() { *m_pImpl = *other.m_pImpl; }
URL::URL(URL && other) : URL() { *m_pImpl = std::move(*other.m_pImpl); }

URL & URL::operator=(const URL &other) { *m_pImpl = *other.m_pImpl; }
URL & URL::operator=(URL &&other) { *m_pImpl = std::move(*other.m_pImpl);}

URL::URL(const char * str, RuntimeError &e) : URL() {
    m_pImpl->parse(str, e);
}

URL::URL(const char *schema, const char *host, int port, RuntimeError &e) : URL() {
    std::string str = m_pImpl->to_string(schema, host, port);
    m_pImpl->parse(str.c_str(), e);
}

const char * URL::schema() const { return m_pImpl->m_schema.c_str(); }
const char * URL::host() const { return m_pImpl->m_host.c_str(); }
int URL::port() const { return m_pImpl->m_port; }

std::string URL::str() const {
    m_pImpl->define_string();
    return m_pImpl->m_string;
}

void URL::URL_Impl::define_string() const {
    if ( !m_string.empty() ) return ;

    std::ostringstream oss;
    if ( !m_schema.empty() ) oss<<m_schema<<":";
    if ( this->is_opaque() ) {
        oss<<this->m_schema_spec_part;
    } else {
        if ( !m_host.empty() ) {
            oss<<"//";
        }
        bool needbrackets = m_host.find_first_of(':') != std::string::npos &&
                            m_host[0] != '[' && m_host[m_host.length() - 1] != ']';
        if ( needbrackets ) oss<<'[';
        oss<<m_host;
        if ( needbrackets ) oss<<']';
        if ( m_port >= 0 ) oss<<':'<<m_port;
    }
    m_string = oss.str();
}

std::string URL::URL_Impl::to_string(const char *schema, const char *host, int port) {
    std::ostringstream oss;
    if ( schema && schema[0] != '\0') oss<<schema<<':';
    if ( host && host[0] != '\0' ) {
        oss<<"//"<<host;
        if ( port >= 0 ) oss<<':'<<port;
    }
    return oss.str();
}

bool URL::URL_Impl::parse(const char * str, RuntimeError &e) {
    int state = ParseState_Init;   // init
    bool isok = true;
    std::ostringstream oss;
    size_t p0 = 0;
 
    // 解析schema部分
    ssize_t n = this->parse_schema(str, p0, e);
    if ( n <= 0 )  return false;   // schema必须存在
    else p0 += (n + 1) ;   // p0 指向冒号(:)之后
    
    // 解析host部分
    n = this->parse_host(str, p0, e);
    if ( n < 0 ) return false;  // host部分可能不存在，host部分不存在，则port也不存在
    else p0 += n;   // 如果port存在p0指向冒号(:)，如果不存在p0指向'/'或者'\0'

    if ( n > 0 && str[p0] == ':') {  // 前一个步骤host存在, 且host之后是一个冒号(:)
        n = this->parse_port(str, p0 + 1, e);
        if ( n < 0 ) return false;
        else p0 += n;
    }

    return true;
}

ssize_t URL::URL_Impl::parse_schema(const char *str, size_t pos, RuntimeError &e) {
    size_t n = 0;
    while ( str[pos + n] != '\0') {
        if ( str[pos + n] == ':') {
            if ( n > 0 ) this->m_schema.assign(str + pos, n - 1);
            return n;
        }
        ++n;
    }
    std::string msg;
    msg.reserve(256);
    msg.append("URL parse schema error: ").append(str);
    e.set(-1, msg.c_str(), "URL::URL_Impl::parse_schema");
    return -1;
}

ssize_t URL::URL_Impl::parse_host(const char *str, size_t pos, RuntimeError &e) {
    if ( str[pos] == '/' && str[pos + 1] == '/') {
        size_t n = 2;
        while ( str[pos + n] != '\0') {
            if ( str[pos + n] == ':' || str[pos + n] == '/') {
                break;
            }
            ++n;
        }
        this->m_host.assign(str + pos + 2, n - 2 -1);
        return n;
    }

    std::string msg;
    msg.reserve(256);
    msg.append("URL invalid host: ").append(str);
    e.set(-1, msg.c_str(), "URL::URL_Impl::parse_host");
    return -1;
}

ssize_t URL::URL_Impl::parse_port(const char * str, size_t pos, RuntimeError &e) {
    size_t n = 0;
    while ( str[pos + n] >= 0 && str[pos + n] <= '9') ++n;
    if ( n > 0 ) {   
        this->m_port = atoi(str + pos);
        return n;
    }
    std::string msg;
    msg.reserve(256);
    msg.append("URL invalid port: ").append(str);
    e.set(-1, msg.c_str(), "URL::URL_Impl::parse_port");
    return -1;
}


} } // end namespace mercury::net