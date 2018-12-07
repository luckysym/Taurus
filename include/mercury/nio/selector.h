#pragma once
#include <mercury/error.h>
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
    SelectableChannel();
    virtual ~SelectableChannel();

protected:
    SelectableChannel(const SelectableChannel &other) = delete;
    SelectableChannel(SelectableChannel && other);

    SelectableChannel & operator=(const SelectableChannel& other) = delete;
    SelectableChannel & operator=(SelectableChannel & other);

public:

    SelectionKey * reg(Selector *selector, int ops, RuntimeError &e);
    SelectionKey * reg(Selector *selector, int ops, void *att, RuntimeError &e); 
    SelectionKey * key_for(Selector *selector);
    bool           is_registered() const;

public:
    virtual int valid_ops() const = 0;
}; // end class SelectableChannel


class Selector final {
private:
    class SelectorImpl;
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


}} // end namespace mercury::nio
