/*
    CMakeLists.txt

    The CMake build manifest for the priority-protocols-sample
    sample application
*/


cmake_minimum_required(VERSION 3.7.2)

project(priority-protocols-sample C)

includeGlobalComponents()

DeclareCAmkESComponent (Task SOURCES
    task.c
)

//CPIs implementing our library's priority protocols must link the associated C files

DeclareCAmkESComponent (ServiceForwarder SOURCES
    service-forwarder.c
    ../priority-protocols/priority-protocols.c
    ../priority-protocols/priority-inheritance.c
    ../priority-protocols/notification-manager.c
)

DeclareCAmkESComponent (ServiceTerminator SOURCES
    service-terminator.c
    ../priority-protocols/priority-protocols.c
    ../priority-protocols/priority-inheritance.c
    ../priority-protocols/notification-manager.c
)

CAmkESAddTemplatesPath("..")

//Declares connectors associated with each threadpool size
foreach(i RANGE 1 100)
    DeclareCAmkESConnector(seL4RPCCallPrioritized${i}
        FROM seL4RPCCall-from.template.c
        TO seL4RPCCallPrioritized-to.template.c
    )
endforeach()

DeclareCAmkESRootserver(task-system.camkes)
