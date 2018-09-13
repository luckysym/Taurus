
#include <taurus/net/network.h>

void print_all_addresses() ;

int main(int argc, char **argv) {
    print_all_addresses();
    return 0;
}

void print_all_addresses() {
    using namespace taurus::net;

    InetAddress::Vector vec;
    vec.reserve(32);
    ssize_t r = InetAddress::GetAllByName(vec, nullptr); 
    for( ssize_t i = 0 ; i < r; ++i) {
        printf("%s\n", vec[i]->ToString().c_str());
    }
}