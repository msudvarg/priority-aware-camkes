#pragma once

//Allows automatic selection of appropriate RPC connector according the number of worker threads
#define rpc_token(num_threads) seL4RPCCallPrioritized##num_threads
#define rpc(num_threads) rpc_token(num_threads)

#define interface_priority_attributes(name) \
    attribute int name##_num_threads; \
    attribute int name##_priority; \
    attribute string name##_priority_protocol; \
    attribute int name##_execution_time;

#define task_priority_attributes() \
    attribute int _priority; \
    attribute int execution_time_pre; \
    attribute int execution_time_post;

#define dispatcher_priority_attributes() \
    attribute int _priority; \
    attribute int release_time; \
    attribute int period; \
    attribute int periods; \
    uses Timer timeout;