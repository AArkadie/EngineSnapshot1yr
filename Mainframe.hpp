#pragma once

#include "Assets.hpp"
#include "Audio.hpp"
//#include "CYOAMaker.hpp"
#include "DataTypes.hpp"
#include "HardwareAcceleration.hpp"
#include "Interactivity.hpp"
#include "Video.hpp"

//systems which need to know about and connect everything to each other
namespace SAGE {
	struct SystemsData {
		EventSystem* eventSystem;
		InputSystem* inputSystem;
		AudioSystem* audioSystem;
		VideoManager* videoSystem;
		struct EntitySystem* entitySystem;
	};



	enum ComponentType {
		TRANSFORM_COMPONENT,
		CONTROLLABILITY_COMPONENT,//AI and Input will use this...or...just input?
		RENDERABILITY_COMPONENT,
		VISION_COMPONENT,//will become relevant when other things need view/proj matricies *cough* shadows
		//SOUND_EMISSION_COMPONENT,
		COMPONENT_LIMIT,
		INVALID_COMPONENT = UINT32_MAX
	};

	struct Entity {
		Entity();
		//size_t EntityID; Does it even need to know?
		std::array<uint32_t, COMPONENT_LIMIT> components;//will every ID 0 be a system default?  Could that be useful?
	};

	struct TransformComponent {//orphaned here, goes in physics
		uint32_t parentEntityID;
		uint32_t myComponentID;

		TransformComponent();
		~TransformComponent() = default;

		glm::vec4 position;
		glm::quat orientation;
		glm::vec4 size;

		glm::vec4 velocity;//xyz direction w magnitude
		glm::vec4 acceleration;//same as above, should we prefix with "net" to be more clear?
		//glm::vec4 torque;//xyz axis w degree
		float mass;

		void rotate(float inRadians, glm::vec3);
		void translate(glm::vec3 direction) { position += glm::vec4(direction,0.f); }
		void scale(float x) { size *= x; }
		void scale(glm::vec3 x) { size *= glm::vec4(x,1.f); }
		void setSize(glm::vec3 s) { size = glm::vec4(s, 1.f); }
		void setPosition(glm::vec3 p) { position = glm::vec4(p, 1.f); }
	};

	struct TransformComponentManager {
		//PhysicsSystem* system;

		TransformComponentManager() = default;
		~TransformComponentManager();

		std::vector<uint32_t> freeID;
		std::vector<TransformComponent*> components;

		size_t makeComponent(uint32_t);
		void returnComponent(uint32_t);
	};

	struct ControllabilityComponentManager {
		InputSystem* system;

		ControllabilityComponentManager(InputSystem*);
		~ControllabilityComponentManager();

		std::vector<uint32_t> freeID;
		std::vector<ControllabilityComponent*> components;

		size_t makeComponent(uint32_t);
		void returnComponent(uint32_t);
	};

	struct RenderabilityComponentManager {
		VideoManager* system;


		RenderabilityComponentManager(VideoManager*);
		~RenderabilityComponentManager();

		std::vector<uint32_t> freeID;
		std::vector<RenderabilityComponent*> components;


		size_t makeComponent(uint32_t);
		void returnComponent(uint32_t);
	};

	struct VisionComponentManager {
		VisionComponentManager() = default;
		~VisionComponentManager();

		std::vector<uint32_t> freeID;
		std::vector<VisionComponent*> components;

		size_t makeComponent(uint32_t);
		void returnComponent(uint32_t);
	};

	struct EntitySystem {
		EntitySystem(SystemsData*);
		~EntitySystem();

		SystemsData* systems;
		std::vector<Entity> entities;
		std::vector<uint32_t> freeID;

		TransformComponentManager* transformComponentManager;
		ControllabilityComponentManager* controllabilityComponentManager;
		RenderabilityComponentManager* renderComponentManager;
		VisionComponentManager* visionComponentManager;

		size_t makeEntity();
		void addComponent(ComponentType, uint32_t&);
		void removeComponent(ComponentType, uint32_t&);
		void returnEntity(uint32_t&);

		TransformComponent* getTransformComponent(uint32_t&);
		ControllabilityComponent* getControllabilityComponent(uint32_t&);
		RenderabilityComponent* getRenderabilityComponent(uint32_t&);
		VisionComponent* getVisionComponent(uint32_t&);
	};

	struct SystemsManager {
		SystemsManager();
		~SystemsManager();

		SystemsData data;

		VideoManager* getVideoSystem() { return data.videoSystem; }
		Accelerator* getAccelerationSystem() { return data.videoSystem->exposeAccelerator(); }
		EntitySystem* getEntitySystem() { return data.entitySystem; }
	};
}