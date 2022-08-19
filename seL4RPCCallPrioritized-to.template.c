/*
 *
 * sel4RPCCallPrioritized-to.template.c
 * 
 * All code taken, except where noted (with a priority-extensions label),
 * from the seL4 camkes-tool repo, /camkes/templates/seL4RPCCall-to.template.c
 *  
 * Implements CAmkES seL4 RPC Call (recipient side) functionality,
 * with additions for the priority-aware concurrency framework extensions.
 * 
 */

/*- if configuration[me.instance.name].get('environment', 'c').lower() == 'c' -*/

/*- from 'rpc-connector.c' import establish_recv_rpc, recv_first_rpc, complete_recv, begin_recv, begin_reply, complete_reply, reply_recv with context -*/

#include <camkes/dataport.h>
#include <camkes/allocator.h>
#include <utils/attribute.h>

/*
  priority-extensions:
  
  Include necessary declarations from the priority protocols library
*/
#include "priority-protocols/priority-protocols.h"

/*? macros.show_includes(me.instance.type.includes) ?*/
/*? macros.show_includes(me.interface.type.includes) ?*/

/*- set connector = namespace() -*/

/*- set buffer = configuration[me.parent.name].get('buffer') -*/
/*- if buffer is none -*/
  /*? establish_recv_rpc(connector, me.interface.name) ?*/
/*- else -*/
  /*- if not isinstance(buffer, six.string_types) -*/
    /*? raise(TemplateError('invalid non-string setting for userspace buffer to back RPC connection', me.parent)) ?*/
  /*- endif -*/
  /*- if len(me.parent.from_ends) != 1 or len(me.parent.to_ends) != 1 -*/
    /*? raise(TemplateError('invalid use of userspace buffer to back RPC connection that is not 1-to-1', me.parent)) ?*/
  /*- endif -*/
  /*- set c = list(filter(lambda('x: x.name == \'%s\'' % buffer), composition.connections)) -*/
  /*- if len(c) == 0 -*/
    /*? raise(TemplateError('invalid setting to non-existent connection for userspace buffer to back RPC connection', me.parent)) ?*/
  /*- endif -*/
  /*- if len(c[0].from_ends) != 1 or len(c[0].to_ends) != 1 -*/
    /*? raise(TemplateError('invalid use of userspace buffer that is not 1-to-1 to back RPC connection', me.parent)) ?*/
  /*- endif -*/
  /*- if not isinstance(c[0].to_end.interface, camkes.ast.Dataport) -*/
    /*? raise(TemplateError('invalid use of non-dataport to back RPC connection', me.parent)) ?*/
  /*- endif -*/
  extern /*? macros.dataport_type(c[0].to_end.interface.type) ?*/ * /*? c[0].to_end.interface.name ?*/;
  /*? establish_recv_rpc(connector, me.interface.name, buffer=('((void*)%s)' % c[0].to_end.interface.name, macros.dataport_size(c[0].to_end.interface.type))) ?*/
/*- endif -*/



/*
  priority-extensions:

  The following code, up to (but not including) the final endif,
  is added to support the priority protocols
*/

//Create a component-scoped struct for the interface Priority_Protocol information
struct Priority_Protocol /*? me.interface.name ?*/_info;

//Include RPC priority connector template instead of default RPC connector template
/*- include 'rpc-priority-connector-common-to.c' -*/

/*
  Template-defined __init function to initialize priority protocols,
  overrides component interface __init function.
  Instead, component writer supplies an _init function
  (note the single, instead of double, underscore)
*/
extern void /*? me.interface.name ?*/_init(void);
void /*? me.interface.name ?*/__init(void) {

    //Get priority protocol specified by component attribute
  
    /*- set protocols = ("propagated", "inherited", "fixed") -*/
    /*- set attr = '%s_priority_protocol' % me.interface.name -*/
    /*- set priority_protocol = configuration[me.instance.name].get(attr) -*/
    /*- if priority_protocol not in protocols -*/
      /*? raise(TemplateError('Invalid attribute "%s" for %s, must be one of "propagated", "inherited", "fixed"' % (priority_protocol, attr), me.parent)) ?*/
    /*- endif -*/

    /*
      Verify that this is not PIP -> PIP or PIP -> Propagated
      Does not yet work: a nested component has a "uses" and a "provides" which are not easily linked.
      We could check all "provides" for a component that connects, via "uses," to a non-fixed-type interface,
      then raise a warning if any "provides" is type inherited,
      but the CAmkES parser does not support checking to verify which interface calls which,
      so we can't raise an error to stop compilation.
      We defer this to future work.
    /*- if priority_protocol != "fixed" -*/
      /*- for from_end in me.parent.from_ends -*/
        /*- set from_attr = '%s_priority_protocol' % from_end.interface.name -*/      
        /*- set from_priority_protocol = configuration[from_end.instance.name].get(from_attr) -*/
        /*- if from_priority_protocol == "inherited" -*/
          /*? raise(TemplateError('Invalid connection from "%s.%s" to "%s.%s", interface of type "propagated" can only send RPC requests to interfaces of type "fixed"'
            % (from_end.instance.name, from_end.interface.name, me.instance.name, me.interface.name), me.parent)) ?*/
        /*- endif -*/
      /*- endfor -*/
    /*- endif -*/
    */

    //Initialize the Priority_Protocol struct

    priority_protocol_init(&/*? me.interface.name ?*/_info,
        /*? priority_protocol ?*/,
        CAMKES_CONST_ATTR(/*? me.interface.name ?*/_priority));
    
    //If necessary, initialize Priority Inheritance Protocol

    /*- if priority_protocol == "inherited" -*/    

      //Get the number of threads specified by component attribute

      /*- set attr = '%s_num_threads' % me.interface.name -*/
      /*- set num_threads = int(configuration[me.instance.name].get(attr)) -*/

      /*
        Allocates a static array of notification objects.
        Even though it's in the init function scope,
        each object's CPtr is bound to a Notification Node
        and so can be accessed from component scope
      */
      static seL4_CPtr ntfn_objs[/*? num_threads ?*/];
      /*- for i in range(num_threads) -*/
          /*- set ntfn = alloc('%s_ntfn_obj_%d' % (me.interface.name, i), seL4_NotificationObject, read=True, write=True) -*/
          ntfn_objs[/*? i ?*/] = /*? ntfn ?*/;
      /*- endfor -*/

      /*
        Initialize Priority_Inheritance and Notification_Manager objects       

        It might be possible to implement the notification manager
        with NUM_THREADS-1 notification nodes.
        We currently use NUM_THREADS for safety.
        We defer analysis and evaluation with NUM_THREADS-1 to future work.
      */
      PRIORITY_INHERITANCE_INIT(&/*? me.interface.name ?*/_info, /*? num_threads ?*/,
          CAMKES_CONST_ATTR(/*? me.interface.name ?*/_priority))
      NOTIFICATION_MANAGER_INIT(&/*? me.interface.name ?*/_info.pip->ntfn_mgr, ntfn_objs, /*? num_threads ?*/);
    /*- endif -*/

    /*? me.interface.name ?*/_init();
}


/*- endif -*/
