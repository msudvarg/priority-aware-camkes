#
#   CMakeLists.txt
#
#   The CMake build manifest for the priority-protocols-sample
#   sample application
#


cmake_minimum_required(VERSION 3.7.2)

project(test-system C)

includeGlobalComponents()

DeclareCAmkESComponent(RootDispatcher SOURCES root_dispatcher.c)

DeclareCamkesComponent(SystemDispatcher SOURCES system_dispatcher.c)

DeclareCamkesComponent(Dispatcher SOURCES dispatcher.c)

DeclareCAmkESComponent (Task SOURCES task.c)

#
#   CPIs implementing our library's priority protocols must link the associated C files
#

DeclareCAmkESComponent (L1 SOURCES
l1.c
../priority-aware-camkes/priority-protocols/priority-protocols.c
../priority-aware-camkes/priority-protocols/priority-propagation.c
../priority-aware-camkes/priority-protocols/priority-inheritance.c
../priority-aware-camkes/priority-protocols/notification-manager.c
)

DeclareCAmkESComponent (L2 SOURCES
    l2.c
    ../priority-aware-camkes/priority-protocols/priority-protocols.c
    ../priority-aware-camkes/priority-protocols/priority-propagation.c
    ../priority-aware-camkes/priority-protocols/priority-inheritance.c
    ../priority-aware-camkes/priority-protocols/notification-manager.c
)

DeclareCAmkESComponent (L3 SOURCES
    l3.c
    ../priority-aware-camkes/priority-protocols/priority-protocols.c
    ../priority-aware-camkes/priority-protocols/priority-propagation.c
    ../priority-aware-camkes/priority-protocols/priority-inheritance.c
    ../priority-aware-camkes/priority-protocols/notification-manager.c
)

# Add connector templates
CAmkESAddTemplatesPath("../priority-aware-camkes")

# Declares connectors associated with each threadpool size
foreach(i RANGE 1 100)
    DeclareCAmkESConnector(seL4RPCCallPrioritized${i}
        FROM seL4RPCCallPrioritized-from.template.c
        TO seL4RPCCallPrioritized-to.template.c
    )
endforeach()

DeclareCAmkESRootserver(task-system.camkes)
