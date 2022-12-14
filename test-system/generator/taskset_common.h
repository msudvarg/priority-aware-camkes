#pragma once

#include <string>

std::string test_system_header_0 = R"(

//Get macros and connectors for priority protocols
#include "../priority-aware-camkes/test-system/priority-protocols.camkes.h"
import "../../../../../priority-aware-camkes/priority-connectors.camkes";
import "../components.camkes";

)";

std::string test_system_header_1 = R"(

    provides SystemDispatch sd;
    uses Timer d1;
    uses Timer d2;
    uses Timer d3;
    uses Timer d4;

    composition {

        //Receives a request from RootDispatcher,
        //which initiates it dispatching each task Dispatcher
        component SystemDispatcher sd;
        export sd.sd -> sd;

        //Task dispatchers dispatch jobs from each corresponding task periodically
        component Dispatcher d1;
        component Dispatcher d2;
        component Dispatcher d3;
        component Dispatcher d4;
        
        //Event notifications to signal the task dispatchers to begin
        connection seL4Notification t1_dispatch(from sd.td1, to d1.td);
        connection seL4Notification t2_dispatch(from sd.td2, to d2.td);
        connection seL4Notification t3_dispatch(from sd.td3, to d3.td);
        connection seL4Notification t4_dispatch(from sd.td4, to d4.td);

        //Event notifications to signal that the task dispatchers have completed
        connection seL4Notification t1_complete(from d1.tdc, to sd.tdc1);
        connection seL4Notification t2_complete(from d2.tdc, to sd.tdc2);
        connection seL4Notification t3_complete(from d3.tdc, to sd.tdc3);
        connection seL4Notification t4_complete(from d3.tdc, to sd.tdc4);

        //Shared data for statistics between task dispatchers and system dispatcher
        connection seL4SharedData channel(
            from d1.miss_data,
            from d2.miss_data,
            from d3.miss_data,
            from d4.miss_data,
            to sd.miss_data);

        //Export for TimeServer
        export d1.timeout -> d1;
        export d2.timeout -> d2;
        export d3.timeout -> d3;
        export d4.timeout -> d4;

        //Tasks
        component Task t1;
        component Task t2;
        component Task t3;
        component Task t4;

        connection seL4RPCCall connt1(from d1.r, to t1.rd);
        connection seL4RPCCall connt2(from d2.r, to t2.rd);
        connection seL4RPCCall connt3(from d3.r, to t3.rd);
        connection seL4RPCCall connt4(from d4.r, to t4.rd);

        //Passive servers
        component L1 l1a;
        component L1 l1b;
        component L2 l2;
        component L3 l3;

        connection rpc(l1a_num_threads) connl1a(from t1.r, from t2.r, to l1a.r);
        connection rpc(l1b_num_threads) connl1b(from t3.r, from t4.r, to l1b.r);
        connection rpc(l2_r1_num_threads) connl2r1(from l1a.r1, from l1b.r1, to l2.r1);
        connection rpc(l2_r2_num_threads) connl2r2(from l1a.r2, from l1b.r2, to l2.r2);
        connection rpc(l3_num_threads) connl3(from l2.r, to l3.r);

    }

    configuration {

        /*
            SystemDispatcher begins at the highest system priority
            (RootDispatcher is also at the highest).
            When it recieves a request from RootDispatcher,
            it alerts all task dispatchers,
            then drops itself to the lowest system priority.
            This prevents it from interfering with task execution
            until all tasks are complete,
            whereupon it receives notifications on its 4 notification objects.
        */

        sd.priority = 255;
        d1.task_id = 1;
        d2.task_id = 2;
        d3.task_id = 3;
        d4.task_id = 4;        

)";

std::string test_system_threads = R"(
        l1a.r_num_threads = l1a_num_threads;
        l1b.r_num_threads = l1b_num_threads;
        l2.r1_num_threads = l2_r1_num_threads;
        l2.r2_num_threads = l2_r2_num_threads;
        l3.r_num_threads = l3_num_threads;
)";

std::string test_system_footer = R"(

		//Set task IDs
		t1.requestor = __COUNTER__;
		t2.requestor = __COUNTER__;
		t3.requestor = __COUNTER__;
		t4.requestor = __COUNTER__;

    }

}

)";