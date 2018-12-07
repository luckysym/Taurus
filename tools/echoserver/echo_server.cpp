#include <mercury/net/network.h>
#include <mercury/nio/channel.h>
#include <mercury/core/integer.h>

#include <cassert>
#include <iostream>
#include <stdio.h>

using namespace mercury;
using namespace std;

int main(int argc, char **argv) 
{
    RuntimeError e;
    
    // 创建监听channel。
    nio::ServerSocketChannel server;
    bool isok = server.create(AF_INET, e);
    if ( !isok ) {
        cerr<<e.str()<<endl;
        return -1;
    }

    // 创建选择器
    nio::Selector selector;
    isok = selector.open(e);
    if ( !isok ) {
        cerr<<e.str()<<endl;
        return -1;
    }

    selector.close(e);
    server.close(e);
    return 0;
}
