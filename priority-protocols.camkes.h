/*

    priority-protocols.camkes.h

    This header can be included directly in a CAmkES component specification
    (i.e., a .camkes file)
    and provides macros to automatically add the necessary attributes
    to each priority-aware component.

    For example, a component encapsulating a task might be specified as follows:

    component Task {
        control;
        uses CPIA;
        uses CPIB;
        task_priority_attributes()
    }

    A shared component encapsulating one (or more) CPIs would specify
    interface_priority_attributes() for each CPI.
    For example, a component encapsulating CPIA and CPIB might be specified as:

    component Service {
        provides CPIA a;
        interface_priority_attributes(a);
        provides CPIB b;
        interface_priority_attributes(b);
    }

    Finally, to prevent value reuse, we allow specification of CPI threadpool size as a macro.
    For example, consider a Service component instance:

    component Service service1;

    This instance will need a pool of 3 threads on CPI a, and 2 threads on CPI b:

    #define service1_a_num_threads 3
    #define service1_b_num_threads 2

    Each definition can then be used both to assign the corresponding component attribute,
    as well as selecting the connector type with the appropriately-sized threadpool:

    service1.a_num_threads = service1_a_num_threads;
    service1.b_num_threads = service1_b_num_threads;
    connection rpc(service1_a_num_threads) conn_a(from task1.a, from task2.a, from task3.a, to service1.a);
    connection rpc(service1_b_num_threads) conn_b(from task1.b, from task2.b, to service1.b);

*/

#pragma once

//Stringification to select appropriate RPC connector according the thread pool size
#define rpc_token(num_threads) seL4RPCCallPrioritized##num_threads
#define rpc(num_threads) rpc_token(num_threads)

#define interface_priority_attributes(name) \
    attribute int name##_num_threads; \
    attribute int name##_priority; \
    attribute string name##_priority_protocol;

#define task_priority_attributes() \
    attribute int _priority;
    attribute int requestor = __COUNTER__;