

#ifdef PRIORITY_QUEUE_LIST

    //Insert node into priority queue in priority order
    void ntfn_mgr_insert(struct Notification_Node ** prio_queue, struct Notification_Node * node) {
        
        //Container is not empty
        if(*prio_queue) {
            struct Notification_Node * it = *prio_queue; //Iterator points to head;

            //Do we insert at head?
            if(it->priority < node->priority) {
                node->next = it;
                node->prev = it->prev;
                it->prev = node;
                if(*prio_queue == it) (*prio_queue) = node;
                return;
            }

            //Otherwise, iterate until next node has lower priority (or is head of list)
            //and insert after iterator
            while(it->next->priority >= node->priority && it->next != *prio_queue) {
                it = it->next;
            }

            //Insert after iterator
            node->next = it->next;
            it->next = node;
            node->prev = it;
        }

        //Container is empty
        else {
            *prio_queue = node; //Node becomes head
            node->next = node->prev = node;
        }

    }

    //Remove head node from priority queue
    void ntfn_mgr_pop(struct Notification_Manager * ntfn_mgr) {
        
        //Obtain head
        struct Notification_Node * head = ntfn_mgr->prio_queue;

        //Only one node left, removing it means priority queue is empty
        if(head->next == head) {
            ntfn_mgr->prio_queue = NULL;
        }

        //There are nodes left, remove the head node
        else {
            ntfn_mgr->prio_queue = head->prev->next = head->next;
            head->next->prev = head->prev;
        }

        //Place node at head of free list
        head->next = ntfn_mgr->free_list;
        ntfn_mgr->free_list = head;
    }

#endif //PRIORITY_QUEUE_LIST