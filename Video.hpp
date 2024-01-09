#pragma once

#include "Assets.hpp"
#include "HardwareAcceleration.hpp"

namespace SAGE {
	struct VideoSettings {
		std::string windowName;
		int windowWidth = 1920, windowHeight = 1080;
		SDL_Window* mainWindow;
		Accelerator* videoCard;
		uint32_t numFramesToFly;

		VkSampleCountFlagBits chosenSampleCount;
		VulkanArray* defaultQuadIndices;

		AssetManager* assetSystem;
		struct SystemsManager* otherSystems;
		class TextureLibrary* texLib;
		class CubemapLibrary* cbmLib;
		class MaterialLibrary* matLib;
		class MeshLibrary* meshLib;
	};
	//0: Renderer Handles, 1: Render Group Handles, 2: Object instance group handles, 3: Object handles
	enum DescriptorBindFrequency {//the set in the shader MUST reflect this ordering
		GLOBAL, PER_FRAME, PER_DRAW, PER_OBJECT
	};

	VkExtent2D getWindowVkExtent(VideoSettings*);

	//we need an idea of an arbitrary rectangle with an arbitrary texture that we can just...use.
	//this is the backbone of 2D rendering, which will be no different from menUI.
	//so text in 2D will be distinct from 3D text, and by extension 2D UI vs 3D UI.

	struct Texture : Asset{
		Texture(VideoSettings*, TextureFileData&, Asset* = nullptr);
		Texture(VideoSettings*, uint32_t width, uint32_t height);
		~Texture();

		VulkanTensor* getImage(SAGETextureMapType m = DIFFUSE_TEXTURE_MAP) { return images[m]; }
		VkSampler& getSampler(SAGETextureMapType m = DIFFUSE_TEXTURE_MAP) { return samplers[m]; }

	private:
		VideoSettings* settings;
		std::array<VulkanTensor*, MAX_TEXTURE_MAPS> images;
		std::array<VkSampler, MAX_TEXTURE_MAPS> samplers;
		bool isPBR;
	};

	class TextureLibrary {
		friend struct VideoManager;

		TextureLibrary(VideoSettings*);
		~TextureLibrary();

		VideoSettings* settings;

		std::unordered_map<uint32_t, Texture*> textures;

		Texture* defaultTexture;
	public:
		Texture* getTexture(uint32_t);
		void returnTexture(Texture*);
	};

	struct Cubemap : Asset {
		Cubemap(VideoSettings*, Image&, Asset* = nullptr);
		~Cubemap();

		VulkanTensor* getMap() { return map; }
		VkSampler& getSampler() { return mapSampler; }
	private:
		VideoSettings* settings;
		VulkanTensor* map;
		VkSampler mapSampler;
	};

	class CubemapLibrary {
		friend struct VideoManager;

		CubemapLibrary(VideoSettings*);
		~CubemapLibrary();

		VideoSettings* settings;

		std::unordered_map<uint32_t, Cubemap*> maps;

		Cubemap* defaultCubemap;
	public:
		Cubemap* getCubemap(uint32_t);
		void returnCubemap(Cubemap*);
	};
	//think about meshes knowing their vertex attributes.  Idk just an idea.
	struct Mesh : Asset{
		Mesh(VideoSettings*, MeshFileData&, Asset* = nullptr);
		Mesh(VideoSettings*, uint32_t pixW, uint32_t pixH);
		~Mesh();

		VideoSettings* settings;
		VulkanArray* vertices, * indices;
		uint32_t numVertices, numIndices;
		SAGEVertexType vertexType;
	};

	class MeshLibrary {
		friend struct VideoManager;

		MeshLibrary(VideoSettings*);
		~MeshLibrary();

		VideoSettings* settings;

		Mesh* defaultMesh;

		std::unordered_map<uint32_t, Mesh*> meshes;
	public:
		Mesh* getMesh(uint32_t);
		void returnMesh(Mesh*);
	};

	struct Material : Asset{
		Material(VideoSettings*, MaterialFileData&, Asset* = nullptr);
		Material() = default;
		~Material();
		AccelerationProgram* shader;
		std::vector<SAGETextureMapType> texMaps;
		VkPipeline activePipeline;
		std::vector<VkPipeline> pipelines;
		void setActivePipeline(size_t i) { if (i < pipelines.size()) activePipeline = pipelines[i]; }
	private:
		VideoSettings* settings;
	};

	class MaterialLibrary{
		friend struct VideoManager;

		MaterialLibrary(VideoSettings*);
		~MaterialLibrary();

		VideoSettings* settings;
		VulkanPipelineLibrary* configurator;
		Material* defaultMaterial;
		std::unordered_map<uint32_t, Material*> materials;
	public:
		Material* getMaterial(uint32_t);
		void returnMaterial(Material*);
	};

	//will evolve this don't forget please I beg you think of the children!!!
	struct VisionComponent {
		uint32_t parentEntityID, myComponentID;
		CameraData data;
	};//will make the renderer take this instead of Camera at a later date.

	struct Camera {//I'm starting to think this is too advanced to be here, may belong in something like GameObjects
		Camera(VideoSettings*);
		~Camera();

		//all will have the ability to interpolate at some point
		//unless we fall back to pure transform component manip.
		//then it all happens with whatever handles that.
		
		glm::vec3 getRight();   // RIGHT   = <1,0,0> rotated by quat
		glm::vec3 getUp();      // UP      = <0,1,0> rotated by quat
		glm::vec3 getForward(); // FORWARD = <0,0,-1> rotated by quat

		void moveUp();
		void moveDown();
		void moveLeft();
		void moveRight();
		void moveForward();
		void moveBackward();
		void panUp();
		void panDown();
		void panLeft();
		void panRight();
		void rollRight();
		void rollLeft();
		
		void zoomIn();
		void zoomOut();
		void changeFarCutoff(float);
		void changeNearCutoff(float);
		void changeFrameWidth(float);
		void changeFrameHeight(float);

		void followMouse(float, float);

		void camCtrl(class InputSystem*);

		void updateData();
		CameraData& getData() { return viewData->data; }

		VideoSettings* settings;
		uint32_t entityID;
		float moveSpeed, rotSpeed, zoomSpeed;
		float fov;
		float mouseX, mouseY;
		struct TransformComponent* transforms;
		//scale: x = increase aspect ratio num; y, denom; z, near plane distance; w, far
		struct ControllabilityComponent* control;
		VisionComponent* viewData;
	};

	struct Light {
		Light(VideoSettings*);
		~Light();

		VideoSettings* settings;
		uint32_t entityID;

		LightData data;
	};

	struct RenderFrameData {
		uint32_t frameNumber;
		VkSemaphore isImageAvailable, isRenderingFinished;
		VkFence isFrameInFlight;
		APSceneData sceneData;
		Camera* mainCamera;
	};

	struct RenderabilityComponent {
//we'll change this up when we get a default material to be able to use defaults.
//May also want to change to support instanced drawing
		RenderabilityComponent(VideoSettings*);
		~RenderabilityComponent();

		void draw(VkCommandBuffer&);

		void updateModel();
		PerObjectRenderData& getData() { return renderData; }

		//strict setters for now, need a protocol for removing from render batches
		void changeMaterial(uint32_t);
		void changeMesh(uint32_t);
		void changeTexture(uint32_t);
		void setMaterial(Material* m) { material = m; }
		void setMesh(Mesh* m) { mesh = m; }
		void setTexture(Texture* t) { texture = t; }

	private:
		uint32_t parentEntityID;
		uint32_t myComponentID;	

		VideoSettings* settings;
		Mesh* mesh;
		Material* material;
		Texture* texture;
		PerObjectRenderData renderData;
		//will we do instancing support here?  Shaders have to be able to support instancing
		//with vertex binding attribute so...?

		friend struct RenderabilityComponentManager;
		friend struct RenderBatch;
	};

	struct RenderBatch {
		RenderBatch(VideoSettings*, RenderabilityComponent*);
		~RenderBatch() = default;

		void add(RenderabilityComponent*);//do we add instancing support somewhere here?
		void remove(RenderabilityComponent*);

		void render(VkCommandBuffer&, RenderFrameData&);

	private:
		VideoSettings* settings;

		std::vector<RenderabilityComponent*> memberList;
		std::vector<size_t> memberTextureDecriptors;
		std::vector<VkBuffer*> memberVertices;//to be used for batch draw?
		std::vector<VkDeviceSize> memberVertexListOffsets;//not used yet.

		std::vector<size_t> frameRelevantDescriptors;
		size_t objectListDescriptor;

		Material* commonMaterial;
		DescriptorSystem* descriptorHandler;
	};

	struct Swapchain {
	
		Swapchain(VideoSettings*, Swapchain* = nullptr);

		~Swapchain();

		VkSwapchainKHR& getVkSwapchain() { return swapchain; }
		int getWidth() { return width; }
		int getHeight() { return height; }
		VkExtent2D dimensions() { return VkExtent2D{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) }; }
		VkRenderingAttachmentInfo& getImageAttachmentInfo(size_t i) { return imageAttachmentDescriptions[i]; }
		VkRenderingAttachmentInfo& getDepthAttachmentInfo() { return depthAttachmentInfo; }

		void prepareImage(size_t, bool, VkCommandBuffer&);
	private:
		VideoSettings* settings;
		VkSwapchainKHR swapchain;
		std::vector<VkImage> images;
		std::vector<VkImageView> imageViews;
		std::vector<VkRenderingAttachmentInfo> imageAttachmentDescriptions;
		VkRenderingAttachmentInfo depthAttachmentInfo;
		std::vector<VulkanTensor*> renderingResources;
		//these now go in some custom struct that we store instead...probly.
		//  Will need to explicitely transition image resources.
		int width, height;
	};

	struct RenderSystem {
	
		RenderSystem(VideoSettings*, Camera*);
		~RenderSystem();

		void rebuildSwapchain();

		void render(std::vector<RenderBatch>&);

		void handleWindowEvent(SDL_WindowEvent e);

		bool isWindowResized() { return windowResize; }

		void changeSkybox(uint32_t);

	private:
		VideoSettings* settings;
		Swapchain* presentingSwapchain;
		Camera* mainCamera;
		std::vector<RenderFrameData> frameData;
		uint32_t renderingFrame;
		VkResult trackedResult;

		bool windowResize, suspended;
		std::function<void(SDL_WindowEvent)> windowCallbackForEventSystem;

		Cubemap* skybox;
		Material* skyboxMat;
		std::vector<size_t> skyboxCamDescriptors;
		size_t skyboxDescriptor;
		DescriptorSystem* ds;

		Light* pointThatsProbablyAHackDontSayIDidntWarnYou;
		RenderBatch* lightingBatchNoteThisIsAHackDoNotKeep;
	};

	struct VideoManager {

		VideoManager();
		VideoManager(const VideoManager&) = delete;
		VideoManager& operator=(const VideoManager&) = delete;
		~VideoManager();

		VideoSettings* getSettings() { return &vs; }

		Accelerator* exposeAccelerator() { return vulkanImplementation; }//this may just be a hack
		//SDL_Window* exposeWindow() { return vs.mainWindow; }
	private:
		Accelerator* vulkanImplementation;
		VideoSettings vs;
	};
}

