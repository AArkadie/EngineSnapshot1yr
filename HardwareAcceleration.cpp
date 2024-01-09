#include "HardwareAcceleration.hpp"

namespace SAGE {

	VulkanTensor::VulkanTensor(VulkanState* vks
		, VkFormat imageFormat
		, VkImageTiling imageTiling
		, VkSampleCountFlagBits samples
		, VkImageUsageFlags usage
		, uint32_t desiredMemoryProperties
		, VkImageType dimensions
		, VkImageViewType viewDimensions
		, VkImageAspectFlags viewAspect
		, uint32_t layers
		, uint32_t dimX, uint32_t dimY, uint32_t dimZ
		, VkImageCreateFlags flags)
	{
		vkState = vks;
		extent = { dimX, dimY , dimZ };

		VkImageCreateInfo ici;
		ici.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		ici.pNext = nullptr;
		ici.flags = flags;
		ici.imageType = dimensions;
		ici.format = imageFormat;
		ici.extent = extent;
		ici.mipLevels = 1;
		ici.arrayLayers = layers;
		ici.samples = samples;
		ici.tiling = imageTiling;
		ici.usage = usage;
		ici.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		ici.queueFamilyIndexCount = 0;
		ici.pQueueFamilyIndices = nullptr;
		ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		vkCreateImage(vkState->vulkanDevice, &ici, nullptr, &image);

		VkMemoryRequirements imr;
		vkGetImageMemoryRequirements(vkState->vulkanDevice, image, &imr);
		allocationSize = imr.size;

		uint32_t mem = 0;
		bool allocSucc = false;
		//for the future: do we want more sophisticated memory type selection?  Seems very likely.
		for (auto& memType : hardwareInfo.graphicsCardInfo.memoryTypes) {
			if ((imr.memoryTypeBits & (1 << mem)) && (memType.propertyFlags & desiredMemoryProperties) == desiredMemoryProperties) {
				allocSucc = true;
				break;
			}
			mem++;
		}if (!allocSucc) throw std::runtime_error("buffer allocation failed!");

		VkMemoryAllocateInfo mai;
		mai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		mai.pNext = nullptr;
		mai.allocationSize = allocationSize;
		mai.memoryTypeIndex = mem;
		vkAllocateMemory(vkState->vulkanDevice, &mai, nullptr, &vulkanDeviceUsableMemory);
		vkBindImageMemory(vkState->vulkanDevice, image, vulkanDeviceUsableMemory, 0);

		VkImageView iv;
		VkImageViewCreateInfo ivci;
		ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ivci.pNext = nullptr;
		ivci.flags = 0;
		ivci.image = image;
		ivci.viewType = viewDimensions;
		ivci.format = imageFormat;
		ivci.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		ivci.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		ivci.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		ivci.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		ivci.subresourceRange.aspectMask = viewAspect;
		ivci.subresourceRange.baseMipLevel = 0;
		ivci.subresourceRange.levelCount = 1;
		ivci.subresourceRange.baseArrayLayer = 0;
		ivci.subresourceRange.layerCount = layers;
		vkCreateImageView(vkState->vulkanDevice, &ivci, nullptr, &iv);
		views.push_back(iv);

		currentAccess = 0;
		currentLayout = ici.initialLayout;
		currentStage = VK_PIPELINE_STAGE_2_NONE;
		currentOwner = UNUSED_FAMILY;
		currentSR = ivci.subresourceRange;
	}
	VulkanTensor::~VulkanTensor() {
		for (VkImageView& iv : views) {
			vkDestroyImageView(vkState->vulkanDevice, iv, nullptr);
		}
		vkDestroyImage(vkState->vulkanDevice, image, nullptr);
		vkFreeMemory(vkState->vulkanDevice, vulkanDeviceUsableMemory, nullptr);
	}
	size_t VulkanTensor::addSlice(VkImageViewType viewType, VkFormat viewFormat, VkImageSubresourceRange srRange, VkComponentMapping swizzle) {
		VkImageView iv;
		VkImageViewCreateInfo ivci;
		ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ivci.pNext = nullptr;
		ivci.flags = 0;
		ivci.image = image;
		ivci.viewType = viewType;
		ivci.format = viewFormat;
		ivci.components = swizzle;
		ivci.subresourceRange = srRange;
		vkCreateImageView(vkState->vulkanDevice, &ivci, nullptr, &iv);
		size_t returner = views.size();
		views.push_back(iv);
		return returner;
	}

	ScannerConfig::ScannerConfig(VulkanState* vks) {
		vkState = vks;

		prototype.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		prototype.pNext = nullptr;
		prototype.flags = 0;
		prototype.magFilter = VK_FILTER_LINEAR;
		prototype.minFilter = VK_FILTER_LINEAR;
		prototype.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		prototype.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		prototype.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		prototype.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		prototype.mipLodBias = 0.f;
		prototype.anisotropyEnable = VK_TRUE;
		prototype.maxAnisotropy = hardwareInfo.graphicsCardInfo.maxSamplerAnisotropy;//vkState->accelerationDevicePropertiesV10.properties.limits.maxSamplerAnisotropy;
		prototype.compareEnable = VK_FALSE;//will revisit
		prototype.compareOp = VK_COMPARE_OP_ALWAYS;//always true, never means always false
		prototype.minLod = 0.f;
		prototype.maxLod = 0.f;//VK_LOD_CLAMP_NONE uncaps this;
		prototype.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
		prototype.unnormalizedCoordinates = VK_FALSE;
	}
	TensorScanner ScannerConfig::makeScanner() {
		TensorScanner returnSampler;
		returnSampler.name = "Jim";
		vkCreateSampler(vkState->vulkanDevice, &prototype, nullptr, &returnSampler.sampler);
		return returnSampler;
	}

	VulkanSamplerLibrary::VulkanSamplerLibrary(VulkanState* vks) {
		vkState = vks;

		VkSampler defaultSampler;
		VkSamplerCreateInfo defaultInfo;
		defaultInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		defaultInfo.pNext = nullptr;
		defaultInfo.flags = 0;
		defaultInfo.magFilter = VK_FILTER_NEAREST;
		defaultInfo.minFilter = VK_FILTER_NEAREST;
		defaultInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		defaultInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		defaultInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		defaultInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		defaultInfo.mipLodBias = 0.f;
		defaultInfo.anisotropyEnable = VK_FALSE;
		defaultInfo.maxAnisotropy = 0.f;
		defaultInfo.compareEnable = VK_FALSE;
		defaultInfo.compareOp = VK_COMPARE_OP_NEVER;
		defaultInfo.minLod = 0.f;
		defaultInfo.maxLod = 0.f;
		defaultInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
		defaultInfo.unnormalizedCoordinates = VK_FALSE;
		vkCreateSampler(vkState->vulkanDevice, &defaultInfo, nullptr, &defaultSampler);

		samplers["default"] = defaultSampler;
	}
	VulkanSamplerLibrary::~VulkanSamplerLibrary() {
		for (auto& s : samplers) {
			vkDestroySampler(vkState->vulkanDevice, s.second, nullptr);
		}
	}
	VkSamplerCreateInfo VulkanSamplerLibrary::extractSCInfo(SCStruct& cfg) {
		VkSamplerCreateInfo returnInfo;
		returnInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		returnInfo.pNext = nullptr;
		returnInfo.flags = 0;
		returnInfo.anisotropyEnable = cfg.settingsField & SCANNER_ANISOTROPY_BIT;
		returnInfo.compareEnable = ((cfg.settingsField & SCANNER_COMPARE_BIT) >> 1);
		returnInfo.unnormalizedCoordinates = ((cfg.settingsField & SCANNER_UNNORMALIZED_BIT) >> 2);
		returnInfo.magFilter = (VkFilter)((cfg.settingsField & SCANNER_MAG_FILTER) >> 3);
		returnInfo.minFilter = (VkFilter)((cfg.settingsField & SCANNER_MIN_FILTER) >> 4);
		returnInfo.mipmapMode = (VkSamplerMipmapMode)((cfg.settingsField & SCANNER_MIPMAP_MODE) >> 5);
		returnInfo.addressModeU = (VkSamplerAddressMode)((cfg.settingsField & SCANNER_ADDRESS_MODE_U) >> 6);
		returnInfo.addressModeV = (VkSamplerAddressMode)((cfg.settingsField & SCANNER_ADDRESS_MODE_U) >> 9);
		returnInfo.addressModeW = (VkSamplerAddressMode)((cfg.settingsField & SCANNER_ADDRESS_MODE_U) >> 12);
		returnInfo.compareOp = (VkCompareOp)((cfg.settingsField & SCANNER_COMPARE_OP) >> 15);
		returnInfo.borderColor = (VkBorderColor)((cfg.settingsField & SCANNER_BORDER_COLOR) >> 18);
		returnInfo.minLod = cfg.minLod;
		returnInfo.maxLod = cfg.maxLod;
		returnInfo.mipLodBias = cfg.mipLod * hardwareInfo.graphicsCardInfo.maxSamplerLoDBias;//vkState->accelerationDevicePropertiesV10.properties.limits.maxSamplerLodBias;
		returnInfo.maxAnisotropy = cfg.maxAniso * hardwareInfo.graphicsCardInfo.maxSamplerAnisotropy;//vkState->accelerationDevicePropertiesV10.properties.limits.maxSamplerAnisotropy;
		return returnInfo;
	}
	VkSampler VulkanSamplerLibrary::getSampler(std::string name) {
		if (!samplers.contains(name)) {
			std::string filepath = "Configs/Samplers/" + name + ".cfg";
			std::vector<std::string> lines;
			if (!readWholeTextFile(filepath, lines))return samplers["default"];
			std::sort(lines.begin(), lines.end());
			SCStruct info{};
			size_t i = 1;
			size_t fulcrum = lines[i].find('=');
			std::string op = lines[i].substr(0, fulcrum);
			std::string val = lines[i].substr(fulcrum + 1);
			if (op == "enableAnisotropicFiltering") {
					info.settingsField |= std::stoul(val);

					if (++i == lines.size()) goto sampCreate;
					fulcrum = lines[i].find('=');
					op = lines[i].substr(0, fulcrum);
					val = lines[i].substr(fulcrum + 1);
				}
			if (op == "enableReferenceComparison") {
					info.settingsField |= std::stoul(val) << 1;

					if (++i == lines.size()) goto sampCreate;
					fulcrum = lines[i].find('=');
					op = lines[i].substr(0, fulcrum);
					val = lines[i].substr(fulcrum + 1);
				}
			if (op == "magnificationFilter") {
					if (val == "linear") info.settingsField |= VK_FILTER_LINEAR << 3;
					else info.settingsField |= VK_FILTER_NEAREST << 3;

					if (++i == lines.size()) goto sampCreate;
					fulcrum = lines[i].find('=');
					op = lines[i].substr(0, fulcrum);
					val = lines[i].substr(fulcrum + 1);
				}
			if (op == "maxLod") {
					if (val == "cap" || val == "max") info.maxLod = VK_LOD_CLAMP_NONE;
					else info.maxLod = std::stof(val);

					if (++i == lines.size()) goto sampCreate;
					fulcrum = lines[i].find('=');
					op = lines[i].substr(0, fulcrum);
					val = lines[i].substr(fulcrum + 1);
				}
			if (op == "minLod") {
					info.minLod = std::stof(val);

					if (++i == lines.size()) goto sampCreate;
					fulcrum = lines[i].find('=');
					op = lines[i].substr(0, fulcrum);
					val = lines[i].substr(fulcrum + 1);
				}
			if (op == "minificationFilter") {
					if (val == "linear") info.settingsField |= VK_FILTER_LINEAR << 4;
					else info.settingsField |= VK_FILTER_NEAREST << 4;

					if (++i == lines.size()) goto sampCreate;
					fulcrum = lines[i].find('=');
					op = lines[i].substr(0, fulcrum);
					val = lines[i].substr(fulcrum + 1);
				}
			if (op == "mipmapMode") {
					if (val == "linear") info.settingsField |= VK_SAMPLER_MIPMAP_MODE_LINEAR << 5;
					else info.settingsField |= VK_SAMPLER_MIPMAP_MODE_NEAREST << 5;

					if (++i == lines.size()) goto sampCreate;
					fulcrum = lines[i].find('=');
					op = lines[i].substr(0, fulcrum);
					val = lines[i].substr(fulcrum + 1);
				}
			if (op == "outOfBoundsBehaviorU") {
					if (val == "mirroredRepeat") info.settingsField |= VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT << 6;
					else if (val == "clampToEdge") info.settingsField |= VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE << 6;
					else if (val == "clampToBorder") info.settingsField |= VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER << 6;
					else if (val == "mirroredClampToEdge") info.settingsField |= VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE << 6;
					else info.settingsField |= VK_SAMPLER_ADDRESS_MODE_REPEAT << 6;

					if (++i == lines.size()) goto sampCreate;
					fulcrum = lines[i].find('=');
					op = lines[i].substr(0, fulcrum);
					val = lines[i].substr(fulcrum + 1);
				}
			if (op == "outOfBoundsBehaviorV") {
					if (val == "mirroredRepeat") info.settingsField |= VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT << 9;
					else if (val == "clampToEdge") info.settingsField |= VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE << 9;
					else if (val == "clampToBorder") info.settingsField |= VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER << 9;
					else if (val == "mirroredClampToEdge") info.settingsField |= VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE << 9;
					else info.settingsField |= VK_SAMPLER_ADDRESS_MODE_REPEAT << 9;

					if (++i == lines.size()) goto sampCreate;
					fulcrum = lines[i].find('=');
					op = lines[i].substr(0, fulcrum);
					val = lines[i].substr(fulcrum + 1);
				}
			if (op == "outOfBoundsBehaviorW") {
					if (val == "mirroredRepeat") info.settingsField |= VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT << 12;
					else if (val == "clampToEdge") info.settingsField |= VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE << 12;
					else if (val == "clampToBorder") info.settingsField |= VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER << 12;
					else if (val == "mirroredClampToEdge") info.settingsField |= VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE << 12;
					else info.settingsField |= VK_SAMPLER_ADDRESS_MODE_REPEAT << 12;

					if (++i == lines.size()) goto sampCreate;
					fulcrum = lines[i].find('=');
					op = lines[i].substr(0, fulcrum);
					val = lines[i].substr(fulcrum + 1);
				}
			if (op == "outOfBoundsColor") {
					if (val == "opaqueWhiteInt") info.settingsField |= VK_BORDER_COLOR_INT_OPAQUE_WHITE << 18;
					else if (val == "opaqueWhiteFloat") info.settingsField |= VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE << 18;
					else if (val == "opaqueBlackInt") info.settingsField |= VK_BORDER_COLOR_INT_OPAQUE_BLACK << 18;
					else if (val == "opaqueBlackFloat") info.settingsField |= VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK << 18;
					else if (val == "transparentBlackInt") info.settingsField |= VK_BORDER_COLOR_INT_TRANSPARENT_BLACK << 18;
					else info.settingsField |= VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK << 18;

					if (++i == lines.size()) goto sampCreate;
					fulcrum = lines[i].find('=');
					op = lines[i].substr(0, fulcrum);
					val = lines[i].substr(fulcrum + 1);
				}
			if (op == "percentMaxAnisotropy") {
					info.maxAniso = std::stof(val);

					if (++i == lines.size()) goto sampCreate;
					fulcrum = lines[i].find('=');
					op = lines[i].substr(0, fulcrum);
					val = lines[i].substr(fulcrum + 1);
				}
			if (op == "percentMipLodBias") {
					info.mipLod = std::stof(val);

					if (++i == lines.size()) goto sampCreate;
					fulcrum = lines[i].find('=');
					op = lines[i].substr(0, fulcrum);
					val = lines[i].substr(fulcrum + 1);
				}
			if (op == "referenceComparisonOperation" || op == "referenceComparisonOp") {
					if (val == "1" || val == "always") info.settingsField |= VK_COMPARE_OP_ALWAYS << 15;
					else if (val == ">=" || val == "greaterOrEqual") info.settingsField |= VK_COMPARE_OP_GREATER_OR_EQUAL << 15;
					else if (val == "!=" || val == "notEqual") info.settingsField |= VK_COMPARE_OP_NOT_EQUAL << 15;
					else if (val == ">" || val == "greater") info.settingsField |= VK_COMPARE_OP_GREATER << 15;
					else if (val == "<=" || val == "lessOrEqual") info.settingsField |= VK_COMPARE_OP_LESS_OR_EQUAL << 15;
					else if (val == "==" || val == "equal") info.settingsField |= VK_COMPARE_OP_EQUAL << 15;
					else if (val == "<" || val == "less") info.settingsField |= VK_COMPARE_OP_LESS << 15;
					else info.settingsField |= VK_COMPARE_OP_NEVER << 15;

					if (++i == lines.size()) goto sampCreate;
					fulcrum = lines[i].find('=');
					op = lines[i].substr(0, fulcrum);
					val = lines[i].substr(fulcrum + 1);
				}
			if (op == "usePixelCoordinates" || op == "unnormalizedCoordinates") {
					info.settingsField |= std::stoul(val) << 2;
				}
			//will we implement a binary file that compiles all configs or read them straight?
			//keep it open as an option just in case we take a performance hit here.
			
			sampCreate:
			VkSamplerCreateInfo sci = extractSCInfo(info);
			VkSampler newSamp;
			vkCreateSampler(vkState->vulkanDevice, &sci, nullptr, &newSamp);
			samplers[name] = newSamp;
		}
		return samplers[name];
	}

	VulkanArray::VulkanArray(VulkanState* vks, VkDeviceSize size, VkBufferUsageFlags use, uint32_t dmp) {
		vkState = vks;
		usage = use;
		desiredMemoryProperties = dmp;

		VkBufferCreateInfo cInfo;
		cInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		cInfo.pNext = nullptr;
		cInfo.flags = 0;
		cInfo.size = size;
		cInfo.usage = usage;
		cInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		cInfo.queueFamilyIndexCount = 0;
		cInfo.pQueueFamilyIndices = nullptr;
		vkCreateBuffer(vkState->vulkanDevice, &cInfo, nullptr, &buffer);

		VkMemoryRequirements memR;
		vkGetBufferMemoryRequirements(vkState->vulkanDevice, buffer, &memR);
		allocationSize = memR.size;

		uint32_t memDex = 0;
		bool allocSucc = false;

		for (auto& memType : hardwareInfo.graphicsCardInfo.memoryTypes) {
			if ((memR.memoryTypeBits & (1 << memDex)) && (memType.propertyFlags & desiredMemoryProperties) == desiredMemoryProperties) {
				allocSucc = true;
				break;
			}
			memDex++;
		}if (!allocSucc) throw std::runtime_error("buffer allocation failed!");

/*
		if (size < allocationSize) {//This was necessary because of an allignment issue
			vkDestroyBuffer(vkState->vulkanDevice, buffer, nullptr);
			cInfo.size = allocationSize;
			vkCreateBuffer(vkState->vulkanDevice, &cInfo, nullptr, &buffer);
		}//If this actually is good practise though...the code's here.
*/
		VkMemoryAllocateInfo allocInfo;
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.allocationSize = allocationSize;
		allocInfo.memoryTypeIndex = memDex;
		vkAllocateMemory(vkState->vulkanDevice, &allocInfo, nullptr, &vulkanDeviceUsableMemory);
		vkBindBufferMemory(vkState->vulkanDevice, buffer, vulkanDeviceUsableMemory, 0);
	}
	VulkanArray::~VulkanArray() {
		vkFreeMemory(vkState->vulkanDevice, vulkanDeviceUsableMemory, nullptr);
		vkDestroyBuffer(vkState->vulkanDevice, buffer, nullptr);
	}
	VulkanStagingBuffer::VulkanStagingBuffer(VulkanArray* va, void* data) :
		VulkanArray(
			va->vkState,
			va->allocationSize,
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		)
	{
		void* temp;
		vkMapMemory(vkState->vulkanDevice, vulkanDeviceUsableMemory, 0, allocationSize, 0, &temp);
		memcpy(temp, data, static_cast<size_t>(allocationSize));
		vkUnmapMemory(vkState->vulkanDevice, vulkanDeviceUsableMemory);
	}
	VulkanStagingBuffer::VulkanStagingBuffer(VulkanTensor* vt, void* data) :
		VulkanArray(
			vt->vkState,
			vt->allocationSize,
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		)
	{
		void* temp;
		vkMapMemory(vkState->vulkanDevice, vulkanDeviceUsableMemory, 0, allocationSize, 0, &temp);
		memcpy(temp, data, static_cast<size_t>(allocationSize));//always fails here when it does, access violation.
		//might be a limitation from not having a dedicated memory manager
		//then again, this errors only for default maps where every allocationSize is 512.  These maps are
		//1x1x4 or 1x1x1.  This is probably the origin of our access error but...why is it rare then?
		//what makes it able to pack the data in some cases but not others?  Is it packing or are we really wasteful?
		//On further investigation, this might just be an Nvidia thing, 1x1 textures are fine in theory.
		//maybe in the future we have a texture of default values that we blit from based on the type of default.
		//But I don't see us having THAT many very small default values.  Back on topic though,
		//Do we want every map to have 4 channels and we take the floating point maps from the alpha channel?
		//Or hell, combine maps to get 4 channels across all: normal + bump, specular + metal + gloss + AO, etc.
		//Guess that's not on topic, whatever, we'll get around to fixing this eventually.  But today is not that day.
		vkUnmapMemory(vkState->vulkanDevice, vulkanDeviceUsableMemory);
	}

	AccelerationQueue::AccelerationQueue(VulkanState* vks, uint32_t famdex, uint32_t quedex, VkCommandPoolCreateFlags f, QueueFamilyType t) {
		vkState = vks;
		FID = famdex;
		QID = quedex;
		type = t;
		vkGetDeviceQueue(vkState->vulkanDevice, FID, QID, &queue);

		VkCommandPoolCreateInfo temp;
		temp.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		temp.pNext = nullptr;
		temp.flags = f;
		temp.queueFamilyIndex = FID;
		vkCreateCommandPool(vkState->vulkanDevice, &temp, nullptr, &commandPool);
	}
	AccelerationQueue::~AccelerationQueue() {
		vkDestroyCommandPool(vkState->vulkanDevice, commandPool, nullptr);
	}

	DedicatedTransferQueue::DedicatedTransferQueue(VulkanState* vks, uint32_t f, uint32_t q):
		AccelerationQueue(vks, f, q, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT, TRANSFER_FAMILY) {

		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.commandPool = commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		cbsInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
		cbsInfo.pNext = nullptr;
		cbsInfo.commandBuffer = cBuffer;
		cbsInfo.deviceMask = 0;

		singleTransferOp.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
		singleTransferOp.pNext = nullptr;
		singleTransferOp.flags = 0;
		singleTransferOp.waitSemaphoreInfoCount = 0;
		singleTransferOp.pWaitSemaphoreInfos = nullptr;
		singleTransferOp.commandBufferInfoCount = 1;
		singleTransferOp.pCommandBufferInfos = &cbsInfo;
		singleTransferOp.signalSemaphoreInfoCount = 0;
		singleTransferOp.pSignalSemaphoreInfos = nullptr;
	}
	VkCommandBuffer DedicatedTransferQueue::beginRecording(size_t index) {
		vkAllocateCommandBuffers(vkState->vulkanDevice, &allocInfo, &cBuffer);
		vkBeginCommandBuffer(cBuffer, &vkState->singleUseBegin);
		return cBuffer;
	}
	VkQueue DedicatedTransferQueue::endRecording(size_t index, VkFence fence, std::vector<VkSemaphoreSubmitInfo> waitForThese, std::vector<VkSemaphoreSubmitInfo> signalThese) {
		vkEndCommandBuffer(cBuffer);
		singleTransferOp.waitSemaphoreInfoCount = static_cast<uint32_t>(waitForThese.size());
		singleTransferOp.pWaitSemaphoreInfos = waitForThese.data();
		cbsInfo.commandBuffer = cBuffer;
		singleTransferOp.signalSemaphoreInfoCount = static_cast<uint32_t>(signalThese.size());
		singleTransferOp.pSignalSemaphoreInfos = signalThese.data();
		vkQueueSubmit2(queue, 1, &singleTransferOp, fence);
		vkQueueWaitIdle(queue);//hard block, maybe need a watcher thread that provides and waits for fences to free
		vkFreeCommandBuffers(vkState->vulkanDevice, commandPool, 1, &cBuffer);
		return queue;
	}
	DedicatedComputeQueue::DedicatedComputeQueue(VulkanState* vks, uint32_t f, uint32_t q, int frames) :
		AccelerationQueue(vks, f, q, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, ASYNC_COMPUTE_FAMILY) {
		//TODO: Compute Stuff
	}
	VkCommandBuffer DedicatedComputeQueue::beginRecording(size_t index) {
		return VK_NULL_HANDLE;
	}
	VkQueue DedicatedComputeQueue::endRecording(size_t index, VkFence fence, std::vector<VkSemaphoreSubmitInfo> waitForThese, std::vector<VkSemaphoreSubmitInfo> SignalThese) {
		return queue;
	}

	GraphicsComputeQueue::GraphicsComputeQueue(VulkanState* vks, uint32_t f, uint32_t q, int frames) :
		AccelerationQueue(vks, f, q, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, GRAPHICS_COMPUTE_FAMILY) {
		cBuffers.resize(frames);
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.commandPool = commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = frames;
		vkAllocateCommandBuffers(vkState->vulkanDevice, &allocInfo, cBuffers.data());

		cbsiTemplate.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
		cbsiTemplate.pNext = nullptr;
		cbsiTemplate.commandBuffer = cBuffers[0];
		cbsiTemplate.deviceMask = 0;

		submitTemplate.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
		submitTemplate.pNext = nullptr;
		submitTemplate.waitSemaphoreInfoCount = 0;
		submitTemplate.pWaitSemaphoreInfos = nullptr;
		submitTemplate.commandBufferInfoCount = 1;
		submitTemplate.pCommandBufferInfos = &cbsiTemplate;
		submitTemplate.signalSemaphoreInfoCount = 0;
		submitTemplate.pSignalSemaphoreInfos = nullptr;
	}
	VkCommandBuffer GraphicsComputeQueue::beginRecording(size_t index) {
		vkResetCommandBuffer(cBuffers[index], 0);
		vkBeginCommandBuffer(cBuffers[index], &vkState->singleUseBegin);
		return cBuffers[index];
	}
	VkQueue GraphicsComputeQueue::endRecording(size_t index, VkFence fence, std::vector<VkSemaphoreSubmitInfo> waitForThese, std::vector<VkSemaphoreSubmitInfo> signalThese) {
		vkEndCommandBuffer(cBuffers[index]);
		submitTemplate.waitSemaphoreInfoCount = static_cast<uint32_t>(waitForThese.size());
		submitTemplate.pWaitSemaphoreInfos = waitForThese.data();
		cbsiTemplate.commandBuffer = cBuffers[index];
		submitTemplate.signalSemaphoreInfoCount = static_cast<uint32_t>(signalThese.size());
		submitTemplate.pSignalSemaphoreInfos = signalThese.data();
		vkQueueSubmit2(queue, 1, &submitTemplate, fence);
		if (fence == VK_NULL_HANDLE) vkQueueWaitIdle(queue);//hard block, maybe need a watcher thread that provides and waits for fences to free
		return queue;
	}

	PresentQueue::PresentQueue(VulkanState* vks, uint32_t f, uint32_t q, int frames) :
		AccelerationQueue(vks, f, q, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, PRESENT_FAMILY) {
		//What do we do when you have graphics but no compute?
	}
	VkCommandBuffer PresentQueue::beginRecording(size_t index) {
		return VK_NULL_HANDLE;
	}
	VkQueue PresentQueue::endRecording(size_t index, VkFence fence, std::vector<VkSemaphoreSubmitInfo> waitForThese, std::vector<VkSemaphoreSubmitInfo> signalThese) {
		return queue;
	}

	AccelerationProgram::AccelerationProgram(VulkanState* vks, ShaderFileData& data, Asset* parent) {
		vkState = vks;
		fileData = std::move(data);

		vertInfo = { fileData.vertInputs };

		if (parent) {
			UUID = parent->UUID;
			listPosition = parent->listPosition;
			numReferences = parent->numReferences;
			delete parent;
		}

		usage = PIPELINE_USE_LIMIT;
		for (auto& s : fileData.shaderStages) {
			if (s & VK_SHADER_STAGE_COMPUTE_BIT) { usage = COMPUTE_PIPELINE; break; }
			if (s & VK_SHADER_STAGE_VERTEX_BIT) { usage = GRAPHICS_VERTEX_PIPELINE; break; }
			//if (s & VK_SHADER_STAGE_MESH_BIT_EXT) { usage = GRAPHICS_MESH_PIPELINE; break; }
			//if (s & VK_SHADER_STAGE_ANY_HIT_BIT_KHR) { usage = RAYTRACING_PIPELINE; break; }
		}

		for (auto& p : fileData.PCRData) {
			VkPushConstantRange temp;
			temp.offset = p.offset;
			temp.size = p.size;
			temp.stageFlags = p.stages;
			pushConstantRanges.push_back(temp);
		}

		for (size_t i = 0; i < fileData.numShaders; i++) {

			VkShaderModule tempModule;
			VkShaderModuleCreateInfo tmCI;
			tmCI.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			tmCI.pNext = nullptr;
			tmCI.flags = 0;
			tmCI.codeSize = fileData.shaderDataSizes[i];
			tmCI.pCode = fileData.shaderData[i].data();
			vkCreateShaderModule(vkState->vulkanDevice, &tmCI, nullptr, &tempModule);
			VkPipelineShaderStageCreateInfo tempInfo;
			tempInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			tempInfo.pNext = nullptr;
			tempInfo.flags = 0;
			tempInfo.stage = fileData.shaderStages[i];
			tempInfo.module = tempModule;
			tempInfo.pName = fileData.shaderEntryPoints[i].data();
			tempInfo.pSpecializationInfo = nullptr;//maybe this is extractable

			shaderModules.push_back(tempModule);
			shaderStages.push_back(tempInfo);
		}//end per-shader thing
/**
		for (size_t i = 2; i < descriptorSetData.size(); i++) {//come back once binding frequency enum is integrated
			for (auto& b : descriptorSetData[i]) {
				if (b.type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) b.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
				if (b.type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) b.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
			}
		}
/**/
		for (size_t i = 0; i < fileData.descriptorSetsLayout.size(); i++) {
			//if (fileData.descriptorSetsLayout[i].size() == 0)continue;
			descriptorSetLayouts.push_back({});
			std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
			if (fileData.descriptorSetsLayout[i].size() != 0) {
				for (auto& b : fileData.descriptorSetsLayout[i]) {
					VkDescriptorSetLayoutBinding temp;
					temp.binding = b.binding;
					temp.descriptorType = b.type;
					temp.descriptorCount = b.count;
					temp.stageFlags = b.stages;
					temp.pImmutableSamplers = nullptr;

					layoutBindings.push_back(temp);
				}
			}
			VkDescriptorSetLayoutCreateInfo tinfo;
			tinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			tinfo.pNext = nullptr;
			tinfo.flags = 0;
			tinfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
			tinfo.pBindings = layoutBindings.data();
			vkCreateDescriptorSetLayout(vkState->vulkanDevice, &tinfo, nullptr, &descriptorSetLayouts[i]);
		}

		VkPipelineLayoutCreateInfo graphicsPipelineLayoutCreateInfo;
		graphicsPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		graphicsPipelineLayoutCreateInfo.pNext = nullptr;
		graphicsPipelineLayoutCreateInfo.flags = 0;
		graphicsPipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		graphicsPipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
		graphicsPipelineLayoutCreateInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size());
		graphicsPipelineLayoutCreateInfo.pPushConstantRanges = pushConstantRanges.data();
		vkCreatePipelineLayout(vkState->vulkanDevice, &graphicsPipelineLayoutCreateInfo, nullptr, &pipelineLayout);
	}
	AccelerationProgram::~AccelerationProgram() {
		for (auto& m : shaderModules) {
			vkDestroyShaderModule(vkState->vulkanDevice, m, nullptr);
		}
		for (auto& sl : descriptorSetLayouts) {
			vkDestroyDescriptorSetLayout(vkState->vulkanDevice, sl, nullptr);
		}
		vkDestroyPipelineLayout(vkState->vulkanDevice, pipelineLayout, nullptr);
	}

	SAGEDescriptorDataType stringToSDDT(std::string& name) {
		if (name == "SAGEObjectDataBuffer") return SAGE_PER_OBJECT_DATA;
		if (name == "SAGECamera") return SAGE_PER_FRAME_CAMERA_DATA;
		if (name == "SAGESceneData") return SAGE_PER_FRAME_SCENE_DATA;
		if (name == "SAGEDiffuseMap" ||
			name == "SAGESpecularMap" ||
			name == "SAGEBumpMap" ||
			name == "SAGEAlbedoMap" ||
			name == "SAGENormalMap" ||
			name == "SAGEReflectivityMap" ||
			name == "SAGEGlossinessMap" ||
			name == "SAGEAmbientOcclusionMap" ||
			name == "SAGEOpacityMap" ||
			name == "SAGERefractionMap" ||
			name == "SAGEEmissiveMap" ||
			name == "SAGESkybox") return SAGE_TEXTURE_DATA;//we don't really use this to control anything so...
		//maybe we just default it as texture data and make sure you're not trying to write a buffer with texture data.
		return SAGE_DESCRIPTOR_DATA_LIMIT;
	}

	DescriptorSystem::DescriptorSystem(VulkanState* vks) {
		vkState = vks;
		
		std::array<VkDescriptorPoolSize, 1> globalPoolSizes;
		globalPoolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		globalPoolSizes[0].descriptorCount = maxSetsPerPool;
		std::array<VkDescriptorPoolSize, 1> framePoolSizes;
		framePoolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		framePoolSizes[0].descriptorCount = maxSetsPerPool * 2;
		std::array<VkDescriptorPoolSize, 3> drawPoolSizes;
		drawPoolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		drawPoolSizes[0].descriptorCount = maxSetsPerPool;
		drawPoolSizes[1].type = VK_DESCRIPTOR_TYPE_SAMPLER;
		drawPoolSizes[1].descriptorCount = maxSetsPerPool;
		drawPoolSizes[2].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		drawPoolSizes[2].descriptorCount = maxSetsPerPool;

		VkDescriptorPoolCreateInfo globalPoolCI, framePoolCI, drawPoolCI;
		globalPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		globalPoolCI.pNext = nullptr;
		globalPoolCI.flags = 0;
		globalPoolCI.maxSets = maxSetsPerPool;
		globalPoolCI.poolSizeCount = static_cast<uint32_t>(globalPoolSizes.size());
		globalPoolCI.pPoolSizes = globalPoolSizes.data();
		vkCreateDescriptorPool(vkState->vulkanDevice, &globalPoolCI, nullptr, &globalPool);
		framePoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		framePoolCI.pNext = nullptr;
		framePoolCI.flags = 0;
		framePoolCI.maxSets = maxSetsPerPool;
		framePoolCI.poolSizeCount = static_cast<uint32_t>(framePoolSizes.size());
		framePoolCI.pPoolSizes = framePoolSizes.data();
		vkCreateDescriptorPool(vkState->vulkanDevice, &framePoolCI, nullptr, &framePool);
		drawPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		drawPoolCI.pNext = nullptr;
		drawPoolCI.flags = 0;
		drawPoolCI.maxSets = maxSetsPerPool;
		drawPoolCI.poolSizeCount = static_cast<uint32_t>(drawPoolSizes.size());
		drawPoolCI.pPoolSizes = drawPoolSizes.data();
		vkCreateDescriptorPool(vkState->vulkanDevice, &drawPoolCI, nullptr, &drawPool);

		infoStorageBuffer = new VulkanArray(
			vkState,
			sizeof(PerObjectRenderData) * DYNAMIC_BUFFER_LIMIT,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);
		vkMapMemory
		(
			vkState->vulkanDevice,
			infoStorageBuffer->data(), 0,
			infoStorageBuffer->size(), 0,
			&infoMappedMemory
		);
		infoActual = (PerObjectRenderData*) infoMappedMemory;
		manifest.push_back({});
	}
	DescriptorSystem::~DescriptorSystem() {
		for (auto& m : manifest) {
			if (m.linkedResource && m.type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
				vkUnmapMemory(vkState->vulkanDevice,m.linkedResource->data());
				m.pMappedMemory = nullptr;
			}
		}
		for (auto b : PerFrameUBO) {
			delete b;
		}
		vkUnmapMemory(vkState->vulkanDevice, infoStorageBuffer->data());
		infoMappedMemory = nullptr;
		delete infoStorageBuffer;

		vkResetDescriptorPool(vkState->vulkanDevice, globalPool, 0);
		vkDestroyDescriptorPool(vkState->vulkanDevice, globalPool, nullptr);
		vkResetDescriptorPool(vkState->vulkanDevice, framePool, 0);
		vkDestroyDescriptorPool(vkState->vulkanDevice, framePool, nullptr);
		vkResetDescriptorPool(vkState->vulkanDevice, drawPool, 0);
		vkDestroyDescriptorPool(vkState->vulkanDevice, drawPool, nullptr);
	}
	void DescriptorSystem::registerProgram(AccelerationProgram* ap) {
		//uint8_t numEmpty = 0;
		for (size_t i = 0; i < ap->fileData.descriptorSetsLayout.size(); i++) {
			std::vector<DescriptorGenerationData> tempG = ap->fileData.descriptorSetsLayout[i];
			MetaDescriptor temp{};
			if (tempG.size() == 0) {
				temp.name = "NULL";
				templates[ap->fileData.name][i].metaBindings.push_back(temp);
				//numEmpty++;
				continue;
			}
			for (auto& b : tempG) {
				if (std::find(descriptorNames.begin(), descriptorNames.end(), b.name) == descriptorNames.end()) {
					temp.name = "NULL";
					continue;
				}
				templates[ap->fileData.name][b.set].layout = ap->descriptorSetLayouts[i];//i = numempty
				temp.name = b.name;
				temp.set = b.set;
				temp.binding = b.binding;
				temp.type = b.type;
				temp.dataType = stringToSDDT(b.name);
				templates[ap->fileData.name][b.set].metaBindings.push_back(temp);
			}
		}
	}
	std::vector<size_t> DescriptorSystem::createDescriptorSet(std::string APname, size_t set) {
		if (!templates.contains(APname)) return {0};//should return an error saying set name not registered
		std::vector<size_t> indices;

		MetaDescriptorSet mSet = templates[APname][set];
		if (mSet.metaBindings[0].name == "NULL") return {0};//error of invalid set, if an empty array doesn't work out
		//then we need a specific number for an error set.  Like SIZE_MAX.  or...zero.

		for (size_t j = 0; j < mSet.metaBindings.size(); j++) {
			indices.push_back(freeID++);
			MetaDescriptor temp = mSet.metaBindings[j];
			temp.setIndex = sets.size();
			manifest.push_back(temp);
		}

		VkDescriptorSet tSet;
		VkDescriptorSetAllocateInfo allInfo;
		allInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allInfo.pNext = nullptr;
		allInfo.descriptorPool = globalPool;
		switch (set) {
		case 0:
			allInfo.descriptorPool = globalPool;
			break;
		case 1:
			allInfo.descriptorPool = framePool;
			break;
		case 2:
			allInfo.descriptorPool = drawPool;
			break;
		case 3:
			break;
		default:
			std::cout << "DANGER!!!";
		}
		allInfo.descriptorSetCount = 1;
		allInfo.pSetLayouts = &mSet.layout;
		vkAllocateDescriptorSets(vkState->vulkanDevice, &allInfo, &tSet);
		sets.push_back(tSet);

		
		return indices;
	}
	void DescriptorSystem::connectBufferDescriptors(std::vector<size_t> ID) {
		if (ID.size() == 0 || ID[0] == 0)return;
		std::vector<VkDescriptorBufferInfo> infos{ID.size()};
		std::vector<VkWriteDescriptorSet> writes{ID.size()};
		for (size_t i = 0; i < ID.size(); i++) {
			VulkanArray* tempBuffer;
			switch (manifest[ID[i]].dataType)
			{
			case SAGE_PER_FRAME_CAMERA_DATA:
					manifest[ID[i]].offset = PerFrameUBO.size();
					manifest[ID[i]].stride = sizeof(CameraData);
					tempBuffer = new VulkanArray(
						vkState,
						sizeof(CameraData),
						VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
					infos[i].buffer = tempBuffer->handle();
					infos[i].offset = 0;
					infos[i].range = manifest[ID[i]].stride;
					writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					writes[i].pNext = nullptr;
					writes[i].dstSet = sets[manifest[ID[i]].setIndex];
					writes[i].dstBinding = manifest[ID[i]].binding;
					writes[i].dstArrayElement = 0;
					writes[i].descriptorCount = 1;
					writes[i].descriptorType = manifest[ID[i]].type;
					writes[i].pBufferInfo = &infos[i];
					writes[i].pImageInfo = nullptr;
					writes[i].pTexelBufferView = nullptr;
					PerFrameUBO.push_back(tempBuffer);
					manifest[ID[i]].linkedResource = PerFrameUBO[manifest[ID[i]].offset];
					vkMapMemory(vkState->vulkanDevice, PerFrameUBO[manifest[ID[i]].offset]->data(), 0, PerFrameUBO[manifest[ID[i]].offset]->size(), 0, &manifest[ID[i]].pMappedMemory);
				break;
			case SAGE_PER_FRAME_SCENE_DATA:
					manifest[ID[i]].offset = PerFrameUBO.size();
					manifest[ID[i]].stride = sizeof(APSceneData);
					tempBuffer = new VulkanArray(
						vkState,
						sizeof(APSceneData),
						VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
					infos[i].buffer = tempBuffer->handle();
					infos[i].offset = 0;
					infos[i].range = manifest[ID[i]].stride;
					writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					writes[i].pNext = nullptr;
					writes[i].dstSet = sets[manifest[ID[i]].setIndex];
					writes[i].dstBinding = manifest[ID[i]].binding;
					writes[i].dstArrayElement = 0;
					writes[i].descriptorCount = 1;
					writes[i].descriptorType = manifest[ID[i]].type;
					writes[i].pBufferInfo = &infos[i];
					writes[i].pImageInfo = nullptr;
					writes[i].pTexelBufferView = nullptr;
					PerFrameUBO.push_back(tempBuffer);
					manifest[ID[i]].linkedResource = PerFrameUBO[manifest[ID[i]].offset];
					vkMapMemory(vkState->vulkanDevice, PerFrameUBO[manifest[ID[i]].offset]->data(), 0, PerFrameUBO[manifest[ID[i]].offset]->size(), 0, &manifest[ID[i]].pMappedMemory);
				break;
			case SAGE_PER_OBJECT_DATA:
					manifest[ID[i]].offset = 0;//tbd
					manifest[ID[i]].stride = infoStorageBuffer->size();//tbd
					infos[i].buffer = infoStorageBuffer->handle();
					infos[i].offset = 0;
					infos[i].range = manifest[ID[i]].stride;
					writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					writes[i].pNext = nullptr;
					writes[i].dstSet = sets[manifest[ID[i]].setIndex];
					writes[i].dstBinding = manifest[ID[i]].binding;
					writes[i].dstArrayElement = 0;
					writes[i].descriptorCount = 1;
					writes[i].descriptorType = manifest[ID[i]].type;
					writes[i].pBufferInfo = &infos[i];
					writes[i].pImageInfo = nullptr;
					writes[i].pTexelBufferView = nullptr;
					manifest[ID[i]].linkedResource = infoStorageBuffer;
					manifest[ID[i]].pMappedMemory = infoMappedMemory;
				break;
			default:
				std::cout << "Unrecognized Data Type, stop messing around!\n\n\n";
				break;
			}
		}
		vkUpdateDescriptorSets(vkState->vulkanDevice, writes.size(), writes.data(), 0, nullptr);
	}
	void DescriptorSystem::connectImageDescriptor(size_t ID, VkImageView& texture, VkSampler& sampler) {
		if (ID == 0) return;
		VkDescriptorImageInfo info;
		VkWriteDescriptorSet write;
		
		info.sampler = sampler;
		info.imageView = texture;
		info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.pNext = nullptr;
		write.dstSet = sets[manifest[ID].setIndex];
		write.dstBinding = manifest[ID].binding;
		write.dstArrayElement = 0;
		write.descriptorCount = 1;
		write.descriptorType = manifest[ID].type;
		write.pImageInfo = &info;
		write.pBufferInfo = nullptr;
		write.pTexelBufferView = nullptr;
		manifest[ID].pImageView = &texture;
		manifest[ID].pSampler = &sampler;
		
		vkUpdateDescriptorSets(vkState->vulkanDevice, 1, &write, 0, nullptr);
	}
	void DescriptorSystem::updateUBODescriptor(size_t ID, void* data) {
		if (ID == 0) return;
		MetaDescriptor toUpdate = manifest[ID];
		//CameraData* hack1 = (CameraData*)data;
		//APSceneData* hack2 = (APSceneData*)data;
		switch (toUpdate.type) {
		case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
			memcpy(toUpdate.pMappedMemory, data, toUpdate.stride);
			//if (ID % 2 == 0) memcpy(toUpdate.pMappedMemory, hack1, toUpdate.stride);
			//else memcpy(toUpdate.pMappedMemory, hack2, toUpdate.stride);
			break;
		default:
			std::cout << "Unsupported type!\n";
			return;
		}
	}
	void DescriptorSystem::updateGlobalDescriptor(size_t ID, PerObjectRenderData& data) {
		if (ID == 0) return;
		infoActual[ID] = data;
	}
	void DescriptorSystem::bindGlobalSet(size_t index, VkCommandBuffer& buffer, VkPipelineLayout& layout) {	
		if (index == 0) return;
		vkCmdBindDescriptorSets(
			buffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			layout,
			0,
			1,
			&sets[manifest[index].setIndex],
			0,
			nullptr);//  Might be work with dynamic offsetting.
	}
	void DescriptorSystem::bindPerFrameSet(size_t index, VkCommandBuffer& buffer, VkPipelineLayout& layout) {
		if (index == 0) return;
		vkCmdBindDescriptorSets(
			buffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			layout,
			1,
			1,
			&sets[manifest[index].setIndex],
			0,
			nullptr);//  Might be work with dynamic offsetting.
	}
	void DescriptorSystem::bindPerDrawSet(size_t index, VkCommandBuffer& buffer, VkPipelineLayout& layout) {
		if (index == 0) return;
		vkCmdBindDescriptorSets(
			buffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			layout,
			2,
			1,
			&sets[manifest[index].setIndex],
			0,
			nullptr);//  Might be work with dynamic offsetting.
	}

	AOSGraphicsConfigInfo::AOSGraphicsConfigInfo(uint64_t code) {
		uint16_t binMask = code & AOS_CONFIG_MASK_BINARY_FLAGS;
		primitiveRestart = AOS_GRAPHICS_CONFIG_PRIMITIVE_RESTART_BIT == (binMask & AOS_GRAPHICS_CONFIG_PRIMITIVE_RESTART_BIT);
		clampDepth = AOS_GRAPHICS_CONFIG_DEPTH_CLAMP_BIT == (binMask & AOS_GRAPHICS_CONFIG_DEPTH_CLAMP_BIT);
		discardRaster = AOS_GRAPHICS_CONFIG_DISCARD_RESULTS_BIT == (binMask & AOS_GRAPHICS_CONFIG_DISCARD_RESULTS_BIT);
		clockwiseFrontFace = AOS_GRAPHICS_CONFIG_CLOCKWISE_FRONT_BIT == (binMask & AOS_GRAPHICS_CONFIG_CLOCKWISE_FRONT_BIT);
		biasDepth = AOS_GRAPHICS_CONFIG_DEPTH_BIAS_BIT == (binMask & AOS_GRAPHICS_CONFIG_DEPTH_BIAS_BIT);
		sampleShading = AOS_GRAPHICS_CONFIG_SAMPLE_SHADING_BIT == (binMask & AOS_GRAPHICS_CONFIG_SAMPLE_SHADING_BIT);
		alphaToCoverage = AOS_GRAPHICS_CONFIG_ALPHA_TO_COVERAGE_BIT == (binMask & AOS_GRAPHICS_CONFIG_ALPHA_TO_COVERAGE_BIT);
		alphaToOne = AOS_GRAPHICS_CONFIG_ALPHA_TO_ONE_BIT == (binMask & AOS_GRAPHICS_CONFIG_ALPHA_TO_ONE_BIT);
		depthTest = AOS_GRAPHICS_CONFIG_DEPTH_TEST_BIT == (binMask & AOS_GRAPHICS_CONFIG_DEPTH_TEST_BIT);
		depthWrite = AOS_GRAPHICS_CONFIG_DEPTH_WRITE_BIT == (binMask & AOS_GRAPHICS_CONFIG_DEPTH_WRITE_BIT);
		depthBoundsTest = AOS_GRAPHICS_CONFIG_DEPTH_BOUNDS_BIT == (binMask & AOS_GRAPHICS_CONFIG_DEPTH_BOUNDS_BIT);
		stencilTest = AOS_GRAPHICS_CONFIG_STENCIL_TEST_BIT == (binMask & AOS_GRAPHICS_CONFIG_STENCIL_TEST_BIT);
		logicBlending = AOS_GRAPHICS_CONFIG_LOGIC_BLENDING_BIT == (binMask & AOS_GRAPHICS_CONFIG_LOGIC_BLENDING_BIT);
		attachmentState.blendEnable = AOS_GRAPHICS_CONFIG_BLENDING_ENABLE_BIT == (binMask & AOS_GRAPHICS_CONFIG_BLENDING_ENABLE_BIT);
		dynamicDepthAttachment = AOS_GRAPHICS_CONFIG_DYN_DEPTH_BIT == (binMask & AOS_GRAPHICS_CONFIG_DYN_DEPTH_BIT);
		dynamicStencilAttachment = AOS_GRAPHICS_CONFIG_DYN_STENCIL_BIT == (binMask & AOS_GRAPHICS_CONFIG_DYN_STENCIL_BIT);

		primitiveTopology = (VkPrimitiveTopology)((code & AOS_CONFIG_MASK_INPUT_VERTEX_TOPOLOGY) >> 16);
		polygonMode = (VkPolygonMode)((code & AOS_CONFIG_MASK_RASTERIZER_POLYGON_MODE) >> 20);
		cullMode = (VkCullModeFlagBits)((code & AOS_CONFIG_MASK_RASTERIZER_CULL_MODE) >> 22);
		sampleCount = (VkSampleCountFlagBits)std::pow(2,((code & AOS_CONFIG_MASK_MULTISAMPLE_COUNT) >> 24)-1);
		depthCompare = (VkCompareOp)((code & AOS_CONFIG_MASK_DEPTH_COMPARE_OP) >> 27);
		blendingLogic = (VkLogicOp)((code & AOS_CONFIG_MASK_BLENDING_LOGIC_OP) >> 30);
		attachmentState.colorWriteMask = (VkColorComponentFlags)((code & AOS_CONFIG_MASK_COLOR_WRITE_MASK) >> 34);
		attachmentState.colorBlendOp = (VkBlendOp)((code & AOS_CONFIG_MASK_COLOR_BLEND_OP) >> 38);
		attachmentState.alphaBlendOp = (VkBlendOp)((code & AOS_CONFIG_MASK_ALPHA_BLEND_OP) >> 41);
		attachmentState.srcColorBlendFactor = (VkBlendFactor)((code & AOS_CONFIG_MASK_COLOR_BLEND_SRC) >> 44);
		attachmentState.dstColorBlendFactor = (VkBlendFactor)((code & AOS_CONFIG_MASK_COLOR_BLEND_DST) >> 49);
		attachmentState.srcAlphaBlendFactor = (VkBlendFactor)((code & AOS_CONFIG_MASK_ALPHA_BLEND_SRC) >> 54);
		attachmentState.dstAlphaBlendFactor = (VkBlendFactor)((code & AOS_CONFIG_MASK_ALPHA_BLEND_DST) >> 59);
	}
	uint64_t AOSGraphicsConfigInfo::bake() {
		uint64_t returnCode = (uint64_t)primitiveRestart;
		returnCode |= (uint64_t)clampDepth << 1;
		returnCode |= (uint64_t)discardRaster << 2;
		returnCode |= (uint64_t)clockwiseFrontFace << 3;
		returnCode |= (uint64_t)biasDepth << 4;
		returnCode |= (uint64_t)sampleShading << 5;
		returnCode |= (uint64_t)alphaToCoverage << 6;
		returnCode |= (uint64_t)alphaToOne << 7;
		returnCode |= (uint64_t)depthTest << 8;
		returnCode |= (uint64_t)depthWrite << 9;
		returnCode |= (uint64_t)depthBoundsTest << 10;
		returnCode |= (uint64_t)stencilTest << 11;
		returnCode |= (uint64_t)logicBlending << 12;
		returnCode |= (uint64_t)attachmentState.blendEnable << 13;
		returnCode |= (uint64_t)dynamicDepthAttachment << 14;
		returnCode |= (uint64_t)dynamicStencilAttachment << 15;
		returnCode |= (uint64_t)primitiveTopology << 16;
		returnCode |= (uint64_t)polygonMode << 20;
		returnCode |= (uint64_t)cullMode << 22;
		uint64_t count = 1;
		for (VkSampleCountFlags f = 1; !(f & sampleCount); f <<= 1) {
			count++;
		}
		returnCode |= count << 24;
		returnCode |= (uint64_t)depthCompare << 27;
		returnCode |= (uint64_t)blendingLogic << 30;
		returnCode |= (uint64_t)attachmentState.colorWriteMask << 34;
		returnCode |= (uint64_t)attachmentState.colorBlendOp << 38;
		returnCode |= (uint64_t)attachmentState.alphaBlendOp << 41;
		returnCode |= (uint64_t)attachmentState.srcColorBlendFactor << 44;
		returnCode |= (uint64_t)attachmentState.dstColorBlendFactor << 49;
		returnCode |= (uint64_t)attachmentState.srcAlphaBlendFactor << 54;
		returnCode |= (uint64_t)attachmentState.dstAlphaBlendFactor << 59;
		return returnCode;
	}

	AcceleratorOperationStateConfig::AcceleratorOperationStateConfig(VulkanState* vks) {
		vkState = vks;
	}
	void AcceleratorOperationStateConfig::reset(PipelineUse pu) {

		/*
		case COMPUTE_PIPELINE:
			computePipelinePrototype.pNext = nullptr;
			computePipelinePrototype.flags = 0;
			computePipelinePrototype.basePipelineHandle = VK_NULL_HANDLE;
			computePipelinePrototype.basePipelineIndex = -1;
			break;
		*/
			gcbAttachments.clear();
			gdrAttachmentFormats.clear();
			viewports.clear();
			scissors.clear();
			dynamicStates.clear();
			multisampleMasks.clear();
			
			gviStates.clear();
			giaStates.clear();
			gtsStates.clear();
			gvpStates.clear();
			grtStates.clear();
			gmsStates.clear();
			gdsStates.clear();
			gcbStates.clear();
			gdyStages.clear();
			gdyrInfos.clear();
/**
			ALpha Blending For Reference:

			VkPipelineColorBlendAttachmentState alphaBlendAttachment;
			alphaBlendAttachment.blendEnable = VK_TRUE;
			alphaBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			alphaBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			alphaBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
			alphaBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			alphaBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			alphaBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
			alphaBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
/**
			Pipeline Setup Vars For Reference:

			graphicsVertexAssemblerState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			graphicsVertexAssemblerState.primitiveRestartEnable = VK_FALSE;


			graphicsTessellatorState.patchControlPoints = 69;

			graphicsViewportState.viewportCount = viewports.size();
			graphicsViewportState.pViewports = viewports.data();//myViewport
			graphicsViewportState.scissorCount = scissors.size();
			graphicsViewportState.pScissors = scissors.data();//myScissor

			graphicsRasterizerState.depthClampEnable = VK_FALSE;
			graphicsRasterizerState.rasterizerDiscardEnable = VK_FALSE;//only relevant if you use as a pseudo-compute pipeline and don't actually render
			graphicsRasterizerState.polygonMode = VK_POLYGON_MODE_FILL;//where you'd toggle wireframe
			graphicsRasterizerState.cullMode = VK_CULL_MODE_BACK_BIT;
			graphicsRasterizerState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
			graphicsRasterizerState.depthBiasEnable = VK_FALSE;
			graphicsRasterizerState.depthBiasConstantFactor = 0.f;
			graphicsRasterizerState.depthBiasClamp = 0.f;
			graphicsRasterizerState.depthBiasSlopeFactor = 0.f;
			graphicsRasterizerState.lineWidth = 1.f;

			graphicsMultisamplerState.rasterizationSamples = vkState->maxMSAA;//smooths out edges
			graphicsMultisamplerState.sampleShadingEnable = VK_FALSE;//smooths out interiors
			graphicsMultisamplerState.minSampleShading = 1.f;//smaller means smoother
			graphicsMultisamplerState.pSampleMask = nullptr;
			graphicsMultisamplerState.alphaToCoverageEnable = VK_FALSE;
			graphicsMultisamplerState.alphaToOneEnable = VK_FALSE;

			graphicsDepthStencilState.depthTestEnable = VK_TRUE;
			graphicsDepthStencilState.depthWriteEnable = VK_TRUE;
			graphicsDepthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
			graphicsDepthStencilState.depthBoundsTestEnable = VK_FALSE;
			graphicsDepthStencilState.minDepthBounds = 0.f;
			graphicsDepthStencilState.maxDepthBounds = 1.f;
			graphicsDepthStencilState.stencilTestEnable = VK_FALSE;
			graphicsDepthStencilState.front = {};
			graphicsDepthStencilState.back = {};


			graphicsFramebufferBlendingState.logicOpEnable = VK_FALSE;
			graphicsFramebufferBlendingState.logicOp = VK_LOGIC_OP_COPY;
			graphicsFramebufferBlendingState.attachmentCount = graphicsBlendingAttachments.size();
			graphicsFramebufferBlendingState.pAttachments = graphicsBlendingAttachments.data();
			graphicsFramebufferBlendingState.blendConstants[0] = 0.f;//R
			graphicsFramebufferBlendingState.blendConstants[1] = 0.f;//G
			graphicsFramebufferBlendingState.blendConstants[2] = 0.f;//B
			graphicsFramebufferBlendingState.blendConstants[3] = 0.f;//A

			graphicsDynamicStages.dynamicStateCount = static_cast<uint32_t>(vkState->graphicsPipelinesDynamicStates.size());
			graphicsDynamicStages.pDynamicStates = vkState->graphicsPipelinesDynamicStates.data();

			graphicsDynamicRenderInfo.viewMask = 0;
			graphicsDynamicRenderInfo.colorAttachmentCount = dynamicColorAttachmentFormats.size();
			graphicsDynamicRenderInfo.pColorAttachmentFormats = dynamicColorAttachmentFormats.data();
			graphicsDynamicRenderInfo.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT;
			graphicsDynamicRenderInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;
/**/
	}
	/* Kept around just in case
	void AcceleratorOperationStateConfig::setGraphicsConfigStates(const AOSGraphicsConfigInfo& helper) {
		graphicsVertexAssemblerState.topology = helper.primitiveTopology;
		graphicsVertexAssemblerState.primitiveRestartEnable = helper.primitiveRestart;

		graphicsRasterizerState.depthClampEnable = helper.clampDepth;
		graphicsRasterizerState.rasterizerDiscardEnable = helper.discardRaster;
		graphicsRasterizerState.polygonMode = helper.polygonMode;
		graphicsRasterizerState.cullMode = helper.cullMode;
		graphicsRasterizerState.frontFace = (VkFrontFace)helper.clockwiseFrontFace;
		graphicsRasterizerState.depthBiasEnable = helper.biasDepth;

		graphicsMultisamplerState.rasterizationSamples = helper.sampleCount <= vkState->maxMSAA ? helper.sampleCount : vkState->maxMSAA;
		//we have to hack it like this until we implement a (better) way to synch multisampling values across everything that uses them.
		graphicsMultisamplerState.sampleShadingEnable = helper.sampleShading;
		graphicsMultisamplerState.alphaToCoverageEnable = helper.alphaToCoverage;
		graphicsMultisamplerState.alphaToOneEnable = helper.alphaToOne;
		
		graphicsDepthStencilState.depthTestEnable = helper.depthTest;
		graphicsDepthStencilState.depthWriteEnable = helper.depthWrite;
		graphicsDepthStencilState.depthCompareOp = helper.depthCompare;
		graphicsDepthStencilState.depthBoundsTestEnable = helper.depthBoundsTest;
		graphicsDepthStencilState.stencilTestEnable = helper.stencilTest;

		graphicsFramebufferBlendingState.logicOpEnable = helper.logicBlending;
		graphicsFramebufferBlendingState.logicOp = helper.blendingLogic;
		uint32_t safety = helper.attachmentState.colorWriteMask == 0 ? graphicsBlendingAttachments[0].colorWriteMask : helper.attachmentState.colorWriteMask;
		graphicsBlendingAttachments[0] = helper.attachmentState;
		graphicsBlendingAttachments[0].colorWriteMask = safety;

		graphicsDynamicRenderInfo.depthAttachmentFormat =  helper.dynamicDepthAttachment ? VK_FORMAT_D32_SFLOAT : VK_FORMAT_UNDEFINED;
		graphicsDynamicRenderInfo.stencilAttachmentFormat= helper.dynamicStencilAttachment ? VK_FORMAT_S8_UINT : VK_FORMAT_UNDEFINED;
	}
	void AcceleratorOperationStateConfig::setGraphicsConfigNumerics(const AOSGraphicsConfigNumerics& helper) {
		graphicsTessellatorState.patchControlPoints = helper.tessellatorPatchControlPoints;

		graphicsRasterizerState.depthBiasConstantFactor = helper.rasterizerDepthBiasConstant;
		graphicsRasterizerState.depthBiasClamp = helper.rasterizerDepthBiasClamp;
		graphicsRasterizerState.depthBiasSlopeFactor = helper.rasterizerDepthBiasSlopeFactor;
		graphicsRasterizerState.lineWidth = helper.rasterizerLineWidth;

		graphicsMultisamplerState.minSampleShading = helper.multisamplerMinSampleShading;
		
		graphicsDepthStencilState.minDepthBounds = helper.minDepthBounds;
		graphicsDepthStencilState.maxDepthBounds = helper.maxDepthBounds;

		graphicsFramebufferBlendingState.blendConstants[0] = helper.blendConstantR;
		graphicsFramebufferBlendingState.blendConstants[1] = helper.blendConstantG;
		graphicsFramebufferBlendingState.blendConstants[2] = helper.blendConstantB;
		graphicsFramebufferBlendingState.blendConstants[3] = helper.blendConstantA;

		graphicsDynamicRenderInfo.viewMask = helper.dynamicViewMask;
	}
	void AcceleratorOperationStateConfig::addGraphicsConfigStructs(AOSGraphicsConfigStructs& helper) {
		if (helper.viewports.size() >= viewports.size()) viewports = std::move(helper.viewports);
		graphicsViewportState.viewportCount = viewports.size();
		graphicsViewportState.pViewports = viewports.data();
		if (helper.scissors.size() >= scissors.size()) scissors = std::move(helper.scissors);
		graphicsViewportState.scissorCount = scissors.size();
		graphicsViewportState.pScissors = scissors.data();

		if (helper.multisamplerSampleMasks.size() != 0) vCombine(multisampleMasks, helper.multisamplerSampleMasks);
		graphicsMultisamplerState.pSampleMask = multisampleMasks.data();

		graphicsDepthStencilState.front = helper.frontFaceStencilOp;
		graphicsDepthStencilState.back = helper.backFaceStencilOp;

		vCombine(graphicsBlendingAttachments, helper.additionalColorBlendAttachments);
		graphicsFramebufferBlendingState.attachmentCount = graphicsBlendingAttachments.size();
		graphicsFramebufferBlendingState.pAttachments = graphicsBlendingAttachments.data();

		vCombine(dynamicColorAttachmentFormats, helper.additionalDynamicColorAttachmentFormats);
		graphicsDynamicRenderInfo.colorAttachmentCount = dynamicColorAttachmentFormats.size();
		graphicsDynamicRenderInfo.pColorAttachmentFormats = dynamicColorAttachmentFormats.data();
	}
	*/
	bool AcceleratorOperationStateConfig::configureGraphicsPipelines(
		AccelerationProgram* program,
		std::vector <std::string>& configNames,
		std::vector<VkGraphicsPipelineCreateInfo>& returnInfos
	){
		if (giaStates.size() > 0)return false;
		size_t arrSizes = configNames.size();
		returnInfos.clear();
		returnInfos.resize(arrSizes, {});
		gviStates.resize(arrSizes, {});
		giaStates.resize(arrSizes, {});
		gtsStates.resize(arrSizes, {});
		gvpStates.resize(arrSizes, {});
		grtStates.resize(arrSizes, {});
		gmsStates.resize(arrSizes, {});
		gdsStates.resize(arrSizes, {});
		gcbStates.resize(arrSizes, {});
		gdyStages.resize(arrSizes, {});
		gdyrInfos.resize(arrSizes, {});
		gcbAttachments.resize(arrSizes);
		gdrAttachmentFormats.resize(arrSizes);
		viewports.resize(arrSizes);
		scissors.resize(arrSizes);
		dynamicStates.resize(arrSizes);
		multisampleMasks.resize(arrSizes);

		for (size_t fileNum = 0; fileNum < arrSizes; fileNum++) {
			std::string filepath = "Configs/Pipelines/" + configNames[fileNum] + ".cfg";
			std::vector<std::string> lines;

			if (!readWholeTextFile(filepath, lines)) return false;
			std::sort(lines.begin(), lines.end());

			returnInfos[fileNum].sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			gviStates[fileNum].sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			giaStates[fileNum].sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			gtsStates[fileNum].sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
			gvpStates[fileNum].sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			grtStates[fileNum].sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			gmsStates[fileNum].sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			gdsStates[fileNum].sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			gcbStates[fileNum].sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			gdyStages[fileNum].sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			gdyrInfos[fileNum] .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
			gviStates[fileNum].pNext = nullptr;
			giaStates[fileNum].pNext = nullptr;
			gtsStates[fileNum].pNext = nullptr;
			gvpStates[fileNum].pNext = nullptr;
			grtStates[fileNum].pNext = nullptr;
			gmsStates[fileNum].pNext = nullptr;
			gdsStates[fileNum].pNext = nullptr;
			gcbStates[fileNum].pNext = nullptr;
			gdsStates[fileNum].pNext = nullptr;
			gdyrInfos[fileNum].pNext = nullptr;
			returnInfos[fileNum].flags = 0;
			gviStates[fileNum].flags = 0;
			giaStates[fileNum].flags = 0;
			gtsStates[fileNum].flags = 0;
			gvpStates[fileNum].flags = 0;
			grtStates[fileNum].flags = 0;
			gmsStates[fileNum].flags = 0;
			gdsStates[fileNum].flags = 0;
			gcbStates[fileNum].flags = 0;
			gdsStates[fileNum].flags = 0;

			returnInfos[fileNum].pNext = &gdyrInfos[fileNum];
			returnInfos[fileNum].flags = 0;
			returnInfos[fileNum].pVertexInputState = &gviStates[fileNum];
			returnInfos[fileNum].pInputAssemblyState = &giaStates[fileNum];
			returnInfos[fileNum].pTessellationState = &gtsStates[fileNum];//not yet?
			returnInfos[fileNum].pViewportState = &gvpStates[fileNum];
			returnInfos[fileNum].pRasterizationState = &grtStates[fileNum];
			returnInfos[fileNum].pMultisampleState = &gmsStates[fileNum];
			returnInfos[fileNum].pDepthStencilState = &gdsStates[fileNum];
			returnInfos[fileNum].pColorBlendState = &gcbStates[fileNum];
			returnInfos[fileNum].pDynamicState = &gdyStages[fileNum];
			returnInfos[fileNum].renderPass = VK_NULL_HANDLE;
			returnInfos[fileNum].basePipelineHandle = VK_NULL_HANDLE;
			returnInfos[fileNum].basePipelineIndex = -1;

			returnInfos[fileNum].stageCount = static_cast<uint32_t>(program->shaderStages.size());
			returnInfos[fileNum].pStages = program->shaderStages.data();
			returnInfos[fileNum].layout = program->pipelineLayout;

			gviStates[fileNum].vertexBindingDescriptionCount = static_cast<uint32_t>(program->vertInfo.vertexBindingData.size());
			gviStates[fileNum].pVertexBindingDescriptions = program->vertInfo.vertexBindingData.data();
			gviStates[fileNum].vertexAttributeDescriptionCount = static_cast<uint32_t>(program->vertInfo.vertexInputData.size());
			gviStates[fileNum].pVertexAttributeDescriptions = program->vertInfo.vertexInputData.data();


			viewports[fileNum].push_back(
				{ 0, 0,
				800, 600,
				0.f, 1.f }
			);
			scissors[fileNum].push_back({{0 , 0},{800 , 600}});

			gcbAttachments[fileNum].push_back(
				{ VK_FALSE,
				VK_BLEND_FACTOR_ONE,
				VK_BLEND_FACTOR_ZERO,
				VK_BLEND_OP_ADD,
				VK_BLEND_FACTOR_ONE,
				VK_BLEND_FACTOR_ZERO,
				VK_BLEND_OP_ADD,
				VK_COLOR_COMPONENT_R_BIT |
				VK_COLOR_COMPONENT_G_BIT |
				VK_COLOR_COMPONENT_B_BIT |
				VK_COLOR_COMPONENT_A_BIT
				}
			);

			gdrAttachmentFormats[fileNum].push_back(vkState->chosenSurfaceFormat.format);

			size_t i = 1;
			size_t fulcrum = lines[i].find('=');
			std::string op = lines[i].substr(0, fulcrum);
			std::string val = lines[i].substr(fulcrum + 1);

			std::vector<VkPipelineColorBlendAttachmentState> additionalColorBlendAttachments;
			std::vector<VkFormat> additionalDynamicColorAttachmentFormats;

			if (op == "addAlphaBlendOps") {
				size_t count = 0;
				while (val.find(' ') != std::string::npos) {
					additionalColorBlendAttachments.push_back({});
					additionalColorBlendAttachments[count++].alphaBlendOp =
						stringToVkBlendOp(val.substr(0, val.find(' ')));
					val = val.substr(val.find(' ') + 1);
				}
				additionalColorBlendAttachments.push_back({});
				additionalColorBlendAttachments[count].alphaBlendOp = stringToVkBlendOp(val);

				val = lines[++i].substr(lines[i].find('=') + 1);

				for (auto& a : additionalColorBlendAttachments) {
					a.blendEnable = std::stoul(val.substr(0, val.find(' ')));
					val = val.substr(val.find(' ') + 1);
					//will overflow to 0 on last line, which gives us what we want anyways
					//if you wonder why this works out
				}

				val = lines[++i].substr(lines[i].find('=') + 1);

				for (auto& a : additionalColorBlendAttachments) {
					a.colorBlendOp = stringToVkBlendOp(val.substr(0, val.find(' ')));
					val = val.substr(val.find(' ') + 1);
				}

				val = lines[++i].substr(lines[i].find('=') + 1);

				for (auto& a : additionalColorBlendAttachments) {
					std::string mask = val.substr(0, val.find(' '));
					std::reverse(mask.begin(), mask.end());
					a.colorWriteMask = std::bitset<4>(mask).to_ulong();
					val = val.substr(val.find(' ') + 1);
				}

				val = lines[++i].substr(lines[i].find('=') + 1);

				for (auto& a : additionalColorBlendAttachments) {
					a.dstAlphaBlendFactor = stringToVkBlendFactor(val.substr(0, val.find(' ')));
					val = val.substr(val.find(' ') + 1);
				}

				val = lines[++i].substr(lines[i].find('=') + 1);

				for (auto& a : additionalColorBlendAttachments) {
					a.dstColorBlendFactor = stringToVkBlendFactor(val.substr(0, val.find(' ')));
					val = val.substr(val.find(' ') + 1);
				}

				val = lines[++i].substr(lines[i].find('=') + 1);

				for (auto& a : additionalColorBlendAttachments) {
					a.srcAlphaBlendFactor = stringToVkBlendFactor(val.substr(0, val.find(' ')));
					val = val.substr(val.find(' ') + 1);
				}

				val = lines[++i].substr(lines[i].find('=') + 1);

				for (auto& a : additionalColorBlendAttachments) {
					a.srcColorBlendFactor = stringToVkBlendFactor(val.substr(0, val.find(' ')));
					val = val.substr(val.find(' ') + 1);
				}

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "alphaBlendingOperation" || op == "alphaBlendOp") {
				gcbAttachments[fileNum][0].alphaBlendOp = stringToVkBlendOp(val);

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "backFaceStencilCompareMask" || op == "bfsCompareMask") {
				gdsStates[fileNum].back.compareMask = std::stoul(val);

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "backFaceStencilComparisonOperation" || op == "bfsCompareOp") {
				gdsStates[fileNum].back.compareOp = stringToVkCompareOp(val);

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "backFaceStencilDepthFailOperation" || op == "bfsDepthFailOp") {
				gdsStates[fileNum].back.depthFailOp = stringToVkStencilOp(val);

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "backFaceStencilFailOperation" || op == "bfsFailOp") {
				gdsStates[fileNum].back.failOp = stringToVkStencilOp(val);

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "backFaceStencilPassOperation" || op == "bfsPassOp") {
				gdsStates[fileNum].back.passOp = stringToVkStencilOp(val);

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "backFaceStencilReference" || op == "bfsReference") {
				gdsStates[fileNum].back.reference = std::stoul(val);

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "backFaceStencilWriteMask" || op == "bfsWriteMask") {
				gdsStates[fileNum].back.writeMask = std::stoul(val);

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "blendConstantA") {
				gcbStates[fileNum].blendConstants[3] = std::stof(val);

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "blendConstantB") {
				gcbStates[fileNum].blendConstants[2] = std::stof(val);

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "blendConstantG") {
				gcbStates[fileNum].blendConstants[1] = std::stof(val);

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "blendConstantR") {
				gcbStates[fileNum].blendConstants[0] = std::stof(val);

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "blendingColorWriteMask") {
				std::reverse(val.begin(), val.end());
				gcbAttachments[fileNum][0].colorWriteMask = std::bitset<4>(val).to_ulong();

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "blendingLogicOperation" || op == "blendingLogicOp") {
				gcbStates[fileNum].logicOp = stringToVkLogicOp(val);

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "colorBlendingOperation" || op == "colorBlendOp") {
				gcbAttachments[fileNum][0].colorBlendOp = stringToVkBlendOp(val);

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "depthBiasClamp") {
				grtStates[fileNum].depthBiasClamp = std::stof(val);

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "depthBiasConstant") {
				grtStates[fileNum].depthBiasConstantFactor = std::stof(val);

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "depthBiasSlope" || op == "depthBiasSlopeFactor") {
				grtStates[fileNum].depthBiasSlopeFactor = std::stof(val);

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "depthComparisonOperation" || op == "depthCompareOp") {
				gdsStates[fileNum].depthCompareOp = stringToVkCompareOp(val);

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "dstAlphaBlendingFactor") {
				gcbAttachments[fileNum][0].dstAlphaBlendFactor = stringToVkBlendFactor(val);

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "dstColorBlendingFactor") {
				gcbAttachments[fileNum][0].dstColorBlendFactor = stringToVkBlendFactor(val);

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "dynamicColorAttachmentFormats") {
				while (val.find(' ') != std::string::npos) {
					std::string comp = val.substr(0, val.find(' '));
					if (comp == "auto" || comp == "swapchain") additionalDynamicColorAttachmentFormats.push_back(vkState->chosenSurfaceFormat.format);
					else additionalDynamicColorAttachmentFormats.push_back((VkFormat)std::stoul(comp));
					val = val.substr(val.find(' ') + 1);
				}
				if (val == "auto" || val == "swapchain") additionalDynamicColorAttachmentFormats.push_back(vkState->chosenSurfaceFormat.format);
				else additionalDynamicColorAttachmentFormats.push_back((VkFormat)std::stoul(val));

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}			
			if (op == "dynamicStates") {
				while (val.find(' ') != std::string::npos) {
					dynamicStates[fileNum].push_back(stringToVkDynamicState(val.substr(0, val.find(' '))));
					val = val.substr(val.find(' ') + 1);
				}
				dynamicStates[fileNum].push_back(stringToVkDynamicState(val));

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}			
			if (op == "dynamicViewMask") {
				gdyrInfos[fileNum].viewMask = std::stoul(val);

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "enableAlphaToCoverage") {
				gmsStates[fileNum].alphaToCoverageEnable = std::stoul(val);

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "enableAlphaToOne") {
				gmsStates[fileNum].alphaToOneEnable = std::stoul(val);

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "enableBlendingByLogic") {
				gcbStates[fileNum].logicOpEnable = std::stoul(val);

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "enableColorBlending") {
				gcbAttachments[fileNum][0].blendEnable = std::stoul(val);

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "enableDepthBiasing") {
				grtStates[fileNum].depthBiasEnable = std::stoul(val);

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "enableDepthBoundsTest") {
				gdsStates[fileNum].depthBoundsTestEnable = std::stoul(val);

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "enableDepthClamping") {
				grtStates[fileNum].depthClampEnable = std::stoul(val);

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "enableDepthTesting") {
				gdsStates[fileNum].depthTestEnable = std::stoul(val);

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "enableDepthWriting") {
				gdsStates[fileNum].depthWriteEnable = std::stoul(val);

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "enablePrimitiveRestarting") {
				giaStates[fileNum].primitiveRestartEnable = std::stoul(val);

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "enableRasterizationDiscarding") {
				grtStates[fileNum].rasterizerDiscardEnable = std::stoul(val);

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "enableSampleShading") {
				gmsStates[fileNum].sampleShadingEnable = std::stoul(val);

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "enableStencilTest") {
				gdsStates[fileNum].stencilTestEnable = std::stoul(val);

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "frontFaceStencilCompareMask" || op == "ffsCompareMask") {
				gdsStates[fileNum].front.compareMask = std::stoul(val);

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "frontFaceStencilCompareOp" || op == "ffsCompareOp") {
				gdsStates[fileNum].front.compareOp = stringToVkCompareOp(val);

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "frontFaceStencilDepthFailOp" || op == "ffsDepthFailOp") {
				gdsStates[fileNum].front.depthFailOp = stringToVkStencilOp(val);

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "frontFaceStencilFailOp" || op == "ffsFailOp") {
				gdsStates[fileNum].front.failOp = stringToVkStencilOp(val);

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "frontFaceStencilPassOp" || op == "ffsPassOp") {
				gdsStates[fileNum].front.passOp = stringToVkStencilOp(val);

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "frontFaceStencilReference" || op == "ffsReference") {
				gdsStates[fileNum].front.reference = std::stoul(val);

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "frontFaceStencilWriteMask" || op == "ffsWriteMask") {
				gdsStates[fileNum].front.writeMask = std::stoul(val);
				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "hasDynamicDepthAttachment") {
				gdyrInfos[fileNum].depthAttachmentFormat = std::stoul(val)? VK_FORMAT_D32_SFLOAT : VK_FORMAT_UNDEFINED;

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "hasDynamicStencilAttachment") {
				gdyrInfos[fileNum].stencilAttachmentFormat = std::stoul(val) ? VK_FORMAT_S8_UINT : VK_FORMAT_UNDEFINED;

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "isFrontFaceClockwise") {
				grtStates[fileNum].frontFace = (VkFrontFace)std::stoul(val);

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "maxDepthBound" || op == "maxDepthBoundary") {
				gdsStates[fileNum].maxDepthBounds = std::stof(val);

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "minDepthBound" || op == "minDepthBoundary") {
				gdsStates[fileNum].minDepthBounds = std::stof(val);

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "minSampleShading") {
				gmsStates[fileNum].minSampleShading = std::stof(val);

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "multisampleCount") {
				VkSampleCountFlagBits samps = (VkSampleCountFlagBits)std::stoul(val);
				gmsStates[fileNum].rasterizationSamples = /*samps <= vkState->maxMSAA ? samps : */hardwareInfo.graphicsCardInfo.maxMSAA;;
//we have to hack it like this until we implement a (better) way to synch multisampling values across everything that uses them.
				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "multisampleSampleMasks") {
				while (val.find(' ') != std::string::npos) {
					multisampleMasks[fileNum].push_back(std::bitset<7>(val.substr(0, val.find(' '))).to_ulong());
					val = val.substr(val.find(' ') + 1);
				}
				multisampleMasks[fileNum].push_back(std::bitset<7>(val).to_ulong());

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "numPatchControlPoints") {
				gtsStates[fileNum].patchControlPoints = std::stoul(val);

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "polygonCullFace") {
				if (val == "front") grtStates[fileNum].cullMode = VK_CULL_MODE_FRONT_BIT;
				else if (val == "back") grtStates[fileNum].cullMode = VK_CULL_MODE_BACK_BIT;
				else if (val == "both") grtStates[fileNum].cullMode = VK_CULL_MODE_FRONT_AND_BACK;
				else grtStates[fileNum].cullMode = VK_CULL_MODE_NONE;

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "polygonRepresentation") {
				if (val == "points") grtStates[fileNum].polygonMode = VK_POLYGON_MODE_POINT;
				else if (val == "lines") grtStates[fileNum].polygonMode = VK_POLYGON_MODE_LINE;
				else grtStates[fileNum].polygonMode = VK_POLYGON_MODE_FILL;

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "rasterLineWidth") {
				grtStates[fileNum].lineWidth = std::stof(val);

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "scissorHeights") {
				size_t count = 0;
				scissors[fileNum].clear();
				while (val.find(' ') != std::string::npos) {
					scissors[fileNum].push_back({});
					scissors[fileNum][count++].extent.height = std::stof(val.substr(0, val.find(' ')));
					val = val.substr(val.find(' ') + 1);
				}
				scissors[fileNum].push_back({});
				scissors[fileNum][count].extent.height = std::stof(val);

				val = lines[++i].substr(lines[i].find('=') + 1);

				for (auto& s : scissors[fileNum]) {
					s.extent.width = std::stof(val.substr(0, val.find(' ')));
					val = val.substr(val.find(' ') + 1);
				}

				val = lines[++i].substr(lines[i].find('=') + 1);

				for (auto& s : scissors[fileNum]) {
					s.offset.x = std::stof(val.substr(0, val.find(' ')));
					val = val.substr(val.find(' ') + 1);
				}
				val = lines[++i].substr(lines[i].find('=') + 1);

				for (auto& s : scissors[fileNum]) {
					s.offset.y = std::stof(val.substr(0, val.find(' ')));
					val = val.substr(val.find(' ') + 1);
				}

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "srcAlphaBlendingFactor") {
				gcbAttachments[fileNum][0].srcAlphaBlendFactor = stringToVkBlendFactor(val);

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "srcColorBlendingFactor") {
				gcbAttachments[fileNum][0].srcColorBlendFactor = stringToVkBlendFactor(val);

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "vertexListTopology") {
				if (val == "triangles") giaStates[fileNum].topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
				else if (val == "lines") giaStates[fileNum].topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
				else if (val == "lineStrips") giaStates[fileNum].topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
				else if (val == "triangleFans") giaStates[fileNum].topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
				else if (val == "triangleStrips") giaStates[fileNum].topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
				else if (val == "adjacentLines") giaStates[fileNum].topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
				else if (val == "adjacentTriangles") giaStates[fileNum].topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
				else if (val == "adjacentLineStrips") giaStates[fileNum].topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
				else if (val == "adjacentTriangleStrips") giaStates[fileNum].topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
				else if (val == "patches") giaStates[fileNum].topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
				else giaStates[fileNum].topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;

				if (++i == lines.size()) goto pipelineDone;
				fulcrum = lines[i].find('=');
				op = lines[i].substr(0, fulcrum);
				val = lines[i].substr(fulcrum + 1);
			}
			if (op == "viewportHeights") {
				viewports[fileNum].clear();
				size_t count = 0;
				while (val.find(' ') != std::string::npos) {
					viewports[fileNum].push_back({});
					viewports[fileNum][count++].height = std::stof(val.substr(0, val.find(' ')));
					val = val.substr(val.find(' ') + 1);
				}
				viewports[fileNum].push_back({});
				viewports[fileNum][count].height = std::stof(val);

				val = lines[++i].substr(lines[i].find('=') + 1);

				for (auto& v : viewports[fileNum]) {
					v.maxDepth = std::stof(val.substr(0, val.find(' ')));
					val = val.substr(val.find(' ') + 1);
				}

				val = lines[++i].substr(lines[i].find('=') + 1);

				for (auto& v : viewports[fileNum]) {
					v.minDepth = std::stof(val.substr(0, val.find(' ')));
					val = val.substr(val.find(' ') + 1);
				}

				val = lines[++i].substr(lines[i].find('=') + 1);

				for (auto& v : viewports[fileNum]) {
					v.width = std::stof(val.substr(0, val.find(' ')));
					val = val.substr(val.find(' ') + 1);
				}

				val = lines[++i].substr(lines[i].find('=') + 1);

				for (auto& v : viewports[fileNum]) {
					v.x = std::stof(val.substr(0, val.find(' ')));
					val = val.substr(val.find(' ') + 1);
				}

				val = lines[++i].substr(lines[i].find('=') + 1);

				for (auto& v : viewports[fileNum]) {
					v.y = std::stof(val.substr(0, val.find(' ')));
					val = val.substr(val.find(' ') + 1);
				}
			}

		pipelineDone:
			vCombine(gdrAttachmentFormats[fileNum], additionalDynamicColorAttachmentFormats);
			vCombine(gcbAttachments[fileNum], additionalColorBlendAttachments);

			gvpStates[fileNum].viewportCount = static_cast<uint32_t>(viewports[fileNum].size());
			gvpStates[fileNum].pViewports = viewports[fileNum].data();
			gvpStates[fileNum].scissorCount = static_cast<uint32_t>(scissors[fileNum].size());
			gvpStates[fileNum].pScissors = scissors[fileNum].data();

			gmsStates[fileNum].pSampleMask = multisampleMasks[fileNum].data();

			gcbStates[fileNum].attachmentCount = static_cast<uint32_t>(gcbAttachments[fileNum].size());
			gcbStates[fileNum].pAttachments = gcbAttachments[fileNum].data();

			gdyStages[fileNum].dynamicStateCount = static_cast<uint32_t>(dynamicStates[fileNum].size());
			gdyStages[fileNum].pDynamicStates = dynamicStates[fileNum].data();

			gdyrInfos[fileNum].colorAttachmentCount = static_cast<uint32_t>(gdrAttachmentFormats[fileNum].size());
			gdyrInfos[fileNum].pColorAttachmentFormats = gdrAttachmentFormats[fileNum].data();
		}

		return true;
	}

	AcceleratorOperationStateCacheManager::AcceleratorOperationStateCacheManager(VulkanState* vks) {
		vkState = vks;
		isCacheActive = false;
		cacheSize = 0;
		activeMatID = 0;//since 0 cannot be a valid Material ID
	}
	AcceleratorOperationStateCacheManager::~AcceleratorOperationStateCacheManager() {
		if (isCacheActive)vkDestroyPipelineCache(vkState->vulkanDevice, activeCache, nullptr);
	}
	bool AcceleratorOperationStateCacheManager::getCacheInfo(uint32_t matID) {
		if (activeMatID != 0)return false;//error->already in use
		std::string filepath = "Binassets/" + std::to_string(matID);
		std::ifstream file(filepath, std::ios::binary | std::ios::ate);
		if (!file.is_open())return false;//file failed error
		VkPipelineCacheCreateInfo cInfo;
		cInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		cInfo.pNext = nullptr;
		cInfo.flags = 0;

		fileSize = (size_t)file.tellg();
		file.seekg(fileSize - sizeof(size_t));
		file.read((char*)&cacheSize, sizeof(size_t));
		std::vector<char> cacheData;
		if (cacheSize > 0) {
			cacheData.resize(cacheSize);
			file.seekg(fileSize - sizeof(size_t) - cacheSize);
			file.read(cacheData.data(), cacheSize);
		}
		file.close();
		cInfo.initialDataSize = cacheSize;
		cInfo.pInitialData = cacheData.data();

		vkCreatePipelineCache(vkState->vulkanDevice, &cInfo, nullptr, &activeCache);
		isCacheActive = true;
		if (cacheSize > 0) {
			VkPipelineCacheHeaderVersionOne* test = (VkPipelineCacheHeaderVersionOne*)cacheData.data();
			VkPipelineCacheHeaderVersionOne icles = *test;
			std::copy(std::begin(icles.pipelineCacheUUID), std::end(icles.pipelineCacheUUID), cacheVendorUUID.begin());
		}
		activeMatID = matID;
		return true;
	}
	bool AcceleratorOperationStateCacheManager::createGraphicsPipelines(
		std::vector<VkGraphicsPipelineCreateInfo>& cInfos,
		std::vector<VkPipeline>& pipes
	) {
		pipes.clear();
		pipes.resize(cInfos.size());
		if (!isCacheActive) {
			vkCreateGraphicsPipelines(
				vkState->vulkanDevice,
				VK_NULL_HANDLE,
				cInfos.size(), cInfos.data(),
				nullptr,
				pipes.data());
			return false;
		}
		vkCreateGraphicsPipelines(
			vkState->vulkanDevice,
			activeCache,
			cInfos.size(), cInfos.data(),
			nullptr,
			pipes.data());


		return true;
	}
	bool AcceleratorOperationStateCacheManager::save() {
		if (!isCacheActive)return false;//no cache to save
		size_t writeSize;
		vkGetPipelineCacheData(vkState->vulkanDevice, activeCache, &writeSize, nullptr);
		if (writeSize == 0)return false;//no data to write
		std::string filepath = "Binassets/" + std::to_string(activeMatID);
		std::vector<char>writeData(writeSize);
		vkGetPipelineCacheData(vkState->vulkanDevice, activeCache, &writeSize, writeData.data());
		VkPipelineCacheHeaderVersionOne* test = (VkPipelineCacheHeaderVersionOne*)writeData.data();
		VkPipelineCacheHeaderVersionOne icles = *test;
		std::array<uint8_t, VK_UUID_SIZE> forTesting;
		std::copy(std::begin(icles.pipelineCacheUUID), std::end(icles.pipelineCacheUUID), forTesting.begin());
		if (cacheSize == writeSize && cacheVendorUUID == forTesting) {
			vkDestroyPipelineCache(vkState->vulkanDevice, activeCache, nullptr);
			isCacheActive = false;
			activeMatID = 0;
			return true;//the data is already in the file
		}
		if (writeSize > cacheSize) {
			std::filesystem::resize_file(filepath, fileSize + writeSize);
		}
		std::fstream file(filepath, std::ios::binary | std::ios::in | std::ios::out);
		if (!file.is_open())return false;//should never happen but file failed to read somehow
		file.seekg(fileSize - cacheSize - /*2**/sizeof(size_t));
		//file.write((char*)&writeSize, sizeof(size_t)); reinstate if we read on load
		file.write(writeData.data(), writeData.size());
		file.write((char*)&writeSize, sizeof(size_t));
		file.close();
		if (writeSize < cacheSize) {
			std::filesystem::resize_file(filepath, fileSize - (cacheSize - writeSize));
		}
		vkDestroyPipelineCache(vkState->vulkanDevice, activeCache, nullptr);
		isCacheActive = false;
		activeMatID = 0;
		return true;
	}

	VulkanPipelineLibrary::VulkanPipelineLibrary(VulkanState* vks) {
		vkState = vks;
		configurator = new AcceleratorOperationStateConfig(vkState);
		cache = new AcceleratorOperationStateCacheManager(vkState);
		/*
		What should a default/error shader look like?
		No vertex inputs, just draws something to the screen.  Default triangle?
		Should we get creative and have a red octagon?  That sounds neat.  Maybe yellow triangle with block outline.
		Maybe fills the screen with a rainbow, have the fragment's color be based of its position.

		Whatever the case...we'll have to figure out how to hardcode the data, and there's quite a bit of it.
		*/
	}
	VulkanPipelineLibrary::~VulkanPipelineLibrary() {
		for (auto& kv : pipelines) {
			for (VkPipeline p : kv.second) {
				vkDestroyPipeline(vkState->vulkanDevice, p, nullptr);
			}
		}
		for (auto p : programs) {
			vkState->assetSystem->returnAsset(p.second);
		}
		delete cache;
		delete configurator;
	}
	AccelerationProgram* VulkanPipelineLibrary::getProgram(uint32_t UUID) {
		UUIDinfo checker(UUID);
		if (checker.getAssetType() != SAGE_SHADER_ASSET)return programs[0];//default shader
		if (!programs.contains(UUID)) {
			ShaderFileData rData{};
			Asset* loan = vkState->assetSystem->getAsset(UUID, &rData);
			if (!loan) return programs[0];//will we ever be able to have a default shader?
			loan = new AccelerationProgram(vkState, rData, loan);
			programs[UUID] = (AccelerationProgram*)loan;
		}
		programs[UUID]->numReferences++;
		return programs[UUID];
	}
	void VulkanPipelineLibrary::returnProgram(AccelerationProgram* prog) {
		if (--prog->numReferences == 0) {
			for (auto& p : pipelines[prog->getID()]) {
				vkDestroyPipeline(vkState->vulkanDevice, p, nullptr);
			}
			pipelines.erase(prog->getID());
			programs.erase(prog->UUID);
			vkState->assetSystem->returnAsset(prog);
		}
	}
	std::vector<VkPipeline>& VulkanPipelineLibrary::getPipelines(uint32_t matID, uint32_t apID, std::vector<std::string>& pipes) {
		std::vector<VkPipeline>garbage;
		if (!programs.contains(apID)) return garbage;
		std::string apName = programs[apID]->getID();
		if (!pipelines.contains(apName)) {
			garbage.resize(pipes.size());
			std::vector<VkGraphicsPipelineCreateInfo> woop;
			configurator->configureGraphicsPipelines(programs[apID], pipes, woop);
			cache->getCacheInfo(matID);
			if(cache->createGraphicsPipelines(woop, garbage)) cache->save();
			configurator->reset(programs[apID]->getUsage());
			pipelines[apName] = std::move(garbage);
		}
		return pipelines[apName];
	}

	Accelerator::Accelerator(SDL_Window* inWin) {
		singleQueue = true;
		transferInUse = 0;
		computeInUse = 0;
		gcInUse = 0;
		presentationQueue = nullptr;
		transferQueue = nullptr;
		computeQueue = nullptr;
		graphicsComputeQueue = nullptr;

		desiredInstanceLayers.push_back("VK_LAYER_KHRONOS_validation");//make debug stuff toggleable here
		desiredInstanceExtensions.push_back("VK_EXT_debug_utils");
		unsigned int SDLExtenstionCount;
		SDL_Vulkan_GetInstanceExtensions(inWin, &SDLExtenstionCount, nullptr);
		std::vector<const char*> SDLRequiredInstanceExtensions(SDLExtenstionCount);
		SDL_Vulkan_GetInstanceExtensions(inWin, &SDLExtenstionCount, SDLRequiredInstanceExtensions.data());
		requireVulkanInstanceExtensions(SDLRequiredInstanceExtensions);

		VkInstanceCreateInfo vkInstanceCreateInfo;
		VkApplicationInfo vkApplicationInfo;
		vkApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		vkApplicationInfo.pNext = nullptr;
		vkApplicationInfo.pApplicationName = "Suck Cess";//revisit this when we can stand up games.
		vkApplicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		vkApplicationInfo.pEngineName = "SAGE";
		vkApplicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		vkApplicationInfo.apiVersion = VK_API_VERSION_1_3;

		vkInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		vkInstanceCreateInfo.pNext = nullptr;
		vkInstanceCreateInfo.flags = 0;
		vkInstanceCreateInfo.pApplicationInfo = &vkApplicationInfo;
		vkInstanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(desiredInstanceLayers.size());
		vkInstanceCreateInfo.ppEnabledLayerNames = desiredInstanceLayers.data();
		vkInstanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(desiredInstanceExtensions.size());
		vkInstanceCreateInfo.ppEnabledExtensionNames = desiredInstanceExtensions.data();

		vkCreateInstance(&vkInstanceCreateInfo, nullptr, &vk.vulkanInstance);

		uint32_t logSeverity = 0
//			| VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
//			| VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
			;

		uint32_t logType = 0 
			| VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
			;
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
		debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugCreateInfo.pNext = nullptr;
		debugCreateInfo.flags = 0;
		debugCreateInfo.messageSeverity = logSeverity;
		debugCreateInfo.messageType = logType;
		debugCreateInfo.pfnUserCallback = debugCallback;
		debugCreateInfo.pUserData = 0;

		createDebug = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vk.vulkanInstance, "vkCreateDebugUtilsMessengerEXT");
		destroyDebug = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vk.vulkanInstance, "vkDestroyDebugUtilsMessengerEXT");

		createDebug(vk.vulkanInstance, &debugCreateInfo, nullptr, &dubigger);

		uint32_t deviceCount, queueCount;

		vkEnumeratePhysicalDevices(vk.vulkanInstance, &deviceCount, nullptr);
		std::vector<VkPhysicalDevice> myPhysicalDevice(deviceCount);
		vkEnumeratePhysicalDevices(vk.vulkanInstance, &deviceCount, myPhysicalDevice.data());

		vk.accelerationDevice = myPhysicalDevice[0];

/**/
		desiredDeviceExtensions.push_back("VK_KHR_spirv_1_4");
		desiredDeviceExtensions.push_back("VK_KHR_swapchain");
/**/

		VkPhysicalDeviceFeatures2 featuresV10{};
		VkPhysicalDeviceVulkan11Features featuresV11{};
		VkPhysicalDeviceVulkan12Features featuresV12{};
		VkPhysicalDeviceVulkan13Features featuresV13{};

		featuresV10.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		featuresV11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
		featuresV12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
		featuresV13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;

		featuresV10.pNext = &featuresV11;
		featuresV11.pNext = &featuresV12;
		featuresV12.pNext = &featuresV13;
		featuresV13.pNext = nullptr;

		featuresV10.features.samplerAnisotropy = VK_TRUE;
		featuresV10.features.fillModeNonSolid = VK_TRUE;
		featuresV11.shaderDrawParameters = VK_TRUE;
		featuresV13.dynamicRendering = VK_TRUE;
		featuresV13.synchronization2 = VK_TRUE;
/**/
		//vkGetPhysicalDeviceFeatures2(vk.accelerationDevice, &featuresV10);

		//when the file stuff is fully up and running, we gotta create the features structs fully with only what we use
		//but that's a big(read: very rote) task so...come back after audio

/**/
		//will want a system that lets us specifically enable all the features we want
		//like with queue selection; will probably have a desired features var that we [&] with the device's features
		// ...DeviceFeatures.properties &= ...DesiredFeatures.properties
		// then we say if(Features ^ Required) { do a failure }

		vkGetPhysicalDeviceQueueFamilyProperties(vk.accelerationDevice, &queueCount, nullptr);
		std::vector<VkQueueFamilyProperties> myDeviceQueueFamilyProperties(queueCount);
		vkGetPhysicalDeviceQueueFamilyProperties(vk.accelerationDevice, &queueCount, myDeviceQueueFamilyProperties.data());

		QueueFamilySorter famSorter{ myDeviceQueueFamilyProperties };
//		famSorter.print();

		std::vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfos;

		famSorter.prepareOutput();

		if (famSorter.reference.size() > 1) singleQueue = false;

		for(size_t i = 0; i < famSorter.reference.size(); i++){
			if (famSorter.reference[i].maxQueues != 0) {
				VkDeviceQueueCreateInfo temp;
				temp.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				temp.pNext = nullptr;
				temp.flags = 0;
				temp.queueFamilyIndex = i;
				temp.queueCount = famSorter.reference[i].maxQueues;
				temp.pQueuePriorities = &famSorter.reference[i].priority;
				deviceQueueCreateInfos.push_back(temp);
			}
		}
		
		VkDeviceCreateInfo deviceCreateInfo;
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.pNext = &featuresV10;
		deviceCreateInfo.flags = 0;
		deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(deviceQueueCreateInfos.size());
		deviceCreateInfo.pQueueCreateInfos = deviceQueueCreateInfos.data();
		deviceCreateInfo.enabledLayerCount = 0;
		deviceCreateInfo.ppEnabledLayerNames = nullptr;
		deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(desiredDeviceExtensions.size());
		deviceCreateInfo.ppEnabledExtensionNames = desiredDeviceExtensions.data();
		deviceCreateInfo.pEnabledFeatures = nullptr;//figure out what to do here

		vkCreateDevice(vk.accelerationDevice, &deviceCreateInfo, nullptr, &vk.vulkanDevice);

		//SDL_Surface* surface = SDL_GetWindowSurface(mainWindow);
		SDL_Vulkan_CreateSurface(inWin, vk.vulkanInstance, &vk.vulkanSurface);
		SDL_UpdateWindowSurface(inWin);

		VkBool32 currentTest = VK_FALSE;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk.accelerationDevice, vk.vulkanSurface, &vk.vulkanSurfaceCapabilities);

		for (size_t i = 0; i < famSorter.reference.size(); i++) {
			switch (famSorter.reference[i].familyUse)
			{
			case TRANSFER_FAMILY:
				for (size_t j = 0; j < famSorter.reference[i].maxQueues; j++) {
					transferQueues.push_back(new DedicatedTransferQueue(&vk, i, j));
				}
				transferQueue = transferQueues[0];
				break;
			case ASYNC_COMPUTE_FAMILY:
				for (size_t j = 0; j < famSorter.reference[i].maxQueues; j++) {
					computeQueues.push_back(new DedicatedComputeQueue(&vk, i, j, hardwareInfo.graphicsCardInfo.numFrames));
				}
				computeQueue = computeQueues[0];
				break;
			case GRAPHICS_COMPUTE_FAMILY:
				presentationQueue = new GraphicsComputeQueue(&vk, i, 0, hardwareInfo.graphicsCardInfo.numFrames);
				for (size_t j = 1; j < famSorter.reference[i].maxQueues; j++) {
					spareGCQueues.push_back(new GraphicsComputeQueue(&vk, i, j, hardwareInfo.graphicsCardInfo.numFrames));
				}
				if (spareGCQueues.size() > 0) graphicsComputeQueue = spareGCQueues[0];
				break;
			case PRESENT_FAMILY:
				if (!presentationQueue) presentationQueue = new PresentQueue(&vk, i, 0, hardwareInfo.graphicsCardInfo.numFrames);
			default:
				break;
			}
		}

		vkGetPhysicalDeviceSurfaceSupportKHR(vk.accelerationDevice, presentationQueue->FID, vk.vulkanSurface, &currentTest);
		//what to do if this fails?  Just quit?

		uint32_t numSurfaceFormats, numPresentModes;
		vkGetPhysicalDeviceSurfaceFormatsKHR(vk.accelerationDevice, vk.vulkanSurface, &numSurfaceFormats, NULL);
		std::vector<VkSurfaceFormatKHR> surfaceFormats(numSurfaceFormats);
		vkGetPhysicalDeviceSurfaceFormatsKHR(vk.accelerationDevice, vk.vulkanSurface, &numSurfaceFormats, surfaceFormats.data());
		vk.chosenSurfaceFormat = surfaceFormats[0];
		for (const auto& f : surfaceFormats) {
			if (f.format == VK_FORMAT_B8G8R8A8_SRGB && f.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR) vk.chosenSurfaceFormat = f;
		}//will we ever have need of other surface formats?

		vkGetPhysicalDeviceSurfacePresentModesKHR(vk.accelerationDevice, vk.vulkanSurface, &numPresentModes, NULL);
		std::vector<VkPresentModeKHR> presentModes(numPresentModes);
		vkGetPhysicalDeviceSurfacePresentModesKHR(vk.accelerationDevice, vk.vulkanSurface, &numPresentModes, presentModes.data());

		vk.chosenPresentMode = VK_PRESENT_MODE_FIFO_KHR;
		for (const auto& t : presentModes) {
			if (t == VK_PRESENT_MODE_MAILBOX_KHR) vk.chosenPresentMode = t;
		}

		VkSemaphoreCreateInfo qotsCI;
		qotsCI.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		qotsCI.pNext = nullptr;
		qotsCI.flags = 0;
		vkCreateSemaphore(vk.vulkanDevice, &qotsCI, nullptr, &queueOwnershipTransferenceSemaphore);
		VkFenceCreateInfo hatcCI;
		hatcCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		hatcCI.pNext = nullptr;
		hatcCI.flags = 0;
		vkCreateFence(vk.vulkanDevice, &hatcCI, nullptr, &hostAccessToCard);

		vk.assetSystem = AssetManager::getManager();
		samplerLibrary = new VulkanSamplerLibrary(&vk);
		descriptorSystem = new DescriptorSystem(&vk);
		stateLibrary = new VulkanPipelineLibrary(&vk);

		//command begin info library, might have to fix this?
		vk.singleUseBegin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		vk.singleUseBegin.pNext = nullptr;
		vk.singleUseBegin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vk.singleUseBegin.pInheritanceInfo = nullptr;
		vk.stdBegin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		vk.stdBegin.pNext = nullptr;
		vk.stdBegin.flags = 0;
		vk.stdBegin.pInheritanceInfo = nullptr;
	}
	Accelerator::~Accelerator() {
		delete descriptorSystem;
		delete stateLibrary;
		delete samplerLibrary;
		vk.assetSystem->release();

		destroyDebug(vk.vulkanInstance, dubigger, nullptr);

		vkDestroySurfaceKHR(vk.vulkanInstance, vk.vulkanSurface, nullptr);

		if (!singleQueue) {
			for (auto q : transferQueues) {
				delete q;
			}
			for (auto q : computeQueues) {
				delete q;
			}
			for (auto q : spareGCQueues) {
				delete q;
			}
		}
		delete presentationQueue;

		vkDestroyFence(vk.vulkanDevice, hostAccessToCard, nullptr);
		vkDestroySemaphore(vk.vulkanDevice, queueOwnershipTransferenceSemaphore, nullptr);

		vkDestroyDevice(vk.vulkanDevice, nullptr);
		vkDestroyInstance(vk.vulkanInstance, nullptr);
	}
	void Accelerator::cycleDedicatedQueue(AccelerationQueue* q) {
		switch (q->type)
		{
		case PRESENT_FAMILY:
			std::cout << "There should be only one present queue!\n";
			break;
		case TRANSFER_FAMILY:
			++transferInUse %= transferQueues.size();
			q = transferQueues[transferInUse];
			break;
		case ASYNC_COMPUTE_FAMILY:
			++computeInUse %= computeQueues.size();
			q = computeQueues[computeInUse];
			break;
		case GRAPHICS_COMPUTE_FAMILY:
			if (q == presentationQueue) {
				std::cout << "You can't cycle this!";
				break;
			}
			++gcInUse %= spareGCQueues.size();
			q = spareGCQueues[gcInUse];
			break;
		default:
			std::cout << "\n\n\nSomething's wrong (with cycling), I can feel it!\n\n\n";
			break;
		}
	}
	void Accelerator::prepareNewTensorAsStagingTarget(VulkanTensor* vt) {
		VkImageMemoryBarrier2 creationToTransferDst;
		creationToTransferDst.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
		creationToTransferDst.pNext = nullptr;
		creationToTransferDst.image = vt->image;
		creationToTransferDst.subresourceRange = vt->currentSR;

		creationToTransferDst.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
		creationToTransferDst.srcAccessMask = VK_ACCESS_2_NONE;
		creationToTransferDst.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		creationToTransferDst.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		creationToTransferDst.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
		creationToTransferDst.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
		creationToTransferDst.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		creationToTransferDst.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		vt->currentAccess = creationToTransferDst.dstAccessMask;
		vt->currentStage = creationToTransferDst.dstStageMask;
		vt->currentLayout = creationToTransferDst.newLayout;

		VkDependencyInfo commandDependency;
		commandDependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		commandDependency.pNext = nullptr;
		commandDependency.dependencyFlags = 0;
		commandDependency.memoryBarrierCount = 0;
		commandDependency.pMemoryBarriers = nullptr;
		commandDependency.bufferMemoryBarrierCount = 0;
		commandDependency.pBufferMemoryBarriers = nullptr;
		commandDependency.imageMemoryBarrierCount = 1;
		commandDependency.pImageMemoryBarriers = &creationToTransferDst;

		if (transferQueue) {
			vkCmdPipelineBarrier2(transferQueue->beginRecording(), &commandDependency);
			transferQueue->endRecording();
			cycleDedicatedQueue(transferQueue);

			vt->currentOwner = transferQueue->FID;
		}
		else if (graphicsComputeQueue) {
			vkCmdPipelineBarrier2(graphicsComputeQueue->beginRecording(), &commandDependency);
			graphicsComputeQueue->endRecording();
			cycleDedicatedQueue(graphicsComputeQueue);

			vt->currentOwner = graphicsComputeQueue->FID;
		}
		else {
			vkCmdPipelineBarrier2(presentationQueue->beginRecording(), &commandDependency);
			presentationQueue->endRecording();
			vt->currentOwner = presentationQueue->FID;
		}
	}
	void Accelerator::prepareNewTensorAsBlitTarget(VulkanTensor* vt, VkClearColorValue cc) {
		VkImageMemoryBarrier2 creationToTransferDst;
		creationToTransferDst.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
		creationToTransferDst.pNext = nullptr;
		creationToTransferDst.image = vt->image;
		creationToTransferDst.subresourceRange = vt->currentSR;

		creationToTransferDst.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
		creationToTransferDst.srcAccessMask = VK_ACCESS_2_NONE;
		creationToTransferDst.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		creationToTransferDst.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		creationToTransferDst.dstStageMask = VK_PIPELINE_STAGE_2_BLIT_BIT;
		creationToTransferDst.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
		creationToTransferDst.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		creationToTransferDst.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		vt->currentAccess = creationToTransferDst.dstAccessMask;
		vt->currentStage = creationToTransferDst.dstStageMask;
		vt->currentLayout = creationToTransferDst.newLayout;

		VkDependencyInfo commandDependency;
		commandDependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		commandDependency.pNext = nullptr;
		commandDependency.dependencyFlags = 0;
		commandDependency.memoryBarrierCount = 0;
		commandDependency.pMemoryBarriers = nullptr;
		commandDependency.bufferMemoryBarrierCount = 0;
		commandDependency.pBufferMemoryBarriers = nullptr;
		commandDependency.imageMemoryBarrierCount = 1;
		commandDependency.pImageMemoryBarriers = &creationToTransferDst;

		VkCommandBuffer commando;

        if (graphicsComputeQueue) {
			commando = graphicsComputeQueue->beginRecording();
			vkCmdPipelineBarrier2(commando, &commandDependency);
			vkCmdClearColorImage(commando, vt->image, vt->currentLayout, &cc, 1, &vt->currentSR);
			graphicsComputeQueue->endRecording();
			cycleDedicatedQueue(graphicsComputeQueue);

			vt->currentOwner = graphicsComputeQueue->FID;
		}
		else {
			commando = presentationQueue->beginRecording();
			vkCmdPipelineBarrier2(commando, &commandDependency);
			vkCmdClearColorImage(commando, vt->image, vt->currentLayout, &cc, 1, &vt->currentSR);
			presentationQueue->endRecording();
			vt->currentOwner = presentationQueue->FID;
		}
	}
	void Accelerator::stage(VulkanStagingBuffer* src, VulkanArray* dst) {
		VkBufferCopy cpy;
		cpy.srcOffset = 0;
		cpy.dstOffset = 0;
		cpy.size = src->allocationSize;
		if (transferQueue != nullptr) {
			vkCmdCopyBuffer(transferQueue->beginRecording(), src->buffer, dst->buffer, 1, &cpy);
			transferQueue->endRecording();
			cycleDedicatedQueue(transferQueue);
		}
		else {
			vkCmdCopyBuffer(presentationQueue->beginRecording(), src->buffer, dst->buffer, 1, &cpy);
			presentationQueue->endRecording();
		}
	}
	void Accelerator::stage(VulkanStagingBuffer* src, VulkanTensor* dst) {
		VkBufferImageCopy cpy;
		cpy.bufferOffset = 0;
		cpy.bufferRowLength = 0;
		cpy.bufferImageHeight = 0;
		cpy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		cpy.imageSubresource.mipLevel = 0;
		cpy.imageSubresource.baseArrayLayer = 0;
		cpy.imageSubresource.layerCount = dst->currentSR.layerCount;
		cpy.imageOffset = { 0,0,0 };
		cpy.imageExtent = dst->extent;
		if (transferQueue != nullptr) {
			vkCmdCopyBufferToImage(transferQueue->beginRecording(), src->buffer, dst->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &cpy);
			transferQueue->endRecording();
			cycleDedicatedQueue(transferQueue);
		}
		else {
			vkCmdCopyBufferToImage(presentationQueue->beginRecording(), src->buffer, dst->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &cpy);
			presentationQueue->endRecording();
		}
	}
	void Accelerator::prepareStagingTargetAsTexture(VulkanTensor* vt) {

		std::vector<VkSemaphoreSubmitInfo> waitinfo(1), siginfo(1);
		siginfo[0].sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
		siginfo[0].pNext = nullptr;
		siginfo[0].semaphore = queueOwnershipTransferenceSemaphore;
		siginfo[0].value = 0;
		siginfo[0].stageMask = vt->currentStage;
		siginfo[0].deviceIndex = 0;

		waitinfo[0].sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
		waitinfo[0].pNext = nullptr;
		waitinfo[0].semaphore = queueOwnershipTransferenceSemaphore;
		waitinfo[0].value = 0;
		waitinfo[0].stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
		waitinfo[0].deviceIndex = 0;

		if (!transferQueue) {
			siginfo.clear();
			waitinfo.clear();
		}

		VkImageMemoryBarrier2 transferToShaderRead;
		transferToShaderRead.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
		transferToShaderRead.pNext = nullptr;
		transferToShaderRead.srcStageMask = vt->currentStage;
		transferToShaderRead.srcAccessMask = vt->currentAccess;
		transferToShaderRead.oldLayout = vt->currentLayout;
		transferToShaderRead.srcQueueFamilyIndex = vt->currentOwner;
		transferToShaderRead.image = vt->image;
		transferToShaderRead.subresourceRange = vt->currentSR;


		transferToShaderRead.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		vt->currentStage = transferToShaderRead.dstStageMask;
		transferToShaderRead.dstAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
		vt->currentAccess = transferToShaderRead.dstAccessMask;
		transferToShaderRead.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		vt->currentLayout = transferToShaderRead.newLayout;

		transferToShaderRead.dstQueueFamilyIndex = graphicsComputeQueue ?
			graphicsComputeQueue->FID : presentationQueue->FID;
		vt->currentOwner = transferToShaderRead.dstQueueFamilyIndex;

		VkDependencyInfo depinfo;
		depinfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		depinfo.pNext = nullptr;
		depinfo.dependencyFlags = 0;
		depinfo.memoryBarrierCount = 0;
		depinfo.pMemoryBarriers = nullptr;
		depinfo.bufferMemoryBarrierCount = 0;
		depinfo.pBufferMemoryBarriers = nullptr;
		depinfo.imageMemoryBarrierCount = 1;
		depinfo.pImageMemoryBarriers = &transferToShaderRead;

		if (transferQueue) {
			vkCmdPipelineBarrier2(transferQueue->beginRecording(0), &depinfo);
			transferQueue->endRecording(0, VK_NULL_HANDLE, {}, siginfo);
			cycleDedicatedQueue(transferQueue);
		}

		if (graphicsComputeQueue) {
			vkCmdPipelineBarrier2(graphicsComputeQueue->beginRecording(0), &depinfo);
			graphicsComputeQueue->endRecording(0, VK_NULL_HANDLE, waitinfo);
			cycleDedicatedQueue(graphicsComputeQueue);
		}
		else {
			vkCmdPipelineBarrier2(presentationQueue->beginRecording(0), &depinfo);
			presentationQueue->endRecording(0, VK_NULL_HANDLE, waitinfo);
		}
	}
	void Accelerator::prepareStagingTargetAsAtlas(VulkanTensor* vt) {
		std::vector<VkSemaphoreSubmitInfo> waitinfo(1), siginfo(1);
		siginfo[0].sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
		siginfo[0].pNext = nullptr;
		siginfo[0].semaphore = queueOwnershipTransferenceSemaphore;
		siginfo[0].value = 0;
		siginfo[0].stageMask = vt->currentStage;
		siginfo[0].deviceIndex = 0;

		waitinfo[0].sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
		waitinfo[0].pNext = nullptr;
		waitinfo[0].semaphore = queueOwnershipTransferenceSemaphore;
		waitinfo[0].value = 0;
		waitinfo[0].stageMask = VK_PIPELINE_STAGE_2_BLIT_BIT;
		waitinfo[0].deviceIndex = 0;

		if (!transferQueue) {
			siginfo.clear();
			waitinfo.clear();
		}

		VkImageMemoryBarrier2 transferToShaderRead;
		transferToShaderRead.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
		transferToShaderRead.pNext = nullptr;
		transferToShaderRead.srcStageMask = vt->currentStage;
		transferToShaderRead.srcAccessMask = vt->currentAccess;
		transferToShaderRead.oldLayout = vt->currentLayout;
		transferToShaderRead.srcQueueFamilyIndex = vt->currentOwner;
		transferToShaderRead.image = vt->image;
		transferToShaderRead.subresourceRange = vt->currentSR;


		transferToShaderRead.dstStageMask = VK_PIPELINE_STAGE_2_BLIT_BIT;
		vt->currentStage = transferToShaderRead.dstStageMask;
		transferToShaderRead.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
		vt->currentAccess = transferToShaderRead.dstAccessMask;
		transferToShaderRead.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		vt->currentLayout = transferToShaderRead.newLayout;

		transferToShaderRead.dstQueueFamilyIndex = graphicsComputeQueue ?
			graphicsComputeQueue->FID : presentationQueue->FID;
		vt->currentOwner = transferToShaderRead.dstQueueFamilyIndex;

		VkDependencyInfo depinfo;
		depinfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		depinfo.pNext = nullptr;
		depinfo.dependencyFlags = 0;
		depinfo.memoryBarrierCount = 0;
		depinfo.pMemoryBarriers = nullptr;
		depinfo.bufferMemoryBarrierCount = 0;
		depinfo.pBufferMemoryBarriers = nullptr;
		depinfo.imageMemoryBarrierCount = 1;
		depinfo.pImageMemoryBarriers = &transferToShaderRead;

		if (transferQueue) {
			vkCmdPipelineBarrier2(transferQueue->beginRecording(0), &depinfo);
			transferQueue->endRecording(0, VK_NULL_HANDLE, {}, siginfo);
			cycleDedicatedQueue(transferQueue);
		}

		if (graphicsComputeQueue) {
			vkCmdPipelineBarrier2(graphicsComputeQueue->beginRecording(0), &depinfo);
			graphicsComputeQueue->endRecording(0, VK_NULL_HANDLE, waitinfo);
			cycleDedicatedQueue(graphicsComputeQueue);
		}
		else {
			vkCmdPipelineBarrier2(presentationQueue->beginRecording(0), &depinfo);
			presentationQueue->endRecording(0, VK_NULL_HANDLE, waitinfo);
		}
	}
	void Accelerator::prepareBlitTargetAsTexture(VulkanTensor* vt) {
		VkImageMemoryBarrier2 transferDstToShaderRead;
		transferDstToShaderRead.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
		transferDstToShaderRead.pNext = nullptr;
		transferDstToShaderRead.image = vt->image;
		transferDstToShaderRead.subresourceRange = vt->currentSR;

		transferDstToShaderRead.srcStageMask = vt->currentStage;
		transferDstToShaderRead.srcAccessMask = vt->currentAccess;
		transferDstToShaderRead.srcQueueFamilyIndex = vt->currentOwner;
		transferDstToShaderRead.oldLayout = vt->currentLayout;

		transferDstToShaderRead.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
		transferDstToShaderRead.dstAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
		transferDstToShaderRead.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		transferDstToShaderRead.dstQueueFamilyIndex = vt->currentOwner;

		vt->currentAccess = transferDstToShaderRead.dstAccessMask;
		vt->currentStage = transferDstToShaderRead.dstStageMask;
		vt->currentLayout = transferDstToShaderRead.newLayout;

		VkDependencyInfo commandDependency;
		commandDependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		commandDependency.pNext = nullptr;
		commandDependency.dependencyFlags = 0;
		commandDependency.memoryBarrierCount = 0;
		commandDependency.pMemoryBarriers = nullptr;
		commandDependency.bufferMemoryBarrierCount = 0;
		commandDependency.pBufferMemoryBarriers = nullptr;
		commandDependency.imageMemoryBarrierCount = 1;
		commandDependency.pImageMemoryBarriers = &transferDstToShaderRead;

		if (graphicsComputeQueue) {
			vkCmdPipelineBarrier2(graphicsComputeQueue->beginRecording(), &commandDependency);
			graphicsComputeQueue->endRecording();
			cycleDedicatedQueue(graphicsComputeQueue);
		}
		else {
			vkCmdPipelineBarrier2(presentationQueue->beginRecording(), &commandDependency);
			presentationQueue->endRecording();
		}
	}
	void Accelerator::prepareTextureAsBlitTarget(VulkanTensor* vt, VkClearColorValue cc) {
		VkImageMemoryBarrier2 shaderReadToTransferDst;
		shaderReadToTransferDst.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
		shaderReadToTransferDst.pNext = nullptr;
		shaderReadToTransferDst.image = vt->image;
		shaderReadToTransferDst.subresourceRange = vt->currentSR;

		shaderReadToTransferDst.srcStageMask = vt->currentStage;
		shaderReadToTransferDst.srcAccessMask = vt->currentAccess;
		shaderReadToTransferDst.srcQueueFamilyIndex = vt->currentOwner;
		shaderReadToTransferDst.oldLayout = vt->currentLayout;

		shaderReadToTransferDst.dstStageMask = VK_PIPELINE_STAGE_2_BLIT_BIT;
		shaderReadToTransferDst.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
		shaderReadToTransferDst.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		shaderReadToTransferDst.dstQueueFamilyIndex = vt->currentOwner;

		vt->currentAccess = shaderReadToTransferDst.dstAccessMask;
		vt->currentStage = shaderReadToTransferDst.dstStageMask;
		vt->currentLayout = shaderReadToTransferDst.newLayout;

		VkDependencyInfo commandDependency;
		commandDependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		commandDependency.pNext = nullptr;
		commandDependency.dependencyFlags = 0;
		commandDependency.memoryBarrierCount = 0;
		commandDependency.pMemoryBarriers = nullptr;
		commandDependency.bufferMemoryBarrierCount = 0;
		commandDependency.pBufferMemoryBarriers = nullptr;
		commandDependency.imageMemoryBarrierCount = 1;
		commandDependency.pImageMemoryBarriers = &shaderReadToTransferDst;

		VkCommandBuffer commando;
		if (graphicsComputeQueue) {
			commando = graphicsComputeQueue->beginRecording();
			vkCmdPipelineBarrier2(commando, &commandDependency);
			vkCmdClearColorImage(commando, vt->image, vt->currentLayout, &cc, 1, &vt->currentSR);
			graphicsComputeQueue->endRecording();
			cycleDedicatedQueue(graphicsComputeQueue);
		}
		else {
			commando = presentationQueue->beginRecording();
			vkCmdPipelineBarrier2(commando, &commandDependency);
			vkCmdClearColorImage(commando, vt->image, vt->currentLayout, &cc, 1, &vt->currentSR);
			presentationQueue->endRecording();
		}
	}
	VkCommandBuffer Accelerator::getAndStartCustomCommandBuffer(QueueFamilyType family, size_t index) {
		if (singleQueue) return presentationQueue->beginRecording(index);
		switch (family) {
		case PRESENT_FAMILY:
		singleQueue:
			return presentationQueue->beginRecording(index);
		case TRANSFER_FAMILY:
			if (transferQueue != nullptr) {
				return transferQueue->beginRecording(index);
			}
			else goto singleQueue;
			break;
		case ASYNC_COMPUTE_FAMILY:
			if (computeQueue != nullptr) {
				return computeQueue->beginRecording(index);
			}
			else goto singleQueue;
			break;
		case GRAPHICS_COMPUTE_FAMILY:
			if (graphicsComputeQueue != nullptr) {
				return graphicsComputeQueue->beginRecording(index);
			}
			else goto singleQueue;
			break;
		default:
			std::cout << "can't do that!\n";
			return nullptr;
		}
	}
	VkQueue Accelerator::endAndSubmitCustomCommandBuffer(QueueFamilyType family, size_t index, VkFence fence, std::vector<VkSemaphoreSubmitInfo> waits, std::vector<VkSemaphoreSubmitInfo> signals) {
		VkQueue returnQueue = VK_NULL_HANDLE;
		if (singleQueue) return presentationQueue->endRecording(index, fence, waits, signals);
		switch (family) {
		case PRESENT_FAMILY:
		singleQueue:
			returnQueue = presentationQueue->endRecording(index, fence, waits, signals);
			break;
		case TRANSFER_FAMILY:
			if (transferQueue != nullptr) {
				returnQueue = transferQueue->endRecording(index, fence, waits, signals);
				cycleDedicatedQueue(transferQueue);
				break;
			}
			else goto singleQueue;
		case ASYNC_COMPUTE_FAMILY:
			if (computeQueue != nullptr) {
				returnQueue = computeQueue->endRecording(index, fence, waits, signals);
				cycleDedicatedQueue(computeQueue);
				break;
			}
			else goto singleQueue;
		case GRAPHICS_COMPUTE_FAMILY:
			if (graphicsComputeQueue != nullptr) {
				returnQueue = graphicsComputeQueue->endRecording(index, fence, waits, signals);
				cycleDedicatedQueue(graphicsComputeQueue);
				break;
			}
			else goto singleQueue;
		default:
			std::cout << "Something ain't right!\n";
			break;
		}
		return returnQueue;
	}
	void Accelerator::idle() {
		vkDeviceWaitIdle(vk.vulkanDevice);
	}
}
