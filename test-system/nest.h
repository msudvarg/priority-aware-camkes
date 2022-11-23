#pragma once

#include "../priority-aware-camkes/priority-protocols/priority-protocols.h"

//Stringification to define nest method for CPI based on interface name
#define NEST_token(INTERFACE) INTERFACE##_nest
#define INFO_token(INTERFACE) INTERFACE##_info
#define NEST(INTERFACE) \
    extern struct Priority_Protocol INFO_token(INTERFACE); \
    void NEST_token(INTERFACE)(const int priority, const int requestor) { \
        nest_rcv(priority, requestor, & INFO_token(INTERFACE)); \
    }
    
#define REQUEST(INTERFACE,fn,...) ( { \
    nested_pre(&priority, requestor, NEST_token(INTERFACE), & INFO_token(INTERFACE)); \
    __auto_type retval = fn(__VA_ARGS__, priority, requestor); \
    nested_post(requestor, & INFO_token(INTERFACE)); \
    retval; } )
    