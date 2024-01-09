#pragma once

#include "Assets.hpp"

namespace SAGE {
	struct EventSystem {
		//TODO: Custom event handling
		std::chrono::time_point<std::chrono::steady_clock> timeAtUpdateRun;
		double lastRecordedDeltaTime;
		//TODO: Priority System?
		EventSystem();
		~EventSystem() = default;

		SDL_Event pending;

		std::vector<std::function<void(SDL_WindowEvent)>> windowEventSubscribers;
		std::vector<std::function<void(SDL_KeyboardEvent)>> keyboardEventSubscribers;
		std::vector<std::function<void(SDL_MouseMotionEvent)>> mouseMovementEventSubscribers;
		std::vector<std::function<void(SDL_MouseWheelEvent)>> mouseScrollEventSubscribers;
		std::vector<std::function<void(SDL_MouseButtonEvent)>> mouseButtonEventSubscribers;

		void processEvents();
		double getDeltaTime() { return lastRecordedDeltaTime; }

		void subscribe(std::function<void(SDL_WindowEvent)>);
		void subscribe(std::function<void(SDL_KeyboardEvent)>);
		void subscribe(std::function<void(SDL_MouseMotionEvent)>);
		void subscribe(std::function<void(SDL_MouseWheelEvent)>);
		void subscribe(std::function<void(SDL_MouseButtonEvent)>);

		void unsubscribe(std::function<void(SDL_WindowEvent)>);
		void unsubscribe(std::function<void(SDL_KeyboardEvent)>);
		void unsubscribe(std::function<void(SDL_MouseMotionEvent)>);
		void unsubscribe(std::function<void(SDL_MouseWheelEvent)>);
		void unsubscribe(std::function<void(SDL_MouseButtonEvent)>);
	};

	class InputState {

		InputState();
		~InputState() = default;

		std::array<bool, MAX_KEYS>keyStates;
		//will we have separate arrays for mouse and joystick buttons?
		//Def need another one for actual numerical values like mouse location or joystick values
		//we want to be able to track multiple controllers, maybe multiple keyboards and mice, we don't know the user's setup.
		//but we want to be able to handle any input and translate those signals into integers usable by functions triggered by events.

		//the input system is our interface to the outside world
		//the ultimate dream would be able to read any and any number of input device and let the user decide what the keys mean
		//without having predefined values like JOYSTICK_AXIS_X.  In this way, we could make maps out of anything
		//without knowing anything about the kinds of peripherals that were actually out there.  Thus allowing players
		//to customize any game with any wacky controller scheme they wanted, even ones they make themselves.
		friend class InputSystem;
	};

	struct KeyMap {
		KeyMap();
		~KeyMap() = default;
		std::array<SAGEKeyLocation, MAX_KEYS> aliases;
		//Doesn't solve naming things methinks.
		//unless the user makes a custom enum for keymap indexing to translate to key aliases?
		//maybe just see how other people do it idk.
	};

	struct ControllabilityComponent {
		uint32_t parentEntityID;
		uint32_t myComponentID;

		ControllabilityComponent(InputSystem*);
		~ControllabilityComponent() = default;

		InputSystem* poller;

		std::function<void(InputSystem*)>customUpdate;
		std::function<void(SDL_UserEvent)> aiController;//not sure, maybe goes in a separate AI component

		void setUpdateFunction(std::function<void(InputSystem*)> func) {	customUpdate = func; }
		void update() {	customUpdate(poller); }
	};

	class InputSystem {
		InputSystem(EventSystem*);
		~InputSystem() = default;

		InputState state;
		EventSystem* eventHandler;

		std::function<void(SDL_KeyboardEvent)> keyboardListner;
		std::function<void(SDL_MouseMotionEvent)> mouseListner;
		std::function<void(SDL_MouseButtonEvent)> mouseButtonListner;

		void recordKeyboardEvent(SDL_KeyboardEvent evt) { state.keyStates[evt.keysym.scancode] = evt.type == SDL_KEYDOWN; }
		void recordMouseMotion(SDL_MouseMotionEvent);
		void recordMouseButtonEvent(SDL_MouseButtonEvent);

		friend class SystemsManager;
	public:
		bool isPressed(SAGEKeyLocation k) { return state.keyStates[k]; }
		// getIntegerValue?
		// getFloatValue?
		// or do we categorize by input type?
	};
}