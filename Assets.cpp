#define STB_IMAGE_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION
#include "Assets.hpp"

#include<spirv_reflect.c>



namespace std {
	template<>
	struct hash<SAGE::Vertex> {
		size_t operator()(SAGE::Vertex const& vert) const {
			size_t seed = 0;
			SAGE::hashCombine(seed, vert.position, vert.normal,  vert.color, vert.texturePosition);
			return seed;
		}
	};
}

namespace SAGE {
	AssetManager* AssetManager::instance = nullptr;
	HardwareInfo hardwareInfo;

	uint32_t roundPow2(uint32_t i) {
		if (std::_Is_pow_2(i)) return i;
		uint32_t pow = 1;
		uint32_t check = i;
		while (check >>= 1) {
			pow<<=1;
		}
		return ( i < pow + (pow >> 1)) ? pow : pow << 1;
	}

	VkCompareOp stringToVkCompareOp(const std::string& s) {
		if (s == "1" || s == "always") return VK_COMPARE_OP_ALWAYS;
		if (s == "==" || s == "equal") return VK_COMPARE_OP_EQUAL;
		if (s == "!=" || s == "notEqual") return VK_COMPARE_OP_NOT_EQUAL;
		if (s == ">=" || s == "greaterOrEqual") return VK_COMPARE_OP_GREATER_OR_EQUAL;
		if (s == "<=" || s == "lessOrEqual") return VK_COMPARE_OP_LESS_OR_EQUAL;
		if (s == ">" || s == "greater") return VK_COMPARE_OP_GREATER;
		if (s == "<" || s == "less") return VK_COMPARE_OP_LESS;
		return VK_COMPARE_OP_NEVER;
	}
	VkLogicOp stringToVkLogicOp(const std::string& s) {
		if (s == "_" || s == "d" || s == "snd" || s == "none") return VK_LOGIC_OP_NO_OP;
		if (s == "1" || s == "one" || s == "setOne") return VK_LOGIC_OP_SET;
		if (s == "c" || s == "s" || s == "fst" || s == "copy") return VK_LOGIC_OP_COPY;
		if (s == "&" || s == "s&d" || s == "and") return VK_LOGIC_OP_AND;
		if (s == "|" || s == "s|d" || s == "or") return VK_LOGIC_OP_OR;
		if (s == "^" || s == "s^d" || s == "xor") return VK_LOGIC_OP_XOR;
		if (s == "!|" || s == "~(s|d)" || s == "nor") return VK_LOGIC_OP_NOR;
		if (s == "!&" || s == "~(s&d)" || s == "nand") return VK_LOGIC_OP_NAND;
		if (s == "!^" || s == "~(s^d)" || s == "xnor" || s == "equal") return VK_LOGIC_OP_EQUIVALENT;
		if (s == "i" || s == "~d" || s == "nsnd" || s == "invert") return VK_LOGIC_OP_INVERT;
		if (s == "~&" || s == "~s&d" || s == "iand") return VK_LOGIC_OP_AND_INVERTED;
		if (s == "&~" || s == "s&~d" || s == "rand") return VK_LOGIC_OP_AND_REVERSE;
		if (s == "~|" || s == "~s|d" || s == "ior") return VK_LOGIC_OP_OR_INVERTED;
		if (s == "|~" || s == "s|~d" || s == "ror") return VK_LOGIC_OP_OR_REVERSE;
		if (s == "~c" || s == "~s" || s == "nfst" || s == "icopy") return VK_LOGIC_OP_COPY_INVERTED;
		return VK_LOGIC_OP_CLEAR;
	}
	VkStencilOp stringToVkStencilOp(const std::string& s) {
		if (s == "0" || s == "zero" || s == "setZero") return VK_STENCIL_OP_ZERO;
		if (s == "r" || s == "replace" || s == "setReference") return VK_STENCIL_OP_REPLACE;
		if (s == "~" || s == "invert" || s == "not") return VK_STENCIL_OP_INVERT;
		if (s == "clamped++" || s == "incrementAndClamp") return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
		if (s == "clamped--" || s == "decrementAndClamp") return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
		if (s == "wrap++" || s == "incrementAndWrap") return VK_STENCIL_OP_INCREMENT_AND_WRAP ;
		if (s == "wrap--" || s == "decrementAndWrap") return VK_STENCIL_OP_DECREMENT_AND_WRAP ;
		return VK_STENCIL_OP_KEEP;
	}
	VkBlendOp stringToVkBlendOp(const std::string& s) {
		if (s == "-" || s == "-+" || s == "subtract") return VK_BLEND_OP_SUBTRACT;
		if (s == "+-" || s == "reverseSubtract") return VK_BLEND_OP_REVERSE_SUBTRACT;
		if (s == "min" || s == "takeMin") return VK_BLEND_OP_MIN;
		if (s == "max" || s == "takeMax") return VK_BLEND_OP_MAX;
		return VK_BLEND_OP_ADD;
	}
	VkBlendFactor stringToVkBlendFactor(const std::string& s) {
		if (s == "1" || s == "one") return VK_BLEND_FACTOR_ONE;
		if (s == "sc" || s == "srcColor") return VK_BLEND_FACTOR_SRC_COLOR;
		if (s == "sa" || s == "srcAlpha") return VK_BLEND_FACTOR_SRC_ALPHA;
		if (s == "dc" || s == "dstColor") return VK_BLEND_FACTOR_DST_COLOR;
		if (s == "da" || s == "dstAlpha") return VK_BLEND_FACTOR_DST_ALPHA;
		if (s == "-sc" || s == "oneMinusSrcColor") return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
		if (s == "-sa" || s == "oneMinusSrcAlpha") return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		if (s == "-dc" || s == "oneMinusDstColor") return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
		if (s == "-da" || s == "oneMinusDstAlpha") return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
		if (s == "cc" || s == "colorConstant") return VK_BLEND_FACTOR_CONSTANT_COLOR;
		if (s == "ca" || s == "alphaConstant") return VK_BLEND_FACTOR_CONSTANT_ALPHA;
		if (s == "-cc" || s == "oneMinusColorConstant") return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
		if (s == "-ca" || s == "oneMinusAlphaConstant") return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
		if (s == "a" || s == "alphaSaturate") return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
		if (s == "s1c" || s == "src1Color") return VK_BLEND_FACTOR_SRC1_COLOR;
		if (s == "s1a" || s == "src1Alpha") return VK_BLEND_FACTOR_SRC1_ALPHA;
		if (s == "-s1c" || s == "oneMinusSrc1Color") return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
		if (s == "-s1a" || s == "oneMinusSrc1Alpha") return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
		return VK_BLEND_FACTOR_ZERO;
	}
	VkDynamicState stringToVkDynamicState(const std::string& s) {
		if (s == "viewport") return VK_DYNAMIC_STATE_VIEWPORT;
		if (s == "scissor") return VK_DYNAMIC_STATE_SCISSOR;
		if (s == "lineWidth" || s == "rasterLineWidth") return VK_DYNAMIC_STATE_LINE_WIDTH;
		if (s == "depthBiasFactors") return VK_DYNAMIC_STATE_DEPTH_BIAS;
		if (s == "blendConstants") return VK_DYNAMIC_STATE_BLEND_CONSTANTS;
		if (s == "depthBounds") return VK_DYNAMIC_STATE_DEPTH_BOUNDS;
		if (s == "stencilCompareMask") return VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK;
		if (s == "stencilWriteMask") return VK_DYNAMIC_STATE_STENCIL_WRITE_MASK;
		if (s == "stencilReference") return VK_DYNAMIC_STATE_STENCIL_REFERENCE;
		if (s == "cullMode") return VK_DYNAMIC_STATE_CULL_MODE;
		if (s == "frontFace") return VK_DYNAMIC_STATE_FRONT_FACE;
		if (s == "primitiveTopology" || s == "vertexListTopology") return VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY;
		if (s == "viewportWithCount" || s == "viewportAndCount") return VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT;
		if (s == "scissorWithCount" || s == "scissorAndCount") return VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT;
		if (s == "vertexInputBindingStride" || s == "vertexStride") return VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE;
		if (s == "depthTest") return VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE;
		if (s == "depthWrite") return VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE;
		if (s == "depthCompare") return VK_DYNAMIC_STATE_DEPTH_COMPARE_OP;
		if (s == "depthBoundsTest") return VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE;
		if (s == "stencilTest") return VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE;
		if (s == "stencilOp") return VK_DYNAMIC_STATE_STENCIL_OP;
		if (s == "rasterizerDiscard") return VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE;
		if (s == "depthBiasEnable") return VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE;
		if (s == "primitiveRestart") return VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE;
		return VK_DYNAMIC_STATE_MAX_ENUM;
	}

	std::string qf2s(QueueFamilyType m) {
		if (m > UNKNOWN_FAMILY) return "you have an error somewhere";
		std::array<std::string, 5> bob{ "Present", "Transfer", "Compute", "Graphics/Compute", "Unclassified" };
		return bob[m];
	}

	std::string queueFlagsToString(VkQueueFlags flags) {
		std::string returnString = "Queue Family Capabilities: ";
		if (flags & VK_QUEUE_GRAPHICS_BIT) returnString += "Graphics, ";
		if (flags & VK_QUEUE_COMPUTE_BIT) returnString += "Compute, ";
		if (flags & VK_QUEUE_TRANSFER_BIT) returnString += "Transfer, ";
		if (flags & VK_QUEUE_SPARSE_BINDING_BIT) returnString += "Sparse Binding, ";
		if (flags & VK_QUEUE_PROTECTED_BIT) returnString += "Protected, ";
#ifdef VK_ENABLE_BETA_EXTENSIONS
		if (flags & VK_QUEUE_VIDEO_DECODE_BIT_KHR) returnString += "Video Decoding, ";
		if (flags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR) returnString += "Video Encoding, ";
#else
		if (flags & 0x00000020 || flags & 0x00000040) returnString += "[Enable Beta Extensions], ";
#endif
		if (flags & VK_QUEUE_OPTICAL_FLOW_BIT_NV) returnString += "Nvidia Optical Flow, ";
		returnString.resize(returnString.size() - 2);
		return returnString;
	}

	QueueFamilySorter::QueueFamilySorter(const std::vector<VkQueueFamilyProperties>& v) {
		for (size_t i = 0; i < v.size(); i++) {
			QueueFam temp;
			temp.maxQueues = v[i].queueCount;
			temp.priority = 1.f;

			if (!(v[i].queueFlags ^ (VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT))) {
				temp.familyUse = TRANSFER_FAMILY;
				exclusiveTransferAvailable = true;
			}
			else if (v[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
				if (v[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
					temp.familyUse = GRAPHICS_COMPUTE_FAMILY;
					gComputeAvailable = true;
					if (v[i].queueCount >= HAX_numPreferredGraphicComputes) enoughGraphics = true;
				}
				else {
					temp.familyUse = ASYNC_COMPUTE_FAMILY;
					asyncComputeAvailable = true;
					if (v[i].queueCount >= HAX_numPreferredAsyncComputes) enoughComputes = true;
				}
			}
			else if (v[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				temp.familyUse = PRESENT_FAMILY;
			}
			else temp.familyUse = UNKNOWN_FAMILY;
			//			std::cout << "Family " << i << " operation bits: " << std::bitset<16>(v[i].queueFlags) << '\n';
			reference.push_back(temp);
		}
	}
	void QueueFamilySorter::prepareOutput() {
		if (exclusiveTransferAvailable && asyncComputeAvailable && enoughComputes && gComputeAvailable && enoughGraphics) {
			for (size_t i = 0; i < reference.size(); i++) {
				switch (reference[i].familyUse)
				{
				case TRANSFER_FAMILY:
					break;
				case ASYNC_COMPUTE_FAMILY:
					reference[i].maxQueues = HAX_numPreferredAsyncComputes;
					break;
				case GRAPHICS_COMPUTE_FAMILY:
					reference[i].maxQueues = HAX_numPreferredGraphicComputes;
					break;
				default:
					reference[i].maxQueues = 0;
					reference[i].familyUse = UNUSED_FAMILY;
					break;
				}
			}
		}
		else {
			std::cout << "panic.";
		}
	}
	void QueueFamilySorter::print() {
		std::cout << std::endl;
		for (size_t i = 0; i < reference.size(); i++) {
			std::cout << "Queue family " << i << ": "
				<< qf2s(reference[i].familyUse) << " family with "
				<< reference[i].maxQueues << " available queues.\n";
		}
		std::cout << std::endl;
	}

	std::string SDL_AudioFormatToString(SDL_AudioFormat f) {
		std::string returnString = "Bit Depth: ";
		Uint16 bitsize = SDL_AUDIO_MASK_BITSIZE & f;

		if (SDL_AUDIO_ISBIGENDIAN(f)) returnString += "Big Endian ";
		else returnString += "Little Endian ";

		switch (bitsize) {
		case 8:
			returnString += "8-bit ";
			break;
		case 16:
			returnString += "16-bit ";
			break;
		case 32:
			returnString += "32-bit ";
			break;
		case 64:
			returnString += "64-bit ";
			break;
		default:
			return "SOMETHING WENT HORRIBLY WRONG HERE!  bitsize is " + std::bitset<8>(bitsize).to_string();
		}

		if (SDL_AUDIO_ISUNSIGNED(f)) returnString += "un";
		returnString += "signed ";

		if (SDL_AUDIO_ISFLOAT(f)) returnString += "float ";
		else returnString += "integer ";

		return returnString;
	}

	SAGEVertexInfo::SAGEVertexInfo(SAGEVertexType v) {
		vertexType = v;

		VkVertexInputBindingDescription temp;
		switch (vertexType)
		{
		case SAGE_NO_VERTEX:
			break;
		case SAGE_DEFAULT_VERTEX:
			temp.binding = 0;
			temp.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			temp.stride = sizeof(Vertex);
			vertexBindingData.push_back(temp);
			vertexInputData.push_back({ 0, temp.binding, VK_FORMAT_R32G32B32_SFLOAT,offsetof(Vertex, position) });
			vertexInputData.push_back({ 1, temp.binding, VK_FORMAT_R32G32B32_SFLOAT,offsetof(Vertex, normal) });
			vertexInputData.push_back({ 2, temp.binding, VK_FORMAT_R32G32B32_SFLOAT,offsetof(Vertex, normalTangent) });
			vertexInputData.push_back({ 3, temp.binding, VK_FORMAT_R32G32B32_SFLOAT,offsetof(Vertex, normalBitangent) });
			vertexInputData.push_back({ 4, temp.binding, VK_FORMAT_R32G32B32A32_SFLOAT,offsetof(Vertex, color) });
			vertexInputData.push_back({ 5, temp.binding, VK_FORMAT_R32G32_SFLOAT,offsetof(Vertex, texturePosition) });
			break;
		case SAGE_SPRITE_VERTEX:
			temp.binding = 0;
			temp.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			temp.stride = sizeof(SpriteVertex);
			vertexBindingData.push_back(temp);
			vertexInputData.push_back({ 0, temp.binding, VK_FORMAT_R32G32B32A32_SFLOAT,offsetof(SpriteVertex, posiTex) });
			break;
		case SAGE_PARTICLE_VERTEX:
			temp.binding = 0;
			temp.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			temp.stride = sizeof(Particle);
			vertexBindingData.push_back(temp);
			vertexInputData.push_back({ 0, temp.binding, VK_FORMAT_R32G32_SFLOAT,offsetof(Particle, position) });
			vertexInputData.push_back({ 1, temp.binding, VK_FORMAT_R32G32_SFLOAT, offsetof(Particle, velocity) });
			vertexInputData.push_back({ 2, temp.binding, VK_FORMAT_R32G32B32A32_SFLOAT,offsetof(Particle, color) });
			break;
		default:
			break;
		}
	}

	bool readWholeBinaryFile(const std::string& path, std::vector<char>& returnVec) {
		std::ifstream file(path, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			return false;
		}

		returnVec.clear();
		size_t fileSize = (size_t)file.tellg();
		returnVec.resize(fileSize);

		file.seekg(0);
		file.read(returnVec.data(), fileSize);

		file.close();

		return true;
	}

	bool readWholeTextFile(const std::string& path, std::vector<std::string>& returnVec) {
		std::ifstream file(path);

		if (!file.is_open())return false;

		returnVec.clear();
		std::string line;
		while (std::getline(file, line)) {
			returnVec.push_back(line);
		}
		file.close();
		return true;
	}

	enum RIFFTag {
		RIFF,
		WAVE,
		FMT,
		DATA,
		FACT,
		UNSUPPORTED
	};

	RIFFTag getTag(std::string& tag) {
		if (tag == "RIFF") return RIFF;
		if (tag == "WAVE") return WAVE;
		if (tag == "fmt ") return FMT;
		if (tag == "fact") return FACT;
		if (tag == "data") return DATA;
		return UNSUPPORTED;
	}

	//for all riff utility functions, we assure that cursor is positioned right after the chunk name
	void parseRIFFHeader(External_WaveFileData& rData, void*& cursor) {
		uint32_t* sizePtr = (uint32_t*)cursor;
		rData.chunkSize = *sizePtr++;
		cursor = sizePtr;
		rData.amtRead += 4;
	}
	void parseWaveFmtChunk(External_WaveFileData& rData, void*& cursor) {
		uint32_t* ptr32 = (uint32_t*)cursor;
		rData.fmtLength = *ptr32++;
		cursor = ptr32;

		uint16_t* ptr16 = (uint16_t*)cursor;
		rData.fmtType = (External_WaveFormats)*ptr16++;
		rData.numChannels = *ptr16++;
		cursor = ptr16;

		ptr32 = (uint32_t*)cursor;
		rData.sampleRate = *ptr32++;
		rData.byteRate = *ptr32++;
		cursor = ptr32;

		ptr16 = (uint16_t*)cursor;
		rData.blockAlign = *ptr16++;
		rData.bitDepth = *ptr16++;
		cursor = ptr16;

		if (rData.fmtLength > 16) {
			if (rData.fmtType == EXTERNAL_WAVE_FORMAT_PCM) rData.isValidFile = false;
			rData.extensionSize = *ptr16++;
			cursor = ptr16;
			if (rData.extensionSize > 0) {
				char* ptr8 = (char*)cursor;
				if (rData.fmtType == WAVE_FORMAT_EXTENSIBLE) {
					rData.validBitsPerSample = *ptr16++;
					cursor = ptr16;

					ptr32 = (uint32_t*)cursor;
					rData.speakerPosMask = *ptr32++;
					cursor = ptr32;

					ptr16 = (uint16_t*)cursor;
					rData.sfmt.subFmtType = (External_WaveFormats)*ptr16++;
					cursor = ptr16;

					ptr8 = (char*)cursor;
					for (uint8_t i = 0; i < 14; i++) {
						rData.sfmt.constant[i] = *ptr8++;
					}
					cursor = ptr8;
				}
				else {
					ptr8 = (char*)cursor;
					std::string log;
					for (size_t i = 0; i < rData.extensionSize; i++) {
						log.push_back(*ptr8++);
					}
					log.push_back('\n');
					cursor = ptr8;
				}
			}
		}
		rData.amtRead += rData.fmtLength;
	}
	void parseWaveDataChunk(External_WaveFileData& rData, void*& cursor) {
		uint32_t* sizePtr = (uint32_t*)cursor;
		rData.dataSize = *sizePtr++;
		cursor = sizePtr;

		if (rData.dataSize & 1) rData.dataSize--;

		char* dataPtr = (char*)cursor;
		rData.data.resize(rData.dataSize);
		for (size_t i = 0; i < rData.data.size(); i++) {
			rData.data[i] = *dataPtr++;
		}
		cursor = dataPtr;
	}
	void parseWaveFactChunk(External_WaveFileData& rData, void*& cursor) {
		uint32_t* ui32Ptr = (uint32_t*)cursor;
		rData.factChunkSize = *ui32Ptr++;
		rData.factNumSamplesPerChannel = *ui32Ptr++;
		cursor = ui32Ptr;
		rData.amtRead += rData.factChunkSize;
	}
	void skipChunk(External_WaveFileData& rData, void*& cursor) {
		uint32_t* sizePtr = (uint32_t*)cursor;
		uint32_t toSkip = *sizePtr++;
		char* skipStep = (char*)sizePtr;
		for (size_t i = 0; i < toSkip; i++) {
			skipStep++;
		}
		cursor = skipStep;
		rData.amtRead += toSkip;
		//we're saved by the fact that in very small files the will usually be big and so we exit immediately
	}

	bool importWaveFile(const std::string& path, External_WaveFileData& rData) {
		std::vector<char> fileData;
		rData.isValidFile = false;
		rData.amtRead = 0;
		if (!readWholeBinaryFile(path, fileData)) {
			return false;
		}
		void* cursor = fileData.data();
		std::string tagTest;
		tagTest.resize(4);
		bool eof = false;

		char* charPtr{};
		while (!eof) {
			charPtr = (char*)cursor;
			tagTest[0] = *charPtr++; tagTest[1] = *charPtr++; tagTest[2] = *charPtr++; tagTest[3] = *charPtr++;
			rData.amtRead += 4;
			cursor = charPtr;

			switch (getTag(tagTest))
			{
			case RIFF:
				strcpy_s(rData.riffMark, tagTest.c_str());
				parseRIFFHeader(rData, cursor);
				break;
			case WAVE:
				strcpy_s(rData.waveMark, tagTest.c_str());
				rData.isValidFile = true;
				break;
			case FMT:
				strcpy_s(rData.fmtMark, tagTest.c_str());
				parseWaveFmtChunk(rData, cursor);
				break;
			case FACT:
				strcpy_s(rData.factMark, tagTest.c_str());
				parseWaveFactChunk(rData, cursor);
				break;
			case DATA:
				strcpy_s(rData.dataMark, tagTest.c_str());
				parseWaveDataChunk(rData, cursor);
				eof = true;
				break;
			case UNSUPPORTED:
			default:
				std::cout << "Unsupported chunk tag: " + tagTest + "\n";
				skipChunk(rData, cursor);
				break;
			}
			if (rData.amtRead >= rData.chunkSize) {
				eof = true;
				rData.isValidFile = false;
			}
		}
		return rData.isValidFile;
	}

	void surveyHardware() {
		HardwareInfo toWrite;
		VulkanImplementationInfoFileData vkInfo;
		AudioDeviceInfoFileData audiInfo;
		SDL_Init(SDL_INIT_EVERYTHING);
		std::string winam = "";
		SDL_Window* testWindow = SDL_CreateWindow(winam.data(), 0, 0, 800, 600, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN);

		unsigned int SDLExtensionCount;
		SDL_Vulkan_GetInstanceExtensions(testWindow, &SDLExtensionCount, nullptr);
		std::vector<const char*> SDLRequiredInstanceExtensions(SDLExtensionCount);
		SDL_Vulkan_GetInstanceExtensions(testWindow, &SDLExtensionCount, SDLRequiredInstanceExtensions.data());
		//come back here when SDL3 releases
		for (auto& e : SDLRequiredInstanceExtensions) {
			vkInfo.requiredExtensions.push_back({ e });
		}
		vkInfo.requiredExtensions.push_back("VK_KHR_swapchain");

		VkInstance testInstance;
		VkInstanceCreateInfo vkici;
		VkApplicationInfo vkai;
		vkai.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		vkai.pNext = nullptr;
		vkai.pApplicationName = "none";
		vkai.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		vkai.pEngineName = "SAGE";
		vkai.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		vkai.apiVersion = VK_API_VERSION_1_3;

		vkici.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		vkici.pNext = nullptr;
		vkici.flags = 0;
		vkici.pApplicationInfo = &vkai;
		vkici.enabledLayerCount = 0;
		vkici.ppEnabledLayerNames = nullptr;
		vkici.enabledExtensionCount = static_cast<uint32_t>(SDLRequiredInstanceExtensions.size());
		vkici.ppEnabledExtensionNames = SDLRequiredInstanceExtensions.data();

		vkCreateInstance(&vkici, nullptr, &testInstance);

		VkPhysicalDevice testCard;
		VkPhysicalDeviceFeatures2        cardFeaturesV10;
		VkPhysicalDeviceVulkan11Features cardFeaturesV11;
		VkPhysicalDeviceVulkan12Features cardFeaturesV12;
		VkPhysicalDeviceVulkan13Features cardFeaturesV13;

		VkPhysicalDeviceMemoryProperties cardMemoryProperties;

		VkPhysicalDeviceProperties2        cardPropertiesV10;
		VkPhysicalDeviceVulkan11Properties cardPropertiesV11;
		VkPhysicalDeviceVulkan12Properties cardPropertiesV12;
		VkPhysicalDeviceVulkan13Properties cardPropertiesV13;

		cardFeaturesV10.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		cardFeaturesV11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
		cardFeaturesV12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
		cardFeaturesV13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;

		cardPropertiesV10.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		cardPropertiesV11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES;
		cardPropertiesV12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES;
		cardPropertiesV13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES;

		cardFeaturesV10.pNext = &cardFeaturesV11;
		cardFeaturesV11.pNext = &cardFeaturesV12;
		cardFeaturesV12.pNext = &cardFeaturesV13;
		cardFeaturesV13.pNext = nullptr;

		cardPropertiesV10.pNext = &cardPropertiesV11;
		cardPropertiesV11.pNext = &cardPropertiesV12;
		cardPropertiesV12.pNext = &cardPropertiesV13;
		cardPropertiesV13.pNext = nullptr;

		uint32_t deviceCount, queueCount;
		vkEnumeratePhysicalDevices(testInstance, &deviceCount, nullptr);
		std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
		vkEnumeratePhysicalDevices(testInstance, &deviceCount, physicalDevices.data());
		testCard = physicalDevices[0];
		vkGetPhysicalDeviceMemoryProperties(testCard, &cardMemoryProperties);

		for (size_t i = 0; i < cardMemoryProperties.memoryHeapCount; i++) {
			toWrite.graphicsCardInfo.memoryHeaps.push_back(cardMemoryProperties.memoryHeaps[i]);
		}
		for (size_t i = 0; i < cardMemoryProperties.memoryTypeCount; i++) {
			toWrite.graphicsCardInfo.memoryTypes.push_back(cardMemoryProperties.memoryTypes[i]);
		}

		uint32_t instanceLayerCount, instanceExtensionCount, deviceExtensionCount;

		vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
		std::vector<VkLayerProperties> VulkanInstanceLayerProperties(instanceLayerCount);
		vkEnumerateInstanceLayerProperties(&instanceLayerCount, VulkanInstanceLayerProperties.data());

		vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, nullptr);
		std::vector<VkExtensionProperties> VulkanInstanceExtensionProperties(instanceExtensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, VulkanInstanceExtensionProperties.data());

		vkEnumerateDeviceExtensionProperties(testCard, nullptr, &deviceExtensionCount, nullptr);
		std::vector<VkExtensionProperties> myDeviceExtensionProperties(deviceExtensionCount);
		vkEnumerateDeviceExtensionProperties(testCard, nullptr, &deviceExtensionCount, myDeviceExtensionProperties.data());

		vkGetPhysicalDeviceFeatures2(testCard, &cardFeaturesV10);
		// V1.0 using: samplerAnisotropy
		// V1.1 using: shaderDrawParameters
		// V1.2 using:
		// V1.3 using: dynamicRendering, syncronization2
		vkGetPhysicalDeviceProperties2(testCard, &cardPropertiesV10);
		toWrite.graphicsCardInfo.maxSamplerAnisotropy = cardPropertiesV10.properties.limits.maxSamplerAnisotropy;
		toWrite.graphicsCardInfo.maxSamplerLoDBias = cardPropertiesV10.properties.limits.maxSamplerLodBias;

		VkSampleCountFlags test = cardPropertiesV10.properties.limits.framebufferColorSampleCounts & cardPropertiesV10.properties.limits.framebufferDepthSampleCounts;
		if (test & VK_SAMPLE_COUNT_64_BIT) toWrite.graphicsCardInfo.maxMSAA = VK_SAMPLE_COUNT_64_BIT;
		else if (test & VK_SAMPLE_COUNT_32_BIT) toWrite.graphicsCardInfo.maxMSAA = VK_SAMPLE_COUNT_32_BIT;
		else if (test & VK_SAMPLE_COUNT_16_BIT) toWrite.graphicsCardInfo.maxMSAA = VK_SAMPLE_COUNT_16_BIT;
		else if (test & VK_SAMPLE_COUNT_8_BIT)  toWrite.graphicsCardInfo.maxMSAA = VK_SAMPLE_COUNT_8_BIT;
		else if (test & VK_SAMPLE_COUNT_4_BIT)  toWrite.graphicsCardInfo.maxMSAA = VK_SAMPLE_COUNT_4_BIT;
		else if (test & VK_SAMPLE_COUNT_2_BIT)  toWrite.graphicsCardInfo.maxMSAA = VK_SAMPLE_COUNT_2_BIT;
		else toWrite.graphicsCardInfo.maxMSAA = VK_SAMPLE_COUNT_1_BIT;

		std::string cardName = cardPropertiesV10.properties.deviceName;
		std::string cardAPIVersion = 
			std::to_string(VK_VERSION_MAJOR(cardPropertiesV10.properties.apiVersion)) + "." +
			std::to_string(VK_VERSION_MINOR(cardPropertiesV10.properties.apiVersion)) + "." +
			std::to_string(VK_VERSION_PATCH(cardPropertiesV10.properties.apiVersion));

		vkGetPhysicalDeviceQueueFamilyProperties(testCard, &queueCount, nullptr);
		std::vector<VkQueueFamilyProperties> myDeviceQueueFamilyProperties(queueCount);
		vkGetPhysicalDeviceQueueFamilyProperties(testCard, &queueCount, myDeviceQueueFamilyProperties.data());

		QueueFamilySorter famSorter{ myDeviceQueueFamilyProperties };
		//famSorter.print();

		std::vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfos;

		famSorter.prepareOutput();

		for (size_t i = 0; i < famSorter.reference.size(); i++) {
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

		std::array<const char*, 1> boink{"VK_KHR_swapchain"};

		VkDevice testDevice;
		VkDeviceCreateInfo dci;
		dci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		dci.pNext = &cardFeaturesV10;
		dci.flags = 0;
		dci.queueCreateInfoCount = static_cast<uint32_t>(deviceQueueCreateInfos.size());
		dci.pQueueCreateInfos = deviceQueueCreateInfos.data();
		dci.enabledLayerCount = 0;
		dci.ppEnabledLayerNames = nullptr;
		dci.enabledExtensionCount = 1;
		dci.ppEnabledExtensionNames = boink.data();
		dci.pEnabledFeatures = nullptr;//figure out what to do here

		vkCreateDevice(testCard, &dci, nullptr, &testDevice);

		VkSurfaceKHR testSurface;
		VkSurfaceCapabilitiesKHR testSurfaceCapabilities;
		SDL_Vulkan_CreateSurface(testWindow, testInstance, &testSurface);
		SDL_UpdateWindowSurface(testWindow);
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(testCard, testSurface, &testSurfaceCapabilities);

		uint32_t targetFrames = 3;

		toWrite.graphicsCardInfo.numFrames = std::min(
			std::max(targetFrames, testSurfaceCapabilities.minImageCount),
			testSurfaceCapabilities.maxImageCount);

		std::vector<VkBool32> isFamilyPresentable(famSorter.reference.size());
		for (size_t i = 0; i < famSorter.reference.size(); i++) {
			vkGetPhysicalDeviceSurfaceSupportKHR(testCard, i, testSurface, &isFamilyPresentable[i]);
		}

		uint32_t numSurfaceFormats, numPresentModes;
		VkSurfaceFormatKHR testFormat;
		vkGetPhysicalDeviceSurfaceFormatsKHR(testCard, testSurface, &numSurfaceFormats, NULL);
		std::vector<VkSurfaceFormatKHR> surfaceFormats(numSurfaceFormats);
		vkGetPhysicalDeviceSurfaceFormatsKHR(testCard, testSurface, &numSurfaceFormats, surfaceFormats.data());
		testFormat = surfaceFormats[0];
		for (const auto& f : surfaceFormats) {
			if (f.format == VK_FORMAT_B8G8R8A8_SRGB && f.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR) testFormat = f;
		}//will we ever have need of other surface formats?

		vkGetPhysicalDeviceSurfacePresentModesKHR(testCard, testSurface, &numPresentModes, NULL);
		std::vector<VkPresentModeKHR> presentModes(numPresentModes);
		vkGetPhysicalDeviceSurfacePresentModesKHR(testCard, testSurface, &numPresentModes, presentModes.data());

		VkPresentModeKHR testMode;
		testMode = VK_PRESENT_MODE_FIFO_KHR;
		for (const auto& t : presentModes) {
			if (t == VK_PRESENT_MODE_MAILBOX_KHR) testMode = t;
		}
//--------------------------------------------------------------------------------------------------------------------------
		int a = SDL_GetNumAudioDrivers();
		int b = SDL_GetNumAudioDevices(0);

		std::cout << "numAudioDrivers: " << a << std::endl;
		std::cout << "numAudioDevices: " << b << std::endl;


		for (int i = 0; i < b - 1; i++) {
			std::cout << "Audio device " << i << ": " << SDL_GetAudioDeviceName(i, 0) << std::endl;
		}
		std::cout << "Current audio driver: " << SDL_GetCurrentAudioDriver() << std::endl;

		//maybe in the future we actually survey and record the info of all audio devices ... maybe.
		SDL_AudioSpec rea, wri;
		rea.freq = AUD_preferredHertz;
		rea.format = AUD_preferredFormat;
		rea.channels = AUD_preferredNumChannels;
		rea.samples = AUD_preferredSampleBufferLength;
		rea.padding = AUD_bufferPadding;

		SDL_AudioDeviceID od = SDL_OpenAudioDevice(NULL, 0, &rea, &wri, SDL_AUDIO_ALLOW_ANY_CHANGE);
		std::string outputDeviceName = std::string(SDL_GetAudioDeviceName(od,0));
		std::string outputBitDepthName = SDL_AudioFormatToString(wri.format);
		toWrite.soundCardInfo.outputBitDepth = wri.format;
		toWrite.soundCardInfo.outputHertz = wri.freq;
		toWrite.soundCardInfo.outputNumChannels = wri.channels;
		toWrite.soundCardInfo.outputSamplesPerChannel = wri.samples;

		SDL_CloseAudioDevice(od);

		std::string vkInfoPath = "Sysinfo/Vulkan.info";
		std::string audinfoPath = "Sysinfo/Audio.info";
		std::string dataBlobPath = "Sysinfo/HardwareGlobals.blob";

		
		
		//by this point we want all of our structs filled out so we can just output the files line by line
		


		std::ofstream outputFile;
		outputFile.open(audinfoPath, std::ios::trunc | std::ios::out);
		outputFile << "Device " << od << ": " << outputDeviceName;
		/*
		typedef struct SDL_AudioSpec
		{
		    int freq;						< DSP frequency -- samples per second 
				SDL_AudioFormat format;     < Audio data format 
				Uint8 channels;             < Number of channels: 1 mono, 2 stereo 
				Uint8 silence;              < Audio buffer silence value (calculated) 
				Uint16 samples;             < Audio buffer size in sample FRAMES (total samples divided by channel count) 
				Uint16 padding;             < Necessary for some compile environments 
				Uint32 size;                < Audio buffer size in bytes (calculated) 
				SDL_AudioCallback callback; < Callback that feeds the audio device (NULL to use SDL_QueueAudio()). 
				void* userdata;             < Userdata passed to callback (ignored for NULL callbacks).
		} SDL_AudioSpec;
		*/
		outputFile.close();

		outputFile.open(vkInfoPath, std::ios::trunc | std::ios::out);
		outputFile << cardName << "\n" <<cardAPIVersion;
		outputFile.close();

		GraphicsCardProperties gsh = toWrite.graphicsCardInfo;
		SoundCardProperties ssh = toWrite.soundCardInfo;

		size_t memTypesSize = gsh.memoryTypes.size();
		size_t memHeapsSize = gsh.memoryHeaps.size();

		outputFile.open(dataBlobPath, std::ios::binary | std::ios::out | std::ios::trunc);
		outputFile.write((char*)&gsh.maxMSAA, sizeof(VkSampleCountFlagBits));
		outputFile.write((char*)&gsh.numFrames, sizeof(uint32_t));
		outputFile.write((char*)&gsh.maxSamplerAnisotropy, sizeof(float));
		outputFile.write((char*)&gsh.maxSamplerLoDBias, sizeof(float));
		outputFile.write((char*)&memTypesSize, sizeof(size_t));
		for (size_t i = 0; i < memTypesSize; i++) {
			outputFile.write((char*)&gsh.memoryTypes[i].propertyFlags, sizeof(VkMemoryPropertyFlags));
			outputFile.write((char*)&gsh.memoryTypes[i].heapIndex, sizeof(uint32_t));
		}
		outputFile.write((char*)&memHeapsSize, sizeof(size_t));
		for (size_t i = 0; i < memHeapsSize; i++) {
			outputFile.write((char*)&gsh.memoryHeaps[i].size, sizeof(VkDeviceSize));
			outputFile.write((char*)&gsh.memoryHeaps[i].flags, sizeof(VkMemoryHeapFlags));
		}
		outputFile.write((char*)&ssh.outputHertz, sizeof(size_t));
		outputFile.write((char*)&ssh.outputSamplesPerChannel, sizeof(size_t));
		outputFile.write((char*)&ssh.outputBitDepth, sizeof(SDL_AudioFormat));
		outputFile.write((char*)&ssh.outputNumChannels, sizeof(uint8_t));
		outputFile.close();

		vkDestroySurfaceKHR(testInstance, testSurface, nullptr);
		vkDestroyDevice(testDevice, nullptr);
		vkDestroyInstance(testInstance, nullptr);
		SDL_DestroyWindow(testWindow);
		SDL_Quit();
	}

	bool loadHardwareInfo() {
		std::string dataBlobPath = "Sysinfo/HardwareGlobals.blob";
		if (!std::filesystem::exists(dataBlobPath))surveyHardware();
		std::vector<char>infoBlob;
		if(!readWholeBinaryFile(dataBlobPath, infoBlob)) return false;

		void* cursor = infoBlob.data();

		VkSampleCountFlagBits* a = (VkSampleCountFlagBits*)cursor;
		hardwareInfo.graphicsCardInfo.maxMSAA = *a++;
		cursor = a;

		uint32_t* ui32ptr = (uint32_t*)cursor;
		hardwareInfo.graphicsCardInfo.numFrames = *ui32ptr++;
		cursor = ui32ptr;

		float* f32ptr = (float*)cursor;
		hardwareInfo.graphicsCardInfo.maxSamplerAnisotropy = *f32ptr++;
		hardwareInfo.graphicsCardInfo.maxSamplerLoDBias = *f32ptr++;
		cursor = f32ptr;

		size_t* sizeptr = (size_t*)cursor;
		hardwareInfo.graphicsCardInfo.memoryTypes.resize(*sizeptr++);
		cursor = sizeptr;
		VkMemoryPropertyFlags* b;
		for (auto& t : hardwareInfo.graphicsCardInfo.memoryTypes) {
			b = (VkMemoryPropertyFlags*)cursor;
			t.propertyFlags = *b++;
			cursor = b;
			ui32ptr = (uint32_t*)cursor;
			t.heapIndex = *ui32ptr++;
			cursor = ui32ptr;
		};

		sizeptr = (size_t*)cursor;
		hardwareInfo.graphicsCardInfo.memoryHeaps.resize(*sizeptr++);
		cursor = sizeptr;
		VkDeviceSize* vksizeptr;
		VkMemoryHeapFlags* c;
		for (auto& t : hardwareInfo.graphicsCardInfo.memoryHeaps) {
			vksizeptr = (VkDeviceSize*)cursor;
			t.size = *vksizeptr++;
			cursor = vksizeptr;
			c = (VkMemoryHeapFlags*)cursor;
			t.flags = *c++;
			cursor = c;
		};

		sizeptr = (size_t*)cursor;
		hardwareInfo.soundCardInfo.outputHertz = *sizeptr++;
		hardwareInfo.soundCardInfo.outputSamplesPerChannel = *sizeptr++;
		cursor = sizeptr;

		SDL_AudioFormat* d = (SDL_AudioFormat*)cursor;
		hardwareInfo.soundCardInfo.outputBitDepth = *d++;
		cursor = d;

		uint8_t* ui8ptr = (uint8_t*)cursor;
		hardwareInfo.soundCardInfo.outputNumChannels = *ui8ptr;

		return true;
	}

	std::string makeUUID(SAGEAssetType type, uint32_t assID, std::filesystem::path name) {
		std::string strRep = name.stem().string();
		std::transform(
			strRep.begin(),
			strRep.end(),
			strRep.begin(),
			[](unsigned char c) {return std::tolower(c); });
		std::filesystem::path dicpath{};//will we ever do anything with this?  Verification?
		switch (type)
		{
		case SAGE_SHADER_ASSET:
			dicpath = "Assets/Shaders";
			strRep = "shader:" + strRep;
			break;
		case SAGE_MESH_ASSET:
			dicpath = "Assets/Meshes";
			strRep = "mesh:" + strRep;
			break;
		case SAGE_TEXTURE_ASSET:
			dicpath = "Assets/Textures";
			strRep = "texture:" + strRep;
			break;
		case SAGE_CUBEMAP_ASSET:
			dicpath = "Assets/Cubemaps";
			strRep = "cubemap:" + strRep;
			break;
		case SAGE_MATERIAL_ASSET:
			dicpath = "Assets/Materials";
			strRep = "material:" + strRep;
			break;
		case SAGE_CUSTOM_ASSET:
			strRep = "whatthehellthisisntsupposedtohappenyet:" + strRep;
			break;
		default:
			strRep += " now wait a minute something went terribly wrong here.  Did you forget to add a case to the switch in makeUUID?  I am so ashamed of you right now.";
			break;
		}

		return strRep + '|' + std::to_string(assID | (type << 24)) + "= ";

	}

	std::string STRINGfromEntry(const std::string& entry) {
		return entry.substr(0, entry.find('|'));
	}
	uint32_t UUIDfromEntry(const std::string& entry) {
		size_t shortcut = entry.find('|') + 1;
		return (uint32_t)std::stoul(entry.substr(shortcut, entry.find('=') - shortcut));
	}
	void swapEntryNames(std::string& a, std::string& b) {
		std::string nameA = STRINGfromEntry(a);
		std::string nameB = STRINGfromEntry(b);
		std::string suffixA = a.substr(a.find('|'));
		std::string suffixB = b.substr(b.find('|'));
		a = nameB + suffixA;
		b = nameA + suffixB;
	}
	void refreshEntryPrefix(std::string& o, const std::string& n) {
		std::string newPrefix = n.substr(0, n.find('='));
		std::string oldSuffix = o.substr(o.find('='));
		o = newPrefix + oldSuffix;
	}

	uint32_t STRINGtoUUID(std::string assetStringName, std::string* pEntryReturn) {
		std::string temp{};
		std::ifstream manifest(ASS_manifestPath);
		while (std::getline(manifest, temp)) {
			if (temp.substr(0, assetStringName.length()) == assetStringName) break;
		}
		manifest.close();
		if (temp.size() == 0) {
			return 0; //error in future
		}
		if (pEntryReturn) *pEntryReturn = temp;
		return UUIDfromEntry(temp);
	}

	std::string UUIDtoSTRING(uint32_t UUID, std::string* pEntryReturn) {
		std::string temp{};
		std::ifstream manifest(ASS_manifestPath);
		while (std::getline(manifest, temp)) {
			if (temp.find('|') == std::string::npos) continue;
			if (UUIDfromEntry(temp) == UUID) break;
		}
		manifest.close();
		if (temp.size() == 0) {
			return "UUID not present in manifest!"; //error in future
		}
		if (pEntryReturn) *pEntryReturn = temp;
		return STRINGfromEntry(temp);
	}

	std::vector<std::string> translateManifestShadersEntry(std::string assetStringName) {
		std::string entry;
		STRINGtoUUID(assetStringName, &entry);
		std::vector<std::string> outVec{};
		if (entry.size() == 0 || entry.substr(0, entry.find(':')) != "shader") return outVec;//some error here
		std::string prependPath = "Binassets/APBuildingBlocks/";
		std::string workingString = entry.substr(entry.find(' ') + 1);
		size_t workingIndex = workingString.find(',');
		while (workingIndex != std::string::npos) {
			outVec.push_back(workingString.substr(0, workingIndex));
			workingString = workingString.substr(workingIndex + 2);
			workingIndex = workingString.find(',');
		}
		outVec.push_back(workingString);
		for (auto& s : outVec) {
			workingIndex = s.find('.');
			s[workingIndex] = ' ';
			s[workingIndex + 1] = std::toupper(s[workingIndex + 1]);
			s = prependPath + s + ".spv";
		}
		return outVec;
	}

	//INFLEXIBILITY WARNING WINDOWS ONLY
	void compileShaders() {
		system("cd Assets/Shaders && compile.bat");
	}//\INFLEXIBILITY WARNING
	
	size_t getManifestEntryTextureMapPosition(std::string& mName) {
		if (mName == "diffuseMap") return 2;
		if (mName == "specularMap") return 4;
		if (mName == "bumpMap" || mName == "heightMap") return 6;
		if (mName == "colorMap" || mName == "albedoMap") return 8;
		if (mName == "normalMap") return 10;
		if (mName == "reflectivityMap" || mName == "metalnessMap") return 12;
		if (mName == "glossinessMap") return 14;
		if (mName == "aoMap" || mName == "ambientOcclusionMap") return 16;
//		if (mName == "opacityMap") return 18;
//		if (mName == "refractionMap") return 20;
//		if (mName == "emissiveMap") return 22;
		return SIZE_MAX;
	}
	std::string getTextureMapNameFromEntryPosition(size_t i) {
		switch (i) {
		case 2:
			return "diffuseMap=default";
		case 4:
			return "specularMap=default";
		case 6:
			return "bumpMap=default";
		case 8:
			return "colorMap=default";
		case 10:
			return "normalMap=default";
		case 12:
			return "metalnessMap=default";
		case 14:
			return "glossinessMap=default";
		case 16:
			return "aoMap=default";
//		case 18:
//			return "opacityMap=default";
//		case 20:
//			return "refractionMap=default";
//		case 22:
//			return "emissiveMap=default";
		default:
			return "noMapHereYouHaveAnError!";
		}
	}
	std::string textureAssetEntryHelperFuntion(std::vector<std::string>& s) {
		std::string defaultSampler = s[0];
		for (size_t i = 1; i < s.size(); i++) {
			std::string here = s[i].substr(0,s[i].find('='));
			if (here != "sampler") {
				s.insert(s.begin() + i, "sampler=" + defaultSampler);
				i--;
				continue;
			}
			i++;
		}

		s.resize(MAX_TEXTURE_MAPS * 2 + 1);

		for (size_t i = 2; i < s.size(); i += 2) {
			if (s[i].size() == 0) {
				s[i - 1] = "sampler=" + defaultSampler;
				s[i] = getTextureMapNameFromEntryPosition(i);
				continue;
			}
			std::string mapName = s[i].substr(0, s[i].find('='));
			size_t j = getManifestEntryTextureMapPosition(mapName);
			if (i != j) {
				std::iter_swap(s.begin() + i - 1, s.begin() + j - 1);
				std::iter_swap(s.begin() + i, s.begin() + j);
				i -= 2;
			}
		}

		s.erase(s.begin());
		defaultSampler.clear();

		for (size_t i = 0; i < s.size(); i+=2) {
			defaultSampler += s[i].substr(s[i].find('=') + 1) + ", ";
			defaultSampler += s[i + 1].substr(s[i+1].find('=') + 1) + "; ";
		}

		defaultSampler = defaultSampler.substr(0,defaultSampler.find_last_of(';'));
		return defaultSampler;
	}

	std::string createTextureAssetList(std::vector<std::filesystem::path>& filenames) {
		std::string cumString{};
		size_t num = 0;
		for (auto& p : filenames) {
			std::string filepath = "Assets/Textures/" + p.string();
			cumString += makeUUID(SAGE_TEXTURE_ASSET, num++, p);
			std::vector<std::string> fileText;
			readWholeTextFile(filepath, fileText);
			cumString += textureAssetEntryHelperFuntion(fileText) + '\n';
		}
		return cumString;
	}
	std::string createMeshAssetList(std::vector<std::filesystem::path>& filenames) {
		std::string cumString{};
		size_t num = 0;
		for (auto& p : filenames) {
			cumString += makeUUID(SAGE_MESH_ASSET, num++, p) + p.string() + '\n';
		}
		return cumString;
	}
	std::string createShaderAssetList(std::vector<std::filesystem::path>& filenames) {
		std::string cumString{};
		size_t num = 0;
		for (size_t i = 0; i < filenames.size(); i++) {
			cumString += makeUUID(SAGE_SHADER_ASSET, num++, filenames[i]);
			cumString += filenames[i].string() + ", ";
			if (i == filenames.size() - 1) {
				cumString = cumString.substr(0, cumString.size() - 2);
				cumString += '\n';
				break;
			}
			std::filesystem::path baseName = filenames[i].stem();
			while (filenames[i + 1].stem() == baseName) {
				cumString += filenames[++i].string() + ", ";
				if (i == filenames.size() - 1) break;
			}
			cumString = cumString.substr(0, cumString.size() - 2);
			cumString += '\n';
		}
		return cumString;
	}
	std::string createCubemapAssetList(std::vector<std::filesystem::path>& filenames) {
		std::string cumString{};
		size_t num = 0;
		for (auto& p : filenames){
			cumString += makeUUID(SAGE_CUBEMAP_ASSET, num++, p);
			std::string base = p.stem().string();
			cumString += base + "_R.png, ";
			cumString += base + "_L.png, ";
			cumString += base + "_U.png, ";
			cumString += base + "_D.png, ";
			cumString += base + "_F.png, ";
			cumString += base + "_B.png\n";
		}
		return cumString;
	}
	std::string createMaterialAssetList(std::vector<std::filesystem::path>& filenames) {
		std::string cumString{};
		std::string fileLine;
		size_t num = 0;
		for (auto& p : filenames) {
			std::string filepath = "Assets/Materials/" + p.string();
			cumString += makeUUID(SAGE_MATERIAL_ASSET, num++, p);

			std::ifstream file(filepath);

			std::getline(file, fileLine);
			cumString += std::to_string(STRINGtoUUID("shader:" + fileLine)) + "; ";

			std::getline(file, fileLine);
			while (fileLine.find(' ') != std::string::npos) {
				cumString += fileLine.substr(0, fileLine.find(' ')) + ", ";
				fileLine = fileLine.substr(fileLine.find(' ') + 1);
			}
			cumString += fileLine + "; ";

			std::getline(file, fileLine);
			file.close();

			while (fileLine.find(' ') != std::string::npos) {
				cumString += fileLine.substr(0, fileLine.find(' ')) + ", ";
				fileLine = fileLine.substr(fileLine.find(' ') + 1);
			}
			cumString += fileLine + '\n';
		}
		return cumString;
	}

	void createManifest() {//will have to change this to be able to write to while being read so that meta-assets can be made at the same time as base assets.
		size_t numFiles = 0;
		std::vector<std::vector<std::filesystem::path>> assetTypes;
		std::vector<std::vector<std::string>> assetTypeSupportedExtensions;
		std::vector<std::string> finalOutput;//everywhere this is written to now becomes a line write later
		for (size_t i = 0; i < SAGE_CUSTOM_ASSET; i++) {//not part of file i/o
			assetTypes.push_back({});
			assetTypeSupportedExtensions.push_back({});
			switch (i)
			{
			case SAGE_SHADER_ASSET:
				assetTypeSupportedExtensions[i].push_back(".vert");
				assetTypeSupportedExtensions[i].push_back(".frag");
				assetTypeSupportedExtensions[i].push_back(".comp");
				break;
			case SAGE_MESH_ASSET:
				assetTypeSupportedExtensions[i].push_back(".obj");
				break;
			case SAGE_TEXTURE_ASSET:
				assetTypeSupportedExtensions[i].push_back(".stx");
				break;
			case SAGE_CUBEMAP_ASSET:
				assetTypeSupportedExtensions[i].push_back(".scm");
				break;
			case SAGE_MATERIAL_ASSET:
				assetTypeSupportedExtensions[i].push_back(".smt");
				break;
			case SAGE_CUSTOM_ASSET:
				std::cout << "You shouldn't be here!\n";
				break;
			default:
				std::cout << "I'd like you to leave.\n";
				break;
			}
		}
		auto rdi = std::filesystem::recursive_directory_iterator("Assets");
		//we're assuming that these are already in alphabetical order.  Might bite us down the road but...meh
		for (auto& entry : rdi) {
			if (std::filesystem::is_directory(entry.path())) continue;
			std::filesystem::path filename = entry.path().filename();
			std::string ext = entry.path().extension().string();
			SAGEAssetType ass = SAGE_CUSTOM_ASSET;

			for (size_t i = 0; i < SAGE_CUSTOM_ASSET; i++) {
				auto& v = assetTypeSupportedExtensions[i];
				if (std::find(v.begin(), v.end(), ext) != v.end()) {
					ass = (SAGEAssetType)i;
					break;
				}
			}
			if (ass != SAGE_CUSTOM_ASSET)assetTypes[ass].push_back(filename);
			numFiles++;
		}
		finalOutput.push_back(std::to_string(numFiles) + "\n");//file will have to be opened here
		for (size_t i = 0; i < assetTypes.size(); i++) {
			switch (i) {
			case SAGE_SHADER_ASSET:
				finalOutput.push_back("[Shaders]");
				finalOutput.push_back(createShaderAssetList(assetTypes[i]));
				break;
			case SAGE_MESH_ASSET:
				finalOutput.push_back("[Meshes]");
				finalOutput.push_back(createMeshAssetList(assetTypes[i]));
				break;
			case SAGE_TEXTURE_ASSET:
				finalOutput.push_back("[Textures]");
				finalOutput.push_back(createTextureAssetList(assetTypes[i]));
				break;
			case SAGE_CUBEMAP_ASSET:
				finalOutput.push_back("[Cubemaps]");
				finalOutput.push_back(createCubemapAssetList(assetTypes[i]));
				break;
			case SAGE_MATERIAL_ASSET:
				finalOutput.push_back("[Materials]");
				finalOutput.push_back(createMaterialAssetList(assetTypes[i]));
				break;
			case SAGE_CUSTOM_ASSET:
				std::cout << "Never should have come here!\n";
				break;
			default:
				std::cout << "Can't wait to count out your coin!\n";
				break;
			}
		}
		if (std::filesystem::exists(ASS_manifestPath)) {
			std::filesystem::rename(ASS_manifestPath, ASS_manifestPath + ".old");
		}
		std::ofstream omega(ASS_manifestPath);
		for (size_t i = 0; i < finalOutput.size(); i++) {
			omega << finalOutput[i] << '\n';
		}
		omega.close();
	}

	bool assetsChanged() {
		if (!std::filesystem::exists(ASS_manifestPath))return true;
		size_t scannedAns = 0;
		auto rdi = std::filesystem::recursive_directory_iterator("Assets");
		for (auto& e : rdi) {
			if (std::filesystem::is_directory(e.path())) continue;
			scannedAns++;
		}
		std::string readAns;
		std::ifstream alpha(ASS_manifestPath);
		std::getline(alpha, readAns);
		alpha.close();

		return scannedAns != std::stoi(readAns);
	}

	void textureDefaultHandler(size_t i, Image& data) {
		SAGETextureMapType mt = (SAGETextureMapType)i;
		switch (mt)
		{
		case DIFFUSE_TEXTURE_MAP:
			data.width = 1;
			data.height = 1;
			data.channels = 4;
			data.imageDataSize = 4;
			data.imageData = { CHAR_MAX, CHAR_MAX, CHAR_MAX, UINT8_MAX };
			break;
		case SPECULAR_TEXTURE_MAP:
			data.width = 1;
			data.height = 1;
			data.channels = 1;
			data.imageDataSize = 1;
			data.imageData = { 0 };
			break;
		case BUMP_TEXTURE_MAP:
			data.width = 1;
			data.height = 1;
			data.channels = 1;
			data.imageDataSize = 1;
			data.imageData = { 0 };
			break;
		case ALBEDO_TEXTURE_MAP:
			data.width = 1;
			data.height = 1;
			data.channels = 4;
			data.imageDataSize = 4;
			data.imageData = { CHAR_MAX, CHAR_MAX, CHAR_MAX, UINT8_MAX };
			break;
		case NORMAL_TEXTURE_MAP:
			data.width = 1;
			data.height = 1;
			data.channels = 4;
			data.imageDataSize = 4;
			data.imageData = { CHAR_MAX, CHAR_MAX, UINT8_MAX, UINT8_MAX };
			break;
		case REFLECTIVITY_TEXTURE_MAP:
			data.width = 1;
			data.height = 1;
			data.channels = 1;
			data.imageDataSize = 1;
			data.imageData = { 0 };
			break;
		case GLOSSINESS_TEXTURE_MAP:
			data.width = 1;
			data.height = 1;
			data.channels = 1;
			data.imageDataSize = 1;
			data.imageData = { 0 };
			break;
		case AO_TEXTURE_MAP:
			data.width = 1;
			data.height = 1;
			data.channels = 1;
			data.imageDataSize = 1;
			data.imageData = { UINT8_MAX };
			break;
//		case OPACITY_TEXTURE_MAP:
//			break;
//		case REFRACTION_TEXTURE_MAP:
//			break;
//		case EMISSIVE_TEXTURE_MAP:
//			break;
		default:
			data.width = 0;
			data.height = 0;
			data.channels = 0;
			data.imageDataSize = 0;
			data.imageData.clear();
			break;
		}
	}
	void compileManifestTextureEntry(const std::string& entry) {
		std::string pathPref = "Assets/Textures/BuildingBlocks/";
		std::string munch = entry.substr(entry.find(' ')+1);


		TextureFileData data{};
		std::array<stbi_uc*, data.maps.size()> garbage;

		for (size_t i = 0; i < data.maps.size(); i++) {
			data.maps[i].samplerName = munch.substr(0, munch.find(','));
			munch = munch.substr(munch.find(',') + 2);
			std::string path = munch.substr(0, munch.find(';'));
			if (path == "default") {
				textureDefaultHandler(i, data.maps[i]);
				garbage[i] = nullptr;
			}
			else {
				path = pathPref + path;
				stbi_set_flip_vertically_on_load(true);
				garbage[i] = stbi_load(path.c_str(), &data.maps[i].width, &data.maps[i].height, &data.maps[i].channels, STBI_rgb_alpha);
				data.maps[i].imageDataSize = data.maps[i].width * data.maps[i].height * 4;//note that texture dimensions MUST be a power of 2
				//note also the 4 comes from how many channels we have: r,g,b,a
				//comes from STBI_rgb_alpha, not tChannels
			}
			if (munch.find(';') == std::string::npos)break;
			munch = munch.substr(munch.find(';') + 2);
		}
		std::string filename = "Binassets/" + entry.substr(entry.find('|') + 1, entry.find('=') - (entry.find('|') + 1));

		std::ofstream texFile(filename, std::ios::binary | std::ios::out | std::ios::trunc);
		for (size_t i = 0; i < data.maps.size(); i++) {
			size_t nameSize = data.maps[i].samplerName.size();
			texFile.write((char*)&nameSize, sizeof(size_t));
			texFile.write(data.maps[i].samplerName.data(), nameSize);
			texFile.write((char*)&data.maps[i].width, sizeof(int));
			texFile.write((char*)&data.maps[i].height, sizeof(int));
			texFile.write((char*)&data.maps[i].channels, sizeof(int));
			texFile.write((char*)&data.maps[i].imageDataSize, sizeof(size_t));
			if (garbage[i])texFile.write((char*)garbage[i], data.maps[i].imageDataSize);
			else texFile.write((char*)data.maps[i].imageData.data(), data.maps[i].imageDataSize);
		}
		texFile.close();

		for (auto p : garbage) {
			if (p) stbi_image_free(p);//free all in garbage that aren't nullptr
		}
	}
	bool loadTexture(const std::string& filepath, TextureFileData& returnVal) {
		std::vector<char> fileData{};
		if (!readWholeBinaryFile(filepath, fileData))	return false;
		void* cursor = fileData.data();
		TextureFileData returnData{};

		for (auto& e : returnData.maps) {
			size_t* sizeData = (size_t*)cursor;
			e.samplerName.resize(*sizeData++);
			cursor = sizeData;
			char* charData = (char*)cursor;
			for (auto& c : e.samplerName) {
				c = *charData++;
			}
			cursor = charData;
			int* intData = (int*)cursor;
			e.width = *intData++;
			e.height = *intData++;
			e.channels = *intData++;
			cursor = intData;
			sizeData = (size_t*)cursor;
			e.imageDataSize = *sizeData++;
			e.imageData.resize(e.imageDataSize);
			cursor = sizeData;
			uint8_t* uint8Data = (uint8_t*)cursor;
			for (auto& c : e.imageData) {
				c = *uint8Data++;
			}
			cursor = uint8Data;
		}
		returnVal = std::move(returnData);
		return true;
	}

	void compileManifestCubemapEntry(const std::string& entry) {
		std::string pathPref = "Assets/Cubemaps/BuildingBlocks/";
		std::string munch = entry.substr(entry.find(' ') + 1);
		Image data{};
		data.samplerName = "demo";//may make custom in future
		std::array<stbi_uc*, 6> garbage;
		for (size_t i = 0; i < 5; i++) {
			std::string path = pathPref + munch.substr(0, munch.find(','));
			stbi_set_flip_vertically_on_load(false);
			garbage[i] = stbi_load(path.c_str(), &data.width, &data.height, &data.channels, STBI_rgb_alpha);
			munch = munch.substr(munch.find(' ') + 1);
		}
		pathPref += munch;
		garbage[5] = stbi_load(pathPref.c_str(), &data.width, &data.height, &data.channels, STBI_rgb_alpha);
		data.imageDataSize = data.width * data.height * 4 * 6;//width * height * 4 channels * 6 images
		data.channels = 4;//just to make sure

		std::string filename = "Binassets/" + entry.substr(entry.find('|') + 1, entry.find('=') - (entry.find('|') + 1));

		std::ofstream cbmFile(filename, std::ios::binary | std::ios::out | std::ios::trunc);
		size_t nameSize = data.samplerName.size();
		cbmFile.write((char*)&nameSize, sizeof(size_t));
		cbmFile.write(data.samplerName.data(), nameSize);
		cbmFile.write((char*)&data.width, sizeof(int));
		cbmFile.write((char*)&data.height, sizeof(int));
		cbmFile.write((char*)&data.channels, sizeof(int));
		cbmFile.write((char*)&data.imageDataSize, sizeof(size_t));
		size_t writeInc = data.imageDataSize / 6;
		for (auto p : garbage) {
			cbmFile.write((char*)p, writeInc);
			stbi_image_free(p);
		}
		cbmFile.close();
	}
	bool loadCubemap(const std::string& filepath, Image& returnVal) {
		std::vector<char> fileData{};
		if (!readWholeBinaryFile(filepath, fileData))	return false;
		void* cursor = fileData.data();
		Image returnData{};

		size_t* sizeData = (size_t*)cursor;
		returnData.samplerName.resize(*sizeData++);
		cursor = sizeData;

		char* charData = (char*)cursor;
		for (auto& c : returnData.samplerName) {
			c = *charData++;
		}
		cursor = charData;

		int* intData = (int*)cursor;
		returnData.width = *intData++;
		returnData.height = *intData++;
		returnData.channels = *intData++;
		cursor = intData;

		sizeData = (size_t*)cursor;
		returnData.imageDataSize = *sizeData++;
		returnData.imageData.resize(returnData.imageDataSize);
		cursor = sizeData;

		uint8_t* uint8Data = (uint8_t*)cursor;
		for (auto& c : returnData.imageData) {
			c = *uint8Data++;
		}
		cursor = uint8Data;

		returnVal = std::move(returnData);
		return true;
	}

	void compileManifestMeshEntry(const std::string& entry) {
		std::string path = "Assets/Meshes/" + entry.substr(entry.find(' ') + 1);
		std::string filename = "Binassets/" + entry.substr(entry.find('|') + 1, entry.find('=') - (entry.find('|') + 1));
		
		MeshFileData data{};

		tinyobj::attrib_t tinyObjAttribute;
		std::vector<tinyobj::shape_t> tinyObjShapes;
		std::vector<tinyobj::material_t> tinyObjMaterials;
		std::string tinyObjWarning, tinyObjError;

		if (!tinyobj::LoadObj(&tinyObjAttribute, &tinyObjShapes, &tinyObjMaterials, &tinyObjWarning, &tinyObjError, path.c_str())) throw std::runtime_error(tinyObjWarning + tinyObjError);
		//this is where we consider going through the file ourselves to make the data instead of hashing
		//could be a valuable exercise as hashing seems to have variable success, while if we roll our own method
		//we could guarantee the least amount of possible unique vertices.
		std::unordered_map<Vertex, uint32_t> uniqueVertices;
		std::unordered_map<uint32_t, size_t> indexCounts;

		for (const auto& shape : tinyObjShapes) {
			for (const auto& index : shape.mesh.indices) {
				Vertex v{};

				v.position = {
					tinyObjAttribute.vertices[3 * index.vertex_index + 0],
					tinyObjAttribute.vertices[3 * index.vertex_index + 1],
					tinyObjAttribute.vertices[3 * index.vertex_index + 2]
				};
				v.normal = {
					tinyObjAttribute.normals[3 * index.normal_index + 0],
					tinyObjAttribute.normals[3 * index.normal_index + 1],
					tinyObjAttribute.normals[3 * index.normal_index + 2]
				};
				v.normal = glm::normalize(v.normal);//just to make sure
				v.texturePosition = {
					tinyObjAttribute.texcoords[2 * index.texcoord_index + 0],
					tinyObjAttribute.texcoords[2 * index.texcoord_index + 1]
				};
				v.color = { 1.f,1.f,1.f,1.f };

				if (uniqueVertices.count(v) == 0) {
					uniqueVertices[v] = static_cast<uint32_t>(data.vertexData.size());
					indexCounts[uniqueVertices[v]] = 0;
					data.vertexData.push_back(v);
				}

				data.indexData.push_back(uniqueVertices[v]);
				indexCounts[uniqueVertices[v]]++;
			}
		}

		for (size_t i = 0; i < data.indexData.size()-2; i+=3) {
			glm::vec3 tempTang;
//			glm::vec3 tempBit;

			Vertex& v1 = data.vertexData[data.indexData[i]];
			Vertex& v2 = data.vertexData[data.indexData[i+1]];
			Vertex& v3 = data.vertexData[data.indexData[i+2]];
			
			glm::vec3 edge1 = v2.position - v1.position;
			glm::vec3 edge2 = v3.position - v1.position;
			glm::vec2 dUV1 = v2.texturePosition - v1.texturePosition;
			glm::vec2 dUV2 = v3.texturePosition - v1.texturePosition;

			float f = 1 / (dUV1.x * dUV2.y - dUV2.x * dUV1.y);
			tempTang.x = f * (dUV2.y * edge1.x - dUV1.y * edge2.x);
			tempTang.y = f * (dUV2.y * edge1.y - dUV1.y * edge2.y);
			tempTang.z = f * (dUV2.y * edge1.z - dUV1.y * edge2.z);
/*
			tempBit.x = f * (-dUV2.x * edge1.x - dUV1.x * edge2.x);
			tempBit.y = f * (-dUV2.x * edge1.y - dUV1.x * edge2.y);
			tempBit.z = f * (-dUV2.x * edge1.z - dUV1.x * edge2.z);
*/
			v1.normalTangent += tempTang;
//			v1.normalBitangent += tempBit;
			v2.normalTangent += tempTang;
//			v2.normalBitangent += tempBit;
			v3.normalTangent += tempTang;
//			v3.normalBitangent += tempBit;
		}

		for (auto& kvp : indexCounts) {
			Vertex& v = data.vertexData[kvp.first];
			v.normalTangent = glm::normalize( v.normalTangent / (float)kvp.second);
//			v.normalBitangent = glm::normalize( v.normalBitangent / (float)kvp.second);

			v.normalTangent = glm::normalize(v.normalTangent - (glm::dot(v.normal, v.normalTangent) * v.normal));
			v.normalBitangent = glm::cross(v.normal, v.normalTangent);
		}

		uint32_t numVertices = data.vertexData.size();
		data.vertexDataBlockSize = sizeof(Vertex) * numVertices;
		Vertex* vertexData = data.vertexData.data();
		uint32_t numIndices = data.indexData.size();
		data.indexDataBlockSize = sizeof(uint32_t) * numIndices;
		uint32_t* indexData = data.indexData.data();

		std::ofstream file(filename, std::ios::binary | std::ios::out | std::ios::trunc);
		file.write((char*)&numVertices, sizeof(uint32_t));
		file.write((char*)&data.vertexDataBlockSize, sizeof(size_t));
		file.write((char*)vertexData, data.vertexDataBlockSize);
		file.write((char*)&numIndices, sizeof(uint32_t));
		file.write((char*)&data.indexDataBlockSize, sizeof(size_t));
		file.write((char*)indexData, data.indexDataBlockSize);
		file.close();
	}
	bool loadMesh(const std::string& filepath, MeshFileData& returnVal) {
		std::vector<char> fileData{};
		if (!readWholeBinaryFile(filepath, fileData)) return false;
		void* cursor = fileData.data();
		MeshFileData returnData{};

		uint32_t* numPointer = (uint32_t*)cursor;
		returnData.vertexData.resize(*numPointer++);
		cursor = numPointer;

		size_t* sizePointer = (size_t*)cursor;
		returnData.vertexDataBlockSize = *sizePointer++;
		cursor = sizePointer;

		Vertex* vertData = (Vertex*)cursor;
		for (size_t i = 0; i < returnData.vertexData.size(); i++) {
			returnData.vertexData[i] = *vertData++;
		}
		cursor = vertData;

		numPointer = (uint32_t*)cursor;
		returnData.indexData.resize(*numPointer++);
		cursor = numPointer;

		sizePointer = (size_t*)cursor;
		returnData.indexDataBlockSize = *sizePointer++;
		cursor = sizePointer;

		numPointer = (uint32_t*)cursor;
		for (size_t i = 0; i < returnData.indexData.size(); i++) {
			returnData.indexData[i] = *numPointer++;
		}
		returnVal = std::move(returnData);
		return true;
	}

	void shaderMetaInfoHandler(const std::string& entry, ShaderFileData& data) {
		std::string filepath = "Assets/Shaders/" + entry.substr(entry.find(' ') + 1, entry.find('.') - entry.find(' ')) + "ssmi";
		std::ifstream file(filepath);
		if (!file.is_open()) {
			data.vertInputs = SAGE_NO_VERTEX;
			return;
		}
		std::getline(file, filepath);
		file.close();
		if (filepath == "none") data.vertInputs = SAGE_NO_VERTEX;
		else if (filepath == "default") data.vertInputs = SAGE_DEFAULT_VERTEX;
		else if (filepath == "sprite") data.vertInputs = SAGE_SPRITE_VERTEX;
		else if (filepath == "particle") data.vertInputs = SAGE_PARTICLE_VERTEX;
	}
	void compileManifestShaderEntry(const std::string& entry) {
		std::vector<std::string> filePaths = translateManifestShadersEntry(entry);
		std::string filename = "Binassets/" + entry.substr(entry.find('|') + 1, entry.find('=') - (entry.find('|') + 1));

		ShaderFileData data{};
		shaderMetaInfoHandler(entry, data);
		data.descriptorSetsLayout.resize(4);
		for (auto& s : data.descriptorSetsLayout) {
			s.push_back({});
		}

		data.name = entry.substr(entry.find(':') + 1, entry.find('|') - (entry.find(':') + 1));
		size_t nameSize = data.name.size();

		std::vector<std::vector<char>> temp;
		data.numShaders = filePaths.size();

		std::vector<SpvReflectShaderModule*> garbageCan;

		for(auto& fp : filePaths){
			std::vector<char> boink{};
			readWholeBinaryFile(fp, boink);

			//reflection begins here.
			SpvReflectShaderModule reflectData;
			spvReflectCreateShaderModule(boink.size(), boink.data(), &reflectData);
			data.shaderEntryPoints.push_back(reflectData.entry_point_name);
			data.shaderStages.push_back((VkShaderStageFlagBits)reflectData.shader_stage);
			uint32_t contextualCount = 0;
			/*
			if (reflectData.shader_stage & SPV_REFLECT_SHADER_STAGE_VERTEX_BIT) {
				spvReflectEnumerateInputVariables(&reflectData, &contextualCount, nullptr);
				if (contextualCount > 0) {

				Reflecting vertex inputs has been tricky
				so we just take it from a file we write ourselves
				and be done with it.

				}
			} 
			*/
			spvReflectEnumerateDescriptorBindings(&reflectData, &contextualCount, nullptr);
			if(contextualCount > 0){
				std::vector<SpvReflectDescriptorBinding*>foundBindings(contextualCount);
				spvReflectEnumerateDescriptorBindings(&reflectData, &contextualCount, foundBindings.data());

				for (auto& b : foundBindings) {
					DescriptorGenerationData bindingData{};
					bindingData.set = b->set;
					bindingData.binding = b->binding;
					bindingData.name = std::string(b->name);
					bindingData.type = (VkDescriptorType)b->descriptor_type;
					bindingData.count = b->count;
					bindingData.stages = (VkShaderStageFlags)reflectData.shader_stage;
					auto& patsy = data.descriptorSetsLayout[b->set];

					if (patsy[0].set != -1) {
						if (b->binding >= patsy[0].binding) {
							bool found = false;
							for (contextualCount = 0; contextualCount < patsy.size(); contextualCount++) {
								if (b->binding == patsy[contextualCount].binding) {
									patsy[contextualCount].stages |= bindingData.stages;
									found = true;
									break;
								}
								if (b->binding < patsy[contextualCount].binding) {
									patsy.insert(patsy.begin() + contextualCount, bindingData);
									found = true;
									break;
								}
							}
							if (!found) patsy.push_back(bindingData);
						}
						else {
							patsy.insert(patsy.begin(), bindingData);
						}
					}
					else {
						patsy[0] = bindingData;
					}
				}
			}
			spvReflectEnumeratePushConstantBlocks(&reflectData, &contextualCount, nullptr);
			if(contextualCount > 0){
				std::vector<SpvReflectBlockVariable*> foundPCB(contextualCount);
				spvReflectEnumeratePushConstantBlocks(&reflectData, &contextualCount, foundPCB.data());

				for (auto p : foundPCB) {//might cause problems with more than one block or even using them in more than one stage
					PushConstantRangeGenerationData ayyyylmao{};
					ayyyylmao.offset = p->offset;
					ayyyylmao.size = p->size;
					ayyyylmao.stages = (VkShaderStageFlags)reflectData.shader_stage;
					data.PCRData.push_back(ayyyylmao);
				}
			}


			temp.push_back(boink);
			garbageCan.push_back(&reflectData);
		}
		for (auto& s : data.descriptorSetsLayout) {
			if (s[0].set == -1)s.clear();
		}
		std::ofstream file(filename, std::ios::binary | std::ios::out | std::ios::trunc);
		file.write(&data.numShaders, sizeof(char));
		file.write((char*)&data.vertInputs, sizeof(SAGEVertexType));
		file.write((char*)&nameSize, sizeof(size_t));
		file.write(data.name.data(), nameSize);
		for (auto& s : data.descriptorSetsLayout) {
			size_t setSize = s.size();
			file.write((char*)&setSize, sizeof(size_t));
			if (setSize > 0) {
				file.write(&s[0].set, sizeof(char));
				for (auto& b : s) {
					file.write((char*)&b.binding, sizeof(uint32_t));
					file.write((char*)&b.stages, sizeof(VkShaderStageFlags));
					size_t nameSize = b.name.size();
					file.write((char*)&nameSize, sizeof(size_t));
					file.write(b.name.data(), nameSize);
					file.write((char*)&b.type, sizeof(VkDescriptorType));
					file.write((char*)&b.count, sizeof(uint32_t));
				}
			}
		}
		size_t pushSize = data.PCRData.size();
		file.write((char*)&pushSize, sizeof(size_t));
		for (auto& b : data.PCRData) {
			file.write((char*)&b.offset, sizeof(uint32_t));
			file.write((char*)&b.size, sizeof(uint32_t));
			file.write((char*)&b.stages, sizeof(VkShaderStageFlags));
		}
		for (size_t i = 0; i < temp.size(); i++) {
			size_t bonk = data.shaderEntryPoints[i].size();
			file.write((char*)&bonk, sizeof(size_t));
			file.write(data.shaderEntryPoints[i].data(), bonk);
			bonk = temp[i].size();
			file.write((char*)&data.shaderStages[i], sizeof(VkShaderStageFlagBits));
			file.write((char*)&bonk, sizeof(size_t));
			file.write(temp[i].data(), bonk);
		}
		file.close();
		for (auto t : garbageCan) {
			spvReflectDestroyShaderModule(t);
		}
	}
	bool loadShader(const std::string& filepath, ShaderFileData& returnVal) {
		std::vector<char> fileData{};
		if (!readWholeBinaryFile(filepath, fileData)) return false;
		void* cursor = fileData.data();
		ShaderFileData returnData{};

		char* charPtr;
		size_t* sizePtr;
		uint32_t* uIntPtr;

		charPtr = (char*)cursor;
		returnData.numShaders = *charPtr++;
		cursor = charPtr;

		SAGEVertexType* vertPtr = (SAGEVertexType*)cursor;
		returnData.vertInputs = *vertPtr++;
		cursor = vertPtr;

		sizePtr = (size_t*)cursor;
		returnData.name.resize(*sizePtr++);
		cursor = sizePtr;

		charPtr = (char*)cursor;
		for (size_t i = 0; i < returnData.name.size(); i++) {
			returnData.name[i] = *charPtr++;
		}
		cursor = charPtr;

		size_t contextualSize;
		for (size_t i = 0; i < 4; i++) {
			returnData.descriptorSetsLayout.push_back({});
			sizePtr = (size_t*)cursor;
			contextualSize = *sizePtr++;
			cursor = sizePtr;
			if (contextualSize > 0) {
				char tSetNum;

				charPtr = (char*)cursor;
				tSetNum = *charPtr++;
				cursor = charPtr;
				for (size_t j = 0; j < contextualSize; j++) {
					DescriptorGenerationData temp;
					temp.set = tSetNum;
					uIntPtr = (uint32_t*)cursor;
					temp.binding = *uIntPtr++;
					cursor = uIntPtr;
					VkShaderStageFlags* ssfPtr = (VkShaderStageFlags*)cursor;
					temp.stages = *ssfPtr++;
					cursor = ssfPtr;
					sizePtr = (size_t*)cursor;
					temp.name.resize(*sizePtr++);
					cursor = sizePtr;
					charPtr = (char*)cursor;
					for (auto& c : temp.name) {
						c = *charPtr++;
					}
					cursor = charPtr;
					VkDescriptorType* dtPtr = (VkDescriptorType*)cursor;
					temp.type = *dtPtr++;
					cursor = dtPtr;
					uIntPtr = (uint32_t*)cursor;
					temp.count = *uIntPtr++;
					cursor = uIntPtr;
					returnData.descriptorSetsLayout[i].push_back(temp);
				}

			}
		}
		sizePtr = (size_t*)cursor;
		returnData.PCRData.resize(*sizePtr++);
		cursor = sizePtr;
		for (auto& p : returnData.PCRData) {
			uIntPtr = (uint32_t*)cursor;
			p.offset = *uIntPtr++;
			p.size = *uIntPtr++;
			cursor = uIntPtr;
			VkShaderStageFlags* ssfPtr = (VkShaderStageFlags*)cursor;
			p.stages = *ssfPtr++;
			cursor = ssfPtr;
		}
		for (size_t i = 0; i < returnData.numShaders; i++) {
			returnData.shaderData.push_back({});

			sizePtr = (size_t*)cursor;
			std::string entryPoint;
			entryPoint.resize(*sizePtr++);
			returnData.shaderEntryPoints.push_back(entryPoint);
			cursor = sizePtr;
			charPtr = (char*)cursor;
			for (auto& c : returnData.shaderEntryPoints[i]) {
				c = *charPtr++;
			}
			cursor = charPtr;

			VkShaderStageFlagBits* ssfbPtr = (VkShaderStageFlagBits*)cursor;
			returnData.shaderStages.push_back(*ssfbPtr++);
			cursor = ssfbPtr;

			sizePtr = (size_t*)cursor;
			returnData.shaderData[i].reserve(*sizePtr);
			returnData.shaderDataSizes.push_back(*sizePtr++);
			cursor = sizePtr;

			uIntPtr = (uint32_t*)cursor;
			for (size_t j = 0; j < returnData.shaderDataSizes[i] / sizeof(uint32_t); j++) {
				returnData.shaderData[i].push_back(*uIntPtr++);
			}
			cursor = uIntPtr;

		}

		returnVal = std::move(returnData);
		return true;
	}

	uint8_t bindingFromEntryString(std::string s) {
		if (s == "diffuseMap") return DIFFUSE_TEXTURE_MAP;
		if (s == "specularMap") return SPECULAR_TEXTURE_MAP;
		if (s == "bumpMap" || s == "heightMap") return BUMP_TEXTURE_MAP;
		if (s == "colorMap" || s == "albedoMap") return ALBEDO_TEXTURE_MAP;
		if (s == "normalMap") return NORMAL_TEXTURE_MAP;
		if (s == "reflectivityMap" || s == "metalnessMap") return REFLECTIVITY_TEXTURE_MAP;
		if (s == "glossinessMap") return GLOSSINESS_TEXTURE_MAP;
		if (s == "aoMap" || s == "ambientOcclusionMap") return AO_TEXTURE_MAP;
//		if (s == "opacityMap") return OPACITY_TEXTURE_MAP;
//		if (s == "refractionMap") return REFRACTION_TEXTURE_MAP;
//		if (s == "emissiveMap") return EMISSIVE_TEXTURE_MAP;
		return NO_MAPS;
	}
	void compileManifestMaterialEntry(const std::string& entry){
		MaterialFileData data{};
		std::string munch = entry.substr(entry.find('|') + 1);
		std::string filename = "Binassets/" + munch.substr(0, munch.find('='));
		munch = munch.substr(munch.find('=') + 2);
		data.shaderId = std::stoul(munch.substr(0, munch.find(';')));
		munch = munch.substr(munch.find(';') + 2);

		while (munch.find(' ') != munch.find(';') + 1) {
			data.pbrBindingSlots.push_back(bindingFromEntryString(munch.substr(0, munch.find(','))));
			munch = munch.substr(munch.find(',') + 2);
		}
		data.pbrBindingSlots.push_back(bindingFromEntryString(munch.substr(0, munch.find(';'))));
		munch = munch.substr(munch.find(';') + 2);

		while (munch.find(' ') != std::string::npos) {
			data.pipelineConfigNames.push_back(munch.substr(0, munch.find(',')));
			munch = munch.substr(munch.find(' ') + 1);
		}
		data.pipelineConfigNames.push_back(munch);

		size_t pbrBindingsSize = data.pbrBindingSlots.size();
		size_t justincase = data.pipelineConfigNames.size();
		std::ofstream matFile(filename, std::ios::binary | std::ios::out | std::ios::trunc);
		matFile.write((char*)&data.shaderId, sizeof(uint32_t));
		matFile.write((char*)&pbrBindingsSize, sizeof(size_t));
		for(auto& b : data.pbrBindingSlots) {
		matFile.write((char*)&b, sizeof(uint8_t));
		}
		matFile.write((char*)&justincase, sizeof(size_t));
		for (auto& n : data.pipelineConfigNames) {
			size_t moreSafty = n.size();
			matFile.write((char*)&moreSafty, sizeof(size_t));
			matFile.write(n.data(), moreSafty);
		}
		size_t safety = 0;
		matFile.write((char*)&safety, sizeof(size_t));
		//matFile.write((char*)&safety, sizeof(size_t)); needed if also reading pipeline data on load
		matFile.close();
	}
	bool loadMaterial(const std::string& filepath, MaterialFileData& returnVal) {
		std::vector<char> fileData{};
		if (!readWholeBinaryFile(filepath, fileData)) return false;
		void* cursor = fileData.data();
		MaterialFileData returnData{};

		uint32_t* pUint32 = (uint32_t*)cursor;
		returnData.shaderId = *pUint32++;
		cursor = pUint32;

		size_t* pSize = (size_t*)cursor;
		returnData.pbrBindingSlots.resize(*pSize++);
		cursor = pSize;
		uint8_t* pUint8 = (uint8_t*)cursor;
		for (auto& i : returnData.pbrBindingSlots) {
			i = *pUint8++;
		}
		cursor = pUint8;

		pSize = (size_t*)cursor;
		returnData.pipelineConfigNames.resize(*pSize++);
		char* pChar;
		for (auto& n : returnData.pipelineConfigNames) {
			n.resize(*pSize++);
			cursor = pSize;
			pChar = (char*)cursor;
			for (auto& c : n) {
				c = *pChar++;
			}
			cursor = pChar;
			pSize = (size_t*)cursor;
		}
		/*
		returnData.pipelineCacheData.resize(*pSize);
		if (returnData.pipelineCacheData.size() > 0) {
			cursor = ++pSize;
			pChar = (char*)cursor;
			for (auto& c : returnData.pipelineCacheData) {
				c = *pChar++;
			}
		}*/// in case we ever need it again
		returnVal = std::move(returnData);
		return true;
	}

	void compileManifestEntry(const std::string& entry) {
		UUIDinfo info{ UUIDfromEntry(entry) };
		switch (info.getAssetType()) {
		case SAGE_SHADER_ASSET:
			compileManifestShaderEntry(entry);
			break;
		case SAGE_MESH_ASSET:
			compileManifestMeshEntry(entry);
			break;
		case SAGE_TEXTURE_ASSET:
			compileManifestTextureEntry(entry);
			break;
		case SAGE_CUBEMAP_ASSET:
			compileManifestCubemapEntry(entry);
			break;
		case SAGE_MATERIAL_ASSET:
			compileManifestMaterialEntry(entry);
			break;
		default:
			std::cout << "We can't compile that type!";
			break;
		}
	}
	bool loadAsset(uint32_t UUID, void* returnData) {
		std::string filePath = "Binassets/" + std::to_string(UUID);
		UUIDinfo info{ UUID };
		switch (info.getAssetType())
		{
		case SAGE_SHADER_ASSET:
			return loadShader(filePath, *(ShaderFileData*)returnData);
		case SAGE_MESH_ASSET:
			return loadMesh(filePath, *(MeshFileData*)returnData);
		case SAGE_TEXTURE_ASSET:
			return loadTexture(filePath, *(TextureFileData*)returnData);
		case SAGE_CUBEMAP_ASSET:
			return loadCubemap(filePath, *(Image*)returnData);
		case SAGE_MATERIAL_ASSET:
			return loadMaterial(filePath, *(MaterialFileData*)returnData);
		default:
			std::cout << "O.O\n";
			break;
		}
		return false;
	}

	bool renameHandler(const std::string& entry, std::vector<std::string>& oldEntries, bool isRecursive = false) {
		std::string newName = STRINGfromEntry(entry);
		uint32_t newNum = UUIDfromEntry(entry);

		for (size_t i = 0; i < oldEntries.size(); i++) {

			std::string oldtry = oldEntries[i];
			std::string oldName = STRINGfromEntry(oldtry);
			uint32_t oldNum = UUIDfromEntry(oldtry);
			std::string oldPath = "Binassets/" + std::to_string(oldNum);
			std::string newPath = "Binassets/" + std::to_string(newNum);

			if (oldName == newName && oldNum == newNum) {
				oldEntries.erase(oldEntries.begin() + i);
				return std::filesystem::exists(newPath) && oldtry == entry;
			}

			if (newNum == oldNum) {
				std::string newEntryFromOld{};
				STRINGtoUUID(oldName, &newEntryFromOld);
				if (newEntryFromOld.size() > 0) {
					std::string& swapEntry = oldtry;
					for (auto& o : oldEntries) {
						if (STRINGfromEntry(o) == newName) {
							swapEntry = o;
							break;
						}
					}
					if (UUIDfromEntry(swapEntry) == UUIDfromEntry(newEntryFromOld)) {
						newPath = "Binassets/" + std::to_string(UUIDfromEntry(newEntryFromOld));
						std::filesystem::rename(newPath, "Binassets/!swapTemp");
						std::filesystem::rename(oldPath, newPath);
						std::filesystem::rename("Binassets/!swapTemp", oldPath);
						swapEntryNames(oldEntries[i], swapEntry);
						i--;
						continue;
					}
					renameHandler(newEntryFromOld, oldEntries, true);
				}
				else {
					std::filesystem::remove("Binassets/" + std::to_string(oldNum));
					oldEntries.erase(oldEntries.begin() + i);
				}
				if (isRecursive) {
					i -= 2;
					continue;
				}
				else {
					i--;
					continue;
				}
			}

			if ((!std::filesystem::exists(newPath)) && newName == oldName) {
				std::filesystem::rename(oldPath, newPath);
				if (isRecursive) refreshEntryPrefix(oldEntries[i], entry);
				else oldEntries.erase(oldEntries.begin() + i);
				return true;
			}

		}

		return false;
	}

	void compileManifest(bool forceFullRecompile) {
		std::string line{};
		std::string oldPath = ASS_manifestPath + ".old";
		std::vector<std::string> oldidates;
		if (!forceFullRecompile) {
			if (std::filesystem::exists(oldPath)) {
				std::ifstream oldifest(oldPath);
				while (std::getline(oldifest, line)) {
					if (line.find(':') == std::string::npos) continue;
					oldidates.push_back(line);
				}
				oldifest.close();
			}
		}
		std::ifstream manifest(ASS_manifestPath);
		while (std::getline(manifest, line)) {
			if (line.find(':') == std::string::npos) continue;
			if (!forceFullRecompile && renameHandler(line, oldidates))continue;
			compileManifestEntry(line);
		}
		manifest.close();
		if (std::filesystem::exists(oldPath)) std::filesystem::remove(oldPath);
	}

	Asset::Asset(size_t pos) {
		listPosition = pos;
		numReferences = 1;
	}

	AssetManager::AssetManager() {
		if (assetsChanged()) {
			compileShaders();
			createManifest();
			compileManifest();
		}
		referenceCount = 0;
	}
	AssetManager::~AssetManager() {
		if (assetList.size() > 0) {
			for (size_t i = 0; i < assetList.size(); i++) {
				if (std::find(freePositions.begin(), freePositions.end(), i) == freePositions.end()) delete assetList[i];
			}
		}
	}
	AssetManager* AssetManager::getManager() {
		if (!instance) instance = new AssetManager();
		instance->referenceCount++;
		return instance;
	}
	void AssetManager::release() {
		if (--referenceCount == 0) delete this;
	}
	Asset* AssetManager::getAsset(uint32_t ID, void* mut) {
		Asset* returnPtr = nullptr;
		if (mut == nullptr){
			returnPtr = assetList[ID];
			returnPtr->numReferences++;
			return returnPtr;
		}
		if (loadAsset(ID, mut)) {
			size_t freebie = assetList.size();
			if (freePositions.size() != 0) {
				freebie = freePositions.back();
				freePositions.pop_back();
			}
			returnPtr = new Asset(freebie);
			returnPtr->UUID = ID;
			assetList.push_back(returnPtr);
		}
		return returnPtr;
	}
	uint32_t AssetManager::getAssetUUID(std::string STRING) {
		if (!entryLookups.contains(STRING)) {
			entryLookups[STRING] = STRINGtoUUID(STRING);
		}
		return entryLookups[STRING];
	}
	void AssetManager::returnAsset(Asset* asset) {

		freePositions.push_back(asset->listPosition);
		delete asset;
	
	}
}