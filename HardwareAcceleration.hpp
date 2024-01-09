//if ever we're to bring back render passes for mobile rendering
//the render pass and framebuffers belong together.
//render pass is the resource descriptor, framebuffer is the resource holder.
#pragma once

#include <vulkan/vulkan.h>
#include <SDL_vulkan.h>
#include <spirv_reflect.h>

#include "Assets.hpp"

namespace {
	std::vector<const char*> desiredInstanceLayers, desiredInstanceExtensions, desiredDeviceExtensions;
	const size_t DYNAMIC_BUFFER_LIMIT = 10000;//actual max limit depends on the VRAM, could be hundreds of millions...but be conservative
}

	static void requireVulkanLayers(std::vector<const char*> layers) {
		if (desiredInstanceLayers.size() == 0) desiredInstanceLayers = layers;
		else {
			bool copy;
			for (auto& l : layers) {
				copy = false;
				for (auto& dil : desiredInstanceLayers) {
					if (l == dil) copy = true;
				}
				if (!copy) desiredInstanceLayers.push_back(l);
			}
		}
	}
	static void requireVulkanInstanceExtensions(std::vector<const char*> extensions) {
		if (desiredInstanceExtensions.size() == 0) desiredInstanceExtensions = extensions;
		else {
			bool copy;
			for (auto& e : extensions) {
				copy = false;
				for (auto& die : desiredInstanceExtensions) {
					if (e == die) copy = true;
				}
				if (!copy) desiredInstanceExtensions.push_back(e);
			}
		}
	}
	static void requireVulkanDeviceExtensions(std::vector<const char*> extensions) {
		if (desiredDeviceExtensions.size() == 0) desiredDeviceExtensions = extensions;
		else {
			bool copy;
			for (auto& e : extensions) {
				copy = false;
				for (auto& dde : desiredDeviceExtensions) {
					if (e == dde) copy = true;
				}
				if (!copy) desiredDeviceExtensions.push_back(e);
			}
		}
	}

static	VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT msgSeverity,
		VkDebugUtilsMessageTypeFlagsEXT msgTypes,
		const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
		void* userData) {
		switch (msgSeverity)
		{
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
			std::cout << "\nCUSTOM Verbose: " << callbackData->pMessage << '\n';
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			std::cout << "\nCUSTOM Info: " << callbackData->pMessage << '\n';
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			std::cout << "\nCUSTOM Warning: " << callbackData->pMessage << '\n';
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			std::cout << "\nCUSTOM Error: " << callbackData->pMessage << '\n';
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT:
			std::cout << "\nlolwat: " << callbackData->pMessage << '\n';
			break;
		default:
			std::cout << "\nnot even god can help you now.\n";
			break;
		}
		return VK_FALSE;
	}

namespace SAGE {
	struct VulkanState {
		VkInstance vulkanInstance;

		VkPhysicalDevice accelerationDevice;

		VkDevice vulkanDevice;

		VkCommandBufferBeginInfo stdBegin, singleUseBegin;

		VkSurfaceKHR vulkanSurface;
		VkSurfaceFormatKHR chosenSurfaceFormat;
		VkSurfaceCapabilitiesKHR vulkanSurfaceCapabilities;
		VkPresentModeKHR chosenPresentMode;

		AssetManager* assetSystem;
	};

	//std::default_random_engine rndEngine((unsigned)time(nullptr));//compute
	//std::uniform_real_distribution<float> rndDist(0.f, 1.f);//stuff

	struct VulkanTensor {
	
		VulkanTensor(VulkanState*
			, VkFormat
			, VkImageTiling
			, VkSampleCountFlagBits
			, VkImageUsageFlags
			, uint32_t memProperties
			, VkImageType
			, VkImageViewType
			, VkImageAspectFlags
			, uint32_t layers
			, uint32_t dimX, uint32_t dimY = 1, uint32_t dimZ = 1
			, VkImageCreateFlags = 0);
		VulkanTensor() = default;
		~VulkanTensor();

		VkImage& handle() {	return image; }
		VkDeviceMemory& data() { return vulkanDeviceUsableMemory; }
		VkDeviceSize& size() { return allocationSize; }
		VkImageLayout getCurrentLayout() { return currentLayout; }
		uint32_t getCurrentOwner() { return currentOwner; }
		size_t addSlice(VkImageViewType, VkFormat, VkImageSubresourceRange
			, VkComponentMapping = {
				 VK_COMPONENT_SWIZZLE_IDENTITY
				,VK_COMPONENT_SWIZZLE_IDENTITY
				,VK_COMPONENT_SWIZZLE_IDENTITY
				,VK_COMPONENT_SWIZZLE_IDENTITY
			}
		);
		VkImageView& sliceHandle(size_t i = 0) { return views[i]; }
		VkExtent3D& dimensions() {	return extent; }

	private:
		VulkanState* vkState;
		VkImage image;
		std::vector<VkImageView> views;
		VkDeviceMemory vulkanDeviceUsableMemory;
		VkDeviceSize allocationSize;
		VkExtent3D extent;
		VkImageLayout currentLayout;
		VkAccessFlags2 currentAccess;
		VkPipelineStageFlagBits2 currentStage;
		uint32_t currentOwner;
		VkImageSubresourceRange currentSR;

		friend class Accelerator;
		friend struct VulkanStagingBuffer;
	};//VkImage

	/*
	Bitfield stuffs
	
	anisotropyEnable - 1 bit (<<0)
	compareEnable - 1 bit (<<1)
	unnormalizedCoordinates - 1 bit (<<2)
	
	magFilter - 1 bit (<<3)
	minFilter - 1 bit (<<4)
	mipmapMode - 1 bit (<<5)
	addressModeU - reluctant 3 bits, just shy of 2 (<<6)
	addressModeV - 3 bits (<<9)
	addressModeW - 3 bits (<<12)
	compareOp - 3 bits (<<15)
	borderColor - 3 bits (<<18)
	unused - 11 bits (<<21)
	maxAnisotropy - float we multiply by the device limit
	mipLodBias - float we multiply by the device limit
	minLod - float
	maxLod - float
	*/
	enum ScannerBits {
		SCANNER_ANISOTROPY_BIT =   0b00000000000000000000000000000001,
		SCANNER_COMPARE_BIT =      0b00000000000000000000000000000010,
		SCANNER_UNNORMALIZED_BIT = 0b00000000000000000000000000000100,
		SCANNER_MAG_FILTER =       0b00000000000000000000000000001000,
		SCANNER_MIN_FILTER =       0b00000000000000000000000000010000,
		SCANNER_MIPMAP_MODE =      0b00000000000000000000000000100000,
		SCANNER_ADDRESS_MODE_U =   0b00000000000000000000000111000000,
		SCANNER_ADDRESS_MODE_V =   0b00000000000000000000111000000000,
		SCANNER_ADDRESS_MODE_W =   0b00000000000000000111000000000000,
		SCANNER_COMPARE_OP =       0b00000000000000111000000000000000,
		SCANNER_BORDER_COLOR =     0b00000000000111000000000000000000,
		SCANNER_UNUSED_MASK =      0b11111111111000000000000000000000
	};
	struct SCStruct {
		uint32_t settingsField;
		float minLod, maxLod, mipLod, maxAniso;
	};

	//guess this is legacy stuff now?
	struct TensorScanner {
		std::string name;
		VkSampler sampler;
	};

	//we'll keep this around just in case, but otherwise it's all handled by the library
	class ScannerConfig {
		friend struct VulkanSamplerLibrary;

		ScannerConfig(VulkanState*);
		ScannerConfig() = default;
		~ScannerConfig() = default;

		TensorScanner makeScanner();

		VulkanState* vkState;
		VkSamplerCreateInfo prototype;
		//TODO: Add all sorts of config functions.
	};

	struct VulkanSamplerLibrary {
		VulkanSamplerLibrary(VulkanState*);
		~VulkanSamplerLibrary();

		VkSampler getSampler(std::string = "default");
	private:
		VulkanState* vkState;
		std::unordered_map<std::string, VkSampler> samplers;

		VkSamplerCreateInfo extractSCInfo(SCStruct&);
	};

	struct VulkanArray {

		VulkanArray(VulkanState*, VkDeviceSize, VkBufferUsageFlags, uint32_t);
		VulkanArray() = default;
		virtual ~VulkanArray();

		VkBuffer& handle() { return buffer; }
		VkDeviceMemory& data() { return vulkanDeviceUsableMemory; }
		VkDeviceSize& size() {	return allocationSize; }

	protected:
		VulkanState* vkState;
		VkBuffer buffer;
		VkDeviceMemory vulkanDeviceUsableMemory;
		VkDeviceSize allocationSize;
		VkBufferUsageFlags usage;
		uint32_t desiredMemoryProperties;
	private:
		friend class Accelerator;
		friend class DedicatedTransferQueue;
		friend struct VulkanStagingBuffer;
	};//VkBuffer
	struct VulkanStagingBuffer : VulkanArray {
		VulkanStagingBuffer(VulkanArray*, void*);
		VulkanStagingBuffer(VulkanTensor*, void*);
		//~VulkanStagingBuffer();
	};

	class AccelerationQueue {
		friend class Accelerator;
	protected:
		AccelerationQueue(VulkanState*, uint32_t, uint32_t, VkCommandPoolCreateFlags, QueueFamilyType);
		virtual ~AccelerationQueue();

		VulkanState* vkState;
		uint32_t FID, QID;
		QueueFamilyType type;
		VkQueue queue;
		VkCommandPool commandPool;

		virtual VkCommandBuffer beginRecording(size_t = 0) = 0;
		virtual VkQueue endRecording(
			size_t = 0,
			VkFence = VK_NULL_HANDLE,
			std::vector<VkSemaphoreSubmitInfo> waitForThese = {},
			std::vector<VkSemaphoreSubmitInfo> signalThese = {}
		) = 0;

	public:
		uint32_t getFamily() { return FID; }
	};

	class DedicatedTransferQueue: private AccelerationQueue {
		friend class Accelerator;

		DedicatedTransferQueue(VulkanState*, uint32_t, uint32_t);
		~DedicatedTransferQueue() = default;

		VkCommandBufferAllocateInfo allocInfo;
		VkCommandBuffer cBuffer;
		VkCommandBufferSubmitInfo cbsInfo;
		VkSubmitInfo2 singleTransferOp;

		VkCommandBuffer beginRecording(size_t = 0);
		VkQueue endRecording(size_t = 0, VkFence = VK_NULL_HANDLE, std::vector<VkSemaphoreSubmitInfo> = {}, std::vector<VkSemaphoreSubmitInfo> = {});
	};

	class DedicatedComputeQueue : private AccelerationQueue {
		friend class Accelerator;

		DedicatedComputeQueue(VulkanState*, uint32_t, uint32_t, int);
		~DedicatedComputeQueue() = default;

		VkCommandBuffer beginRecording(size_t = 0);
		VkQueue endRecording(size_t = 0, VkFence = VK_NULL_HANDLE, std::vector<VkSemaphoreSubmitInfo> = {}, std::vector<VkSemaphoreSubmitInfo> = {});
	};

	class GraphicsComputeQueue : private AccelerationQueue {//this is practically the default...right?
		friend class Accelerator;

		VkCommandBufferAllocateInfo allocInfo;
		std::vector<VkCommandBuffer> cBuffers;
		VkCommandBufferSubmitInfo cbsiTemplate;
		VkSubmitInfo2 submitTemplate;

		GraphicsComputeQueue(VulkanState*, uint32_t, uint32_t, int);
		~GraphicsComputeQueue() = default;

		VkCommandBuffer beginRecording(size_t = 0);
		VkQueue endRecording(size_t = 0, VkFence = VK_NULL_HANDLE, std::vector<VkSemaphoreSubmitInfo> = {}, std::vector<VkSemaphoreSubmitInfo> = {});
	};

	class PresentQueue : private AccelerationQueue { //we found graphics without compute somehow...
		friend class Accelerator;

		PresentQueue(VulkanState*, uint32_t, uint32_t, int);
		~PresentQueue() = default;

		VkCommandBuffer beginRecording(size_t = 0);
		VkQueue endRecording(size_t = 0, VkFence = VK_NULL_HANDLE, std::vector<VkSemaphoreSubmitInfo> = {}, std::vector<VkSemaphoreSubmitInfo> = {});
	};

	enum PipelineUse {
		COMPUTE_PIPELINE,
		GRAPHICS_VERTEX_PIPELINE,
		GRAPHICS_MESH_PIPELINE,
		RAYTRACING_PIPELINE,
		PIPELINE_USE_LIMIT
	};

	struct AccelerationProgram : Asset {
		AccelerationProgram(VulkanState*, ShaderFileData&, Asset* = nullptr);
		~AccelerationProgram();

		std::string getID() { return fileData.name; }
		PipelineUse getUsage() { return usage; }
		SAGEVertexType getVertexType() { return vertInfo.vertexType; }
		size_t getSetSize(size_t x) { return fileData.descriptorSetsLayout[x].size(); }
		bool hasPushConstants() { return fileData.PCRData.size() > 0; }

		std::vector<VkPipelineShaderStageCreateInfo>& getStages() { return shaderStages; }
		VkPipelineLayout& getPipelineLayout() { return pipelineLayout; }
	private:
		ShaderFileData fileData;
		SAGEVertexInfo vertInfo;

		VulkanState* vkState;

		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		std::vector<VkShaderModule> shaderModules;

		VkPipelineLayout pipelineLayout;
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
		std::vector<VkPushConstantRange> pushConstantRanges;//need to know push constant stages as well

		PipelineUse usage;
		friend class DescriptorSystem;
		friend struct AcceleratorOperationStateConfig;
		friend struct Material;
	};

	struct MetaDescriptor {
		std::string name;
		uint32_t set, binding, setIndex;
		VkDescriptorType type;
		SAGEDescriptorDataType dataType;
		size_t offset, stride;
		VulkanArray* linkedResource;
		void* pMappedMemory;
		VkSampler* pSampler;
		VkImageView* pImageView;
	};

	struct MetaDescriptorSet {
		VkDescriptorSetLayout layout;
		std::vector<MetaDescriptor> metaBindings;
	};

	struct DescriptorPoolGenerationData {
		VkDescriptorType type;
		double weight;
		uint32_t numLeft;
	};
	/* The one-size-fits-all solution would be to enumerate through all descriptor
		types and generate one of these for every type and go from there but...we'll
		reserve that for if it ends up being actually needed (we use extension types).
		Otherwise, we'll just be using our solution for the base 11 types.
	*/

	// a good strategy for pool generation might be to actually delay pool creation until
	// the first set is to be doled out.  In that time, we adjust weights depending on the
	// set registration requests and only finalize what we make when the first request for
	// a write comes in.  After that, weights can continue to be adjusted with more requests
	// and frees until a pool runs out and a new one is needed.

	class DescriptorSystem {
		DescriptorSystem(VulkanState*);
		~DescriptorSystem();

		VulkanState* vkState;

		const std::vector<std::string> descriptorNames = {
			"SAGEObjectDataBuffer",
			"SAGECamera",
			"SAGESceneData",
			"SAGEDiffuseMap",
			"SAGESpecularMap",
			"SAGEBumpMap",
			"SAGEAlbedoMap",
			"SAGENormalMap",
			"SAGEReflectivityMap",
			"SAGEGlossinessMap",
			"SAGEAmbientOcclusionMap",
			"SAGEOpactityMap",
			"SAGERefractionMap",
			"SAGEEmissiveMap",
			"SAGESkybox"
		};
		const uint32_t maxSetsPerPool = 100;

		VkDescriptorPool globalPool, framePool, drawPool;

		size_t freeID = 1;// 0 will be our error ID, our NULL_HANDLE if you will.

		std::unordered_map<std::string, std::array<MetaDescriptorSet, 4>> templates;

		std::vector<VkDescriptorSet> sets;
		//should be map of <std::string(name), std::vector<MetaDescriptor>>?
		std::vector<MetaDescriptor> manifest;

		VulkanArray* infoStorageBuffer;//TODO: make a dynamic VkBuffer struct... or just make this super large
		PerObjectRenderData* infoActual;
		void* infoMappedMemory;
		std::vector<VulkanArray*> PerFrameUBO;
		//std::vector<VkSampler*> activeSamplers; //may be a hack idk yet
		//std::vector<VkImageView> textureStorageBuffer; //inactive ATM, maybe bindless will bring it in
		//size_t freeOffset = 0; //used for dynamic offsetting but...idk if it will be needed

		friend class Accelerator;
	public:
		void registerProgram(AccelerationProgram*);
		//Returns one index for each descriptor binding in the set requested.
		std::vector<size_t> createDescriptorSet(std::string, size_t);
		//need a separate method to request sets from an already registered layout if so
		void connectBufferDescriptors(std::vector<size_t>);
		void connectImageDescriptor(size_t, VkImageView&, VkSampler&);
		void updateUBODescriptor(size_t, void*);
		void updateGlobalDescriptor(size_t, PerObjectRenderData&);
//		void updateImageDescriptor(size_t, VkImageView, VkSampler = VK_NULL_HANDLE); //idk if it will become useful
		void bindGlobalSet(size_t, VkCommandBuffer&, VkPipelineLayout&);
		void bindPerFrameSet(size_t, VkCommandBuffer&, VkPipelineLayout&);
		void bindPerDrawSet(size_t, VkCommandBuffer&, VkPipelineLayout&);
	};

	// 64-bit config field breakdown, right to left: 
	// 00000 00000 00000 00000 000 000 0000 0000 000 000 00 00 0000 0000000000000000
	// binary flags      - 16 bits (<< 0)
	// topology          - 4 bits  (<< 16)
	// fill mode         - 2 bits  (<< 20)
	// cull mode         - 2 bits  (<< 22)
	// MSAA Samples      - 3 bits  (<< 24)
	// Compare Op        - 3 bits  (<< 27)
	// Logic Op          - 4 bits  (<< 30)
	// -Color Write Mask - 4 bits  (<< 34)
	// -Color Blend Op   - 3 bits  (<< 38)
	// -Alpha Blend Op   - 3 bits  (<< 41)
	// -Src Color Blend  - 5 bits  (<< 44)
	// -Dst Color Blend  - 5 bits  (<< 49)
	// -Src Alpha Blend  - 5 bits  (<< 54)
	// -Dst Alpha Blend  - 5 bits  (<< 59)
	// Reserved          - 0 bits  (<< 64)
	// Later, if we need more space, we can use binary bits to change the interpretation of
	// other bit ranges.  Very simplistic right now.

	// Maybe want 2 config fields. One that is user-defined as above, and another that
	// Represents values influenced by the capabilities of the system like sample count.
	// The former defines pipeline configs for shader uses and the latter is constant across
	// all of those configs for the current graphics settings.
	enum AOSGraphicsConfigFlags {
		AOS_GRAPHICS_CONFIG_NONE =                                   0,
		AOS_GRAPHICS_CONFIG_PRIMITIVE_RESTART_BIT = 0b0000000000000001,
		AOS_GRAPHICS_CONFIG_DEPTH_CLAMP_BIT =       0b0000000000000010,
		AOS_GRAPHICS_CONFIG_DISCARD_RESULTS_BIT =   0b0000000000000100,
		AOS_GRAPHICS_CONFIG_CLOCKWISE_FRONT_BIT =   0b0000000000001000,
		AOS_GRAPHICS_CONFIG_DEPTH_BIAS_BIT =        0b0000000000010000,
		AOS_GRAPHICS_CONFIG_SAMPLE_SHADING_BIT =    0b0000000000100000,
		AOS_GRAPHICS_CONFIG_ALPHA_TO_COVERAGE_BIT = 0b0000000001000000,
		AOS_GRAPHICS_CONFIG_ALPHA_TO_ONE_BIT =      0b0000000010000000,
		AOS_GRAPHICS_CONFIG_DEPTH_TEST_BIT =        0b0000000100000000,
		AOS_GRAPHICS_CONFIG_DEPTH_WRITE_BIT =       0b0000001000000000,
		AOS_GRAPHICS_CONFIG_DEPTH_BOUNDS_BIT =      0b0000010000000000,
		AOS_GRAPHICS_CONFIG_STENCIL_TEST_BIT =      0b0000100000000000,
		AOS_GRAPHICS_CONFIG_LOGIC_BLENDING_BIT =    0b0001000000000000,
		AOS_GRAPHICS_CONFIG_BLENDING_ENABLE_BIT =   0b0010000000000000,
		AOS_GRAPHICS_CONFIG_DYN_DEPTH_BIT =         0b0100000000000000,
		AOS_GRAPHICS_CONFIG_DYN_STENCIL_BIT =       0b1000000000000000,
		AOS_GRAPHICS_CONFIG_ALL_ON = UINT16_MAX
	};
	enum AOSGraphicsConfigMasks : uint64_t {
		AOS_CONFIG_MASK_BINARY_FLAGS =            0b0000000000000000000000000000000000000000000000001111111111111111,
		AOS_CONFIG_MASK_INPUT_VERTEX_TOPOLOGY =   0b0000000000000000000000000000000000000000000011110000000000000000,
		AOS_CONFIG_MASK_RASTERIZER_POLYGON_MODE = 0b0000000000000000000000000000000000000000001100000000000000000000,
		AOS_CONFIG_MASK_RASTERIZER_CULL_MODE =    0b0000000000000000000000000000000000000000110000000000000000000000,
		AOS_CONFIG_MASK_MULTISAMPLE_COUNT =       0b0000000000000000000000000000000000000111000000000000000000000000,
		AOS_CONFIG_MASK_DEPTH_COMPARE_OP =        0b0000000000000000000000000000000000111000000000000000000000000000,
		AOS_CONFIG_MASK_BLENDING_LOGIC_OP =       0b0000000000000000000000000000001111000000000000000000000000000000,
		AOS_CONFIG_MASK_COLOR_WRITE_MASK =        0b0000000000000000000000000011110000000000000000000000000000000000,
		AOS_CONFIG_MASK_COLOR_BLEND_OP =          0b0000000000000000000000011100000000000000000000000000000000000000,
		AOS_CONFIG_MASK_ALPHA_BLEND_OP =          0b0000000000000000000011100000000000000000000000000000000000000000,
		AOS_CONFIG_MASK_COLOR_BLEND_SRC =         0b0000000000000001111100000000000000000000000000000000000000000000,
		AOS_CONFIG_MASK_COLOR_BLEND_DST =         0b0000000000111110000000000000000000000000000000000000000000000000,
		AOS_CONFIG_MASK_ALPHA_BLEND_SRC =         0b0000011111000000000000000000000000000000000000000000000000000000,
		AOS_CONFIG_MASK_ALPHA_BLEND_DST =         0b1111100000000000000000000000000000000000000000000000000000000000
	};
	struct AOSGraphicsConfigInfo {
		AOSGraphicsConfigInfo(uint64_t);
		AOSGraphicsConfigInfo() = default;
		VkPipelineColorBlendAttachmentState attachmentState;
		VkPrimitiveTopology primitiveTopology;
		VkPolygonMode polygonMode;
		VkCullModeFlagBits cullMode;
		VkSampleCountFlagBits sampleCount;
		VkCompareOp depthCompare;
		VkLogicOp blendingLogic;
		VkBool32 primitiveRestart, clampDepth, discardRaster, clockwiseFrontFace, biasDepth,
			sampleShading, alphaToCoverage, alphaToOne, depthTest, depthWrite, depthBoundsTest,
			stencilTest, logicBlending, dynamicDepthAttachment, dynamicStencilAttachment;
		uint64_t bake();
	};
	struct AOSGraphicsConfigNumerics {
		uint32_t tessellatorPatchControlPoints, dynamicViewMask;
		float rasterizerDepthBiasConstant,
			rasterizerDepthBiasClamp, rasterizerDepthBiasSlopeFactor, rasterizerLineWidth,
			multisamplerMinSampleShading,
			minDepthBounds, maxDepthBounds,
			blendConstantR, blendConstantG, blendConstantB, blendConstantA;
	};
	struct AOSGraphicsConfigStructs {
		std::vector<VkViewport> viewports;
		std::vector<VkRect2D> scissors;
		std::vector<VkSampleMask> multisamplerSampleMasks;
		VkStencilOpState frontFaceStencilOp, backFaceStencilOp;
		std::vector<VkPipelineColorBlendAttachmentState> additionalColorBlendAttachments;
		//std::vector<VkDynamicState> dynamicStates;
		std::vector<VkFormat> additionalDynamicColorAttachmentFormats;
	};
	//the above structs are defunct for now but...nice to have around in case?

	struct AcceleratorOperationStateConfig {
		AcceleratorOperationStateConfig(VulkanState*);
		AcceleratorOperationStateConfig() = default;
		~AcceleratorOperationStateConfig() = default;

		void reset(PipelineUse);
		bool configureGraphicsPipelines(AccelerationProgram*, std::vector <std::string>&, std::vector<VkGraphicsPipelineCreateInfo>&);

		/*
		void setGraphicsConfigStates(const AOSGraphicsConfigInfo&);
		void setGraphicsConfigNumerics(const AOSGraphicsConfigNumerics&);
		void addGraphicsConfigStructs(AOSGraphicsConfigStructs&);
		*/

	private:
		VulkanState* vkState;
		//all need to be turned into vectors that we hold on to as long as we still need to create pipelines
		//reset, then, clears all vectors to prepare for another batch of creates.
		//the create infos aren't stored here, they're doled out, to be thrown away after a create.

		std::vector<std::vector<VkPipelineColorBlendAttachmentState>> gcbAttachments;
		std::vector<std::vector<VkFormat>> gdrAttachmentFormats;
		std::vector<std::vector<VkViewport>> viewports;
		std::vector<std::vector<VkRect2D>> scissors;
		std::vector <std::vector<VkDynamicState>> dynamicStates;
		std::vector<std::vector<VkSampleMask>> multisampleMasks;

		std::vector<VkPipelineVertexInputStateCreateInfo>   gviStates;
		std::vector<VkPipelineInputAssemblyStateCreateInfo> giaStates;
		std::vector<VkPipelineTessellationStateCreateInfo>  gtsStates;
		std::vector<VkPipelineViewportStateCreateInfo>      gvpStates;
		std::vector<VkPipelineRasterizationStateCreateInfo> grtStates;
		std::vector<VkPipelineMultisampleStateCreateInfo>   gmsStates;
		std::vector<VkPipelineDepthStencilStateCreateInfo>  gdsStates;
		std::vector<VkPipelineColorBlendStateCreateInfo>    gcbStates;
		std::vector<VkPipelineDynamicStateCreateInfo>       gdyStages;
		std::vector<VkPipelineRenderingCreateInfo>          gdyrInfos;

//		VkRayTracingPipelineCreateInfoKHR raytracingPipelinePrototype;
//		VkRayTracingShaderGroupCreateInfoKHR raytracingShaders;
//		uint32_t raytracingBounces;
//		VkPipelineLibraryCreateInfoKHR raytracingLibraries;
//		VkRayTracingPipelineInterfaceCreateInfoKHR libraryUsage;

	};//pipeline factory
	
	//before the library, we'll need a cache handler to load and save pipeline caches.
	struct AcceleratorOperationStateCacheManager {
		AcceleratorOperationStateCacheManager() = default;
		AcceleratorOperationStateCacheManager(VulkanState*);
		~AcceleratorOperationStateCacheManager();

		bool createGraphicsPipelines(std::vector<VkGraphicsPipelineCreateInfo>&, std::vector<VkPipeline>&);
		bool save();

		VulkanState* vkState;
		uint32_t activeMatID;

		VkPipelineCache activeCache;
		std::array<uint8_t, VK_UUID_SIZE> cacheVendorUUID;
		size_t cacheSize;
		bool isCacheActive;

		size_t fileSize;

		bool getCacheInfo(uint32_t);
	};
	//may have a PipelineTypes enum to classify pipelines in the vector
	//things like SOLID_DRAW, WIREFRAME, NO_MSAA, etc.
	struct VulkanPipelineLibrary {
		VulkanPipelineLibrary(VulkanState*);
		~VulkanPipelineLibrary();

		AccelerationProgram* getProgram(uint32_t);
		void returnProgram(AccelerationProgram*);
		std::vector<VkPipeline>& getPipelines(uint32_t matID, uint32_t apID, std::vector<std::string>&);

		VulkanState* vkState;
		AcceleratorOperationStateConfig* configurator;
		AcceleratorOperationStateCacheManager* cache;
		//the first argument is a program's name
		std::unordered_map<std::string, std::vector<VkPipeline>> pipelines;
		std::unordered_map<uint32_t, AccelerationProgram*> programs;
	};

	class Accelerator {

		Accelerator(SDL_Window*);
		~Accelerator();

		std::vector<DedicatedTransferQueue*> transferQueues;
		size_t transferInUse;
		std::vector<DedicatedComputeQueue*> computeQueues;
		size_t computeInUse;
		std::vector<GraphicsComputeQueue*> spareGCQueues;
		size_t gcInUse;

		AccelerationQueue* presentationQueue,* graphicsComputeQueue,* transferQueue,* computeQueue;
		bool singleQueue;

		VkSemaphore queueOwnershipTransferenceSemaphore;
		VkFence hostAccessToCard;

		VulkanSamplerLibrary* samplerLibrary;
		DescriptorSystem* descriptorSystem;
		VulkanPipelineLibrary* stateLibrary;

		PFN_vkCreateDebugUtilsMessengerEXT createDebug;
		PFN_vkDestroyDebugUtilsMessengerEXT destroyDebug;
		VkDebugUtilsMessengerEXT dubigger;

		VulkanState vk;

		friend struct VideoManager;

		void cycleDedicatedQueue(AccelerationQueue*);

	public:
		Accelerator(const Accelerator&) = delete;
		Accelerator& operator=(const Accelerator&) = delete;

		VulkanState* getState() { return &vk; }
		VkDevice* getDevice() { return &vk.vulkanDevice; }
		AccelerationQueue* getPresentationQueuea() { return presentationQueue; }

		DescriptorSystem* getDescriptorSystem() { return descriptorSystem; }
		VulkanPipelineLibrary* getStateManager() { return stateLibrary; }
		VkSampler getSampler(std::string s) { return samplerLibrary->getSampler(s); }

		void prepareNewTensorAsStagingTarget(VulkanTensor*);
		void prepareNewTensorAsBlitTarget(VulkanTensor*, VkClearColorValue);

		void stage(VulkanStagingBuffer*, VulkanArray*);
		void stage(VulkanStagingBuffer*, VulkanTensor*);
		
		//right now all of the preparations track the tensor's current state
		//even though the nature of them means we have a definite state we expect them to be in
		//the question is: will this hinder us, or is there an opportunity in so doing?

		void prepareStagingTargetAsTexture(VulkanTensor*);
		void prepareStagingTargetAsAtlas(VulkanTensor*);

		void prepareBlitTargetAsTexture(VulkanTensor*);

		void prepareTextureAsBlitTarget(VulkanTensor*, VkClearColorValue);

		VkCommandBuffer getAndStartCustomCommandBuffer(QueueFamilyType, size_t = 0);
		VkQueue endAndSubmitCustomCommandBuffer(QueueFamilyType, size_t = 0, VkFence = VK_NULL_HANDLE, std::vector<VkSemaphoreSubmitInfo> = {}, std::vector<VkSemaphoreSubmitInfo> = {});
		void idle();

	};
}