#include <mercury/net/network.h>
#include <regex>

namespace mercury {
namespace net {

class URIParserImpl;

struct URI::URI_Impl {
    std::string m_schema_spec_part;
    std::string m_schema;
    std::string m_host;
    int         m_port;
    mutable std::string m_string;

    bool is_opaque() const { return true; }
    void define_string() const;
    std::string to_string(const char *schema, const char *host, int port);
    bool parse(const char *str, RuntimeError &e);

private:
    bool is_token_char(char ch) const;
    bool is_seperate_char(char ch) const;
    size_t parse_token(const char *str, size_t p0, size_t p1, char sep, int *state);
}; // end struct URI::URI_Impl;

URI::URI() : m_pImpl (new URI_Impl) {}
URI::~URI()  { delete m_pImpl; m_pImpl = nullptr; }

URI::URI(const URI &other) : URI() { *m_pImpl = *other.m_pImpl; }
URI::URI(URI && other) : URI() { *m_pImpl = std::move(*other.m_pImpl); }

URI & URI::operator=(const URI &other) { *m_pImpl = *other.m_pImpl; }
URI & URI::operator=(URI &&other) { *m_pImpl = std::move(*other.m_pImpl);}

URI::URI(const char * str, RuntimeError &e) : URI() {
    m_pImpl->parse(str, e);
}

URI::URI(const char *schema, const char *host, int port, RuntimeError &e) : URI() {
    std::string str = m_pImpl->to_string(schema, host, port);
    m_pImpl->parse(str.c_str(), e);
}

const char * URI::schema() const { return m_pImpl->m_schema.c_str(); }
const char * URI::host() const { return m_pImpl->m_host.c_str(); }
int URI::port() const { return m_pImpl->m_port; }

std::string URI::str() const {
    m_pImpl->define_string();
    return m_pImpl->m_string;
}

void URI::URI_Impl::define_string() const {
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

std::string URI::URI_Impl::to_string(const char *schema, const char *host, int port) {
    std::ostringstream oss;
    if ( schema && schema[0] != '\0') oss<<schema<<':';
    if ( host && host[0] != '\0' ) {
        oss<<"//"<<host;
        if ( port >= 0 ) oss<<':'<<port;
    }
    return oss.str();
}

bool URI::URI_Impl::parse(const char * str, RuntimeError &e) {
    int state = 0;   // init
    bool isok = true;
    std::ostringstream oss;

    size_t p0 = 0;
    size_t i  = 0;
    while ( str[i] != '\0' ) {
        char ch = str[i];
        if ( this->is_token_char(ch) ) ++i;   // 可包含在关键字的字符，包括字母、数字、下划线
        else if ( this->is_seperate_char(ch) ) {   // 间隔符，如 ":/[]."这些
            size_t n = this->parse_token(str, p0, i, ch, &state);
            if ( n > 0 ) {
                p0 += n;
                i = p0;
            } else {
                oss<<"bad URI ("<<i + 1<<"): "<<str;
                e.set(-1, oss.str().c_str(), "URI_Impl::parse");
                return false;
            }
        } else {
            oss<<"bad URI, unknown char: "<< ch <<" ("<<i + 1<<"): "<<str;
            e.set(-1, oss.str().c_str(), "URI_Impl::parse");
            return false;
        }
    }
    return true;
}

} } // end namespace mercury::net