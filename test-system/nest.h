#pragma once

#include "../priority-aware-camkes/priority-protocols/priority-protocols.h"

//Stringification for nest method and protocol struct based on interface name
#define NEST_token(INTERFACE) INTERFACE##_nest
#define INFO_token(INTERFACE) INTERFACE##_info
#define REQUEST_token(interface_to,method) interface_to##_##method

//Define the interface's nest method (handles upstream priority inheritance)
#define NEST(INTERFACE) \
    extern struct Priority_Protocol INFO_token(INTERFACE); \
    void NEST_token(INTERFACE)(const int priority, const int requestor) { \
        nest_rcv(priority, requestor, & INFO_token(INTERFACE)); \
    }
    
//Function macro for nested requests to a downstream CPI
#define REQUEST(interface_from,interface_to,method,...) ( { \
    extern struct Priority_Protocol INFO_token(interface_from); \
    nested_pre(&priority, requestor, NEST_token(interface_to), & INFO_token(interface_from)); \
    __auto_type retval = REQUEST_token(interface_to,method)(__VA_ARGS__, priority, requestor); \
    nested_post(requestor, & INFO_token(interface_from)); \
    retval; } )
    