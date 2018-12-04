#pragma once
#include <mercury/error_info.h>
#include <mercury/net/network.h>
#include <vector>

namespace mercury {
namespace nio {

class Selector;
class ServerSocketChannel;
class StreamSocketChannel;
class DatagramSocketChannel;

class SelectionKey {
public:
    const static int OpAccept  = 1;
    const static int OpConnect = 2;
    const static int OpRead    = 4;
    const static int OpWrite   = 8;

public:
    SelectionKey();
    ~SelectionKey();

    void * attach(void *obj);
    void * attachment() const;

    int    interest_ops() const;
    void   interest_ops(int ops) const;
    int    ready_ops() const;

    bool   is_acceptable() const;
    bool   is_connectable() const;
    bool   is_readable() const;
    bool   is_writable() const;
    bool   is_valid() const;
}; // end class SelectionKey

class SelectableChannel {
public:
    SelectionKey * reg(Selector *selector, int ops, RuntimeError &e);
    SelectionKey * reg(Selector *selector, int ops, void *att, RuntimeError &e); 
    SelectionKey * key_for(Selector *selector);
    bool           is_registered() const;
public:
    virtual int valid_ops() const = 0;
}; // end class SelectableChannel


class SelectorImpl;

class Selector final {
private:
    SelectorImpl * m_pImpl;

public:
    Selector();
    Selector(const Selector &other) = delete;
    Selector(Selector &&other);
    ~Selector();

    Selector & operator=(const Selector &other) = delete;
    Selector & operator=(Selector &&other);

    bool close(RuntimeError &e);
    bool open(RuntimeError &e);
    bool is_open() const;
    int  select(RuntimeError &e);
    int  select(long timeout, RuntimeError &e);
    void wakeup();

    size_t keys(std::vector<SelectionKey*> & keys);
    size_t selected_keys(std::vector<SelectionKey*> & keys);

    SelectionKey * reg(SelectableChannel *pch, int ops, void *att, RuntimeError &e);
    bool           unreg(SelectionKey *key, RuntimeError &e);

}; // end class Selector

class ServerSocketChannel : public SelectableChannel {
public:
    ServerSocketChannel();
    ~ServerSocketChannel();

    bool accept(StreamSocketChannel &rSocketChannel, RuntimeError &e);
    bool bind(const char * ip, int port, RuntimeError &e);
    bool create(int domain, RuntimeError &e);
    bool close(RuntimeError &e);
    std::string local_address() const;
    std::string local_address(RuntimeError &e) const;

public:  // SelectableChannel
    virtual int valid_ops() const;
}; // end class ServerSocketChannel

class StreamSocketChannel : public SelectableChannel {
}; // end class StreamSocketChannel

class DatagramSocketChannel : public SelectableChannel {
}; // end class DatagramSocketChannel

}} // end namespace mercury::nio
