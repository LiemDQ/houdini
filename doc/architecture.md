# Architecture
This document discusses the rationale behind of the architectural decisions behind Estate. 

## Why template metaprogramming?

State machines are particularly well-suited to metaprogramming. Almost by definition, they are easy to model and have well-understood inputs and outputs. 
A highly scalable, production-quality state machine framework needs a couple of things:
* A way to "link" all the states with their transitions
* A way to "register" the states with the state machine
* A means of [multiple dispatch](https://en.wikipedia.org/wiki/Multiple_dispatch) to respond to events
* A means of generating events to be processed

With the exception of the last point, these can all be done at compile-time, which confers substantial benefits:
* **Type safety**: possible to determine if your state machine is ill-formed at compile time, reducing likelihood of error.
* **Better performance**: the state-event reactions are resolved at compile time, so at runtime the state machine only has to transition to 
the new state; it does not have to spend time "figuring out" what the correct transition is.
* **Scalability**: For larger state machines, connecting all the states and components together manually is tedious and error-prone. 
Having a framework that can do the work automatically allows for much larger and comprehensive state machines.
In this case, the advantages of template metaprogramming (type safety and scalability in particular) outweigh the downsides 
(increased complexity, lower maintainability, increased compile times). The complexity is hidden behind the backend, allowing for the application code to be much simpler.

## Compile time resolution
The general idea is this:
1. Create a DSL that represents UML state transitions, i.e. something of the form `origin_state + event[guard]/action = destination_state`. This information is stored at compile time in templates via type deduction. 
2. Recursively descend the hierarchical states, collecting all `Transition` objects into a type list. Each state is associated with a unique index, based on its position in the list. This position can be found using metafunctions like `boost::mp11::mp_find`.
3. Iterate through the transitions. For each transition, store the transition information in a 2D array, where the indices represent the event and state, respectively.
4. The transition information consists of:
   1. The index of the next state(s) to go to
   2. Any guards that need to be verified, wrapped in a lambda.
   3. Any actions that need to be executed upon transitioning.
   4. Whether this transition is a history transition, and if so, whether it is a deep or shallow history transition.
   5. Whether this is an internal transition (i.e. not triggered by an event).
   6. Whether or not the transition should be deferred. 

## Setting up the dispatch structure
The tricky part here is designing a data structure that allows for retaining the transition information (guards, actions, events and destination information) at compile time while allowing for traversing a runtime tree. 

There are two main approaches we could take:
1. Use a flat structure, like an array, to store the states via pointers. Transitions occur via array indices that are resolved at compile time and provided whenever a event is processed. In this case, the state tree hierarchy exists only implicitly. For each transition, a type-based search needs to be done for every state in the hierarchy, which will be costly from a compile time perspective and is pretty similar to what we're doing now. 
2. Store pointers to child states in an array within the states themselves. Transitions occur by looping through states and performing a linear search (at runtime) for the right state, or having array indices resolved at compile time (although the indices would be different in this case).
