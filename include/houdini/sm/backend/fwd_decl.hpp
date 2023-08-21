#pragma once
#include "houdini/util/types.hpp"

namespace houdini {
namespace sm {

struct NoEvent;
struct NoAction;
struct NoGuard;

template <JEvent Event, class Guard> class TransitionEG;
template <JEvent Event, class Action> class TransitionEA;
template <JEvent Event, class Guard, class Action> class TransitionEGA;
template <class Source, class Action> class TransitionSA;
template <class Source, class Guard> class TransitionSG;
template <class Source, JEvent Event> class TransitionSE;
template <class Source, JEvent Event, class Action> class TransitionSEA;
template <class Source, JEvent Event, class Guard> class TransitionSEG;
template <class Source, JEvent Event, class Guard, class Action> class TransitionSEGA;

template <class Source> struct TInitial; 
} // namespace sm
} // namespace houdini