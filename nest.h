//Stringification to define nest method for CPI based on interface name
#define NEST_token(INTERFACE) INTERFACE##_nest
#define INFO_token(INTERFACE) INTERFACE##_info
#define NEST(INTERFACE) \
    void NEST_token(INTERFACE)(const int priority, const char * requestor) { \
        nest_rcv(priority, requestor, & INFO_token(INTERFACE)); \
    }