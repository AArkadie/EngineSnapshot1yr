// Anything that needs accelerating will take in a pointer to an object that represents the vulkan state
// In time, this will be an optional parameter which activates the hardware-accelerated functionality of whatever accepts it.

// Is it the case where we want 0 to be a default/error value for everything
// and throughout all of our/user code, we start indexing at 1?

//Linear Algebra coming to std?  Let's gooooooo!
#include<iostream>
#include<SDL.h>

#include"Mainframe.hpp"
using namespace SAGE;//The SynAesthesia (SynAesthetic?) Game Engine
//in loving memory of KAGE, the Kick-Ass Game Engine ;(

/*
class Computer {
public:
	Computer() {

		const int PARTICLE_COUNT = 256;
		std::vector<Particle> testParticles(PARTICLE_COUNT);

		for (auto& particle : testParticles) {
			float r = 0.25f * sqrt(rndDist(rndEngine));
			float theta = rndDist(rndEngine) * 2 * M_PI;
			float x = r * cos(theta) * windowWidth / windowHeight;
			float y = r * sin(theta);
			particle.position = glm::vec2(x, y);
			particle.velocity = glm::normalize(glm::vec2(x,y)) * 0.00025f;
			particle.color = glm::vec4(rndDist(rndEngine), rndDist(rndEngine), rndDist(rndEngine), 1.f);
		}

		VkCommandBufferAllocateInfo cbai;
		cbai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cbai.pNext = nullptr;
		cbai.commandPool = computeCommandPool;
		cbai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cbai.commandBufferCount = 1;
		vkAllocateCommandBuffers(vulkanDevice, &cbai, &cBuffer);

		VkDescriptorPoolSize dps;
		dps.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		dps.descriptorCount = static_cast<uint32_t>(numFramesToFly) * 2;
		VkDescriptorPoolCreateInfo dpci;
		dpci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		dpci.pNext = nullptr;
		dpci.flags = 0;
		dpci.poolSizeCount = 1;
		dpci.pPoolSizes = &dps;
		dpci.maxSets = static_cast<uint32_t>(numFramesToFly);
		vkCreateDescriptorPool(vulkanDevice, &dpci, nullptr, &descriptorPool);

		std::vector<VkDescriptorSetLayoutBinding> computeBinds(numFramesToFly);
		for (size_t i = 0; i < numFramesToFly; i++) {
			computeBinds[i].binding = i;
			computeBinds[i].descriptorCount = 1;
			computeBinds[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			computeBinds[i].pImmutableSamplers = nullptr;
			computeBinds[i].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_VERTEX_BIT;
		}

		VkDescriptorSetLayoutCreateInfo dslci;
		dslci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		dslci.pNext = nullptr;
		dslci.flags = 0;
		dslci.bindingCount = static_cast<uint32_t>(computeBinds.size());
		dslci.pBindings = computeBinds.data();
		vkCreateDescriptorSetLayout(vulkanDevice, &dslci, nullptr, &descriptorSetLayout);

		descriptorSets.resize(numFramesToFly);
		std::vector<VkDescriptorSetLayout> ssboShit(numFramesToFly, descriptorSetLayout);
		VkDescriptorSetAllocateInfo dsai;
		dsai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		dsai.pNext = nullptr;
		dsai.descriptorPool = descriptorPool;
		dsai.descriptorSetCount = static_cast<uint32_t>(numFramesToFly);
		dsai.pSetLayouts = ssboShit.data();
		vkAllocateDescriptorSets(vulkanDevice, &dsai, descriptorSets.data());

		VkShaderModule mod = VK_NULL_HANDLE;// = shaderReadSPIRV("Shaders/comp.spv");
		VkPipelineShaderStageCreateInfo computeStage;
		computeStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		computeStage.pNext = nullptr;
		computeStage.flags = 0;
		computeStage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		computeStage.module = mod;
		computeStage.pName = "main";
		computeStage.pSpecializationInfo = nullptr;

		VkPushConstantRange computePushRange;
		computePushRange.offset = 0;
		computePushRange.size = sizeof(float);
		computePushRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

		VkPipelineLayoutCreateInfo plci;
		plci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		plci.pNext = nullptr;
		plci.flags = 0;
		plci.setLayoutCount = 1;
		plci.pSetLayouts = &descriptorSetLayout;
		plci.pushConstantRangeCount = 1;
		plci.pPushConstantRanges = &computePushRange;
		vkCreatePipelineLayout(vulkanDevice, &plci, nullptr, &layout);

		VkComputePipelineCreateInfo cpci;
		cpci.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		cpci.pNext = nullptr;
		cpci.flags = 0;
		cpci.stage = computeStage;
		cpci.layout = layout;
		cpci.basePipelineHandle = VK_NULL_HANDLE;
		cpci.basePipelineIndex = -1;
		vkCreateComputePipelines(vulkanDevice, VK_NULL_HANDLE, 1, &cpci, nullptr, &pipeline);

		vkDestroyShaderModule(vulkanDevice, mod, nullptr);

		fComputation.resize(numFramesToFly);
		sComputeFinished.resize(numFramesToFly);
		VkFenceCreateInfo fci;
		fci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fci.pNext = nullptr;
		fci.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		VkSemaphoreCreateInfo sci;
		sci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		sci.pNext = nullptr;
		sci.flags = 0;

		for (size_t i = 0; i < numFramesToFly; i++) {
			vkCreateFence(vulkanDevice, &fci, nullptr, &fComputation[i]);
			vkCreateSemaphore(vulkanDevice, &sci, nullptr, &sComputeFinished[i]);
		}

	}
	~Computer() {
		for (auto& f : fComputation) {
			vkDestroyFence(vulkanDevice, f, nullptr);
		}
		for (auto& s : sComputeFinished) {
			vkDestroySemaphore(vulkanDevice, s, nullptr);
		}
		vkDestroyDescriptorPool(vulkanDevice, descriptorPool, nullptr);
		vkDestroyDescriptorSetLayout(vulkanDevice, descriptorSetLayout, nullptr);
		vkDestroyPipeline(vulkanDevice, pipeline, nullptr);
		vkDestroyPipelineLayout(vulkanDevice, layout, nullptr);
	}

	VkDescriptorSet& getVkDescriptorSet(size_t i = 0) {
		return descriptorSets[i];
	}
private:
	VkDescriptorSetLayout descriptorSetLayout;
	std::vector<VkDescriptorSet> descriptorSets;
	VkDescriptorPool descriptorPool;
	VkPipeline pipeline;
	VkPipelineLayout layout;
	VkCommandBuffer cBuffer;
	std::vector<VkFence> fComputation;
	std::vector <VkSemaphore> sComputeFinished;
};
*/
//come back to -- compute stuff.

struct PhysicsParams {
	float spaceUnit;
	float perFrameTimeUnit;
	float gravity;
	float airFriction;
	float groundFriction;

	//actually, these should be split among physics structures
	//like resistance mediums (air, water) and ground materials (sand, ice) and such.
};

struct MovementGroup {
	std::vector<TransformComponent*> memberList;

	void add(TransformComponent* member) {
		memberList.push_back(member);
	}

	void move() {
		for (TransformComponent* c : memberList) {
			c->rotate(5.f, glm::vec3(0.f, 1.f, 0.f));
		}
	}
};

struct PhysicsSystem {
	PhysicsParams* constants;
};

struct Atlas {//will become an asset
	Atlas(VideoSettings* vs) {
		stbi_uc* ping = stbi_load(
			"Assets/Atlases/BuildingBlocks/myFont.png",
			&basis.width,
			&basis.height,
			&basis.channels,
			STBI_rgb_alpha
		);
		stepX = 4;
		stepY = 8;
		entriesPerRow = basis.width / stepX;
		entriesPerCol = basis.height / stepY;
		targets.resize(entriesPerRow * entriesPerCol);

		for (int32_t i = 0; i < entriesPerCol; i++) {
			for (int32_t j = 0; j < entriesPerRow; j++) {
				VkImageBlit2& alias = targets[i * entriesPerRow + j];
				int32_t bull = (int32_t)stepX;
				int32_t shin = (int32_t)stepY;
				alias.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2;
				alias.pNext = nullptr;
				alias.srcOffsets[0] = { j * bull, i * shin,0 };
				alias.srcOffsets[1] = { (j * bull) + bull, (i * shin) + shin, 1 };
				alias.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };//color, no mips, layer 0, 1 layer
			}
		}

		atlas = new VulkanTensor(
			vs->videoCard->getState(),
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT,
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_IMAGE_TYPE_2D, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, 1,
			basis.width, basis.height
		);
		vs->videoCard->prepareNewTensorAsStagingTarget(atlas);

		basis.imageDataSize = basis.width * basis.height * 4;

		VulkanStagingBuffer toTheWindow(atlas, ping);
		vs->videoCard->stage(&toTheWindow, atlas);
		vs->videoCard->prepareStagingTargetAsAtlas(atlas);
		stbi_image_free(ping);
	}
	~Atlas() {
		delete atlas;
	}

	Image basis;
	VulkanTensor* atlas;
	uint32_t stepX, stepY, entriesPerRow, entriesPerCol;
	std::vector<VkImageBlit2> targets;
};

struct TextBox {
	TextBox(VideoSettings* vs, int32_t x, int32_t y, uint32_t w, uint32_t h) {
		settings = vs;
		EntitySystem* conv = settings->otherSystems->getEntitySystem();
		entityID = conv->makeEntity();
		conv->addComponent(TRANSFORM_COMPONENT, entityID);
		conv->addComponent(RENDERABILITY_COMPONENT, entityID);
		dimensions = conv->getTransformComponent(entityID);
		dimensions->setPosition({ x,y,0.f });
		dimensions->setSize({ w,h,0.f });
		sprite = new Mesh(settings, w, h);
		text = new Texture(settings, w, h);
		settings->videoCard->prepareNewTensorAsBlitTarget(text->getImage(), { 0.f,0.f,0.f,0.f });
		//will have a default message to blit here?
		settings->videoCard->prepareBlitTargetAsTexture(text->getImage());
		RenderabilityComponent* conv2 = conv->getRenderabilityComponent(entityID);
		conv2->changeMaterial(settings->assetSystem->getAssetUUID("material:simple"));
		conv2->setMesh(sprite);
		conv2->setTexture(text);
	}
	~TextBox() {
		settings->otherSystems->getEntitySystem()->returnEntity(entityID);
		delete text;
		delete sprite;
	}

	void display(std::string message, Atlas* font, size_t fontPoint) {
		settings->videoCard->prepareTextureAsBlitTarget(text->getImage(), {0.f,0.f,0.f,0.f});

		std::vector<VkImageBlit2> blits;
		size_t maxColumns, maxRows;
		int32_t xPos = 0, yPos = 0, blitSpanX, blitSpanY;

		do {
			blitSpanY = fontPoint-- * (4.f / 3) + 0.5;
			blitSpanX = blitSpanY * ((float)font->stepX / font->stepY) + 0.5;

			maxColumns = dimensions->size.x / blitSpanX;
			maxRows = dimensions->size.y / blitSpanY;
		} while (maxColumns * maxRows < message.size());

		for (char c : message) {
			VkImageBlit2 tBlit = font->targets[c];
			tBlit.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0,0,1 };
			tBlit.dstOffsets[0] = { xPos * blitSpanX, yPos * blitSpanY, 0 };
			tBlit.dstOffsets[1] = { xPos * blitSpanX + blitSpanX, yPos * blitSpanY + blitSpanY, 1 };
			blits.push_back(tBlit);
			++xPos %= maxColumns;
			if (xPos == 0) yPos++;
		}

		VkBlitImageInfo2 blinfo;
		blinfo.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2;
		blinfo.pNext = nullptr;
		blinfo.srcImage = font->atlas->handle();
		blinfo.srcImageLayout = font->atlas->getCurrentLayout();
		blinfo.dstImage = text->getImage()->handle();
		blinfo.dstImageLayout = text->getImage()->getCurrentLayout();
		blinfo.regionCount = blits.size();
		blinfo.pRegions = blits.data();
		blinfo.filter = VK_FILTER_NEAREST;

		vkCmdBlitImage2(settings->videoCard->getAndStartCustomCommandBuffer(GRAPHICS_COMPUTE_FAMILY), &blinfo);
		settings->videoCard->endAndSubmitCustomCommandBuffer(GRAPHICS_COMPUTE_FAMILY);
		settings->videoCard->prepareBlitTargetAsTexture(text->getImage());
	}

	uint32_t entityID;
	VideoSettings* settings;
	TransformComponent* dimensions;
	Texture* text;
	Mesh* sprite;
};

/* hi mom */

std::vector<double> decodeFloat(External_WaveFileData& data) {
	std::vector<double> returnVec;

	uint8_t byteStride = data.blockAlign / data.numChannels;
	size_t dataLength = data.data.size() / byteStride;
	float* fptr = (float*)data.data.data();
	double* dptr = (double*)data.data.data();

	if (byteStride == 8) {
		for (size_t i = 0; i < dataLength; i++) {
			returnVec.push_back(*dptr++);
		}
	}
	else {
		for (size_t i = 0; i < dataLength; i++) {
			returnVec.push_back(*fptr++);
		}
	}
	double max = 0.0;
	for (double& d : returnVec) {
		double absolute = abs(d);
		max = std::max(max, absolute);
	}
	for (double& d : returnVec) {
		d /= max;
	}
	return returnVec;
}

std::vector<double> decodeALAWandNormalize(std::vector<char>& data) {
	//double normalizationDenom = pow(2, 13) / 2; reminder that decoded bit depth is 13
	std::vector<int16_t> preturnVec;
	std::vector<double> returnVec;
	int16_t max = INT16_MIN;
	for (const char& c : data) {
		int16_t decNum = c ^ 0x55; //re-toggles even bits
		decNum &= 0x007F; //null the sign bit
		int16_t exponent = decNum >> 4; //exponent
		int16_t mantissa = decNum & 0x000F; //mantissa
		if (exponent > 0) mantissa += 0x0010; //adds a leading 1
		mantissa = (mantissa << 4) + 8; //left-justify and add 0.5 step
		if (exponent > 1) mantissa <<= exponent - 1;
		decNum = c & 128 ? mantissa : -mantissa;//re-sign the int

		max = max >= abs(decNum) ? max : abs(decNum);
		preturnVec.push_back(decNum);
	}
	for (int16_t& i : preturnVec) {
		returnVec.push_back((double)i / max);
	}
	return returnVec;
}//shout out to the ITU-T

std::vector<double> decodeMULAWandNormalize(std::vector<char>& data) {
	//double normalizationDenom = pow(2, 14) / 2; reminder that decoded bit depth is 14
	std::vector<int16_t> preturnVec;
	std::vector<double> returnVec;
	int16_t max = INT16_MIN;
	for (const char& c : data) {
		int16_t outputSign = c & 128 ? 1 : -1;
		int16_t mantissa = ~c; //1's complement
		int16_t exponent = (mantissa >> 4) & 0x0007;
		int16_t segment = exponent + 1; //ref: G.711 Table 2 Column 1
		mantissa &= 0x000F;
		int16_t step = 4 << segment; //LSB = 1 step

		//and now for the big equation
		outputSign *= (
			(0x0080 << exponent) //'1', preceding the mantissa
			+ step * mantissa //left-justify
			+ step / 2 //0.5 step
			- 4 * 33 //extract from compression group
			);

		max = max >= abs(outputSign) ? max : abs(outputSign);
		preturnVec.push_back(outputSign);
	}
	for (int16_t& i : preturnVec) {
		returnVec.push_back((double)i / max);
	}
	return returnVec;
}

struct MsWave49Packet {
	uint8_t fst_LAR2__2_1___LAR1__6_1;
	uint8_t fst_LAR3__4_1___LAR2__6_3;
	uint8_t fst_LAR5__2_1___LAR4__5_1___LAR3__5;
	uint8_t fst_LAR7__2_1___LAR6__4_1___LAR5__4_3;
	uint8_t fst_N1__4_1___LAR8__3_1___LAR7__3;
	uint8_t fst_Xmax1__1___M1__2_1___b1__2_1___N1__7_5;
	uint8_t fst_x1_0__3_1___Xmax1__6_2;
	uint8_t fst_x1_3__2_1___x1_2__3_1___x1_1__3_1;
	uint8_t fst_x1_6__1___x1_5__3_1___x1_4__3_1___x1_3__3;
	uint8_t fst_x1_8__3_1___x1_7__3_1___x1_6__3_2;
	uint8_t fst_x1_11__2_1___x1_10__3_1___x1_9__3_1;
	uint8_t fst_N2__4_1___x1_12__3_1___x1_11__3;
	uint8_t fst_Xmax2__1___M2__2_1___b2__2_1___N2__7_5;
	uint8_t fst_x2_0__3_1___Xmax2__6_2;
	uint8_t fst_x2_3__2_1___x2_2__3_1___x2_1__3_1;
	uint8_t fst_x2_6__1___x2_5__3_1___x2_4__3_1___x2_3__3;
	uint8_t fst_x2_8__3_1___x2_7__3_1___x2_6__3_2;
	uint8_t fst_x2_11__2_1___x2_10__3_1___x2_9__3_1;
	uint8_t fst_N3__4_1___x2_12__3_1___x2_11__3;
	uint8_t fst_Xmax3__1___M3__2_1___b3__2_1___N3__7_5;
	uint8_t fst_x3_0__3_1___Xmax3__6_2;
	uint8_t fst_x3_3__2_1___x3_2__3_1___x3_1__3_1;
	uint8_t fst_x3_6__1___x3_5__3_1___x3_4__3_1___x3_3__3;
	uint8_t fst_x3_8__3_1___x3_7__3_1___x3_6__3_2;
	uint8_t fst_x3_11__2_1___x3_10__3_1___x3_9__3_1;
	uint8_t fst_N4__4_1___x3_12__3_1___x3_11__3;
	uint8_t fst_Xmax4__1___M4__2_1___b4__2_1___N4__7_5;
	uint8_t fst_x4_0__3_1___Xmax4__6_2;
	uint8_t fst_x4_3__2_1___x4_2__3_1___x4_1__3_1;
	uint8_t fst_x4_6__1___x4_5__3_1___x4_4__3_1___x4_3__3;
	uint8_t fst_x4_8__3_1___x4_7__3_1___x4_6__3_2;
	uint8_t fst_x4_11__2_1___x4_10__3_1___x4_9__3_1;
	uint8_t snd_LAR1__4_1___fst_x4_12__3_1___x4_11__3;
	//there are 2 packets of this info in one set
	//so for each ms packet we reconstruct 320 samples as opposed to the typical 160
	//unless we want to use a variable to change how we read each set of 23
	//like at the beginning of parsing, we have an isFst flag set to false, then flip it every 33 bytes to change how we parse
	//would need a nibble of info to carry over though
	uint8_t snd_LAR2__6_1___LAR1__6_5;
	uint8_t snd_LAR4__3_1___LAR3__5_1;
	uint8_t snd_LAR6__2_1___LAR5__4_1___LAR4__5_4;
	uint8_t snd_LAR8__3_1___LAR7__3_1___LAR6__4_3;
	uint8_t snd_b1__1___N1__7_1;
	uint8_t snd_Xmax1__5_1___M1__2_1___b1__2;
	uint8_t snd_x1_2__1___x1_1___3_1___x1_0__3_1___Xmax1__6;
	uint8_t snd_x1_4__3_1___x1_3__3_1___x1_2__3_2;
	uint8_t snd_x1_7__2_1___x1_6__3_1___x1_5__3_1;
	uint8_t snd_x1_10__1___x1_9__3_1___x1_8__3_1___x1_7__3;
	uint8_t snd_x1_12__3_1___x1_11__3_1___x1_10__2_1;
	uint8_t snd_b2__1___N2__7_1;
	uint8_t snd_Xmax2__5_1___M2__2_1___b2__2;
	uint8_t snd_x2_2__1___x2_1___3_1___x2_0__3_1___Xmax2__6;
	uint8_t snd_x2_4__3_1___x2_3__3_1___x2_2__3_2;
	uint8_t snd_x2_7__2_1___x2_6__3_1___x2_5__3_1;
	uint8_t snd_x2_10__1___x2_9__3_1___x2_8__3_1___x2_7__3;
	uint8_t snd_x2_12__3_1___x2_11__3_1___x2_10__2_1;
	uint8_t snd_b3__1___N3__7_1;
	uint8_t snd_Xmax3__5_1___M3__2_1___b3__2;
	uint8_t snd_x3_2__1___x3_1___3_1___x3_0__3_1___Xmax3__6;
	uint8_t snd_x3_4__3_1___x3_3__3_1___x3_2__3_2;
	uint8_t snd_x3_7__2_1___x3_6__3_1___x3_5__3_1;
	uint8_t snd_x3_10__1___x3_9__3_1___x3_8__3_1___x3_7__3;
	uint8_t snd_x3_12__3_1___x3_11__3_1___x3_10__2_1;
	uint8_t snd_b4__1___N4__7_1;
	uint8_t snd_Xmax4__5_1___M4__2_1___b4__2;
	uint8_t snd_x4_2__1___x4_1___3_1___x4_0__3_1___Xmax4__6;
	uint8_t snd_x4_4__3_1___x4_3__3_1___x4_2__3_2;
	uint8_t snd_x4_7__2_1___x4_6__3_1___x4_5__3_1;
	uint8_t snd_x4_10__1___x4_9__3_1___x4_8__3_1___x4_7__3;
	uint8_t snd_x4_12__3_1___x4_11__3_1___x4_10__2_1;
	//we'll keep this for reference...why not?
};

int16_t GSMAdd(int16_t x, int16_t y) {
	int32_t test = x + y;
	if (test > INT16_MAX) return INT16_MAX;
	if (test < INT16_MIN) return INT16_MIN;
	return x + y;
}
int16_t GSMSub(int16_t x, int16_t y) {
	return GSMAdd(x, -y);
}
int32_t GSMLongAdd(int32_t x, int32_t y) {
	int64_t test = x + y;
	if (test > INT32_MAX) return INT32_MAX;
	if (test < INT32_MIN) return INT32_MIN;
	return x + y;
}
int32_t GSMLongSub(int32_t x, int32_t y) {
	return GSMLongAdd(x, -y);
}
int16_t GSMMult(int16_t x, int16_t y) {
	int32_t test = x * y;
	return test >> 15;
}
int16_t GSMMultR(int16_t x, int16_t y) {
	int32_t test = x * y;
	return (test + 16384) >> 15;
}
int32_t GSMMultL(int16_t x, int16_t y) {
	int32_t test = x * y;
	return test << 1;
}
int16_t GSMDiv(int16_t num, int16_t denom) {
	int32_t L_num = num;
	int32_t L_denom = denom;
	int16_t div = 0;
	for (size_t i = 0; i < 15; i++) {
		div <<= 1;
		L_num <<= 1;
		if (L_num >= L_denom) {
			L_num = GSMLongSub(L_num, L_denom);
			div = GSMAdd(div, 1);
		}
	}
	return div;
}
//int32_t GSMNorm(int32_t x) {/* We'll not need to use this in the decoder and if we ever do need to use it then we'll just straight copy it from toast because...I have no idea what this is attempting to do and I'm not taking a tangent into fixed point arithmetic to find out.*/}

std::vector<double> decodeGSM610WAV49andNormalize(std::vector<char>& data) {
	std::vector<double>returnVec;
	std::vector<int16_t>workingVec;
	int16_t max = INT16_MIN;

	std::array<int16_t, 8> NRFAC = {
		29128,
		26215,
		23832,
		21846,
		20165,
		18725,
		17476,
		16384
	};

	std::array<int16_t, 8> FAC = {
		18431,
		20479,
		22527,
		24575,
		26623,
		28671,
		30719,
		32767
	};

	std::array<int16_t, 4> QLB = { 
		3277,
		11469,
		21299,
		32767
	};

	std::array<int16_t, 8> B = {
		0,
		0,
		2048,
		-2560,
		94,
		-1792,
		-341,
		-1144
	};

	std::array<int16_t, 8> MIC = {
		-32,
		-32,
		-16,
		-16,
		-8,
		-8,
		-4,
		-4 
	};

	std::array<int16_t, 8> INVA = {
		13107,
		13107,
		13107,
		13107,
		19223,
		17476,
		31454,
		29708 
	};

	bool isEvenPass = true;
	uint8_t carryNibble = 0;
	size_t i = 0;
	while (i < data.size() - 33) {
		std::array<int16_t, 160> samples{}; //our output
		std::array<int16_t, 8> LARcoe{}; //The coded LAR coefficients for the frame (LARc)
		std::array<int16_t, 4> LTPlag{}; //The LTP lags of each subsection (Nc)
		std::array<int16_t, 4> LTPgai{}; //The LTP gains of each subsection (bc)
		std::array<int16_t, 4> RPEgri{}; //The RPE Grid selections of each subsection (Mc)
		std::array<int16_t, 4> RPEmax{}; //The RPE max amplitudes of each subsection (Xmaxc)
		std::array<int16_t, 13 * 4> RPEpul{}; //The codes of the normalized RPE samples of each subsection (xc)

		std::array<int16_t, 160> strsNeg120toPos40{};

		if (isEvenPass) {
			uint8_t currentByte = data[i++];
			LARcoe[0] |= (currentByte & 0b00111111)     ;
			LARcoe[1] |= (currentByte & 0b11000000) >> 6;
			currentByte = data[i++];
			LARcoe[1] |= (currentByte & 0b00001111) << 2;
			LARcoe[2] |= (currentByte & 0b11110000) >> 4;
			currentByte = data[i++];
			LARcoe[2] |= (currentByte & 0b00000001) << 4;
			LARcoe[3] |= (currentByte & 0b00111110) >> 1;
			LARcoe[4] |= (currentByte & 0b11000000) >> 6;
			currentByte = data[i++];
			LARcoe[4] |= (currentByte & 0b00000011) << 2;
			LARcoe[5] |= (currentByte & 0b00111100) >> 2;
			LARcoe[6] |= (currentByte & 0b11000000) >> 6;
			currentByte = data[i++];
			LARcoe[6] |= (currentByte & 0b00000001) << 2;
			LARcoe[7] |= (currentByte & 0b00001110) >> 1;
			LTPlag[0] |= (currentByte & 0b11110000) >> 4;
			currentByte = data[i++];
			LTPlag[0] |= (currentByte & 0b00000111) << 4;
			LTPgai[0] |= (currentByte & 0b00011000) >> 3;
			RPEgri[0] |= (currentByte & 0b01100000) >> 5;
			RPEmax[0] |= (currentByte & 0b10000000) >> 1;
			currentByte = data[i++];
			RPEmax[0] |= (currentByte & 0b00011111) << 1;
			RPEpul[0] |= (currentByte & 0b11100000) >> 5;//13 * subsection + param#
			currentByte = data[i++];
			RPEpul[1] |= (currentByte & 0b00000111)     ;
			RPEpul[2] |= (currentByte & 0b00111000) >> 3;
			RPEpul[3] |= (currentByte & 0b11000000) >> 6;
			currentByte = data[i++];
			RPEpul[3] |= (currentByte & 0b00000001) << 2;
			RPEpul[4] |= (currentByte & 0b00001110) >> 1;
			RPEpul[5] |= (currentByte & 0b01110000) >> 4;
			RPEpul[6] |= (currentByte & 0b10000000) >> 7;
			currentByte = data[i++];
			RPEpul[6] |= (currentByte & 0b00000011) << 1;
			RPEpul[7] |= (currentByte & 0b00011100) >> 2;
			RPEpul[8] |= (currentByte & 0b11100000) >> 5;
			currentByte = data[i++];
			RPEpul[9] |= (currentByte & 0b00000111)     ;
			RPEpul[10]|= (currentByte & 0b00111000) >> 3;
			RPEpul[11]|= (currentByte & 0b11000000) >> 6;
			currentByte = data[i++];
			RPEpul[11]|= (currentByte & 0b00000001) << 2;
			RPEpul[12]|= (currentByte & 0b00001110) >> 1;
			LTPlag[1] |= (currentByte & 0b11110000) >> 4;
			currentByte = data[i++];
			LTPlag[1] |= (currentByte & 0b00000111) << 4;
			LTPgai[1] |= (currentByte & 0b00011000) >> 3;
			RPEgri[1] |= (currentByte & 0b01100000) >> 5;
			RPEmax[1] |= (currentByte & 0b10000000) >> 1;
			currentByte = data[i++];
			RPEmax[1] |= (currentByte & 0b00011111) << 1;
			RPEpul[13]|= (currentByte & 0b11100000) >> 5;
			currentByte = data[i++];
			RPEpul[14]|= (currentByte & 0b00000111);
			RPEpul[15]|= (currentByte & 0b00111000) >> 3;
			RPEpul[16]|= (currentByte & 0b11000000) >> 6;
			currentByte = data[i++];
			RPEpul[16]|= (currentByte & 0b00000001) << 2;
			RPEpul[17]|= (currentByte & 0b00001110) >> 1;
			RPEpul[18]|= (currentByte & 0b01110000) >> 4;
			RPEpul[19]|= (currentByte & 0b10000000) >> 7;
			currentByte = data[i++];
			RPEpul[19]|= (currentByte & 0b00000011) << 1;
			RPEpul[20]|= (currentByte & 0b00011100) >> 2;
			RPEpul[21]|= (currentByte & 0b11100000) >> 5;
			currentByte = data[i++];
			RPEpul[22]|= (currentByte & 0b00000111)     ;
			RPEpul[23]|= (currentByte & 0b00111000) >> 3;
			RPEpul[24]|= (currentByte & 0b11000000) >> 6;
			currentByte = data[i++];
			RPEpul[24]|= (currentByte & 0b00000001) << 2;
			RPEpul[25]|= (currentByte & 0b00001110) >> 1;
			LTPlag[2] |= (currentByte & 0b11110000) >> 4;
			currentByte = data[i++];
			LTPlag[2] |= (currentByte & 0b00000111) << 4;
			LTPgai[2] |= (currentByte & 0b00011000) >> 3;
			RPEgri[2] |= (currentByte & 0b01100000) >> 5;
			RPEmax[2] |= (currentByte & 0b10000000) >> 1;
			currentByte = data[i++];
			RPEmax[2] |= (currentByte & 0b00011111) << 1;
			RPEpul[26]|= (currentByte & 0b11100000) >> 5;
			currentByte = data[i++];
			RPEpul[27]|= (currentByte & 0b00000111)     ;
			RPEpul[28]|= (currentByte & 0b00111000) >> 3;
			RPEpul[29]|= (currentByte & 0b11000000) >> 6;
			currentByte = data[i++];
			RPEpul[29]|= (currentByte & 0b00000001) << 2;
			RPEpul[30]|= (currentByte & 0b00001110) >> 1;
			RPEpul[31]|= (currentByte & 0b01110000) >> 4;
			RPEpul[32]|= (currentByte & 0b10000000) >> 7;
			currentByte = data[i++];
			RPEpul[32]|= (currentByte & 0b00000011) << 1;
			RPEpul[33]|= (currentByte & 0b00011100) >> 2;
			RPEpul[34]|= (currentByte & 0b11100000) >> 5;
			currentByte = data[i++];
			RPEpul[35]|= (currentByte & 0b00000111)     ;
			RPEpul[36]|= (currentByte & 0b00111000) >> 3;
			RPEpul[37]|= (currentByte & 0b11000000) >> 6;
			currentByte = data[i++];
			RPEpul[37]|= (currentByte & 0b00000001) << 2;
			RPEpul[38]|= (currentByte & 0b00001110) >> 1;
			LTPlag[3] |= (currentByte & 0b11110000) >> 4;
			currentByte = data[i++];
			LTPlag[3] |= (currentByte & 0b00000111) << 4;
			LTPgai[3] |= (currentByte & 0b00011000) >> 3;
			RPEgri[3] |= (currentByte & 0b01100000) >> 5;
			RPEmax[3] |= (currentByte & 0b10000000) >> 1;
			currentByte = data[i++];
			RPEmax[3] |= (currentByte & 0b00011111) << 1;
			RPEpul[39]|= (currentByte & 0b11100000) >> 5;
			currentByte = data[i++];
			RPEpul[40]|= (currentByte & 0b00000111)     ;
			RPEpul[41]|= (currentByte & 0b00111000) >> 3;
			RPEpul[42]|= (currentByte & 0b11000000) >> 6;
			currentByte = data[i++];
			RPEpul[42]|= (currentByte & 0b00000001) << 2;
			RPEpul[43]|= (currentByte & 0b00001110) >> 1;
			RPEpul[44]|= (currentByte & 0b01110000) >> 4;
			RPEpul[45]|= (currentByte & 0b10000000) >> 7;
			currentByte = data[i++];
			RPEpul[45]|= (currentByte & 0b00000011) << 1;
			RPEpul[46]|= (currentByte & 0b00011100) >> 2;
			RPEpul[47]|= (currentByte & 0b11100000) >> 5;
			currentByte = data[i++];
			RPEpul[48]|= (currentByte & 0b00000111);
			RPEpul[49]|= (currentByte & 0b00111000) >> 3;
			RPEpul[50]|= (currentByte & 0b11000000) >> 6;
			currentByte = data[i++];
			RPEpul[50]|= (currentByte & 0b00000001) << 2;
			RPEpul[51]|= (currentByte & 0b00001110) >> 1;
			carryNibble|=(currentByte & 0b11110000) >> 4;
		}
		else {
			uint8_t currentByte = data[i++];
			LARcoe[0] |= carryNibble | ((currentByte & 0b00000011) << 4);
			LARcoe[1] |= (currentByte & 0b11111100) >> 2;
			currentByte = data[i++];
			LARcoe[2] |= (currentByte & 0b00011111)     ;
			LARcoe[3] |= (currentByte & 0b11100000) >> 5;
			currentByte = data[i++];
			LARcoe[3] |= (currentByte & 0b00000011) << 3;
			LARcoe[4] |= (currentByte & 0b00111100) >> 2;
			LARcoe[5] |= (currentByte & 0b11000000) >> 6;
			currentByte = data[i++];
			LARcoe[5] |= (currentByte & 0b00000011) << 2;
			LARcoe[6] |= (currentByte & 0b00011100) >> 2;
			LARcoe[7] |= (currentByte & 0b11100000) >> 5;
			currentByte = data[i++];
			LTPlag[0] |= (currentByte & 0b01111111)     ;
			LTPgai[0] |= (currentByte & 0b10000000) >> 7;
			currentByte = data[i++];
			LTPgai[0] |= (currentByte & 0b00000001) << 1;
			RPEgri[0] |= (currentByte & 0b00000110) >> 1;
			RPEmax[0] |= (currentByte & 0b11111000) >> 3;
			currentByte = data[i++];
			RPEmax[0] |= (currentByte & 0b00000001) << 5;
			RPEpul[0] |= (currentByte & 0b00001110) >> 1;
			RPEpul[1] |= (currentByte & 0b01110000) >> 4;
			RPEpul[2] |= (currentByte & 0b10000000) >> 7;
			currentByte = data[i++];
			RPEpul[2] |= (currentByte & 0b00000011) << 1;
			RPEpul[3] |= (currentByte & 0b00011100) >> 2;
			RPEpul[4] |= (currentByte & 0b11100000) >> 5;
			currentByte = data[i++];
			RPEpul[5] |= (currentByte & 0b00000111)     ;
			RPEpul[6] |= (currentByte & 0b00111000) >> 3;
			RPEpul[7] |= (currentByte & 0b11000000) >> 6;
			currentByte = data[i++];
			RPEpul[7] |= (currentByte & 0b00000001) << 2;
			RPEpul[8] |= (currentByte & 0b00001110) >> 1;
			RPEpul[9] |= (currentByte & 0b01110000) >> 4;
			RPEpul[10]|= (currentByte & 0b10000000) >> 7;
			currentByte = data[i++];
			RPEpul[10]|= (currentByte & 0b00000011) << 1;
			RPEpul[11]|= (currentByte & 0b00011100) >> 2;
			RPEpul[12]|= (currentByte & 0b11100000) >> 5;
			currentByte = data[i++];
			LTPlag[1] |= (currentByte & 0b01111111);
			LTPgai[1] |= (currentByte & 0b10000000) >> 7;
			currentByte = data[i++];
			LTPgai[1] |= (currentByte & 0b00000001) << 1;
			RPEgri[1] |= (currentByte & 0b00000110) >> 1;
			RPEmax[1] |= (currentByte & 0b11111000) >> 3;
			currentByte = data[i++];
			RPEmax[1] |= (currentByte & 0b00000001) << 5;
			RPEpul[13]|= (currentByte & 0b00001110) >> 1;
			RPEpul[14]|= (currentByte & 0b01110000) >> 4;
			RPEpul[15]|= (currentByte & 0b10000000) >> 7;
			currentByte = data[i++];
			RPEpul[15]|= (currentByte & 0b00000011) << 1;
			RPEpul[16]|= (currentByte & 0b00011100) >> 2;
			RPEpul[17]|= (currentByte & 0b11100000) >> 5;
			currentByte = data[i++];
			RPEpul[18]|= (currentByte & 0b00000111);
			RPEpul[19]|= (currentByte & 0b00111000) >> 3;
			RPEpul[20]|= (currentByte & 0b11000000) >> 6;
			currentByte = data[i++];
			RPEpul[20]|= (currentByte & 0b00000001) << 2;
			RPEpul[21]|= (currentByte & 0b00001110) >> 1;
			RPEpul[22]|= (currentByte & 0b01110000) >> 4;
			RPEpul[23]|= (currentByte & 0b10000000) >> 7;
			currentByte = data[i++];
			RPEpul[23]|= (currentByte & 0b00000011) << 1;
			RPEpul[24]|= (currentByte & 0b00011100) >> 2;
			RPEpul[25]|= (currentByte & 0b11100000) >> 5;
			currentByte = data[i++];//begin
			LTPlag[2] |= (currentByte & 0b01111111);
			LTPgai[2] |= (currentByte & 0b10000000) >> 7;
			currentByte = data[i++];
			LTPgai[2] |= (currentByte & 0b00000001) << 1;
			RPEgri[2] |= (currentByte & 0b00000110) >> 1;
			RPEmax[2] |= (currentByte & 0b11111000) >> 3;
			currentByte = data[i++];
			RPEmax[2] |= (currentByte & 0b00000001) << 5;
			RPEpul[26]|= (currentByte & 0b00001110) >> 1;
			RPEpul[27]|= (currentByte & 0b01110000) >> 4;
			RPEpul[28]|= (currentByte & 0b10000000) >> 7;
			currentByte = data[i++];
			RPEpul[28]|= (currentByte & 0b00000011) << 1;
			RPEpul[29]|= (currentByte & 0b00011100) >> 2;
			RPEpul[30]|= (currentByte & 0b11100000) >> 5;
			currentByte = data[i++];
			RPEpul[31]|= (currentByte & 0b00000111);
			RPEpul[32]|= (currentByte & 0b00111000) >> 3;
			RPEpul[33]|= (currentByte & 0b11000000) >> 6;
			currentByte = data[i++];
			RPEpul[33]|= (currentByte & 0b00000001) << 2;
			RPEpul[34]|= (currentByte & 0b00001110) >> 1;
			RPEpul[35]|= (currentByte & 0b01110000) >> 4;
			RPEpul[36]|= (currentByte & 0b10000000) >> 7;
			currentByte = data[i++];
			RPEpul[36]|= (currentByte & 0b00000011) << 1;
			RPEpul[37]|= (currentByte & 0b00011100) >> 2;
			RPEpul[38]|= (currentByte & 0b11100000) >> 5;
			currentByte = data[i++];
			LTPlag[3] |= (currentByte & 0b01111111);
			LTPgai[3] |= (currentByte & 0b10000000) >> 7;
			currentByte = data[i++];
			LTPgai[3] |= (currentByte & 0b00000001) << 1;
			RPEgri[3] |= (currentByte & 0b00000110) >> 1;
			RPEmax[3] |= (currentByte & 0b11111000) >> 3;
			currentByte = data[i++];
			RPEmax[3] |= (currentByte & 0b00000001) << 5;
			RPEpul[39]|= (currentByte & 0b00001110) >> 1;
			RPEpul[40]|= (currentByte & 0b01110000) >> 4;
			RPEpul[41]|= (currentByte & 0b10000000) >> 7;
			currentByte = data[i++];
			RPEpul[41]|= (currentByte & 0b00000011) << 1;
			RPEpul[42]|= (currentByte & 0b00011100) >> 2;
			RPEpul[43]|= (currentByte & 0b11100000) >> 5;
			currentByte = data[i++];
			RPEpul[44]|= (currentByte & 0b00000111);
			RPEpul[45]|= (currentByte & 0b00111000) >> 3;
			RPEpul[46]|= (currentByte & 0b11000000) >> 6;
			currentByte = data[i++];
			RPEpul[46]|= (currentByte & 0b00000001) << 2;
			RPEpul[47]|= (currentByte & 0b00001110) >> 1;
			RPEpul[48]|= (currentByte & 0b01110000) >> 4;
			RPEpul[49]|= (currentByte & 0b10000000) >> 7;
			currentByte = data[i++];
			RPEpul[49]|= (currentByte & 0b00000011) << 1;
			RPEpul[50]|= (currentByte & 0b00011100) >> 2;
			RPEpul[51]|= (currentByte & 0b11100000) >> 5;
			carryNibble = 0;
		}
		isEvenPass = !isEvenPass;
		//Begin per-subsection calculations
		int16_t lagCheck = 40;//n_p
		std::array<uint16_t, 160> tempArr;
		for (size_t ssIndex = 0; ssIndex < 4; ssIndex++) {
			//Begin RPE Decoding
			//get exp and mant
			int16_t exp = 0;
			if (RPEmax[ssIndex] > 15) exp = GSMSub(RPEmax[ssIndex] >> 3, 1);
			int16_t mant = GSMSub(RPEmax[ssIndex], (exp << 3));
			//normalize mantisa [0,7]
			if (mant == 0) {
				exp = -4;
				mant = 7;
			}
			else {
				while (mant < 8) {
					mant = (mant << 1) | 1;
					exp--;
				}
				mant -= 8;
			}
			//APCM inverse quantization
			int16_t temp1 = FAC[mant];
			int16_t temp2 = GSMSub(6, exp);
			int16_t temp3 = 1 << GSMSub(temp2, 1);
			std::array<int16_t, 13> DecodedRPEPulse;
			for (size_t dex1 = 0; dex1 < 13; dex1++) {
				int16_t temp = GSMSub(RPEpul[ssIndex * 13 + dex1] << 1, 7);//restore pulse sign
				temp <<= 12;
				temp = GSMMultR(temp1, temp);
				temp = GSMAdd(temp, temp3);
				DecodedRPEPulse[dex1] = temp >> temp2;
			}
			//RPE grid positioning
			std::array<int16_t, 40> reconstLTRSignal{};//er_p
			for (size_t dex1 = 0; dex1 < 13; dex1++) {
				reconstLTRSignal[RPEgri[ssIndex] + (3 * dex1)] = DecodedRPEPulse[dex1];
			}
			//End RPE decoding

			//Begin Long Term Synthesis Filtering
			//Check the limits of longTermLag
			int16_t longTermLag = LTPlag[ssIndex];//Nr
			if (40 > longTermLag || longTermLag > 120) longTermLag = lagCheck;
			lagCheck = longTermLag;
			//Decode LTP Gain
			int16_t longTermGain = QLB[LTPgai[ssIndex]];
			//Compute reconstructed short term residual signal
			for (size_t dex1 = 120; dex1 < 160; dex1++) {
				int16_t doctorPeePee = GSMMultR(longTermGain, strsNeg120toPos40[dex1 - longTermLag]);
				strsNeg120toPos40[dex1] = GSMAdd(reconstLTRSignal[dex1 - 120], doctorPeePee);
			}
			//Update the negative indices
			for (size_t dex1 = 0; dex1 < 120; dex1++) {
				strsNeg120toPos40[dex1] = strsNeg120toPos40[40 + dex1];
			}
			//End Long Term Synthesis Filtering
			//Initialize working array values
			for (size_t dex1 = 0; dex1 < 40; dex1++) {
				tempArr[ssIndex * 40 + dex1] = strsNeg120toPos40[120 + dex1];
			}
			//End per-subsection calculations
		}
		//Begin per-frame calculations
		//Begin Computing reflection coefficients
		std::array<int16_t, 8> LARVars;
		for (size_t dex1 = 0; dex1 < 8; dex1++) {
			int16_t temp1 = GSMAdd(LARcoe[dex1], MIC[dex1]) << 10; //restores LARcoe sign
			int16_t temp2 = B[dex1] << 1;
			temp1 = GSMSub(temp1, temp2);
			temp1 = GSMMultR(INVA[dex1], temp1);
			LARVars[dex1] = GSMAdd(temp1, temp2);
		}
		//4.2.9.1 will become relevant when we need to use LAR_p and samples together
		std::array<int16_t, 8> decodedLAR;
		std::array<int16_t, 8> prevLARVars{};
		//4.2.9.2 is the same story
		std::array<int16_t, 8> reflectionCoef;
		//Begin Short Term Synthesis Filtering
		std::array<uint16_t, 9> v{};

		bool checkFlag = true;

		for (size_t sampleDex = 0; sampleDex < 160; sampleDex++) {

			if (checkFlag) {
				if (sampleDex == 0) {
					//Interpolate LAR constants to get true values
					for (size_t dex1 = 0; dex1 < 8; dex1++) {
						decodedLAR[dex1] = GSMAdd(prevLARVars[dex1] >> 2, LARVars[dex1] >> 2);
						decodedLAR[dex1] = GSMAdd(decodedLAR[dex1], prevLARVars[dex1] >> 1);
					}
					//Compute reflection Coefficients
					for (size_t dex1 = 0; dex1 < 8; dex1++) {
						int16_t temp = abs(decodedLAR[dex1]);
						if (temp < 11059) temp <<= 1;
						else if (temp < 20070) temp = GSMAdd(temp, 11059);
						else temp = GSMAdd((temp >> 2), 26112);
						reflectionCoef[dex1] = decodedLAR[dex1] < 0 ? -temp : temp;
					}
				}
				if (sampleDex == 13) {
					for (size_t dex1 = 0; dex1 < 8; dex1++) {
						decodedLAR[dex1] = GSMAdd(prevLARVars[dex1] >> 1, LARVars[dex1] >> 1);
					}
					for (size_t dex1 = 0; dex1 < 8; dex1++) {
						int16_t temp = abs(decodedLAR[dex1]);
						if (temp < 11059) temp <<= 1;
						else if (temp < 20070) temp = GSMAdd(temp, 11059);
						else temp = GSMAdd((temp >> 2), 26112);
						reflectionCoef[dex1] = decodedLAR[dex1] < 0 ? -temp : temp;
					}
				}
				if (sampleDex == 27) {
					for (size_t dex1 = 0; dex1 < 8; dex1++) {
						decodedLAR[dex1] = GSMAdd(prevLARVars[dex1] >> 2, LARVars[dex1] >> 2);
						decodedLAR[dex1] = GSMAdd(decodedLAR[dex1], LARVars[dex1] >> 1);
					}
					for (size_t dex1 = 0; dex1 < 8; dex1++) {
						int16_t temp = abs(decodedLAR[dex1]);
						if (temp < 11059) temp <<= 1;
						else if (temp < 20070) temp = GSMAdd(temp, 11059);
						else temp = GSMAdd((temp >> 2), 26112);
						reflectionCoef[dex1] = decodedLAR[dex1] < 0 ? -temp : temp;
					}
				}
				if (sampleDex == 40) {
					for (size_t dex1 = 0; dex1 < 8; dex1++) {
						decodedLAR[dex1] = LARVars[dex1];
					}
					for (size_t dex1 = 0; dex1 < 8; dex1++) {
						int16_t temp = abs(decodedLAR[dex1]);
						if (temp < 11059) temp <<= 1;
						else if (temp < 20070) temp = GSMAdd(temp, 11059);
						else temp = GSMAdd((temp >> 2), 26112);
						reflectionCoef[dex1] = decodedLAR[dex1] < 0 ? -temp : temp;
					}
					checkFlag = false;
				}
			}
			
			//Commence to filtering
			int16_t	temp = tempArr[sampleDex];
			for (size_t dex1 = 0; dex1 < 8; dex1++) {
				temp = GSMSub(temp, GSMMultR(reflectionCoef[7 - dex1], v[7 - dex1]));
				v[8 - dex1] = GSMAdd(v[7 - dex1], GSMMultR(reflectionCoef[7 - dex1], temp));
				/* procedure found in that other code, idk just thought it was neat

				int16_t temp1 = reflectionCoef[7 - dex1];
				int16_t temp2 = v[7 - dex1];
				temp2 = temp1 == INT16_MIN && temp2 == INT16_MIN ? INT16_MAX :
					0x0FFFF & (((int32_t)temp1 * (int32_t) temp2 + 16384) >> 15);
				temp = GSMSub(temp, temp2);
				temp1 = temp1 == INT16_MIN && temp == INT16_MIN ? INT16_MAX :
					0x0FFFF & (((int32_t)temp1 * (int32_t)temp + 16384) >> 15);
				v[8 - dex1] = GSMAdd(v[7 - dex1], temp1);

				*/
			}
			samples[sampleDex] = temp;
			v[0] = temp;
		}
		//End Short Term Synthesis Filtering
		//Begin Postprocessing
		int16_t msr = 0;
		for (int16_t& s : samples) {
			//Deemphasis filterng
			msr = GSMAdd(s, GSMMultR(msr, 28180));
			//Upscale and truncate output signal
			s = GSMAdd(msr, msr) & 0xFFF8;
			//And now for what we want it for
			if (abs(s) > max) max = abs(s);
			workingVec.push_back(s);
		}
		//End Postprocessing
	}

//Thanks to ETSI and https://www.quut.com/gsm
/*
	That being said, having reviewed the code against the toast implementation, I don't see anything that deviates seriously.
	I've even taken some queues from it.  Nevertheless, the test file playback sounds rather shit.
	I'll chalk this up to either GSM being a lossy encoding	or us reading the file wrong in some way.
	Maybe depending on the compiler to do some operations instead of rolling our own is introducing imprecision somewhere.
	Maybe doing all calculations in floating point rather than fixed point will yield better results.
	Maybe the media player converts this to a less lossy and more modern way of doing things.  Whatever the case,
	I'll just move on since I seriously doubt I'll be using GSM as other than a practice exercise.
*/

//and finally...we normalize
	for (int16_t& i : workingVec) {
		returnVec.push_back((double)i / max);
	}

return returnVec;
}

std::vector<double> dataConvertAndNormalize(External_WaveFileData& data) {
	External_WaveFormats fmt = data.fmtType == WAVE_FORMAT_EXTENSIBLE ? data.sfmt.subFmtType : data.fmtType;
	switch (fmt)
	{
	case EXTERNAL_WAVE_FORMAT_IEEE_FLOAT:
		return decodeFloat(data);
		break;
	case EXTERNAL_WAVE_FORMAT_ALAW:
		return decodeALAWandNormalize(data.data);
	case EXTERNAL_WAVE_FORMAT_MULAW:
		return decodeMULAWandNormalize(data.data);
	case EXTERNAL_WAVE_FORMAT_GSM610:
		//apparently this is incompatable with the other format?  Refer to this if future problems crop up here.
		//specifically, this format was developed by microsoft for use in .wav files, and the other is...more general?
		//from what I gather, the difference between this and the other is how the variables are packed into the data stream.
		//The acronym does stand for Global System for Mobile communications.  6.10 refers specifcally to the Full Rate codec
	case EXTERNAL_WAVE_FORMAT_GSM_610:
		return decodeGSM610WAV49andNormalize(data.data);
		//we'll figure out what to do about this maybe...way down the line.  Might be a fun exercise.
	default:
		break;
	}
	std::vector<double> returnVec;
	std::vector<int64_t> forInts;

	uint8_t byteStride = data.blockAlign / data.numChannels;
	size_t dataLength = data.data.size() / byteStride;

	void* start = data.data.data();

	int16_t* int2 = (int16_t*)start;
	int32_t* int4 = (int32_t*)start;
	int64_t* int8 = (int64_t*)start;

	switch (byteStride)	{
	case 1:
		for (char& c : data.data) {
			uint8_t actual = (uint8_t)c;
			forInts.push_back(actual - 128);
		}
		break;
	case 2:
		for (size_t i = 0; i < dataLength; i++) {
			forInts.push_back(*int2++);
		}
		break;
	case 3:
		for (size_t i = 0; i < data.data.size(); i+=3) {
			uint8_t byte0 = data.data[i + 0];
			uint8_t byte1 = data.data[i + 1];
			uint8_t byte2 = data.data[i + 2];
			uint8_t extension = byte2 & 128 ? 0xFF : 0;

			int32_t  donk = byte0 | (byte1 << 8) | (byte2 << 16) | (extension << 24);
			forInts.push_back(donk);
		}
		break;
	case 4:
		for (size_t i = 0; i < dataLength; i++) {
			forInts.push_back(*int4++);
		}
		break;
	case 6:
	case 8:
		for (size_t i = 0; i < dataLength; i++) {
			forInts.push_back(*int8++);
		}
		break;
	default:
		std::cout << "error with wave data conversion\n";
		return returnVec;
	}

	int64_t normalizationfactor = 0;
	for (int64_t& i : forInts) {
		int64_t abso = abs(i);
		normalizationfactor = std::max(abso, normalizationfactor);
	}
	for (int64_t& i : forInts) {
		returnVec.push_back((double)i / normalizationfactor);
	}

	return returnVec;
}

//question: how do we incorporate the subwoofer (aka Low Frequency) both as a position and in our synthesizer?
//idea: woofer at origin and any frequencies there are automatically mapped to...however those work.  Have to read up on what makes the "0.1" hardware different
//or maybe we say "any frequency below this threshold goes to the subwoofer channel at this position instead/as well"


//do we actually want to move our virtual channel array into a struct?

//struct FreeChannel {}; I question if we even need this, because an audio source contains the data for playing audio
//meanwhile we can derive the normal and distance from a transform component
//so the question in the air is: how do we target a synthesized note into a channel?

//on the topic of directionally filling audio channels: It's like backface culling.  Basically phong shading...but with sound.
//idea: we always have a virtual hemisphere where some audio fidelity setting determines how many normals we check on that 'sphere.
//all of these are then blended into the fixed audio channels (which are always on the sphere by default).

struct someTupleThingy {
	std::vector<float> channel;
	VirtualChannelSpeakerPosition target;
};

struct someAudioChannels {
	std::array<std::vector<float>, ALL_SPEAKER_POSITIONS> channels;
	//do we do it like this, knowing that most channels will go unused?
	//or do we pair channels with a position in a tuple and have a vector of those?
	//yeah that sounds good, maybe have 2 constructors: one takes position and the other takes dst mixer channel directly.
};

struct somePlaybackParameters {
	int theParams;
};

struct someAudioFunctions {
	void theOperations();
};//thinking about splitting the below struct into these separate components, think that makes more sense than a monolithic struct
//for what could be hundred of different audio streams
/*
struct someAudioStruct {
	std::vector<float> samples;
	int playbackCursor;
	size_t loopBeg, loopEnd;//note: denotes indices so loopEnd should go to size - 1;
	int16_t playbackFrequency;//if this goes negative, we can reverse :)
	float playbackSpeed;
	float playbackVolume;
	bool isPaused, isMuted;
	//start drafting for things we'd want to do with audio
	//we should separate the functions from the data methinks.  And the data from the samples maybe.
	float runPlayback() {
		if (isPaused) return 0.f;
		
		playbackCursor = playbackCursor + (int)(playbackFrequency * playbackSpeed);
		if (playbackCursor > loopEnd) playbackCursor = loopBeg + (playbackCursor - loopEnd);
		if (playbackCursor < loopBeg) playbackCursor = loopEnd - (loopBeg - playbackCursor);

		if (isMuted) return 0.f;

		return samples[playbackCursor] * playbackVolume;
		//consider having this return an array of floats corresponding to virtual channels that the mixer can...mix.
	}

	void togglePlay() { isPaused = !isPaused; }//play/pause
	void stop() {
		isPaused = true;
		playbackCursor = 0;
	}
	void setVolumeFactor(double v) { playbackVolume = fmax(fmin(0.0, v), 1.0); }//make sure to guard against negative values and high values that may cause damage
	void toggleMute() { isMuted = !isMuted; }
	void reverse() { playbackFrequency = -playbackFrequency; }//make frequency negative
	void skipSamples(int step) { playbackCursor += step; }//both forward and back.  Do we want to be able to skip by time?
	//consider skipTime() here
	void dilate(double playbackSpeedChangeFactor) { playbackSpeed *= playbackSpeedChangeFactor; }
	void moveLoopStart(size_t x) {
		if (x > loopEnd) {
			loopBeg = loopEnd;
			moveLoopEnd(x);
		}
		else loopBeg = std::max((size_t)0, x);
	}
	void moveLoopEnd(size_t x) {
		if (x < loopBeg) {
			loopEnd = loopBeg;
			moveLoopStart(x);
		}
		else loopEnd = std::min(x, samples.size());
	}
	void setLoopRange(size_t start, size_t end) {
		moveLoopStart(start);
		moveLoopEnd(end);
	}
	//we guarantee two predefined loop points at the start and end of the buffer
};
*/
MixerDirectInput somethingSomethingWaves(External_WaveFileData& data) {//want to turn this into something that returns a common struct that the synthesizer shares (MixerPacket?)
	MixerDirectInput toReturn{};
	toReturn.channelBuffers.resize(hardwareInfo.soundCardInfo.outputNumChannels);
	std::vector<std::vector<float>> workingBuffers;
	workingBuffers.resize(data.numChannels);
	std::vector<std::vector<float>> virtualChannelStagingSpace;
	virtualChannelStagingSpace.reserve(ALL_SPEAKER_POSITIONS);
	std::vector<double> convertedData = dataConvertAndNormalize(data);//still interleaved

	double interpolation = (double)data.sampleRate / hardwareInfo.soundCardInfo.outputHertz ;
	double sizeDiff = 1.0 / interpolation;
	size_t channelSize = ((double)convertedData.size() / data.numChannels) * sizeDiff;
	for (auto& b : workingBuffers) b.resize(channelSize);
	for (auto& c : toReturn.channelBuffers) c.resize(channelSize);
	std::vector<float> silentChannel(channelSize, 0.f);
	//if we end up using the silence value retrieved by SDL, this is where we'd fill all buffers with silence initially

	//de-interleave and convert sample rate per channel.  We'll just assume this works until a problem presents itself
	double doubleDex = 0.0;
	if (interpolation >= 1.0) {
		//downsampling
		interpolation *= data.numChannels;
		for (size_t i = 0; i < workingBuffers[0].size(); i++) {
			size_t r = round(doubleDex);
			for (size_t c = 0; c < data.numChannels; c++) {
				workingBuffers[c][i] = convertedData[r + c];
			}
			doubleDex += interpolation;
		}
	}
	else {
		//upsampling
		size_t wrap = convertedData.size() - data.numChannels;
		for (size_t i = 0; i <= wrap; i += data.numChannels) {
			for (size_t c = 0; c < data.numChannels; c++) {
				double thisVal = convertedData[i + c];
				double nextVal = convertedData[(i + c + data.numChannels) % wrap];
				size_t thisDex = round(doubleDex);//we don't add c because dst buffers aren't interleaved
				size_t nextDex = round(doubleDex + sizeDiff);
				workingBuffers[c][thisDex] = thisVal;
				for (size_t j = 1; j < nextDex - thisDex; j++) {
					workingBuffers[c][thisDex + j] = lerp(thisVal, nextVal, j * interpolation);
				}
			}
			doubleDex += sizeDiff;
		}
	}


	//virtually we assume we have every channel, then at the very end we merge according to what we actually have.
	size_t wDex = 0;
	uint8_t channelsInMask = 0;
	for (size_t i = 0; i < 18; i++) {
		if ((data.speakerPosMask >> i) & 1) channelsInMask++;
	}
	char numFreeChannels = data.numChannels - channelsInMask;
	for (size_t i = 0; i < ALL_SPEAKER_POSITIONS; i++) {
		if ((data.speakerPosMask >> i) & 1) virtualChannelStagingSpace.push_back(workingBuffers[wDex++]);
		else if (numFreeChannels > 0) {
			virtualChannelStagingSpace.push_back(workingBuffers[wDex++]);
			numFreeChannels--;
		}
		else virtualChannelStagingSpace.push_back(silentChannel);
	}


	//and now we collapse virtual space into actual space.
	/*
		For multi-channel audio, the default SDL channel mapping is:
		2:  FL FR	                    (stereo)
		3:  FL FR LF                   (2.1 surround)
		4:  FL FR BL BR                (quad)
		5:  FL FR LF BL BR             (4.1 surround)
		6:  FL FR FC LF SL SR          (5.1 surround - last two can also be BL BR)
		7:  FL FR FC LF BC SL SR       (6.1 surround)
		8:  FL FR FC LF BL BR SL SR    (7.1 surround)
	*/

	//I feel like everything below this comment should be a method in the mixer, applicable not to just an external sound file
	switch (toReturn.channelBuffers.size()) {
	case 0:
		std::cout << "What kind of sound device doesn't have any channels to output to?";
		break;
	case 1:
		for (size_t i = 0; i < channelSize; i++) {
			for (size_t j = 0; j < virtualChannelStagingSpace.size(); j++) {
				toReturn.channelBuffers[0][i] += virtualChannelStagingSpace[j][i];
			}
		}
		break;
	case 2:
		for (size_t i = 0; i < channelSize; i++) {
			//sL, sR
			toReturn.channelBuffers[0][i] =
				virtualChannelStagingSpace[MID_FRONT_LEFT][i] * vspd(TRUE_LEFT, MID_FRONT_LEFT) +
				virtualChannelStagingSpace[MID_FRONT_CENTER][i] * 0.5f +
				virtualChannelStagingSpace[SUBWOOFER][i] * 0.5f +
				virtualChannelStagingSpace[MID_BACK_LEFT][i] * vspd(TRUE_LEFT, MID_BACK_LEFT) +
				virtualChannelStagingSpace[MID_FRONT_CENTER_LEFT][i] * vspd(TRUE_LEFT, MID_FRONT_CENTER_LEFT) +
				virtualChannelStagingSpace[MID_BACK_CENTER][i] * 0.5f +
				virtualChannelStagingSpace[TRUE_LEFT][i] +
				virtualChannelStagingSpace[OVERHEAD][i] * 0.5f +
				virtualChannelStagingSpace[UP_FRONT_LEFT][i] * vspd(TRUE_LEFT, UP_FRONT_LEFT) +
				virtualChannelStagingSpace[UP_FRONT_CENTER][i] * 0.5f +
				virtualChannelStagingSpace[UP_BACK_LEFT][i] * vspd(TRUE_LEFT, UP_BACK_LEFT) +
				virtualChannelStagingSpace[UP_BACK_CENTER][i] * 0.5f;

			toReturn.channelBuffers[1][i] = 
				virtualChannelStagingSpace[MID_FRONT_RIGHT][i] * vspd(TRUE_RIGHT, MID_FRONT_RIGHT) +
				virtualChannelStagingSpace[MID_FRONT_CENTER][i] * 0.5f +
				virtualChannelStagingSpace[SUBWOOFER][i] * 0.5f +
				virtualChannelStagingSpace[MID_BACK_RIGHT][i] * vspd(TRUE_RIGHT, MID_BACK_RIGHT) +
				virtualChannelStagingSpace[MID_FRONT_CENTER_RIGHT][i] * vspd(TRUE_RIGHT, MID_FRONT_CENTER_RIGHT) +
				virtualChannelStagingSpace[MID_BACK_CENTER][i] * 0.5f +
				virtualChannelStagingSpace[TRUE_RIGHT][i] +
				virtualChannelStagingSpace[OVERHEAD][i] * 0.5f +
				virtualChannelStagingSpace[UP_FRONT_RIGHT][i] * vspd(TRUE_RIGHT, UP_FRONT_RIGHT) +
				virtualChannelStagingSpace[UP_FRONT_CENTER][i] * 0.5f+
				virtualChannelStagingSpace[UP_BACK_RIGHT][i] * vspd(TRUE_RIGHT, UP_BACK_RIGHT) +
				virtualChannelStagingSpace[UP_BACK_CENTER][i] * 0.5f;
		}
		break;
	case 3:
		for (size_t i = 0; i < channelSize; i++) {
			//sL, sR, sW
			toReturn.channelBuffers[0][i] =
				virtualChannelStagingSpace[MID_FRONT_LEFT][i] * vspd(TRUE_LEFT, MID_FRONT_LEFT) +
				virtualChannelStagingSpace[MID_FRONT_CENTER][i] * 0.5f +
				virtualChannelStagingSpace[MID_BACK_LEFT][i] * vspd(TRUE_LEFT, MID_BACK_LEFT) +
				virtualChannelStagingSpace[MID_FRONT_CENTER_LEFT][i] * vspd(TRUE_LEFT, MID_FRONT_CENTER_LEFT) +
				virtualChannelStagingSpace[MID_BACK_CENTER][i] * 0.5f +
				virtualChannelStagingSpace[TRUE_LEFT][i] +
				virtualChannelStagingSpace[OVERHEAD][i] * 0.5f +
				virtualChannelStagingSpace[UP_FRONT_LEFT][i] * vspd(TRUE_LEFT, UP_FRONT_LEFT) +
				virtualChannelStagingSpace[UP_FRONT_CENTER][i] * 0.5f +
				virtualChannelStagingSpace[UP_BACK_LEFT][i] * vspd(TRUE_LEFT, UP_BACK_LEFT) +
				virtualChannelStagingSpace[UP_BACK_CENTER][i] * 0.5f;

			toReturn.channelBuffers[1][i] =
				virtualChannelStagingSpace[MID_FRONT_RIGHT][i] * vspd(TRUE_RIGHT, MID_FRONT_RIGHT) +
				virtualChannelStagingSpace[MID_FRONT_CENTER][i] * 0.5f +
				virtualChannelStagingSpace[MID_BACK_RIGHT][i] * vspd(TRUE_RIGHT, MID_BACK_RIGHT) +
				virtualChannelStagingSpace[MID_FRONT_CENTER_RIGHT][i] * vspd(TRUE_RIGHT, MID_FRONT_CENTER_RIGHT) +
				virtualChannelStagingSpace[MID_BACK_CENTER][i] * 0.5f +
				virtualChannelStagingSpace[TRUE_RIGHT][i] +
				virtualChannelStagingSpace[OVERHEAD][i] * 0.5f +
				virtualChannelStagingSpace[UP_FRONT_RIGHT][i] * vspd(TRUE_RIGHT, UP_FRONT_RIGHT) +
				virtualChannelStagingSpace[UP_FRONT_CENTER][i] * 0.5f +
				virtualChannelStagingSpace[UP_BACK_RIGHT][i] * vspd(TRUE_RIGHT, UP_BACK_RIGHT) +
				virtualChannelStagingSpace[UP_BACK_CENTER][i] * 0.5f;

			toReturn.channelBuffers[2][i] = virtualChannelStagingSpace[SUBWOOFER][i];
		}
		break;
	case 4:
		for (size_t i = 0; i < channelSize; i++) {
			//fL, fR, bL, bR
			toReturn.channelBuffers[0][i] =
				virtualChannelStagingSpace[MID_FRONT_LEFT][i] +
				virtualChannelStagingSpace[MID_FRONT_CENTER][i] * vspd(MID_FRONT_LEFT, MID_FRONT_CENTER) +
				virtualChannelStagingSpace[SUBWOOFER][i] * 0.25f +
				virtualChannelStagingSpace[MID_FRONT_CENTER_LEFT][i] * vspd(MID_FRONT_LEFT, MID_FRONT_CENTER_LEFT) +
				virtualChannelStagingSpace[TRUE_LEFT][i] * vspd(MID_FRONT_LEFT, TRUE_LEFT) +
				virtualChannelStagingSpace[OVERHEAD][i] * 0.25f +
				virtualChannelStagingSpace[UP_FRONT_LEFT][i] * vspd(MID_FRONT_LEFT, UP_FRONT_LEFT) +
				virtualChannelStagingSpace[UP_FRONT_CENTER][i] * vspd(MID_FRONT_LEFT, UP_FRONT_CENTER);


			toReturn.channelBuffers[1][i] =
				virtualChannelStagingSpace[MID_FRONT_RIGHT][i] +
				virtualChannelStagingSpace[MID_FRONT_CENTER][i] * vspd(MID_FRONT_RIGHT, MID_FRONT_CENTER) +
				virtualChannelStagingSpace[SUBWOOFER][i] * 0.25f +
				virtualChannelStagingSpace[MID_FRONT_CENTER_RIGHT][i] * vspd(MID_FRONT_RIGHT, MID_FRONT_CENTER_RIGHT) +
				virtualChannelStagingSpace[TRUE_RIGHT][i] * vspd(MID_FRONT_RIGHT, TRUE_RIGHT) +
				virtualChannelStagingSpace[OVERHEAD][i] * 0.25f +
				virtualChannelStagingSpace[UP_FRONT_RIGHT][i] * vspd(MID_FRONT_RIGHT, UP_FRONT_RIGHT) +
				virtualChannelStagingSpace[UP_FRONT_CENTER][i] * vspd(MID_FRONT_RIGHT, UP_FRONT_CENTER);

			toReturn.channelBuffers[2][i] =
				virtualChannelStagingSpace[SUBWOOFER][i] * 0.25f +
				virtualChannelStagingSpace[MID_BACK_LEFT][i] +
				virtualChannelStagingSpace[MID_BACK_CENTER][i] * vspd(MID_BACK_LEFT, MID_BACK_CENTER) +
				virtualChannelStagingSpace[TRUE_LEFT][i] * vspd(MID_BACK_LEFT, TRUE_LEFT) +
				virtualChannelStagingSpace[OVERHEAD][i] * 0.25f +
				virtualChannelStagingSpace[UP_BACK_LEFT][i] * vspd(MID_BACK_LEFT, UP_BACK_LEFT) +
				virtualChannelStagingSpace[UP_BACK_CENTER][i] * vspd(MID_BACK_LEFT, UP_BACK_CENTER);

			toReturn.channelBuffers[3][i] =
				virtualChannelStagingSpace[SUBWOOFER][i] * 0.25f +
				virtualChannelStagingSpace[MID_BACK_RIGHT][i] +
				virtualChannelStagingSpace[MID_BACK_CENTER][i] * vspd(MID_BACK_RIGHT, MID_BACK_CENTER) +
				virtualChannelStagingSpace[TRUE_RIGHT][i] * vspd(MID_BACK_RIGHT, TRUE_RIGHT) +
				virtualChannelStagingSpace[OVERHEAD][i] * 0.25f +
				virtualChannelStagingSpace[UP_BACK_RIGHT][i] * vspd(MID_BACK_RIGHT, UP_BACK_RIGHT) +
				virtualChannelStagingSpace[UP_BACK_CENTER][i] * vspd(MID_BACK_RIGHT, UP_BACK_CENTER);
		}
		break;
	case 5:
		for (size_t i = 0; i < channelSize; i++) {
			//fL, fR, sW, bL, bR
			toReturn.channelBuffers[0][i] =
				virtualChannelStagingSpace[MID_FRONT_LEFT][i] +
				virtualChannelStagingSpace[MID_FRONT_CENTER][i] * vspd(MID_FRONT_LEFT, MID_FRONT_CENTER) +
				virtualChannelStagingSpace[MID_FRONT_CENTER_LEFT][i] * vspd(MID_FRONT_LEFT, MID_FRONT_CENTER_LEFT) +
				virtualChannelStagingSpace[TRUE_LEFT][i] * vspd(MID_FRONT_LEFT, TRUE_LEFT) +
				virtualChannelStagingSpace[OVERHEAD][i] * 0.25f +
				virtualChannelStagingSpace[UP_FRONT_LEFT][i] * vspd(MID_FRONT_LEFT, UP_FRONT_LEFT) +
				virtualChannelStagingSpace[UP_FRONT_CENTER][i] * vspd(MID_FRONT_LEFT, UP_FRONT_CENTER);


			toReturn.channelBuffers[1][i] =
				virtualChannelStagingSpace[MID_FRONT_RIGHT][i] +
				virtualChannelStagingSpace[MID_FRONT_CENTER][i] * vspd(MID_FRONT_RIGHT, MID_FRONT_CENTER) +
				virtualChannelStagingSpace[MID_FRONT_CENTER_RIGHT][i] * vspd(MID_FRONT_RIGHT, MID_FRONT_CENTER_RIGHT) +
				virtualChannelStagingSpace[TRUE_RIGHT][i] * vspd(MID_FRONT_RIGHT, TRUE_RIGHT) +
				virtualChannelStagingSpace[OVERHEAD][i] * 0.25f +
				virtualChannelStagingSpace[UP_FRONT_RIGHT][i] * vspd(MID_FRONT_RIGHT, UP_FRONT_RIGHT) +
				virtualChannelStagingSpace[UP_FRONT_CENTER][i] * vspd(MID_FRONT_RIGHT, UP_FRONT_CENTER);

			toReturn.channelBuffers[2][i] = virtualChannelStagingSpace[SUBWOOFER][i];

			toReturn.channelBuffers[3][i] =
				virtualChannelStagingSpace[MID_BACK_LEFT][i] +
				virtualChannelStagingSpace[MID_BACK_CENTER][i] * vspd(MID_BACK_LEFT, MID_BACK_CENTER) +
				virtualChannelStagingSpace[TRUE_LEFT][i] * vspd(MID_BACK_LEFT, TRUE_LEFT) +
				virtualChannelStagingSpace[OVERHEAD][i] * 0.25f +
				virtualChannelStagingSpace[UP_BACK_LEFT][i] * vspd(MID_BACK_LEFT, UP_BACK_LEFT) +
				virtualChannelStagingSpace[UP_BACK_CENTER][i] * vspd(MID_BACK_LEFT, UP_BACK_CENTER);

			toReturn.channelBuffers[4][i] =
				virtualChannelStagingSpace[MID_BACK_RIGHT][i] +
				virtualChannelStagingSpace[MID_BACK_CENTER][i] * vspd(MID_BACK_RIGHT, MID_BACK_CENTER) +
				virtualChannelStagingSpace[TRUE_RIGHT][i] * vspd(MID_BACK_RIGHT, TRUE_RIGHT) +
				virtualChannelStagingSpace[OVERHEAD][i] * 0.25f +
				virtualChannelStagingSpace[UP_BACK_RIGHT][i] * vspd(MID_BACK_RIGHT, UP_BACK_RIGHT) +
				virtualChannelStagingSpace[UP_BACK_CENTER][i] * vspd(MID_BACK_RIGHT, UP_BACK_CENTER);
		}
		break;
	case 6:
		for (size_t i = 0; i < channelSize; i++) {
			//fL, fR, fC, sW, sL, sR
			toReturn.channelBuffers[0][i] =
				virtualChannelStagingSpace[MID_FRONT_LEFT][i] +
				virtualChannelStagingSpace[MID_FRONT_CENTER_LEFT][i] * vspd(MID_FRONT_LEFT, MID_FRONT_CENTER_LEFT) +
				virtualChannelStagingSpace[OVERHEAD][i] * 0.2f +
				virtualChannelStagingSpace[UP_FRONT_LEFT][i] * vspd(MID_FRONT_LEFT, UP_FRONT_LEFT);

			toReturn.channelBuffers[1][i] =
				virtualChannelStagingSpace[MID_FRONT_RIGHT][i] +
				virtualChannelStagingSpace[MID_FRONT_CENTER_RIGHT][i] * vspd(MID_FRONT_RIGHT, MID_FRONT_CENTER_RIGHT) +
				virtualChannelStagingSpace[OVERHEAD][i] * 0.2f +
				virtualChannelStagingSpace[UP_FRONT_RIGHT][i] * vspd(MID_FRONT_RIGHT, UP_FRONT_RIGHT);

			toReturn.channelBuffers[2][i] =
				virtualChannelStagingSpace[MID_FRONT_CENTER][i] +
				virtualChannelStagingSpace[MID_FRONT_CENTER_LEFT][i] * vspd(MID_FRONT_CENTER, MID_FRONT_CENTER_LEFT) +
				virtualChannelStagingSpace[MID_FRONT_CENTER_RIGHT][i] * vspd(MID_FRONT_CENTER, MID_FRONT_CENTER_RIGHT) +
				virtualChannelStagingSpace[OVERHEAD][i] * 0.2f +
				virtualChannelStagingSpace[UP_FRONT_CENTER][i] * vspd(MID_FRONT_CENTER, UP_FRONT_CENTER);

			toReturn.channelBuffers[3][i] =
				virtualChannelStagingSpace[SUBWOOFER][i];

			toReturn.channelBuffers[4][i] =
				virtualChannelStagingSpace[MID_BACK_LEFT][i] * vspd(TRUE_LEFT, MID_BACK_LEFT) +
				virtualChannelStagingSpace[MID_BACK_CENTER][i] * 0.5f +
				virtualChannelStagingSpace[TRUE_LEFT][i] +
				virtualChannelStagingSpace[OVERHEAD][i] * 0.2f +
				virtualChannelStagingSpace[UP_BACK_LEFT][i] * vspd(TRUE_LEFT, UP_BACK_LEFT) +
				virtualChannelStagingSpace[UP_BACK_CENTER][i] * 0.5f;

			toReturn.channelBuffers[5][i] =
				virtualChannelStagingSpace[MID_BACK_RIGHT][i] * vspd(TRUE_RIGHT, MID_BACK_RIGHT) +
				virtualChannelStagingSpace[MID_BACK_CENTER][i] * 0.5f +
				virtualChannelStagingSpace[TRUE_RIGHT][i] +
				virtualChannelStagingSpace[OVERHEAD][i] * 0.2f +
				virtualChannelStagingSpace[UP_BACK_RIGHT][i] * vspd(TRUE_RIGHT, UP_BACK_RIGHT) +
				virtualChannelStagingSpace[UP_BACK_CENTER][i] * 0.5f;
		}
		break;
	case 7:
		for (size_t i = 0; i < channelSize; i++) {
			//fL, fR, fC, sW, bC, sL, sR
			toReturn.channelBuffers[0][i] =
				virtualChannelStagingSpace[MID_FRONT_LEFT][i] +
				virtualChannelStagingSpace[MID_FRONT_CENTER_LEFT][i] * vspd(MID_FRONT_LEFT, MID_FRONT_CENTER_LEFT) +
				virtualChannelStagingSpace[OVERHEAD][i] / 6.f +
				virtualChannelStagingSpace[UP_FRONT_LEFT][i] * vspd(MID_FRONT_LEFT, UP_FRONT_LEFT);

			toReturn.channelBuffers[1][i] =
				virtualChannelStagingSpace[MID_FRONT_RIGHT][i] +
				virtualChannelStagingSpace[MID_FRONT_CENTER_RIGHT][i] * vspd(MID_FRONT_RIGHT, MID_FRONT_CENTER_RIGHT) +
				virtualChannelStagingSpace[OVERHEAD][i] / 6.f +
				virtualChannelStagingSpace[UP_FRONT_RIGHT][i] * vspd(MID_FRONT_RIGHT, UP_FRONT_RIGHT);

			toReturn.channelBuffers[2][i] =
				virtualChannelStagingSpace[MID_FRONT_CENTER][i] +
				virtualChannelStagingSpace[MID_FRONT_CENTER_LEFT][i] * vspd(MID_FRONT_CENTER, MID_FRONT_CENTER_LEFT) +
				virtualChannelStagingSpace[MID_FRONT_CENTER_RIGHT][i] * vspd(MID_FRONT_CENTER, MID_FRONT_CENTER_RIGHT) +
				virtualChannelStagingSpace[OVERHEAD][i] / 6.f +
				virtualChannelStagingSpace[UP_FRONT_CENTER][i] * vspd(MID_FRONT_CENTER, UP_FRONT_CENTER);

			toReturn.channelBuffers[3][i] = virtualChannelStagingSpace[SUBWOOFER][i];

			toReturn.channelBuffers[4][i] =
				virtualChannelStagingSpace[MID_BACK_LEFT][i] * vspd(MID_BACK_CENTER, MID_BACK_LEFT) +
				virtualChannelStagingSpace[MID_BACK_RIGHT][i] * vspd(MID_BACK_CENTER, MID_BACK_RIGHT) +
				virtualChannelStagingSpace[MID_BACK_CENTER][i] +
				virtualChannelStagingSpace[OVERHEAD][i] / 6.f +
				virtualChannelStagingSpace[UP_BACK_LEFT][i] * vspd(MID_BACK_CENTER, UP_BACK_LEFT) +
				virtualChannelStagingSpace[UP_BACK_CENTER][i] * vspd(MID_BACK_CENTER, UP_BACK_CENTER) +
				virtualChannelStagingSpace[UP_BACK_RIGHT][i] * vspd(MID_BACK_CENTER, UP_BACK_RIGHT);

			toReturn.channelBuffers[5][i] =
				virtualChannelStagingSpace[MID_BACK_LEFT][i] * vspd(TRUE_LEFT, MID_BACK_LEFT) +
				virtualChannelStagingSpace[TRUE_LEFT][i] +
				virtualChannelStagingSpace[OVERHEAD][i] / 6.f +
				virtualChannelStagingSpace[UP_BACK_LEFT][i] * vspd(TRUE_LEFT, UP_BACK_LEFT);

			toReturn.channelBuffers[6][i] =
				virtualChannelStagingSpace[MID_BACK_RIGHT][i] * vspd(TRUE_RIGHT, MID_BACK_RIGHT) +
				virtualChannelStagingSpace[TRUE_RIGHT][i] +
				virtualChannelStagingSpace[OVERHEAD][i] / 6.f +
				virtualChannelStagingSpace[UP_BACK_RIGHT][i] * vspd(TRUE_RIGHT, UP_BACK_RIGHT);
		}
		break;
	case 8:	
		for (size_t i = 0; i < channelSize; i++) {
			//fL, fR, fC, sW, bL, bR, sL, sR
			toReturn.channelBuffers[0][i] =
				virtualChannelStagingSpace[MID_FRONT_LEFT][i] +
				virtualChannelStagingSpace[MID_FRONT_CENTER_LEFT][i] * vspd(MID_FRONT_LEFT, MID_FRONT_CENTER_LEFT) +
				virtualChannelStagingSpace[OVERHEAD][i] / 7.f +
				virtualChannelStagingSpace[UP_FRONT_LEFT][i] * vspd(MID_FRONT_LEFT, UP_FRONT_LEFT);

			toReturn.channelBuffers[1][i] =
				virtualChannelStagingSpace[MID_FRONT_RIGHT][i] +
				virtualChannelStagingSpace[MID_FRONT_CENTER_RIGHT][i] * vspd(MID_FRONT_RIGHT, MID_FRONT_CENTER_RIGHT) +
				virtualChannelStagingSpace[OVERHEAD][i] / 7.f +
				virtualChannelStagingSpace[UP_FRONT_RIGHT][i] * vspd(MID_FRONT_RIGHT, UP_FRONT_RIGHT);

			toReturn.channelBuffers[2][i] =
				virtualChannelStagingSpace[MID_FRONT_CENTER][i] +
				virtualChannelStagingSpace[MID_FRONT_CENTER_LEFT][i] * vspd(MID_FRONT_CENTER, MID_FRONT_CENTER_LEFT) +
				virtualChannelStagingSpace[MID_FRONT_CENTER_RIGHT][i] * vspd(MID_FRONT_CENTER, MID_FRONT_CENTER_RIGHT) +
				virtualChannelStagingSpace[OVERHEAD][i] / 7.f +
				virtualChannelStagingSpace[UP_FRONT_CENTER][i] * vspd(MID_FRONT_CENTER, UP_FRONT_CENTER);

			toReturn.channelBuffers[3][i] = virtualChannelStagingSpace[SUBWOOFER][i];

			toReturn.channelBuffers[4][i] =
				virtualChannelStagingSpace[MID_BACK_LEFT][i] +
				virtualChannelStagingSpace[MID_BACK_CENTER][i] * vspd(MID_BACK_LEFT, MID_BACK_CENTER) +
				virtualChannelStagingSpace[OVERHEAD][i] / 7.f +
				virtualChannelStagingSpace[UP_BACK_LEFT][i] * vspd(MID_BACK_LEFT, UP_BACK_LEFT) +
				virtualChannelStagingSpace[UP_BACK_CENTER][i] * vspd(MID_BACK_LEFT, UP_BACK_CENTER);

			toReturn.channelBuffers[5][i] =
				virtualChannelStagingSpace[MID_BACK_RIGHT][i] +
				virtualChannelStagingSpace[MID_BACK_CENTER][i] * vspd(MID_BACK_RIGHT, MID_BACK_CENTER) +
				virtualChannelStagingSpace[OVERHEAD][i] / 7.f +
				virtualChannelStagingSpace[UP_BACK_CENTER][i] * vspd(MID_BACK_RIGHT, UP_BACK_CENTER) +
				virtualChannelStagingSpace[UP_BACK_RIGHT][i] * vspd(MID_BACK_RIGHT, UP_BACK_RIGHT);

			toReturn.channelBuffers[6][i] =
				virtualChannelStagingSpace[TRUE_LEFT][i] +
				virtualChannelStagingSpace[OVERHEAD][i] / 7.f;

			toReturn.channelBuffers[7][i] =
				virtualChannelStagingSpace[TRUE_RIGHT][i] +
				virtualChannelStagingSpace[OVERHEAD][i] / 7.f;
		}
		break;
	default:
		std::cout << "We don't support that many channels yet! (but I like your style)";
		break;
	}

	//now just peak normalize per channel and we're golden

	for (auto& c : toReturn.channelBuffers) {
		float channelMax = -1.f;
		float channelMin = 1.f;
		for (float& s : c) {
			if (s > channelMax) channelMax = s;
			if (s < channelMin) channelMin = s;
		}
		channelMax = channelMax > 1.f ? channelMax - 1.f : 0.f;
		channelMin = channelMin < -1.f ? channelMin + 1.f : 0.f;
		for (float& s : c) {
			s -= s > 0.f ? channelMax : channelMin;
		}
	}
	return toReturn;
}

//because the synthesizer is generated here, it should always work in a format native to the hardware
//this means we don't have to worry about channel number or sample rate conversions
//what we do have to worry about is what channels to write to.
//I'm thinking we have a directional sound channel and an optional bass channel
//The directional fills out hardware channels based on normals and such, it defaults to the mid front center channel
//or maybe we default to overhead speaker since as it stands, that fills all channels equally.
//the bass channel fills the LF channel directly.  Reccommendation is that anything up to 120Hz gets put here.
//Reccommended 80 as your center for smooth mixing in this channel
//dB is another matter.  Apparently woofers auto-boost anything sent to them by +10 dB.  Keep this in mind.
//advice: the LF channel should be used like the audio version of controller rumble, and sparingly.  Kinda obvious.

//also for both files and synth, need ways of adding and removing from mixer

//another thing: synth helps prototype for file audio streaming?

//reminder of the pending huge refactor!!!

int main(int argc, char* args[]) {
	/**
	surveyHardware();
	/**
	compileShaders();
	/**
	createManifest();
	/**
	compileManifest(true);
	/**/
	std::string testString;
	External_WaveFileData
		testData1{},
		testData2{},
		testData3{},
		testData4{},
		testData5{},
		testData6{},
		testData7{},
		testData8{},
		testData9{},
		testData10{},
		testData11{},
		testData12{},
		testData13{},
		testData14{},
		testData15{},
		testData16{},
		testData17{},
		testData18{},

		testData19{},
		testData20{},
		testData21{},

		testData22{},
		testData23{},

		testData24{},
		testData25{},
		testData26{},
		testData27{},

		errorData1{},
		errorData2{},
		errorData3{},
		errorData4{},

		sunshine{},
		starwars{},
		cheese{};

	testString = "Assets/Sounds/ALAW_STEREO.wav";
	importWaveFile(testString, testData1);
	testString = "Assets/Sounds/ALAW_STEREO_EXT.wav";
	importWaveFile(testString, testData2);
	testString = "Assets/Sounds/MULAW_STEREO.wav";
	importWaveFile(testString, testData3);
	testString = "Assets/Sounds/MULAW_STEREO_EXT.wav";
	importWaveFile(testString, testData4);
	testString = "Assets/Sounds/PCM8_STEREO.wav";
	importWaveFile(testString, testData5);
	testString = "Assets/Sounds/PCM8_STEREO_EXT.wav";
	importWaveFile(testString, testData6);
	testString = "Assets/Sounds/PCM12_STEREO.wav";
	importWaveFile(testString, testData7);
	testString = "Assets/Sounds/PCM12_STEREO_EXT.wav";
	importWaveFile(testString, testData8);
	testString = "Assets/Sounds/PCM16_STEREO.wav";
	importWaveFile(testString, testData9);
	testString = "Assets/Sounds/PCM16_STEREO_EXT.wav";
	importWaveFile(testString, testData10);
	testString = "Assets/Sounds/PCM24_STEREO.wav";
	importWaveFile(testString, testData11);
	testString = "Assets/Sounds/PCM24_STEREO_EXT.wav";
	importWaveFile(testString, testData12);
	testString = "Assets/Sounds/PCM32_STEREO.wav";
	importWaveFile(testString, testData13);
	testString = "Assets/Sounds/PCM32_STEREO_EXT.wav";
	importWaveFile(testString, testData14);
	testString = "Assets/Sounds/FLOAT32_STEREO.wav";
	importWaveFile(testString, testData15);
	testString = "Assets/Sounds/FLOAT32_STEREO_EXT.wav";
	importWaveFile(testString, testData16);
	testString = "Assets/Sounds/FLOAT64_STEREO.wav";
	importWaveFile(testString, testData17);
	testString = "Assets/Sounds/FLOAT64_STEREO_EXT.wav";
	importWaveFile(testString, testData18);
	testString = "Assets/Sounds/ALAW_MONO.wav";
	importWaveFile(testString, testData19);
	testString = "Assets/Sounds/MULAW_MONO.wav";
	importWaveFile(testString, testData20);
	testString = "Assets/Sounds/GSM610_MONO.wav";//note: the devil
	importWaveFile(testString, testData21);
	testString = "Assets/Sounds/PCM16_6CHAN_63.wav";
	importWaveFile(testString, testData22);
	testString = "Assets/Sounds/PCM24_8CHAN_63.wav";
	importWaveFile(testString, testData23);
	testString = "Assets/Sounds/PCM16_STEREO_MISCCHUNKS.wav";
	importWaveFile(testString, testData24);
	testString = "Assets/Sounds/FLOAT32_STEREO_MISCCHUNKS.wav";
	importWaveFile(testString, testData25);
	testString = "Assets/Sounds/PCM16_4CHAN_263.wav";
	importWaveFile(testString, testData26);
	testString = "Assets/Sounds/PCM16_4CHAN_51.wav";
	importWaveFile(testString, testData27);
	testString = "Assets/Sounds/ERROR_BAD_CHUNK.wav";
	importWaveFile(testString, errorData1);
	testString = "Assets/Sounds/ERROR_JUNK_AFTER_RIFF.wav";
	importWaveFile(testString, errorData2);
	testString = "Assets/Sounds/ERROR_WRONG_RIFF_CHUNK_SIZE.wav";
	importWaveFile(testString, errorData3);
	testString = "Assets/Sounds/ERROR_BAD_FACT.wav";
	importWaveFile(testString, errorData4);
	//the remaining 2 files on the site are more speech codecs and...I'm just done with those. (for now?)
	testString = "Assets/Sounds/BunBine.wav";//will have to investigate why this is SO LOUD
	importWaveFile(testString, sunshine);
	testString = "Assets/Sounds/ImperialMarch60.wav";
	importWaveFile(testString, starwars);
	testString = "Assets/Sounds/sample.wav";
	importWaveFile(testString, cheese);
	testString = "Hello World!";
//*********************************************************************************** Init ***************************************************************************
	SystemsManager* systems = new SystemsManager();
	EntitySystem* ez = systems->getEntitySystem();
	VideoManager* vz = systems->getVideoSystem();
	EventSystem* eventest = systems->data.eventSystem;

	Camera* someCamera = new Camera(vz->getSettings());
	RenderSystem* graphicsRenderer = new RenderSystem(vz->getSettings(), someCamera);
	graphicsRenderer->changeSkybox(STRINGtoUUID("cubemap:sky"));

	uint32_t cTexID = STRINGtoUUID("texture:cat");
	uint32_t mTexID = STRINGtoUUID("texture:dennis");
	uint32_t tTexID = STRINGtoUUID("texture:turkey");
	uint32_t vTexID = STRINGtoUUID("texture:viking_room");
	uint32_t hTexID = STRINGtoUUID("texture:hat");

	uint32_t cModID = STRINGtoUUID("mesh:cat");
	uint32_t mModID = STRINGtoUUID("mesh:dennis");
	uint32_t tModID = STRINGtoUUID("mesh:turkey");
	uint32_t vModID = STRINGtoUUID("mesh:viking_room");
	uint32_t hModID = STRINGtoUUID("mesh:hat");

	uint32_t gMatID = STRINGtoUUID("material:test");
	uint32_t lMatID = STRINGtoUUID("material:lite");
	uint32_t sMatID = STRINGtoUUID("material:simple");
	//--------------------------------------------------------------------------------------------------------

	uint32_t planeEnt = ez->makeEntity();
	ez->addComponent(TRANSFORM_COMPONENT, planeEnt);
	ez->addComponent(RENDERABILITY_COMPONENT, planeEnt);
	RenderabilityComponent* planeRC = ez->getRenderabilityComponent(planeEnt);
	planeRC->changeMaterial(gMatID);
	TransformComponent* planeTC = ez->getTransformComponent(planeEnt);
	planeTC->rotate(glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));
	planeTC->translate(glm::vec3(0.f, -1.f, 0.f));
	planeTC->scale(500.f);

	//Computer* computer = new Computer();

	Atlas* alas = new Atlas(vz->getSettings());
	

	uint32_t textBoxW = vz->getSettings()->windowWidth / 2;
	uint32_t textBoxH = vz->getSettings()->windowHeight / 2;

	TextBox* texMex = new TextBox(vz->getSettings(), 0, 0, textBoxW, textBoxH);
	texMex->display("Have a Holly Jolly Christmas!", alas, 75);
	//there's an error where if the point is too big, everything breaks.  Not critical for now so...just note it.


	//note: There are 4 pixels for every 3 points of font, point measures font height
	//also note: apparently there's a different measure that CS use from Printed Font for point
	//look into it when we circle back to do UI in earnest


	uint32_t testObject1 = ez->makeEntity();
	ez->addComponent(TRANSFORM_COMPONENT, testObject1);
	ez->addComponent(RENDERABILITY_COMPONENT, testObject1);
	uint32_t testObject2 = ez->makeEntity();
	ez->addComponent(TRANSFORM_COMPONENT, testObject2);
	ez->addComponent(RENDERABILITY_COMPONENT, testObject2);
	uint32_t testObject3 = ez->makeEntity();
	ez->addComponent(TRANSFORM_COMPONENT, testObject3);
	ez->addComponent(RENDERABILITY_COMPONENT, testObject3);
	uint32_t testObject4 = ez->makeEntity();
	ez->addComponent(TRANSFORM_COMPONENT, testObject4);
	ez->addComponent(RENDERABILITY_COMPONENT, testObject4);
	//uint32_t testLight1 =  ez->makeEntity();
	//ez->addComponent(TRANSFORM_COMPONENT, testLight1);
	//ez->addComponent(RENDERABILITY_COMPONENT, testLight1);
	uint32_t hohoho1 = ez->makeEntity();
	ez->addComponent(TRANSFORM_COMPONENT, hohoho1);
	ez->addComponent(RENDERABILITY_COMPONENT, hohoho1);
	RenderabilityComponent* horc1 = ez->getRenderabilityComponent(hohoho1);
	horc1->changeMaterial(gMatID);
	horc1->changeMesh(hModID);
	horc1->changeTexture(hTexID);
	TransformComponent* hotc1 = ez->getTransformComponent(hohoho1);
	hotc1->translate(glm::vec3(10.f, 172.f, 0.f));
	hotc1->scale(2.70f);
	hotc1->scale(glm::vec3(1.35f, 1.25f, 1.f));
	hotc1->rotate(-0.5f, glm::vec3(0.f, 1.f, 0.f));

	uint32_t hohoho2 = ez->makeEntity();
	ez->addComponent(TRANSFORM_COMPONENT, hohoho2);
	ez->addComponent(RENDERABILITY_COMPONENT, hohoho2);
	RenderabilityComponent* horc2 = ez->getRenderabilityComponent(hohoho2);
	horc2->changeMaterial(gMatID);
	horc2->changeMesh(hModID);
	horc2->changeTexture(hTexID);
	TransformComponent* hotc2 = ez->getTransformComponent(hohoho2);
	hotc2->translate(glm::vec3(15.f, 33.f, 21.f));
	hotc2->scale(1.2f);

	uint32_t hohoho3 = ez->makeEntity();
	ez->addComponent(TRANSFORM_COMPONENT, hohoho3);
	ez->addComponent(RENDERABILITY_COMPONENT, hohoho3);
	RenderabilityComponent* horc3 = ez->getRenderabilityComponent(hohoho3);
	horc3->changeMaterial(gMatID);
	horc3->changeMesh(hModID);
	horc3->changeTexture(hTexID);
	TransformComponent* hotc3 = ez->getTransformComponent(hohoho3);
	hotc3->translate(glm::vec3(35.f, 58.f, 50.f));
	hotc3->rotate(0.25f, glm::vec3(0.f, 0.f, 1.f));

	RenderabilityComponent* testRC1 = ez->getRenderabilityComponent(testObject1);
	testRC1->changeMaterial(gMatID);
	testRC1->changeMesh(vModID);
	testRC1->changeTexture(vTexID);
	RenderabilityComponent* testRC2 = ez->getRenderabilityComponent(testObject2);
	testRC2->changeMaterial(gMatID);
	testRC2->changeMesh(cModID);
	testRC2->changeTexture(cTexID);
	RenderabilityComponent* testRC3 = ez->getRenderabilityComponent(testObject3);
	testRC3->changeMaterial(gMatID);
	testRC3->changeMesh(mModID);
	testRC3->changeTexture(mTexID);
	//RenderabilityComponent* testRC4 = ez->getRenderabilityComponent(testLight1);
	//testRC4->changeMaterial(lMatID);
	RenderabilityComponent* testRC5 = ez->getRenderabilityComponent(testObject4);
	testRC5->changeMaterial(gMatID);
	testRC5->changeMesh(tModID);
	testRC5->changeTexture(tTexID);

	RenderBatch hickory(vz->getSettings(), ez->getRenderabilityComponent(texMex->entityID));
	//RenderBatch dickory(vz->getSettings(), testRC4);
	RenderBatch dock(vz->getSettings(), testRC1);
	dock.add(testRC2);
	dock.add(testRC3);
	dock.add(testRC5);
	dock.add(planeRC);
	dock.add(horc1);
	dock.add(horc2);
	dock.add(horc3);

	std::vector<RenderBatch> scene;
	scene.push_back(hickory);
	//todo:         dickory
	scene.push_back(dock);
	//text:         clock

	TransformComponent* testTC1 = ez->getTransformComponent(testObject1);
	TransformComponent* testTC2 = ez->getTransformComponent(testObject2);
	testTC2->rotate(M_PI / 2, glm::vec3(0, 1, 0));
	testTC2->rotate(-M_PI / 2, glm::vec3(1, 0, 0));
	testTC2->translate(glm::vec3(0, 0, 20));
	TransformComponent* testTC3 = ez->getTransformComponent(testObject3);
	//TransformComponent* testTC4 = ez->getTransformComponent(testLight1);
	TransformComponent* testTC5 = ez->getTransformComponent(testObject4);
	testTC5->rotate(-M_PI/2, glm::vec3(1, 0, 0));
	testTC5->translate(glm::vec3(0, 0, 50));

	MovementGroup wao;
	wao.add(testTC1);
	wao.add(testTC2);
	wao.add(testTC3);
	//wao.add(testTC4);
	wao.add(testTC5);

	bool playing = true, suspended = false;
	int loopTimes = 0;

	auto brap = [&](SDL_WindowEvent e) -> void {
		if (e.event == SDL_WINDOWEVENT_CLOSE) playing = false;
		if (e.event == SDL_WINDOWEVENT_MINIMIZED) suspended = true;
		if (e.event == SDL_WINDOWEVENT_RESTORED) suspended = false;
		};
	std::function<void(SDL_WindowEvent)> dirk = brap;
	eventest->subscribe(dirk);

	MixerDirectInput atestin = somethingSomethingWaves(cheese);
	atestin.playbackCursor = 0;
	atestin.loopBeg = 0;
	atestin.loopEnd = atestin.channelBuffers[0].size() - 1;
	atestin.playbackFrequency = 1;
	atestin.playbackSpeed = 1.f;
	atestin.playbackVolume = 1.f;
	atestin.isMuted = false;
	atestin.isPaused = false;
	//MixerDirectInput btestin = somethingSomethingWaves(starwars);
	systems->data.audioSystem->outputMixer->recordFromFile(&atestin);
	//systems->data.audioSystem->outputMixer->recordFromFile(btestin);

	auto musCtrlTest = [&](SDL_KeyboardEvent e) -> void {
		if (e.type == SDL_KEYDOWN) {
			switch (e.keysym.scancode)
			{
			case SDL_SCANCODE_GRAVE:
				atestin.stop();
				break;
			case SDL_SCANCODE_0:
				atestin.togglePlay();
				break;
			case SDL_SCANCODE_9:
				atestin.toggleMute();
				break;
			case SDL_SCANCODE_BACKSPACE:
				atestin.reverse();
				break;
			case SDL_SCANCODE_MINUS:
				atestin.dilate(0.5);
				break;
			case SDL_SCANCODE_EQUALS:
				atestin.dilate(2.0);
				break;
			case SDL_SCANCODE_1:
				atestin.setVolumeFactor(atestin.playbackVolume - 0.05);
				break;
			case SDL_SCANCODE_2:
				atestin.setVolumeFactor(atestin.playbackVolume + 0.05);
				break;
			case SDL_SCANCODE_3:
				atestin.skipSamples(-(int)hardwareInfo.soundCardInfo.outputSamplesPerChannel);
				break;
			case SDL_SCANCODE_4:
				atestin.skipSamples((int)hardwareInfo.soundCardInfo.outputSamplesPerChannel);
				break;
			case SDL_SCANCODE_5:
				atestin.moveLoopStart(atestin.loopBeg - hardwareInfo.soundCardInfo.outputSamplesPerChannel);
				break;
			case SDL_SCANCODE_6:
				atestin.moveLoopStart(atestin.loopBeg + hardwareInfo.soundCardInfo.outputSamplesPerChannel);
				break;
			case SDL_SCANCODE_7:
				atestin.moveLoopEnd(atestin.loopEnd - hardwareInfo.soundCardInfo.outputSamplesPerChannel);
				break;
			case SDL_SCANCODE_8:
				atestin.moveLoopEnd(atestin.loopEnd + hardwareInfo.soundCardInfo.outputSamplesPerChannel);
				break;
			default:
				break;
			}
		}
	};
	std::function<void(SDL_KeyboardEvent)> dork = musCtrlTest;
	eventest->subscribe(dork);

	Synthesizer::Instrument* testinst = new Synthesizer::SinInstrument();
	//systems->data.audioSystem->testSequencer->add(testinst, 0, 440);
	//systems->data.audioSystem->testSequencer->add(testinst, 19, 270);
	systems->data.audioSystem->beginPlayback();

	while (playing) {
//*********************************************************************************** Input ***************************************************************************
		/*
		when we have events dispatching to a job system, do we want to process events at the end of the frame?
		The idea being that we have a target framerate that we want to hit and if we finish our processing early then
		we use the remaining frametime to work through events.  Then, if we run out of time before the queue is empty
		we save what's in the queue to process next frame.  All this so that we're not stalling anything in order
		to process events.

		We want to peep up here for quitting, where we'll quit and stop processing.
		if (SDL_HasEvent(SDL_QUIT)) ...
		We actually process the event queue at the tail end otherwise.
		*/

		eventest->processEvents();
		//tip for keyboard events: use scancodes for controls and keycodes for typing.

		systems->data.audioSystem->testSequencer->update(eventest->getDeltaTime());

//*********************************************************************************** Update **************************************************************************

		if (!suspended) {
			//*********************************************************************************** Feedback *************************************************************************
			someCamera->control->update();//maybe all component updates should be registered with and called by the input system.
			//if (++loopTimes % 10 == 0) {
			//	std::string dynamism = "Happy Thanksgiving!  We've looped about [" + std::to_string(loopTimes) + "] times.";
			//	texMex->display(dynamism, alas, 24);
			//}

			//wao.rotateAll();
			graphicsRenderer->render(scene);
		}
	};
//*********************************************************************************** Shutdown ************************************************************************
	vz->exposeAccelerator()->idle();//may be replaced when we have a proper event system
	std::cout << "\n\n\n\n\n" << std::endl;

	delete someCamera;

	delete alas;
	delete texMex;

	delete graphicsRenderer;//despite the struct name I'm not sure this is a system.

	delete systems;//every delete of a non-system represents work still to do.
	SDL_Quit();
	return EXIT_SUCCESS;
}
