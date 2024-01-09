#pragma once
//encompasses serialization and utilities.  Seems like it should be another program absent the asset manager.
#include <filesystem>
#include <iostream>
#include <fstream>
#include <array>
#include <algorithm>
#include <vector>
//#include <queue>
#include <unordered_map>
#include <random>
#include <bitset>
#include <functional>
#include <chrono>

#include <vulkan/vulkan.h>
#include <spirv_reflect.h>
#include <SDL.h>
#include <SDL_vulkan.h>

#include <stb_image.h>
#include <tiny_obj_loader.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <glm/gtx/quaternion.hpp>

#include "DataTypes.hpp"
/*
Constant Prefix Key:
ASS_: Asset System
AUD_: Audio System
HAX_: Hardware Accelerator
SVK_: relevant to SAGE's implementation of Vulkan functionality, probably to limit Enum ranges
	  to non-extended functionality.
VID_: Video System
*/
namespace {
	uint8_t HAX_numPreferredGraphicComputes = 2, HAX_numPreferredAsyncComputes = 1;
	std::string ASS_manifestPath = "Assets/MANIFEST.txt";
	int AUD_preferredHertz = 48000;
	SDL_AudioFormat AUD_preferredFormat = AUDIO_F32;
	Uint8 AUD_preferredNumChannels = 2;
	Uint16 AUD_preferredSampleBufferLength = 512, AUD_bufferPadding = 0;
}
//if stb loading fails, call stbi_err(0,0) to clear the error.
namespace SAGE {
	// from: https://www.stackoverflow.com/a/57595105
	//technically also from Brendan Galea's series but...you know.
	template <typename T, typename... Rest>
	void hashCombine(std::size_t& seed, const T& v, const Rest&... rest) {
		seed ^= std::hash<T>{}(v)+0x9e3779b9 + (seed << 6) + (seed >> 2);
		(hashCombine(seed, rest), ...);
	};//might want to get rid of hashing in favor of pre-deleting duplicate vertices thoroughly

	template <typename BitCollection, typename BitMask>
	void clearMaskBits(BitCollection& field, const BitMask& mask) {
		BitCollection xorRes = field ^ mask;
		field &= xorRes;
	}
	template <typename T>
	void vCombine(std::vector<T>& vec, std::vector<T>& vic) {
		vec.insert(vec.end(), vic.begin(), vic.end());
	}
	template <typename T>
	double lerp(T a, T b, double d) {
		return a + d * (b - a);
	}

	/*
	template <typename Member>
	class MemberList {
		std::vector<Member> memberList;
		std::vector<size_t> freeIDList;
	public:
		MemberList() = default;
		~MemberList() = default;

		Member& operator[](size_t i) { return memberList[i]; }
		size_t size() { return memberList.size(); }
		iterator begin() { return memberList.begin(); }
		iterator end() { return memberList.end(); }

		size_t add(Member);
		void remove(size_t i);
	};
	template <typename Member>
	size_t MemberList<Member>::add(Member m) {
			size_t index = memberList.size();
			if (freeIDList.size() > 0) {
				index = freeIDList.back();
				freeIDList.pop_back();
				memberList[index] = m;
				return index;
			}
			memberList.push_back(m);
			return index;
	}
	template <typename Member>
	void MemberList<Member>::remove(size_t i) {
			freeIDList.push_back(i);
			memberList[i] = nullptr;
	}

	Lord knows I tried to make this damned thing work.
	Let this block stand as testament to the shame that is C++ Template classes, I curse thee!

	*/

	uint32_t roundPow2(uint32_t);

	VkCompareOp stringToVkCompareOp(const std::string&);
	VkLogicOp stringToVkLogicOp(const std::string&);
	VkStencilOp stringToVkStencilOp(const std::string&);
	VkBlendOp stringToVkBlendOp(const std::string&);
	VkBlendFactor stringToVkBlendFactor(const std::string&);
	VkDynamicState stringToVkDynamicState(const std::string&);

	struct QueueFamilySorter {
		std::vector<QueueFam> reference;
		bool exclusiveTransferAvailable{}, asyncComputeAvailable{}, gComputeAvailable{};
		bool enoughComputes{}, enoughGraphics{};

		QueueFamilySorter(const std::vector<VkQueueFamilyProperties>&);

		void prepareOutput();
		void print();

	};
	
	struct GraphicsCardProperties {
		VkSampleCountFlagBits maxMSAA;
		uint32_t numFrames;
		float maxSamplerAnisotropy;
		float maxSamplerLoDBias;
		std::vector<VkMemoryType> memoryTypes;
		std::vector<VkMemoryHeap> memoryHeaps;
	};
	//std::string SDL_AudioFormatToString(SDL_AudioFormat);
	struct SoundCardProperties {
		size_t outputHertz;
		size_t outputSamplesPerChannel;
		SDL_AudioFormat outputBitDepth;
		uint8_t outputNumChannels;
	};

	extern struct HardwareInfo {
		GraphicsCardProperties graphicsCardInfo;
		SoundCardProperties soundCardInfo;
	}hardwareInfo;

	struct SAGEVertexInfo {
		std::vector<VkVertexInputBindingDescription> vertexBindingData;
		std::vector<VkVertexInputAttributeDescription> vertexInputData;
		SAGEVertexType vertexType;

		SAGEVertexInfo() = default;
		SAGEVertexInfo(SAGEVertexType);
	};

	class UUIDinfo {
		static const uint32_t assetTypeBitmask = 0b11111111000000000000000000000000;
		static const uint32_t numberBitmask    = 0b00000000111111111111111111111111;
		uint32_t actualUUID;
		UUIDinfo() = delete;
	public:
		UUIDinfo(uint32_t ID) { actualUUID = ID; }
		uint32_t getUUID() { return actualUUID; }
		SAGEAssetType getAssetType() { return (SAGEAssetType)((actualUUID & assetTypeBitmask) >> 24);}
		uint32_t getAssetID() { return actualUUID & numberBitmask; }
	};

	bool readWholeBinaryFile(const std::string&, std::vector<char>&);
	bool readWholeTextFile(const std::string&, std::vector<std::string>&);

	bool importWaveFile(const std::string&, External_WaveFileData&);
 
	void surveyHardware();

	bool loadHardwareInfo();

	std::string makeUUID(SAGEAssetType, uint32_t, std::filesystem::path);

	std::string STRINGfromEntry(const std::string&);
	uint32_t UUIDfromEntry(const std::string&);
	//these functions double as manifest entry getters
	uint32_t STRINGtoUUID(std::string, std::string* = nullptr);
	//will put the entry in the value of the optional parameter
	std::string UUIDtoSTRING(uint32_t, std::string* = nullptr);

	std::vector<std::string> translateManifestShadersEntry(std::string);//argument is STRING
	//will have our own process?  Might have to to replace the batch file.
	//as this will only work on Windows
	void compileShaders();

	//sound asset placeholder
	std::string createTextureAssetList(std::vector<std::filesystem::path>&);
	std::string createMeshAssetList(std::vector<std::filesystem::path>&);
	std::string createShaderAssetList(std::vector<std::filesystem::path>&);
	std::string createCubemapAssetList(std::vector<std::filesystem::path>&);
	std::string createMaterialAssetList(std::vector<std::filesystem::path>&);

	void createManifest();

	bool assetsChanged();
	
	//sound asset stuff up here

	void textureDefaultHandler(size_t, Image&);
	void compileManifestTextureEntry(const std::string&);
	bool loadTexture(const std::string&, TextureFileData&);

	void compileManifestCubemapEntry(const std::string&);
	bool loadCubemap(const std::string&, Image&);

	void compileManifestMeshEntry(const std::string&);
	bool loadMesh(const std::string&, MeshFileData&);

	void compileManifestShaderEntry(const std::string&);
	bool loadShader(const std::string&, ShaderFileData&);

	void compileManifestMaterialEntry(const std::string&);
	bool loadMaterial(const std::string&, MaterialFileData&);

	void compileManifestEntry(const std::string&);
	bool loadAsset(uint32_t, void*);
	//the bool should only need to be true if we fucked something catastrophically
	void compileManifest(bool = false);

	struct Asset {
		//constructor registers with asset handler
		Asset() = default;
		Asset(size_t);
		virtual ~Asset() = default;
		//Will need AssetID or UUID?  Name?
		uint32_t UUID;
		size_t listPosition;
		size_t numReferences;//implement the reference counting in the libraries themselves
	};

	class AssetManager {
		static AssetManager* instance;
		uint8_t referenceCount;
		std::vector<Asset*> assetList;
		std::vector<size_t> freePositions;
		std::unordered_map<std::string, uint32_t> entryLookups;
		AssetManager();
		~AssetManager();
	public:
		static AssetManager* getManager();
		AssetManager(AssetManager const&) = delete;
		void operator=(AssetManager const&) = delete;
		//Only for use by systems shutting down
		void release();

		Asset* getAsset(uint32_t, void*);
		uint32_t getAssetUUID(std::string);
		//if the reference count reaches 0, delete the asset and add the position to available ones.
		void returnAsset(Asset*);
	};


}

