/*
    notification-manager.h

    The standard (i.e. non-MCS) version of the seL4 kernel
    implements FIFO notification objects that are not priority-ordered.
    If a collection of threads are waiting for a signal,
    we want the signal to wake the highest-priority thread.

    So, we implement a Notification Manager,
    which provides the same wait and signal functions as a standard seL4
    notification object,
    while ensuring that the highest-priority waiter is woken on a signal.

    To do so, we define a Notification_Node,
    which couples a notification object to a priority.
    This allows multiple notification objects,
    on which threads are waiting, to be inserted into a data-structure in priority order
    (we use a max-heap, allowing for quasilinear time complexity on insertion and removal).

    The Notification_Node also has previous and next pointers,
    allowing it to be used as a node in a variety of containers
    (in our case, linked lists and heaps)

    Our Notification Manager does require an array of Notification Nodes,
    with the corresponding notification objects,
    be supplied to it in its init function.

    From there, it ihitializes a singly-linked list of free nodes.
    When a thread requests to wait on the Notification Manager,
    it grabs the first free node, assigns it the appropriate priority,
    then inserts it into a priority queue, implemented as a binary heap.

    When the Notification Manager is signaled,
    it sends a signal to the notification object bound to the head node.

    Once a thread wakes from waiting,
    it pops its Notification Node from the head of the priority queue.

    For more details, see the associated paper
    (available at https://www.sudvarg.com/priority-aware-camkes/)
    or the README.
*/

#pragma once

//#define DEBUG

#include <camkes.h>

struct Notification_Node {
    int * priority;
    int requestor;
    unsigned long long insert_order;
    seL4_CPtr ntfn_obj;
    struct Notification_Node * next;
};

struct Notification_Manager {

    bool initialized;

    //Array of Notification Nodes, passed at initialization
    struct Notification_Node * node_arr;

    //Pointer to array of pointers to Notification Nodes,
    //serving as the head of the priority queue
    struct Notification_Node ** prio_queue; 
    unsigned num_waiters;
    unsigned long long insert_order;

    //Pointer to head of singly-linked of available Notification Nodes, NULL if all waiting
    struct Notification_Node * free_list;

    unsigned arr_size;

};

/*
    Notification Manager Init

    Allocates static memory for the Notification_Nodes.
    Even though it's in the init function scope,
    this array is accessible through the pointer in the Notification_Manager object.
    
    Additionally allocates static memory for an array of pointers to Notification_Nodes,
    which implements the binary heap, serving as a priority queue.
    Again, despite being in the init function scope,
    this array is accessible through the pointer in the Notification_Manager object.

    Finally allocates static memory for the seL4 object allocator,
    which is then passed to the ntfn_mgr_init function.
*/

#define NOTIFICATION_MANAGER_INIT(NOTIFICATION_MANAGER_PTR, NTFN_OBJ_ARR, ARR_SIZE) \
    static struct Notification_Node ntfns[ARR_SIZE]; \
    static struct Notification_Node * prio_queue[ARR_SIZE]; \
    ntfn_mgr_init(NOTIFICATION_MANAGER_PTR, ntfns, \
            prio_queue, NTFN_OBJ_ARR, ARR_SIZE);


//Initialize Notification Manager
void ntfn_mgr_init(struct Notification_Manager * ntfn_mgr, struct Notification_Node * node_arr,
        struct Notification_Node ** prio_queue, seL4_CPtr * ntfn_objs, unsigned arr_size);

//Wait on the Notification Manager as if it's a Notification Object
void ntfn_mgr_wait(int * priority, int requestor, struct Notification_Manager * ntfn_mgr);

//Update priority for a node in the Notification Manager
void ntfn_mgr_update(int priority, int requestor, struct Notification_Manager * ntfn_mgr);

//Signal on the Notification Manager as if it's a Notification Object
void ntfn_mgr_signal(struct Notification_Manager * ntfn_mgr);

//The following are for testing purposes
void ntfn_mgr_simulate_wait(int * priority, struct Notification_Manager * ntfn_mgr);
void ntfn_mgr_simulate_wait_wake(int * priority, struct Notification_Manager * ntfn_mgr);
void ntfn_mgr_simulate_reset(struct Notification_Manager * ntfn_mgr);