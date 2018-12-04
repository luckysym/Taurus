#include <mercury/net/network.h>
#include <mercury/nio/selector.h>
#include <mercury/core/integer.h>
#include <cassert>
#include <stdio.h>

using namespace mercury;

int main(int argc, char **argv)
{
    if ( argc <= 1 ) {
        printf("%s <port>\n", argv[0]);
        return 0;
    }
    int port = Integer::parse(argv[1]);
    assert(port >= 0 && port <= 65535);

    // 创建Selector
    nio::Selector selector;

    // 创建ServerSocketChannel，用于监听
    RuntimeError e;
    nio::ServerSocketChannel servch;
    bool isok = servch.create(AF_INET, e);
    if ( !isok ) {
        printf("server channel create failed %s\n", e.str().c_str());
        return 0;
    }
    return 0;
}
