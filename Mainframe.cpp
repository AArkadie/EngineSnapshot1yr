#include "Mainframe.hpp"

namespace SAGE {



	Entity::Entity() {
		for (auto& c : components) c = INVALID_COMPONENT;
	}

	TransformComponent::TransformComponent() {
		position = glm::vec4(0.f, 0.f, 0.f, 1.f);
		orientation = glm::quat(1.f, 0.f, 0.f, 0.f);
		size = glm::vec4(1.f, 1.f, 1.f, 1.f);
		velocity = acceleration = glm::vec4(0.f);
		mass = 0.f;
	}
	void TransformComponent::rotate(float degree, glm::vec3 axis) {
		orientation = glm::rotate(orientation, degree, axis);
	}

	TransformComponentManager::~TransformComponentManager() {
		for (auto c : components) {
			if (c) delete c;
		}
	}
	size_t TransformComponentManager::makeComponent(uint32_t entity) {
		size_t returndex = components.size();
		if (freeID.size() > 0) {
			returndex = freeID.back();
			freeID.pop_back();
		}
		else components.push_back(nullptr);
		components[returndex] = new TransformComponent();
		components[returndex]->parentEntityID = entity;
		components[returndex]->myComponentID = returndex;
		return returndex;
	}
	void TransformComponentManager::returnComponent(uint32_t c) {
		if (c == INVALID_COMPONENT) return;
		if (components[c]) {
			delete components[c];
			components[c] = nullptr;
			freeID.push_back(c);
		}
	}

	ControllabilityComponentManager::ControllabilityComponentManager(InputSystem* sys) {
		system = sys;
	}
	ControllabilityComponentManager::~ControllabilityComponentManager() {
		for (auto c : components) {
			if (c) delete c;
		}
	}
	size_t ControllabilityComponentManager::makeComponent(uint32_t entity) {
		size_t returndex = components.size();
		if (freeID.size() > 0) {
			returndex = freeID.back();
			freeID.pop_back();
		}
		else components.push_back(nullptr);
		components[returndex] = new ControllabilityComponent(system);
		components[returndex]->parentEntityID = entity;
		components[returndex]->myComponentID = returndex;
		return returndex;
	}
	void ControllabilityComponentManager::returnComponent(uint32_t c) {
		if (c == INVALID_COMPONENT) return;
		if (components[c]) {
			delete components[c];
			components[c] = nullptr;
			freeID.push_back(c);
		}
	}

	RenderabilityComponentManager::RenderabilityComponentManager(VideoManager* vids) {
		system = vids;
	}
	RenderabilityComponentManager::~RenderabilityComponentManager() {
		for (auto c : components) {
			if (c) delete c;
		}
	}
	size_t RenderabilityComponentManager::makeComponent(uint32_t entity) {
		size_t returndex = components.size();
		if (freeID.size() > 0) {
			returndex = freeID.back();
			freeID.pop_back();
		}
		else components.push_back(nullptr);
		components[returndex] = new RenderabilityComponent(system->getSettings());
		components[returndex]->parentEntityID = entity;
		components[returndex]->myComponentID = returndex;
		return returndex;
	}
	void RenderabilityComponentManager::returnComponent(uint32_t c) {
		if (c == INVALID_COMPONENT) return;
		if (components[c]) {
			delete components[c];
			components[c] = nullptr;
			freeID.push_back(c);
		}
	}

	//VisionComponentManager::VisionComponentManager(){}
	VisionComponentManager::~VisionComponentManager() {
		for (auto c : components) {
			if (c) delete c;
		}
	}
	size_t VisionComponentManager::makeComponent(uint32_t entity) {
		size_t returndex = components.size();
		if (freeID.size() > 0) {
			returndex = freeID.back();
			freeID.pop_back();
		}
		else components.push_back(nullptr);
		components[returndex] = new VisionComponent();
		components[returndex]->parentEntityID = entity;
		components[returndex]->myComponentID = returndex;
		return returndex;
	}
	void VisionComponentManager::returnComponent(uint32_t c) {
		if (c == INVALID_COMPONENT) return;
		if (components[c]) {
			delete components[c];
			components[c] = nullptr;
			freeID.push_back(c);
		}
	}

	EntitySystem::EntitySystem(SystemsData* sys) {
		systems = sys;

		transformComponentManager = new TransformComponentManager();
		controllabilityComponentManager = new ControllabilityComponentManager(sys->inputSystem);
		renderComponentManager = new RenderabilityComponentManager(systems->videoSystem);
		visionComponentManager = new VisionComponentManager();
	}
	EntitySystem::~EntitySystem() {
		//clean up all entities here
		delete visionComponentManager;
		delete renderComponentManager;
		delete controllabilityComponentManager;
		delete transformComponentManager;
	}
	size_t EntitySystem::makeEntity() {
			size_t returndex;
			if (freeID.size() != 0) {
				returndex = freeID.back();
				freeID.pop_back();
				return returndex;
			}
			returndex = entities.size();
			entities.push_back({});
			return returndex;
	}
	void EntitySystem::addComponent(ComponentType type, uint32_t& entity) {
			if (type >= COMPONENT_LIMIT) return;
			switch (type)
			{
			case TRANSFORM_COMPONENT:
				entities[entity].components[type] = transformComponentManager->makeComponent(entity);
				break;
			case CONTROLLABILITY_COMPONENT:
				entities[entity].components[type] = controllabilityComponentManager->makeComponent(entity);
				break;
			case RENDERABILITY_COMPONENT:
				entities[entity].components[type] = renderComponentManager->makeComponent(entity);
				break;
			case VISION_COMPONENT:
				entities[entity].components[type] = visionComponentManager->makeComponent(entity);
				break;
			default:
				std::cout << "\naddComponent Problem\n";
				break;
			}
	}
	void EntitySystem::removeComponent(ComponentType type, uint32_t& entity) {
			if (type >= COMPONENT_LIMIT) return;
			switch (type)
			{
			case TRANSFORM_COMPONENT:
				transformComponentManager->returnComponent(entities[entity].components[type]);
				entities[entity].components[type] = INVALID_COMPONENT;
				break;
			case CONTROLLABILITY_COMPONENT:
				controllabilityComponentManager->returnComponent(entities[entity].components[type]);
				entities[entity].components[type] = INVALID_COMPONENT;
				break;
			case RENDERABILITY_COMPONENT:
				renderComponentManager->returnComponent(entities[entity].components[type]);
				entities[entity].components[type] = INVALID_COMPONENT;
				break;
			case VISION_COMPONENT:
				visionComponentManager->returnComponent(entities[entity].components[type]);
				entities[entity].components[type] = INVALID_COMPONENT;
				break;
			default:
				std::cout << "\nremoveComponent Problem\n";
				break;
			}
	}
	void EntitySystem::returnEntity(uint32_t& entity) {
			if (entity >= entities.size()) return;
			for (size_t i = 0; i < COMPONENT_LIMIT; i++) {
				removeComponent((ComponentType)i, entity);
			}
			freeID.push_back(entity);
	}
	TransformComponent* EntitySystem::getTransformComponent(uint32_t& entity) {
		uint32_t compdex = entities[entity].components[TRANSFORM_COMPONENT];
		if (compdex == INVALID_COMPONENT) return nullptr;
		return transformComponentManager->components[compdex];
	}
	ControllabilityComponent* EntitySystem::getControllabilityComponent(uint32_t& entity) {
		uint32_t compdex = entities[entity].components[CONTROLLABILITY_COMPONENT];
		if (compdex == INVALID_COMPONENT) return nullptr;
		return controllabilityComponentManager->components[compdex];
	}
	RenderabilityComponent* EntitySystem::getRenderabilityComponent(uint32_t& entity) {
		uint32_t compdex = entities[entity].components[RENDERABILITY_COMPONENT];
		if (compdex == INVALID_COMPONENT) return nullptr;
		return renderComponentManager->components[compdex];
	}
	VisionComponent* EntitySystem::getVisionComponent(uint32_t& entity) {
		uint32_t compdex = entities[entity].components[VISION_COMPONENT];
		if (compdex == INVALID_COMPONENT) return nullptr;
		return visionComponentManager->components[compdex];
	}

	SystemsManager::SystemsManager() {
		//surveyHardware(); toggle in main for now
		loadHardwareInfo();
		data.eventSystem = new EventSystem();
		data.inputSystem = new InputSystem(data.eventSystem);
		data.audioSystem = new AudioSystem();
		data.videoSystem = new VideoManager();
		data.entitySystem = new EntitySystem(&data);
		data.videoSystem->getSettings()->otherSystems = this;
	}
	SystemsManager::~SystemsManager() {
		delete data.entitySystem;
		delete data.videoSystem;
		delete data.audioSystem;
		delete data.inputSystem;
		delete data.eventSystem;
	}

}