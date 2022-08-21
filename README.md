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

To guarantee the absence of deadlock one would have to ensure that no cycles exist in the digraph of connections. For a given system specification, CAmkES can generate a digraph representation in the DOT language; this may be used to detect cycles, which alert to possible deadlock.

Currently, our framework does not support nested PIP; a CPI implementing PIP can only send requests to a CPI implementing NPCS or IPCP.

## Installation and Use

The `priority-protocols-sample` directory contains a sample CAmkES application system that serves as a guide to the installation and use of our library.

To get started, you will need to already have CAmkES, as well as the associated build tools, on your system. Follow the instructions at https://docs.sel4.systems/projects/camkes/ for more information.

The easiest way to use our extensions in the context of the existing CAmkES buildsystem is to clone this repository directly into the CAmkES directory, then copy the sample application into the `projects/camkes/apps/` directory. For example, if you following the linked instructions for downloading CAmkES, navigate into the `camkes-project` directory, then:

    git clone https://github.com/msudvarg/priority-aware-camkes
    cp -r priority-aware-camkes/priority-protocols-sample/ projects/camkes/apps/

You can then use the `priority-protocols-sample` as a starting point for your own project. To build it, use the same instructions for building and running a sample application that are provided in the instructions linked above, but change the `-DCAMKES_APP=adder` argument to `-DCAMKES_APP=priority-protocols-sample`.

### The Sample Application

If you successfully build and run the sample application, you've done everything right! The application will rapidly output to the terminal as each task releases jobs. You should see something like:

    Task t4: 40^0=1
    Task t2: 30^0=1
    Task t3: 20^0=1
    Task t1: 10^0=1
    Task t4: 40^1=40
    Task t4: 40^2=1600
    Task t2: 30^1=30
    ...
    
This will continue indefinitely until you kill the system (e.g., by closing the QEMU monitor, or, if terminal based, using `CTRL+a` then `c`).

The sample application implements four tasks in originating components:

* t1, period = 1000ms, priority = 10
* t2, period =  200ms, priority = 30
* t3, period =  500ms, priority = 20
* t4, period =  100ms, priority = 40

Periodic dispatch is implemented within each component, which registers a timeout to a common TimeServer component, provided as part of the CAmkES global-components repository. We provide a fork of this repository (https://github.com/msudvarg/global-components/) that adds support for the BCM2837 chip (used in the Raspberry Pi Model 3B and 3B+, later versions of the Raspberry Pi Model 2B, and the Raspberry Pi Compute Module 3).

The component layout is illustrated roughly as:

    t1 -----v
            propagation ----|
    t2 -----^				v
                            ipcp
    t3 -----v				^
            pip ------------|
    t4 -----^

Here, tasks t1 and t2 request a common CPI implementing pip (in a component of type ServiceForwarder), tasks t3 and t4 request a common CPI implementing priority propagation (in a component of type ServiceForwarder), and the common CPIs forward nested requests to a common CPI implementing ipcp (in a component of type ServiceTerminator).

Priorities are set according to our laddering scheme, and threadpools are appropriately sized.

Each task registers a periodic timeout with a CAmkES TimeServer global component. At each job release, it increments a component-local iterations variable, then prints the result of raising task priority to that number of iterations. The power is implemented as a request to a ServiceForwarder component, which itself forwards the request to the ServiceTerminator.

### Usage Details

We assume that a user of our framework is already familiar with CAmkES. If not, the CAmkES manual is available from https://docs.sel4.systems/projects/camkes/manual.html.

Our framework requires few changes to an existing CAmkES specification. Primarily, to assign one of our priority protocols to a procedure interface, connections to that interface will need to be over one of the `seL4RPCCallPrioritizedN` connector types that we supply, where `N` is the size of the associated threadpool (see the __Overview__ above for details). The interface will then need the following attributes:

    attribute int NAME_num_threads;
    attribute int NAME_priority;
    attribute string NAME_priority_protocol;

Here, `NAME` is to be replaced with the name of the procedure interface.

To help prevent parameter reuse (as the threadpool size must be set for both the `NAME_num_threads` attribute and the connector type), and to ease the assignment of these attributes, we provide several macros in the `priority-protocols.camkes.h` header. In our sample applications system specification (`task-system.camkes`), you can see how these macros are used:

* A threadpool size is defined for each interface using a C-style macro declaration
* The appropriate connector type is selected by using the rpc() function macro, which takes the threadpool-size constant macro as its parameter
* The `NAME_num_threads` attributes are set using the same constant macro

The `NAME_priority` parameter for each procedure interface, as well as the `_priority` parameter for each active (task) component, must be explicitly declared and assigned a value (see the discussion of priority laddering under the __Round Robin Scheduling__ subsection of the __Overview__ for more details). By explicitly declaring the attribute, CAmkES will make it available as a constant in the underlying C code. Failure to do so will cause a compilation error. We also provide the `task_priority_attributes()` macro (which takes no argument) to be added to task component specifications.

The `NAME_priority_protocol` can take one of 3 values: "propagated", "inherited", or "fixed" (which enables either IPCP or NPCS, depending on the assigned priority). Failure to supply one of these 3 values will result in compilation error.

You'll also notice, in `task-system.camkes`, that any procedures provided by interfaces using our library must include priority as the last parameter of any function signatures, e.g.:

	int pow(in int base, in int exponent, in int priority);
	
When defining the corresponding function in a component's backing C code, be sure to include this in the signature as well (though you do not need to use the variable in the function).

When calling the function, however, you do need to pass the appropriate priority via the function call. The priority depends on what is calling the function:

* Task Component: `_priority` (pass the task's priority)
* Priority Propagation: `priority` (pass the request priority)
* Fixed Priority (IPCP or NPCS): `NAME_priority` (pass the priority assigned to the CPI) 
* PIP: Does not matter. PIP should *only* send priority-based requests to fixed priority CPIs. However, *if* you want to pass the priority as information, you can use:
    * `NAME_priority` (pass the priority assigned to the CPI)
    * `priority` (pass the request priority)
    * `lock->inherited_priority` (pass the current inherited priority)

### Build Considerations

The sample application's `CMakeLists.txt` illustrates some of the subtleties of using our library. Notice that any components implementing one of our protocols must be linked to the appropriate source files. At a minimum, `priority-protocols.c` is needed; for any implementing PIP, `priority-inheritance.c` and `notification-manager.c` must also be linked.

Additionally, because (as previously stated) a different connector type is necessary for each threadpool size, we have to both add the path to the templates using `CAmkESAddTemplatesPath("../priority-aware-camkes")`, as well as declare the connectors for each size. To prevent mistakes if threadpools need to be later resized in the component specification, we do this using a `foreach` loop to support threadpools of up to 100 threads.

Various include and import directives within the code are structured under the assumption that you implement your CAmkES system by building off of the provided sample application built according to the above instructions. If you take a different approach (e.g., via a different directory structure), you'll need to make sure that the include and import paths are all structured correctly.

The following paths are all relative to the `build` directory:

* `priority-protocols-sample/CMakeLists.txt`:
    * `DeclareCAmkESComponent` sources
    * `CAmkESAddTemplatesPath`
* `priority-protocols-sample/task-system.camkes`:
    * `#include ... priority-protocols.camkes.h`
* `seL4RPCCallPrioritized-to.template.c`:
    *  `#include ... priority-protocols.h`
 
The following, however, is relative to the file from which it is referenced:

* `priority-protocols-sample/task-system.camkes`:
    * `import ... priority-connectors.camkes`


## System Digraph Analysis

Development of a tool to analyze the system digraph is underway. Soon, you will be able to automate:

* Identification of request cycles, indicating possible deadlock
* Determining the maximum priority among all requesters (HLP) to assign CPI thread/threadpool priorities
* Counting the number of tasks that request a shared CPI to assign threadpool sizes

For now, CAmkES automatically creates a system digraph representation in the DOT language, which you can analyze by hand or with the tool of your choice. The file is under the build directory, and is named `graph.dot`