/*

    notification-manager.c

    The implementation of the Notification Manager.
    See notification-manager.h for more details.

*/

#include "notification-manager.h"

#include <camkes.h>
#include <camkes/allocator.h>
#include <utils/attribute.h>



//Initialize the Notification Manager
void ntfn_mgr_init(struct Notification_Manager * ntfn_mgr, struct Notification_Node * node_arr,
        struct Notification_Node ** prio_queue, seL4_CPtr * ntfn_objs, unsigned arr_size) {

    //Only run on first thread
    if(!ntfn_mgr->initialized) {

        ntfn_mgr->initialized = true;

        //Set pointer to array of nodes
        ntfn_mgr->node_arr = node_arr;
        ntfn_mgr->arr_size = arr_size;

        //Initialize priority queue
        ntfn_mgr->prio_queue = prio_queue;
        ntfn_mgr->num_waiters = 0;
        ntfn_mgr->insert_order = 0;

        //Initialize free list
        ntfn_mgr->free_list = node_arr;
        for (unsigned i = 0; i < arr_size - 1; i++) {
            node_arr[i].next = node_arr + i + 1;
        }

        // Assign CAmkES-created notification objects
        for (unsigned i = 0; i < arr_size; i++) {
            node_arr[i].ntfn_obj = ntfn_objs[i];
        }

    }
}

//Check if a node is greater than another node
bool ntfn_greater_than(struct Notification_Node * lhs, struct Notification_Node * rhs) {

    //The lhs is greater if its priority is greater
    if(*(lhs->priority) > *(rhs->priority)) return true;
    
    //The lhs is not greater if its priority is less
    if(*(lhs->priority) < *(rhs->priority)) return false;

    //If the priorities are equal, compare the insertion orders
    if(lhs->insert_order < rhs->insert_order) return true;

    return false;
}

//Swap two pointers to nodes
void ntfn_swap(struct Notification_Node ** a, struct Notification_Node ** b) {
    struct Notification_Node * c = *a;
    *a = *b;
    *b = c;
}

//Recursively swap a node with its parents until the heap property is satisfied
void swap_parent(struct Notification_Manager * ntfn_mgr, unsigned child_index) {

    //Base case: head
    if (child_index == 0) return;
    
    struct Notification_Node ** prio_queue = ntfn_mgr->prio_queue;

    /*
    If we 1-index the heap,
    a node at level n can be indexed by n bits.
    The parent will be the first (n-1) bits,
    so we find the parent by
        * converting to 1-index
        * dropping the last bit
        * converting back to 0-index
    */
    unsigned parent_index = ((child_index + 1) >> 1) - 1;

    //Must we swap?
    if(ntfn_greater_than(prio_queue[child_index], prio_queue[parent_index])) {

        //If so, swap
        ntfn_swap(&prio_queue[child_index], &prio_queue[parent_index]);

        //Recurse
        swap_parent(ntfn_mgr, parent_index);
    }

}


//Recursively swap a node with its children until the heap property is satisfied
void swap_children(struct Notification_Manager * ntfn_mgr, unsigned parent_index) {

    /*
    If we 1-index the heap,
    a node at level n can be indexed by n bits.
    The children will share the first n bits,
    so we find the children by
        * converting to 1-index
        * adding a bit
        * add 0 or 1 to get the two children
        * (convert back to 0-index by actually subtracting 1 or adding 0)
    */   
    unsigned child_left_index = ((parent_index + 1) << 1) - 1;
    
    //Base case: no children
    if (child_left_index >= ntfn_mgr->num_waiters) return;
    
    struct Notification_Node ** prio_queue = ntfn_mgr->prio_queue;

    //Is there a right child, and is it greater than the left child?
    unsigned child_right_index = child_left_index + 1;
    if (child_right_index < ntfn_mgr->num_waiters &&
            ntfn_greater_than(prio_queue[child_right_index], prio_queue[child_left_index])) {

        //If so, swap right child with parent if needed
        if(ntfn_greater_than(prio_queue[child_right_index], prio_queue[parent_index])) {

            ntfn_swap(&prio_queue[child_right_index], &prio_queue[parent_index]);
            
            //Recurse
            swap_children(ntfn_mgr, child_right_index);
        }
    }
    else {

        //Otherwise, swap left child with parent if needed
        if(ntfn_greater_than(prio_queue[child_left_index], prio_queue[parent_index])) {

            ntfn_swap(&prio_queue[child_left_index], &prio_queue[parent_index]);

            //Recurse
            swap_children(ntfn_mgr, child_left_index);
        }

    }   

}

//Insert node into priority queue in priority order
void ntfn_mgr_insert(struct Notification_Manager * ntfn_mgr, struct Notification_Node * node) {

    //Increase insertion order to maintain stable sort
    node->insert_order = ntfn_mgr->insert_order;
    ntfn_mgr->insert_order++;

    unsigned index = ntfn_mgr->num_waiters;
    ntfn_mgr->num_waiters++;

    //Insert node at end of binary heap
    ntfn_mgr->prio_queue[index] = node;

    //Swap node with parents as needed
    swap_parent(ntfn_mgr, index);
    
}

//Remove head node from priority queue
void ntfn_mgr_pop(struct Notification_Manager * ntfn_mgr) {

    //Don't do anything if the heap is empty
    if (!ntfn_mgr->num_waiters) return;

    ntfn_mgr->num_waiters--;
    struct Notification_Node ** prio_queue = ntfn_mgr->prio_queue;

    //Return head node to free list
    prio_queue[0]->next = ntfn_mgr->free_list;
    ntfn_mgr->free_list = prio_queue[0];

    //There are still waiters in the heap, sort accordingly
    if(ntfn_mgr->num_waiters) {

        //Replace head node with last node
        prio_queue[0] = prio_queue[ntfn_mgr->num_waiters];

        //Swap node with children as needed
        swap_children(ntfn_mgr, 0);
    }

    //No waiters left, mark empty
    else {
        prio_queue[0] = NULL;
    }

}

void ntfn_mgr_update(int priority, char * requestor, struct Notification_Manager * ntfn_mgr) {

    //Search waiters
    for(int index = 0; index < ntfn_mgr->num_waiters; index++) {
        struct Notification_Node * node = ntfn_mgr->prio_queue[index];

        //Requestor found
        if(!strcmp(node->requestor),requestor) {
            //Update priority
            *(node->priority) = priority;

            //Reorder heap
            swap_parent(ntfn_mgr, index);

            //Stop searching
            break;
        }
    }
}

void ntfn_mgr_wait(int * priority, char * requestor, struct Notification_Manager * ntfn_mgr) {

    //Obtain notification object from head of free list
    struct Notification_Node * node = ntfn_mgr->free_list;
    ntfn_mgr->free_list = node->next;

    //Set notification object priority
    node->priority = priority;

    //Set notification object requestor for identification
    node->requestor = requestor;

    //Insert into priority queue
    ntfn_mgr_insert(ntfn_mgr, node);

    //Wait on notification object    
    seL4_Wait(node->ntfn_obj, NULL);

    //Once a thread wakes up, pop from head of priority queue
    ntfn_mgr_pop(ntfn_mgr);


}

void ntfn_mgr_signal(struct Notification_Manager * ntfn_mgr) {
    struct Notification_Node * head = ntfn_mgr->prio_queue[0];
    if(head) {
        seL4_Signal(head->ntfn_obj);
    }
}

//The following are for testing purposes
void ntfn_mgr_simulate_wait(int * priority, struct Notification_Manager * ntfn_mgr) {

    //Obtain notification object from head of free list
    struct Notification_Node * node = ntfn_mgr->free_list;
    ntfn_mgr->free_list = node->next;

    //Set notification object priority
    node->priority = priority;

    //Insert into priority queue
    ntfn_mgr_insert(ntfn_mgr, node);

}

void ntfn_mgr_simulate_wait_wake(int * priority, struct Notification_Manager * ntfn_mgr) {

    //Obtain notification object from head of free list
    struct Notification_Node * node = ntfn_mgr->free_list;
    ntfn_mgr->free_list = node->next;

    //Set notification object priority
    node->priority = priority;

    //Insert into priority queue
    ntfn_mgr_insert(ntfn_mgr, node);

    //Once a thread wakes up, pop from head of priority queue
    ntfn_mgr_pop(ntfn_mgr);

}

void ntfn_mgr_simulate_reset(struct Notification_Manager * ntfn_mgr) {

    unsigned arr_size = ntfn_mgr->arr_size;

    //Reset free list
    ntfn_mgr->free_list = ntfn_mgr->node_arr;
    for (unsigned i = 0; i < arr_size - 1; i++) {
        ntfn_mgr->node_arr[i].next = ntfn_mgr->node_arr + i + 1;
    }

    //Reset priority queue
    for (unsigned i = 0; i < arr_size; ++i) {
        ntfn_mgr->prio_queue[i] = NULL;
    }

    //Reset num_waiters and insert order
    ntfn_mgr->num_waiters = 0;
    ntfn_mgr->insert_order = 0;
}