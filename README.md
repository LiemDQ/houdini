# Houdini framework
Houdini is a hierarchical state machine framework that is inspired by the [Harel Statechart](http://www.inf.ed.ac.uk/teaching/courses/seoc/2005_2006/resources/statecharts.pdf)
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
* Not requiring the use of a code generator, which makes verification efforts significantly more difficult and adds more "friction" to the development cycle.****
