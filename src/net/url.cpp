#include <mercury/net/network.h>
#include <regex>

namespace mercury {
namespace net {

class URLParserImpl;

struct URL::URL_Impl {
    std::string m_string;
    std::string m_schema;
    std::string m_host;
    int         m_port {-1};
    

    void define_string();
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
    bool isok = m_pImpl->parse(str, e);
    if ( isok ) m_pImpl->m_string = str;
}

URL::URL(const char *schema, const char *host, int port) : URL() {
    m_pImpl->m_schema.assign(schema);
    m_pImpl->m_host.assign(host);
    m_pImpl->m_port = port;
    m_pImpl->define_string();

    printf("sizeof(URL_Impl) = %ld\n", sizeof(URL_Impl)); 
}

const char * URL::schema() const { return m_pImpl->m_schema.c_str(); }
const char * URL::host() const { return m_pImpl->m_host.c_str(); }
int URL::port() const { return m_pImpl->m_port; }

std::string URL::str() const {
    return m_pImpl->m_string;
}

void URL::URL_Impl::define_string() {
    if ( !m_string.empty() ) return ;

    std::ostringstream oss;
    if ( !m_schema.empty() ) oss<<m_schema<<":";
    if ( !m_host.empty() ) {
        oss<<"//";
    }
    bool needbrackets = m_host.find_first_of(':') != std::string::npos &&
                        m_host[0] != '[' && m_host[m_host.length() - 1] != ']';
    if ( needbrackets ) oss<<'[';
    oss<<m_host;
    if ( needbrackets ) oss<<']';
    if ( m_port >= 0 ) oss<<':'<<m_port;
    m_string = oss.str();
}

bool URL::URL_Impl::parse(const char * str, RuntimeError &e) {
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
            if ( n > 0 ) this->m_schema.assign(str + pos, n);
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
    int in_bracket = 0;
    if ( str[pos] == '/' && str[pos + 1] == '/') {
        size_t n = 2;
        while ( str[pos + n] != '\0') {
            char ch = str[pos + n];
            if ( ch == '[') ++in_bracket;  // [包含级数递增]
            else if ( ch == ']') --in_bracket;  // [包含级数递减]
            else if ( str[pos + n] == ':' || str[pos + n] == '/' ) {
                if ( !in_bracket ) break;
            }
            ++n;
        }
        this->m_host.assign(str + pos + 2, n - 2);
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