# A Concurrency Framework for Priority-Aware Intercomponent Requests in CAmkES on seL4

## Introduction

Under priority-based scheduling, inter-component control flow should be coupled with priority information, so that task execution can be prioritized appropriately end-to-end. However, the CAmkES component architecture (https://docs.sel4.systems/projects/camkes/) for the seL4 microkernel (https://sel4.systems/) does not adequately support priority propagation across intercomponent requests: component interfaces are bound to threads that execute at fixed priorities provided at compile-time in the component specification. This library provides CAmkES with a thread model that supports (1) multiple concurrent requests to the same component endpoint; (2) propagation and enforcement of priority metadata, such that those requests are appropriately prioritized; and (3) implementations of Non-Preemptive Critical Sections, the Immediate Priority Ceiling Protocol and the Priority Inheritance Protocol for components encapsulating critical sections of exclusive access to a shared resource.

More details on the design, implementation, and evaluation of this library are available in the following paper:

*M. Sudvarg and C. Gill. "A Concurrency Framework for Priority-Aware Intercomponent Requests in CAmkES on seL4." The 28th IEEE International Conference on Embedded and Real-Time Computing Systems and Applications (RTCSA), August 2022.*

The paper can be accessed at https://www.sudvarg.com/priority-aware-camkes/

## Overview

This library targets implicit-deadline, sporadic task systems, using fixed-priority, preemptive scheduling on a uniprocessor. As a fully userspace implementation, we target closed, trusted systems. Tasks and dependent components must execute on the same core, meaning that both unicore or fully-partitioned multicore systems are supported.

We assume that a system is described (in CAmkES) as a set of components, for which some *originate* tasks, and others provide *component procedural interfaces* (CPIs) across which task execution proceeds sequentially via synchronous RPC requests. Our library provides mechanisms supporting CPI behavior according to the following four protocols:

### Priority Propagation

For components encapsulating thread-safe, reentrant shared functionality, a CPI can provide *priority propagation*. CPI execution should be at the priority of the requesting task, and should be preemptible by requests from higher priority tasks. To achieve this, the CPI is coupled with a thread pool sized according to the number of possible concurrent requests. Threads wait at the priority ceiling of all requestors. When a request is sent, the recipient thread demotes itself to the request priority before executing the procedure.

### Shared Resource Access Protocols

For components encapsulating mutually exclusive access to shared state, we provide three priority-based locking protocols for CPIs executing critical sections.

__Non-Preemptive Critical Sections__

Under NPCS, the CPI is assigned a single thread at the maximum system priority. This implicitly prevents its execution from being preempted, but NPCS suffers from inflated worst-case blocking times.

__Immediate Priority Ceiling Protocol__

Under IPCP, the CPI is assigned a single thread at the maximum priority among all tasks that can send it a request (also called the Highest Lockers Priority, HLP). Of the protocols we support, IPCP provides the best upper bound on worst-case blocking times.

__Priority Inheritance Protocol__

Under PIP, the CPI is coupled with a threadpool at the HLP, sized according to the number of possible concurrent requests, similarly to the priority propagation protocol. The CPI is additionally provided with three associated variables: a non-atomic boolean lock, a pointer to the Thread Control Block (TCB) of the lock-holder, and the current inherited priority. When a request arrives, the responding thread checks the lock. If the lock is already held, it proceeds to check the inherited priority variable against its own request priority. If the request priority is higher, it is inherited by the thread currently holding the lock: the responding thread updates the inherited priority variable, then elevates the priority of the locking thread's TCB. At this point, it waits for a signal indicating that the lock has been freed.

If, however, the lock is unlocked, the thread marks the lock as locked, sets the inherited priority variable to the request priority, sets the TCB pointer to itself, then demotes its priority to the request priority, runs the interface's procedure code to handle the request. Once complete, it promotes itself back to the priority ceiling, marks the lock as unlocked, signals any threads waiting for the lock, then finally (16) replies to the requestor and returns to waiting on the endpoint.

The seL4 kernel provides *notification objects*, which are simple signaling mechanisms that support blocked waiting. Notification objects are not priority aware if the seL4 kernel is compiled without MCS features: when a signal is received, the kernel wakes the first waiting thread. Thus, a single notification object is insufficient for signalling the threads waiting on a held lock, as the highest priority waiting thread is not guaranteed to be the first to obtain the lock when it becomes available. For the default kernel, we implement a priority-aware signaling mechanism that we call a __*notification manager*__.

The notification manager contains a priority queue (implemented as a max-heap) of notification objects, sorted by priority, with ties broken by earliest insertion. When initialized, the notification manager creates an array of notification objects, equal to the size of the thread pool, by using the CAmkES seL4 object allocator. The notification manager reveals two public functions, `wait` and `signal`, similar to the seL4 system calls of the same names for notification objects. The request priority is passed with the wait call, allowing the notification manager to retrieve a notification object from the free list, then insert it into the heap. The wait function then uses a system call to wait on that notification object.
The notification manager signals the notification object at the head of the heap. The awakened thread returns from the seL4 wait system call; its control flow remains in the notification manager's wait function, which pops its notification object from the head of the priority queue. The thread then proceeds as if it had found the lock available.

### Notification Manager and Priority-Based Locking

Our __notification manager__ solves the problem presented by the seL4 default kernel: it uses priority-based, rather than FIFO, ordering to wake waiting threads. While the seL4 MCS kernel does provide a priority-ordered notification object, the implementation uses a linked list, rather than a max-heap, which has linear (rather than logarithmic) asymptotic complexity over the number of waiting threads.

As detailed in the associated paper, the priority semantics of our locking protocols allow the boolean lock variable used by PIP to be non-atomic. The notification manager serves an additional purpose for the semantics of our PIP implementation: it enables waiting in order of *request priority*, rather than *thread priority*. This allows the waiting threads to *remain at the highest lockers priority* when waiting for the lock, rather than having to demote to the request priority before waiting. Thread priority demotion (besides during the execution of the procedure, when the lock is held) may introduce race conditions that break the semantics of the priority-based locking protocol.

### Round Robin Scheduling

Under round-robin scheduling, threads of *equal* priority may preempt each other, which breaks the semantics of our priority-based shared resource access protocols. Because seL4 implements round-robin scheduling by default, we adopt a priority-laddering scheme whereby tasks (and their originating components' threads) are restricted to even priority values (from seL4's 0-254 priority range), and reserve odd priorities for threadpools associated with CPIs implementing locking protocols, which are assigned a priority of HLP+1.

While this reduces the number of effective priority levels from 256 to 128, this still leaves more priorities available than are provided by Linux's fixed-priority scheduling classes which should be sufficient for most real-world task systems.

### Nested Locking

Nested locking can be achieved through a chain of requests: a CPI encapsulating one lock can make a request to another CPI encapsulating the second lock.

To guarantee the absence of deadlock one would have to ensure that no cycles exist in the digraph of connections. For a given system specification, CAmkES can generate a digraph representation in the DOT language; this may be used to
detect cycles, which alert to possible deadlock.

Currently, our framework does not support nested PIP; a CPI implementing PIP can only send requests to a CPI implementing NPCS or IPCP.

## Installation and Use



## System Digraph Analysis

Development of a tool to analyze the system digraph is underway. Soon, you will be able to automate:

* Identification of request cycles, indicating possible deadlock
* Determining the maximum priority among all requesters (HLP) to assign CPI thread/threadpool priorities
* Counting the number of tasks that request a shared CPI to assign threadpool sizes