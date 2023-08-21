#pragma once
#include <cstddef>
#include <iterator>
#include "houdini/sm/backend/algorithms.hpp"
#include "houdini/sm/backend/fill_dispatch_table.hpp"
#include "houdini/sm/backend/index.hpp"
#include "houdini/sm/backend/state.hpp"
#include "houdini/sm/backend/traits.hpp"
#include "houdini/sm/backend/dispatch_table.hpp"
#include "houdini/sm/backend/transition_table_traits.hpp"
#include "houdini/sm/backend/variant_queue.hpp"
#include "houdini/sm/backend/collect.hpp"
#include "houdini/sm/backend/collect_initial_states.hpp"
#include "houdini/sm/backend/type_list.hpp"
#include "houdini/sm/frontend/base_state.hpp"
#include "houdini/sm/frontend/static_stack.hpp"

#include "houdini/actor/context.hpp"
#include "houdini/brokers/message_broker.hpp"

#include "houdini/util/mp11.hpp"
#include "houdini/util/type_name.hpp"
#include "houdini/util/static_typeid.hpp"
#include "houdini/util/types.hpp"

#include "houdini/actor/context.hpp"
#include "houdini/brokers/message_broker.hpp"
#include "houdini/util/utility_functions.hpp"

#include <array>
#include <cstdint>
#include <vector>
#include <iostream>
#include <tuple>
#include <memory>
#include <type_traits>
#include <memory>
#include <deque>
#include <queue>
#include <variant>
#include <string_view>
namespace houdini {
namespace sm {

enum class SMResult {
	SUCCESS,
	DEFERRED,
	NOTHING,
	FAILED,
	ERROR
};


/**
 * @brief State machine engine. The root state of all state machines must be passed in as a template. 
 * All transitions are resolved at compile time using template metaprogramming and constexpr control flow.
 * The dispatch table is stored in static global memory and is unique to each state machine type. 
 * 
 * The active state is 
 */
template <class RootState, class EventEnum, 
	class Context = act::BaseContext, class Broker = brokers::BaseBroker, 
	class... OptionalArgs> 
class SM {
	public:
	using Root = TState<RootState>;
	static constexpr Root root_state {};
	using Events = EventEnum;	
	

	public:
	static_assert(std::is_enum_v<EventEnum>, "Event must be an enum type.");
	static_assert(std::is_same_v<std::underlying_type_t<EventEnum>, JEvent>, 
			"Enum representational type must be JEvent type.");	
	
	using Transitions = decltype(flattenTransitionTable(root_state));
	using StateMap = decltype(getCombinedStateTypeIDs(root_state)); //note that root state is at front
	static constexpr std::size_t SM_DEPTH = mp::mp_max_element<mp::mp_transform<mp::mp_size, StateMap>, mp::mp_less>::value;
	static constexpr std::size_t NUM_STATES = mp::mp_size<StateMap>::value;
	static constexpr JEvent NO_EVENT_VALUE = util::enum_max_value<EventEnum>()+1;

	Context& context;
	Broker& broker;
	StaticStack<StateIndex, SM_DEPTH> current_state_indices;
	StateIndex initial_state;
	std::array<StaticStack<StateIndex, SM_DEPTH>,
	 history_size(root_state, NUM_STATES)> history;
	std::array<std::string_view, NUM_STATES> state_names;
	std::array<std::unique_ptr<State<Context, Broker>>, NUM_STATES> states;
	std::array<std::array<std::vector<NextState<SM_DEPTH>>, NUM_STATES>,
		NO_EVENT_VALUE> dispatch_map;
	std::size_t current_depth{}; 
	
	DeferQueue defer_queue;	
	std::size_t current_regions{};

	public:
		SM(Context& context_, Broker& broker_, OptionalArgs&... optional_args) :
		context(context_),
		broker(broker_),
		initial_state(1),
		history(),
		defer_queue()
	{
		static_assert(
			HasTransitionTable<decltype(root_state)>::value, "Root state has no make_transition_table method defined."
		);
		static_assert(
			mp::mp_size<Transitions>::value, "Transition table needs at least one transition"
		);
		static_assert(
			maxInitialStates(root_state), "Transition table needs at least one initial state"
		);
	
		//make a deduced type list of the optional arguments supplied to the state machine. 
		auto optional_dependency = std::make_tuple(std::ref(context_), std::ref(broker_), std::ref(optional_args)...);
		
		//create the state machine dispatch table. This is resolved at compile time.
		fillDispatchTable(optional_dependency);
		populateArrays();
		//fillInitialStateTable(root_state, this->initial_states);
		//fillInitialStateTable(root_state, this->history);
		initCurrentState();		
	}

	/**
	 * @brief Process an event and trigger a state machine transition (if applicable).
	 * If there are deferred events from previous transitions, process them now as well. 
	 * 
	 * @param event The event to be processed. The state machine recognizes 
	 * events by their value. 
	 * 
	 * @return The result of processing the event. Results from deferred events are ignored. 
	 */
	SMResult processEvent(EventEnum event){
		
		SMResult result = processEventInternal(static_cast<JEvent>(event));

		processDeferredEvents();
		
		return result;
	}

	void setDependency(OptionalArgs&... optional_args){
		auto optional_dependency = std::make_tuple(std::ref(optional_args)...);
		fillDispatchTable(optional_dependency);
	}

	template <class State> bool is(State) {
		static_assert(mp::mp_similar<State, houdini::sm::TState<State>>::value, "Type passed to `is` must be a template of houdini::state.");
		using States = mp::mp_push_front<detail::TypeList<Root>, State>;
		std::size_t index = mp::mp_find<StateMap, States>::value;

		return currentState() == index;
	}

	template <class State, class... ParentStates> bool is(State, ParentStates... parent_states){
		static_assert(mp::mp_similar<State, houdini::sm::TState<State>>::value, "Type passed to `is` must be a template of houdini::state.");
		auto parents = detail::makeTypeList(parent_states...);
		using States = mp::mp_push_back<mp::mp_push_front<decltype(parents), State>,Root>;
		std::size_t index = mp::mp_find<StateMap, States>::value;
		return currentState() == index;
	}
	
	std::string_view currentStateName(){
		return state_names[this->current_state_indices.back()];
	}

	StateIndex currentState(){
		return this->current_state_indices.back();
	}
	
	void update(){
		for (StateIndex state_index:this->current_state_indices){
			this->states[state_index]->updateImpl(this->context, this->broker);
		}
	}

	private:
		void populateArrays(){
			using States = mp::mp_transform<mp::mp_front, StateMap>;
			for_each_index_mp<States>(
				[this](auto state, std::size_t index){
					//all state types are wrapped in TState<>
					using UnderlyingState = typename decltype(state)::type;
					//std::cout << "State, name: " << index << ", " << util::type_name<UnderlyingState>() << "\n";
					this->state_names.at(index) = util::type_name<UnderlyingState>();
					this->states.at(index) = std::unique_ptr<State<Context,Broker>>(new UnderlyingState);
				}
			);
		}

		/**
		 * @brief Determine if the event has a valid transition in the current state 
		 * and if so, update the state of the state machine. 
		 * 
		 * `processEventInternal` iterates through the regions, and determines if there
		 * are any results in the static sm table for the current active state and event. The list
		 * of results is returned as a `std::deque` of an `EventVariant` contains a `NextState<Event>`. 
		 * 
		 * @param event The event to be processed. 
		 */
		SMResult processEventInternal(JEvent&& event) {
			bool all_guards_failed = true;
			bool all_transitions_invalid = true;


			auto& results = getDispatchTableEntry(event);

			if(results.empty()) {
				return SMResult::NOTHING;
			}

			for (auto& result: results){

				if (result.defer){
					return SMResult::DEFERRED;
				}

				if (!result.transition->executeGuard(event)) {
					all_transitions_invalid = false;
					continue;
				}
				// std::cout << "Valid: " << result.valid << std::endl;
				// std::cout << "Size: " << result.destination_states.size() << std::endl;
				//std::cout << "Final destination: " << result.destination_states.back() << std::endl;

				all_transitions_invalid = false;
				all_guards_failed = false;
				updathoudiniAndExecuteCallbacks(event, result);
				
				break;
			}

			if (all_transitions_invalid){
				return SMResult::NOTHING;
			}
			if (all_guards_failed){
				return SMResult::FAILED;
			}
			
			processAnonymousTransitions();
			return SMResult::SUCCESS;
		}

		void updathoudiniAndExecuteCallbacks(JEvent event, const NextState<SM_DEPTH>& result){
			//std::cout << "Updating and executing callbacks." << std::endl;
			
			auto destination_stack = result.destination_states;
			
			if constexpr (has_history(root_state)){
				//std::cout << "Setting history" << std::endl;
				for (auto iter = this->current_state_indices.cbegin(); iter != this->current_state_indices.cend(); iter++){
					this->history[*iter].clear();
					for (auto iter2 = iter+1; iter2 != this->current_state_indices.cend(); iter2++){
						this->history[*iter].push_back(*iter2);
					}
				}
				
				if (result.history){
					StateIndex lowest_destination = destination_stack.back();
					for (StateIndex index:this->history[lowest_destination]){
						destination_stack.push_back(index);
					}
				}
			}
			StateIndex back_state = current_state_indices.back();
			auto back_dest_state_iter = destination_stack.crbegin();
			
			//TODO: make sure order of activation is correct
			std::ptrdiff_t stack_size_diff = current_state_indices.size() - destination_stack.size();
			if (stack_size_diff > 0){ 
				//current state is deeper in the hierarchy
				for (auto i = stack_size_diff; i > 0; --i){
					this->states[back_state]->onExitImpl(this->context, this->broker);
					current_state_indices.pop();
					back_state = current_state_indices.back();
				}

			} else if (stack_size_diff < 0){
				//destination state is deeper in the hierarchy
				//therefore move dest pointer to same level as current state before making comparison
				for (auto i = stack_size_diff; i < 0; ++i){
					back_dest_state_iter++;
				}
			}

			while (back_state != *back_dest_state_iter){
				this->states[back_state]->onExitImpl(this->context, this->broker);
				current_state_indices.pop(); 
				back_state = current_state_indices.back();
				back_dest_state_iter++;
			}
			result.transition->executeAction(event);

			while(back_dest_state_iter != destination_stack.crbegin()) { 
				//TODO: this currently fails if the state machine is supposed to transition to the same state. 
				back_dest_state_iter--;				
				back_state = *back_dest_state_iter;
				current_state_indices.push_back(back_state);
				this->states[back_state]->onEntryImpl(this->context, this->broker);
			}

		}

		/** @brief Process anonymous transitions (transitions without a triggering event)
		 *	These transitions will automatically occur when the state machine enters a state with 
		 *  one of these transitions assigned. 
		 */

		void processAnonymousTransitions(){
			if constexpr (has_anonymous_transition(root_state)){
				while (true){
					bool all_guards_failed = true;

					JEvent event = NO_EVENT_VALUE;

					auto& results = getDispatchTableEntry(event);

					if (results.empty()){
						return;
					}

					for (auto& result: results){
						if (!result.transition->executeGuard(event)){
							continue;
						}

						updathoudiniAndExecuteCallbacks(event, result);
						all_guards_failed = false;
						break;
					}
					
					if (all_guards_failed){
						return;
					}
				}
			}
		}

		void processDeferredEvents() {
			if constexpr (HasDeferredEvents<decltype(root_state)>::value){
				if (!this->defer_queue.empty()){
					this->defer_queue.visit([this](auto event){this->processEventInternal(event);});
				}
			}
		}

		/**
		 * @brief Retrieve transition information, given the current active state
		 * and the event code.  This transition information contains all the information
		 * needed to transition to the next state.
		 */
		constexpr auto getDispatchTableEntry(JEvent event) -> decltype(auto) {
			//std::cout <<  "Current state and event: " << this->currentStateName() << "(" << current_state_indices.back() << ")" << ", " << event << std::endl;
			return this->dispatch_map[event][this->current_state_indices.back()];
		}

		constexpr void initCurrentState(){

			this->current_state_indices.push_back(0);
			this->current_state_indices.push_back(this->initial_state);
			if constexpr (has_history(root_state)){
			//TODO: need to fill history with appropriate initial states.

			}
		}

		template <class OptionalDependency>
		void fillDispatchTable(OptionalDependency& optional_dependency){
			fillDispatchTableWithInternalTransitions(
				*this, optional_dependency);
			fillDispatchTableWithExternalTransitions(
				*this, optional_dependency);
			fillDispatchTableWithDeferredEvents(*this, this->dispatch_map, optional_dependency);
		}

};

} //namespace sm
} //namespace houdini