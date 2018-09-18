#include <taurus/net/network.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

void print_help(const char * program);
void list_host_addresses(const char * hostname);
void list_local_addresses();

int main(int argc, char *argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "lhn:")) != -1) {
        switch (opt) {
            case 'n':
                list_host_addresses(optarg);
                break;
            case 'h':
                print_help(argv[0]);
                break;
            case 'l':
                list_local_addresses();
                break;
        }
    }

    /* Other code omitted */
    exit(EXIT_SUCCESS);
}

void print_help(const char * program) {
    printf("%s <options>\n\n", program);
}

void list_host_addresses(const char * hostname) {
    using namespace taurus::net;
    InetAddress::Vector addrlist;
    size_t n = InetAddress::GetAllByName(addrlist, hostname);
    printf("%ld ip addresses found for hostname %s. \n", n, hostname);
    for ( size_t i = 0; i < n; ++i ) 
        printf("%s\n", addrlist[i]->ToString().c_str());
}

void list_local_addresses() {
    using namespace taurus::net;
    InetAddress::Vector addrlist;
    size_t n = InetAddress::GetLocalHost(addrlist);
    if ( n > 0 ) {
        for ( size_t i = 0; i < n; ++i ) 
        printf("%s\n", addrlist[i]->ToString().c_str());
    } else {
        printf("%ld ip addresses found in localhost. \n", n);
    }
     
    
}