# Janus State Machine
JSM is a hierarchical state machine that is inspired by the [Harel Statechart](http://www.inf.ed.ac.uk/teaching/courses/seoc/2005_2006/resources/statecharts.pdf)
formalism and UML state machine specification, but takes some notable departures from them, and does not attempt
to fully implement all of their features, the most notable missing one being orthogonal states (having multiple active states in parallel), 
as the gains in granularity were deemed not worth the increased implementation complexity.

A non-exhaustive list of design requirements is:

* Declarative syntax for describing and constructing state machines. This is essential to being to describe and configure the state machine in the most expressive way possible.
* A set of modular, abstracted building blocks to rapidly and easily build up complex state machines.
* No dynamic memory allocation required.
* Support for shallow and deep history transitions.
* Support for hierarchical states.
* Fast, deterministic performance.
* Type safety, with state mismatches and incorrectly specified transitions flagged at compile time
* Not requiring the use of a code generator, which makes verification efforts significantly more difficult and adds more "friction" to the development cycle.

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

# Current status
The current architecture is heavily based on using template metaprogramming to perform a lot of computation at compile time and reduce the dispatch computation to accessing a few array indices and a single virtual pointer function call at runtime. This is very efficient but leads to some architectural limitations, as everything about the transition needs to be known at compile time. This means that in particular we cannot call entry and exit actions when using history states, as the knowledge of which state to transition to (and consequently which callbacks to call) is only available at runtime. 

We also want to, as much as possible, preserve the current API as it is very convenient and easy to work with. 

## Architectural changes for runtime dispatch
In order to preserve the API and keep all the functionality we currently have, we should:
* Keep the "transition table" API intact, although we could possibly simplify the `Transition` class a little bit to reduce the amount of template instantiations needed and therefore reduce compilation times. 
* Have all states inherit from a single base state, and use a specific templated inheritance approach (see below) to maintain type safety while allowing for the use of virtual functions to reduce boilerplate. 
* When the state machine is instantiated, iterate through the transition table using a `for_each` function and construct a tree on the heap using `State*` pointers.

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

## Behaviors
Behaviors are modular units that can be added to a state in order to give it generic, predefined actions. They group together entry actions, exit actions and update actions. Implementing these is tricky as there is a delicate balance of compile time and run time information that needs to be blended together. Different behaviors have different types, but we want to minimize the use of templates as they won't place nicely with virtual functions. We also want to avoid any dynamic memory allocations in the main event loop, although allocations at startup are acceptable. 

One possible implementation:
```cpp
class BaseBehavior {
	virtual void onEntryImpl(Context&, Broker& broker) = 0;
	virtual void onExitImpl(Context&, Broker& broker) = 0;
	virtual void updateImpl(Context&, Broker& broker) = 0;
	
	std::chrono::time_point<std::chrono::monotonic_clock> last_update;
	std::chrono::milliseconds update_frequency = 50;
}

template <typename C, typename B>
class Behavior {
	void onEntryImpl(Context& context, Broker& broker) override final {
		onEntry(static_cast<C&>(context), static_cast<B&>(broker));
	}
	
	void updateImpl(Context& context, Broker& broker){
		if (std::chrono::monotonic_clock::now()-last_update >= update_frquency ) {
			update(static_cast<C&>(context), static_cast<B&>(broker));
		}
	}
	
	virtual void update(C& context, B& broker){}
	
	virtual void onEntry(C& context, B& broker){} //something
	
	virtual void onExit(...); //...
};
```