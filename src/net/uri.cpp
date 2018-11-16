#include <mercury/net/network.h>

namespace mercury {
namespace net {

class URIParserImpl;

struct URI::URI_Impl {
    std::string m_schema;
    std::string m_host;
    int         m_port;

    std::string toString(const char *schema, const char *host, int port);
}; // end struct URI::URI_Impl;

class URI_ParserImpl {
public:
    URI_ParserImpl(URI::URI_Impl * pImpl, const char *str);
    bool parse(bool rsa, RuntimeError &e);
}; // end class URIParserImpl

URI::URI() : m_pImpl (new URI_Impl) {}
URI::~URI()  { delete m_pImpl; m_pImpl = nullptr; }

URI::URI(const URI &other) : URI() { *m_pImpl = *other.m_pImpl; }
URI::URI(URI && other) : URI() { *m_pImpl = std::move(*other.m_pImpl); }

URI & URI::operator=(const URI &other) { *m_pImpl = *other.m_pImpl; }
URI & URI::operator=(URI &&other) { *m_pImpl = std::move(*other.m_pImpl);}

URI::URI(const char * str, RuntimeError &e) : URI() {
    URI_ParserImpl parser(m_pImpl, str);
    parser.parse(false, e);
}

URI::URI(const char *schema, const char *host, int port, RuntimeError &e) : URI() {
    std::string str = m_pImpl->toString(schema, host, port);
    URI_ParserImpl parser(m_pImpl, str.c_str());
    parser.parse(false, e);
}

const char * URI::schema() const { return m_pImpl->m_schema.c_str(); }
const char * URI::host() const { return m_pImpl->m_host.c_str(); }
int URI::port() const { return m_pImpl->m_port; }




} } // end namespace mercury::net