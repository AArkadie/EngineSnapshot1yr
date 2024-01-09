#include "Interactivity.hpp"

namespace SAGE {
	EventSystem::EventSystem() {
		timeAtUpdateRun = std::chrono::steady_clock::now();
		lastRecordedDeltaTime = 0.0;
	}
	void EventSystem::processEvents() {
		std::chrono::time_point<std::chrono::steady_clock> clock = std::chrono::steady_clock::now();
		lastRecordedDeltaTime = std::chrono::duration<double, std::chrono::seconds::period>(clock - timeAtUpdateRun).count();
		timeAtUpdateRun = clock;
		/*
		SDL_PumpEvents();
		int numEvents = SDL_PeepEvents(pending, 256, SDL_PEEKEVENT, SDL_QUIT, SDL_APP_TERMINATING);

		if (SDL_HasEvent(SDL_QUIT)) {
			//this system should implicitly know which system handles quitting and call on it to handle that
			std::cout << "What a twist!";
			return;
		}
		*/

		while (SDL_PollEvent(&pending)) {

			switch (pending.type) {
			case SDL_QUIT:
				//something something test later
				break;
			case SDL_WINDOWEVENT:
				for (auto f : windowEventSubscribers) f(pending.window);
				break;
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				for (auto f : keyboardEventSubscribers) f(pending.key);
				break;
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				for (auto f : mouseButtonEventSubscribers) f(pending.button);
				break;
			case SDL_MOUSEMOTION:
				for (auto f : mouseMovementEventSubscribers) f(pending.motion);
				break;
			case SDL_MOUSEWHEEL:
				for (auto f : mouseScrollEventSubscribers) f(pending.wheel);
				break;
			}

		}

	}
	void EventSystem::subscribe(std::function<void(SDL_WindowEvent)> c) {
		windowEventSubscribers.push_back(c);
	}
	void EventSystem::subscribe(std::function<void(SDL_KeyboardEvent)> c) {
		keyboardEventSubscribers.push_back(c);
	}
	void EventSystem::subscribe(std::function<void(SDL_MouseMotionEvent)> c) {
		mouseMovementEventSubscribers.push_back(c);
	}
	void EventSystem::subscribe(std::function<void(SDL_MouseWheelEvent)> c) {
		mouseScrollEventSubscribers.push_back(c);
	}
	void EventSystem::subscribe(std::function<void(SDL_MouseButtonEvent)> c) {
		mouseButtonEventSubscribers.push_back(c);
	}
	void EventSystem::unsubscribe(std::function<void(SDL_WindowEvent)> c) {
		for (size_t i = 0; i < windowEventSubscribers.size(); i++) {
			if (windowEventSubscribers[i].target_type().hash_code() == c.target_type().hash_code()) {
				windowEventSubscribers.erase(windowEventSubscribers.begin() + i);
				return;
			}
		}
	}
	void EventSystem::unsubscribe(std::function<void(SDL_KeyboardEvent)> c) {
		for (size_t i = 0; i < keyboardEventSubscribers.size(); i++) {
			if (keyboardEventSubscribers[i].target_type().hash_code() == c.target_type().hash_code()) {
				keyboardEventSubscribers.erase(keyboardEventSubscribers.begin() + i);
				return;
			}
		}
	}
	void EventSystem::unsubscribe(std::function<void(SDL_MouseMotionEvent)> c) {
		for (size_t i = 0; i < mouseMovementEventSubscribers.size(); i++) {
			if (mouseMovementEventSubscribers[i].target_type().hash_code() == c.target_type().hash_code()) {
				mouseMovementEventSubscribers.erase(mouseMovementEventSubscribers.begin() + i);
				return;
			}
		}
	}
	void EventSystem::unsubscribe(std::function<void(SDL_MouseWheelEvent)> c) {
		for (size_t i = 0; i < mouseScrollEventSubscribers.size(); i++) {
			if (mouseScrollEventSubscribers[i].target_type().hash_code() == c.target_type().hash_code()) {
				mouseScrollEventSubscribers.erase(mouseScrollEventSubscribers.begin() + i);
				return;
			}
		}
	}
	void EventSystem::unsubscribe(std::function<void(SDL_MouseButtonEvent)> c) {
		for (size_t i = 0; i < mouseButtonEventSubscribers.size(); i++) {
			if (mouseButtonEventSubscribers[i].target_type().hash_code() == c.target_type().hash_code()) {
				mouseButtonEventSubscribers.erase(mouseButtonEventSubscribers.begin() + i);
				return;
			}
		}
	}

	InputState::InputState() {
		for (auto& b : keyStates) {
			b = false;
		}
	}

	KeyMap::KeyMap() {
		for (size_t i = UNKNOWN_KEY; i < MAX_KEYS; i++) {
			aliases[i] = (SAGEKeyLocation)i;
		}
	}

	static void doNothing(InputSystem*) {}

	ControllabilityComponent::ControllabilityComponent(InputSystem* is) {
		poller = is;
		customUpdate = &doNothing;
	}

	InputSystem::InputSystem(EventSystem* e) {
			eventHandler = e;

			keyboardListner = std::bind(&InputSystem::recordKeyboardEvent, this, std::placeholders::_1);
			eventHandler->subscribe(keyboardListner);
	}
}