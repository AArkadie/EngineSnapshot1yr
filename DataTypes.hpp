#pragma once

#include <array>
#include <vector>
#include <unordered_map>

#include <vulkan/vulkan.h>
#include <SDL_scancode.h>
#include <SDL_keycode.h>
#include <glm/glm.hpp>

namespace SAGE{

	enum SAGEKeyLocation { //SDL Scancodes
		UNKNOWN_KEY = SDL_SCANCODE_UNKNOWN,

		A_KEY = SDL_SCANCODE_A,
		B_KEY = SDL_SCANCODE_B,
		C_KEY = SDL_SCANCODE_C,
		D_KEY = SDL_SCANCODE_D,
		E_KEY = SDL_SCANCODE_E,
		F_KEY = SDL_SCANCODE_F,
		G_KEY = SDL_SCANCODE_G,
		H_KEY = SDL_SCANCODE_H,
		I_KEY = SDL_SCANCODE_I,
		J_KEY = SDL_SCANCODE_J,
		K_KEY = SDL_SCANCODE_K,
		L_KEY = SDL_SCANCODE_L,
		M_KEY = SDL_SCANCODE_M,
		N_KEY = SDL_SCANCODE_N,
		O_KEY = SDL_SCANCODE_O,
		P_KEY = SDL_SCANCODE_P,
		Q_KEY = SDL_SCANCODE_Q,
		R_KEY = SDL_SCANCODE_R,
		S_KEY = SDL_SCANCODE_S,
		T_KEY = SDL_SCANCODE_T,
		U_KEY = SDL_SCANCODE_U,
		V_KEY = SDL_SCANCODE_V,
		W_KEY = SDL_SCANCODE_W,
		X_KEY = SDL_SCANCODE_X,
		Y_KEY = SDL_SCANCODE_Y,
		Z_KEY = SDL_SCANCODE_Z,

		ESC_KEY = SDL_SCANCODE_ESCAPE,
		BACKSPACE_KEY = SDL_SCANCODE_BACKSPACE,
		SPACE_BAR = SDL_SCANCODE_SPACE,
		TAB_KEY = SDL_SCANCODE_TAB,
		CAPS_KEY = SDL_SCANCODE_CAPSLOCK,
		LSHIFT_KEY = SDL_SCANCODE_LSHIFT,
		RSHIFT_KEY = SDL_SCANCODE_RSHIFT,
		RETURN_KEY = SDL_SCANCODE_RETURN,
		LCTRL_KEY = SDL_SCANCODE_LCTRL,
		RCTRL_KEY = SDL_SCANCODE_RCTRL,
		//leave out the windows key?
		//same goes for that one with a list, will have to look at images of keyboards to see what's common
		LALT_KEY = SDL_SCANCODE_LALT,
		RALT_KEY = SDL_SCANCODE_RALT,

		F1_KEY = SDL_SCANCODE_F1,
		F2_KEY = SDL_SCANCODE_F2,
		F3_KEY = SDL_SCANCODE_F3,
		F4_KEY = SDL_SCANCODE_F4,
		F5_KEY = SDL_SCANCODE_F5,
		F6_KEY = SDL_SCANCODE_F6,
		F7_KEY = SDL_SCANCODE_F7,
		F8_KEY = SDL_SCANCODE_F8,
		F9_KEY = SDL_SCANCODE_F9,
		F10_KEY = SDL_SCANCODE_F10,
		F11_KEY = SDL_SCANCODE_F11,
		F12_KEY = SDL_SCANCODE_F12,

		PRINT_SCREEN_KEY = SDL_SCANCODE_PRINTSCREEN,
		SCROLL_LOCK_KEY = SDL_SCANCODE_SCROLLLOCK,
		PAUSE_KEY = SDL_SCANCODE_PAUSE,
		INSERT_KEY = SDL_SCANCODE_INSERT,
		HOME_KEY = SDL_SCANCODE_HOME,
		PAGE_UP_KEY = SDL_SCANCODE_PAGEUP,
		DEL_KEY = SDL_SCANCODE_DELETE,
		END_KEY = SDL_SCANCODE_END,
		PAGE_DOWN_KEY = SDL_SCANCODE_PAGEDOWN,

		RIGHT_KEY = SDL_SCANCODE_RIGHT,
		LEFT_KEY = SDL_SCANCODE_LEFT,
		DOWN_KEY = SDL_SCANCODE_DOWN,
		UP_KEY = SDL_SCANCODE_UP,

		GRAVE_TILDE_KEY = SDL_SCANCODE_GRAVE,
		ONE_EXCLAMATION_KEY = SDL_SCANCODE_1,
		TWO_ASPERAND_KEY = SDL_SCANCODE_2,
		THREE_POUND_KEY = SDL_SCANCODE_3,
		FOUR_USD_KEY = SDL_SCANCODE_4,
		FIVE_PERCENT_KEY = SDL_SCANCODE_5,
		SIX_CARET_KEY = SDL_SCANCODE_6,
		SEVEN_AMPERSAND_KEY = SDL_SCANCODE_7,
		EIGHT_ASTERISK_KEY = SDL_SCANCODE_8,
		NINE_LPAREN_KEY = SDL_SCANCODE_9,
		ZERO_RPAREN_KEY = SDL_SCANCODE_0,
		MINUS_UNDERSCORE_KEY = SDL_SCANCODE_MINUS,
		EQUALS_PLUS_KEY = SDL_SCANCODE_EQUALS,
		LBRACKET_LBRACE_KEY = SDL_SCANCODE_LEFTBRACKET,
		RBRACKET_RBRACE_KEY = SDL_SCANCODE_RIGHTBRACKET,
		BACKSLASH_PIPE_KEY = SDL_SCANCODE_BACKSLASH,
		SEMI_COLON_KEY = SDL_SCANCODE_SEMICOLON,
		QUOTES_KEY = SDL_SCANCODE_APOSTROPHE,
		COMMA_LESS_KEY = SDL_SCANCODE_COMMA,
		DOT_GREATER_KEY = SDL_SCANCODE_PERIOD,
		SLASH_QUESTION_KEY = SDL_SCANCODE_SLASH,

		NUMPAD_LOCK_KEY = SDL_SCANCODE_NUMLOCKCLEAR,
		NUMPAD_DIVIDE_KEY = SDL_SCANCODE_KP_DIVIDE,
		NUMPAD_MULTIPLY_KEY = SDL_SCANCODE_KP_MULTIPLY,
		NUMPAD_SUBTRACT_KEY = SDL_SCANCODE_KP_MINUS,
		NUMPAD_ADD_KEY = SDL_SCANCODE_KP_PLUS,
		NUMPAD_ENTER_KEY = SDL_SCANCODE_KP_ENTER,
		NUMPAD_0_KEY = SDL_SCANCODE_KP_0,
		NUMPAD_1_KEY = SDL_SCANCODE_KP_1,
		NUMPAD_2_KEY = SDL_SCANCODE_KP_2,
		NUMPAD_3_KEY = SDL_SCANCODE_KP_3,
		NUMPAD_4_KEY = SDL_SCANCODE_KP_4,
		NUMPAD_5_KEY = SDL_SCANCODE_KP_5,
		NUMPAD_6_KEY = SDL_SCANCODE_KP_6,
		NUMPAD_7_KEY = SDL_SCANCODE_KP_7,
		NUMPAD_8_KEY = SDL_SCANCODE_KP_8,
		NUMPAD_9_KEY = SDL_SCANCODE_KP_9,
		NUMPAD_DECIMAL_KEY = SDL_SCANCODE_KP_DECIMAL,//may not end up using keypad
		//seems like there are so many different layouts for the thing idk.

		MAX_KEYS = SDL_NUM_SCANCODES
	};

	enum SAGEKeyGlyph { //SDL Keycodes
		A_GLYPH = SDLK_a, //question is, do we really need these?  We'll leave underdeveloped for now
		B_GLYPH = SDLK_b,
		C_GLYPH = SDLK_c,
		D_GLYPH = SDLK_d,
		E_GLYPH = SDLK_e,
		F_GLYPH = SDLK_f,
		G_GLYPH = SDLK_g,
		H_GLYPH = SDLK_h,
		I_GLYPH = SDLK_i,
		J_GLYPH = SDLK_j,
		K_GLYPH = SDLK_k,
		L_GLYPH = SDLK_l,
		M_GLYPH = SDLK_m,
		N_GLYPH = SDLK_n,
		O_GLYPH = SDLK_o,
		P_GLYPH = SDLK_p,
		Q_GLYPH = SDLK_q,
		R_GLYPH = SDLK_r,
		S_GLYPH = SDLK_s,
		T_GLYPH = SDLK_t,
		U_GLYPH = SDLK_u,
		V_GLYPH = SDLK_v,
		W_GLYPH = SDLK_w,
		X_GLYPH = SDLK_x,
		Y_GLYPH = SDLK_y,
		Z_GLYPH = SDLK_z
	};

	enum QueueFamilyType {
		PRESENT_FAMILY,
		TRANSFER_FAMILY,
		ASYNC_COMPUTE_FAMILY,
		GRAPHICS_COMPUTE_FAMILY,
		UNKNOWN_FAMILY,
		UNUSED_FAMILY
	};//should we make it so that these line up with the queue families stored by the HA system?
	//it would help with specifying ownership transfers, no?

	enum SAGEVertexType {
		SAGE_NO_VERTEX,
		SAGE_DEFAULT_VERTEX,
		SAGE_SPRITE_VERTEX,
		SAGE_PARTICLE_VERTEX
	};
	
	enum SAGEDescriptorDataType {
		SAGE_PER_FRAME_CAMERA_DATA,
		SAGE_PER_FRAME_SCENE_DATA,
		SAGE_PER_OBJECT_DATA,
		SAGE_TEXTURE_DATA,
		SAGE_DESCRIPTOR_DATA_LIMIT
	};

	enum SAGETextureMapType {
		DIFFUSE_TEXTURE_MAP,//We question what to do with these outdated maps
		SPECULAR_TEXTURE_MAP,//Do we want them to become generic data maps?
		BUMP_TEXTURE_MAP,//Or do we just phase them out entirely for a pbr workflow?
		ALBEDO_TEXTURE_MAP,//We might not even need all these maps, just have a generic workflow
		NORMAL_TEXTURE_MAP,//with like 4 or 5 maps and the shader does the work
		REFLECTIVITY_TEXTURE_MAP,//of interpreting the data in each slot correctly
		GLOSSINESS_TEXTURE_MAP,
		AO_TEXTURE_MAP,
//		OPACITY_TEXTURE_MAP,
//		REFRACTION_TEXTURE_MAP,
//		EMISSIVE_TEXTURE_MAP,
		MAX_TEXTURE_MAPS,
		NO_MAPS = UINT8_MAX
	};

	enum SAGEAssetType {
		SAGE_SHADER_ASSET,
		//we want a sound asset here, but what form does that take?  Sound packs?
		//We wrap a .wav in something?  Surely not just a bare .wav
		//Think about sound data with directives of some sort
		SAGE_MESH_ASSET,
		SAGE_TEXTURE_ASSET,
		SAGE_CUBEMAP_ASSET,
		SAGE_MATERIAL_ASSET,
		SAGE_CUSTOM_ASSET,//becomes GAME_ASSET, need the dev to supply custom loaders
		//keymaps fall here as well, I feel; key bindings should be game-specific.
		//or do we want a GAME_SETTINGS asset type?  Are we being too stingy with asset types?
		DEFAULT_ASSET = UINT8_MAX //putting in -1 will also get to this value, what a steal!
	};

	struct SAGEVolume {//consider merging with transform component... or is it something else?  Boundary component?
		int32_t x, y, z;
		uint32_t width, height, depth;

		SAGEVolume() = default;
		~SAGEVolume() = default;

		VkExtent2D getVkExt2D() { return { width, height }; }
		VkExtent3D getVkExt3D() { return { width, height, depth };	}
		VkOffset2D getVkPos2D() { return { x, y }; }
		VkOffset3D getVkPos3D() { return { x, y, z }; }
		VkRect2D getVkRect() { return { getVkPos2D(), getVkExt2D() }; }
		glm::vec2 getCenter2D() { return glm::vec2(width / 2, height / 2); }
		glm::vec3 getCenter3D() { return glm::vec3(width / 2, height / 2, depth / 2); }
	};

	struct QueueFam {
		QueueFamilyType familyUse;
		uint32_t maxQueues;
		float priority;
	};

	struct QueueFamilyPropertyFileInfo {
		std::string queueFamilyNumberString;
		std::string numMembersString;
		std::string capabilities;
	};

	struct VulkanImplementationInfoFileData {
		std::string graphicsCardName;
		std::string vulkanVersionNumber;
		std::vector<std::string> version10Features;
		std::vector<std::string> version11Features;
		std::vector<std::string> version12Features;
		std::vector<std::string> version13Features;
		std::vector<std::string> availableInstanceLayers;
		std::vector<std::string> availableInstanceExtensions;
		std::vector<std::string> availableDeviceExtensions;
		std::vector<std::string> requiredExtensions;
		std::vector<QueueFamilyPropertyFileInfo> queueFamilyInfo;
		//todo: figure out a way to chart out memory types as a fun exercise
	};

	struct AudioDeviceInfoFileData {
		std::string aString;
	};

	struct Vertex {
		glm::vec3 position;//xyz
		glm::vec3 normal;//xyz
		glm::vec3 normalTangent;
		glm::vec3 normalBitangent;
		glm::vec4 color;///rgba
		alignas(16)glm::vec2 texturePosition;//uv

		bool operator==(const Vertex& other) const {
			return position == other.position
				&& normal == other.normal
				&& color == other.color
				&& texturePosition == other.texturePosition;
		}
	};

	struct SpriteVertex {
		glm::vec4 posiTex;
	};

	struct Particle {
		glm::vec2 position;
		glm::vec2 velocity;
		glm::vec4 color;
	};

	struct MVPInfo {
		glm::mat4 modelMatrix;//transform to world space, relative to a global origin
		glm::mat4 viewMatrix;//transform to viewing space, a camera's point of view; this is the camera.
		glm::mat4 projectionMatrix;//transform to clip space, a -1 to 1 space with a perspective
		glm::mat4 padding;//Viewport transform takes into screen pixel fragment space
	};//this sturct is pretty much useless methinks

	struct CameraData {
		glm::mat4 view;
		glm::mat4 proj;
		glm::mat4 projview;
		glm::vec4 forward;
		glm::vec4 right;
		glm::vec4 up;
		glm::vec4 reserved;
	};

	struct LightData {
		glm::vec4 lightPos;//xyz, w means point if 1, directional if 0
		glm::vec4 lightColor;//xyz base color, w for brightness
		glm::vec4 attenuationData;//xyz attenuation factors, w store phi for a spotlight
		glm::vec4 spotData;//if nonzero we know it's a spotlight.  xyz for front, w for gamma
	};

	struct PushIndices {
		uint32_t modelIndex;//each int is 4 bytes, so we can fit 64 indices!
		uint32_t textureIndex;
		uint32_t someIndex;
		uint32_t anotherIndex;
	};

	struct PerObjectRenderData {
		glm::mat4 modelMatrix;
		glm::mat4 normalMatrix;
		glm::mat4 reserve192;
		//208
		//224
		//240
		glm::mat4 reserve256;
	};

	struct APSceneData {
		glm::vec4 fogColor;// w is for exponent
		glm::vec4 fogDistance;//x min, y max, zw reserved
		glm::vec4 ambientColor;
		glm::vec4 reserve64;

		LightData sceneLights[3];//increase by adding 4 for each 256 byte stretch after this to reserve.
		//But there's got to be a better way!
	};

	struct DescriptorGenerationData {
		char set;
		uint32_t binding;
		std::string name;
		VkDescriptorType type;
		uint32_t count;
		VkShaderStageFlags stages;

		DescriptorGenerationData() {
			set = -1;
			binding = 0;
			name = "NULL";
			type = VK_DESCRIPTOR_TYPE_MAX_ENUM;
			count = 0;
			stages = 0;
		}
	};
	struct PushConstantRangeGenerationData {
		uint32_t offset, size;
		VkShaderStageFlags stages;
	};

	struct Image {
		std::string samplerName;
		int width;
		int height;
		int channels;
		size_t imageDataSize;
		std::vector<uint8_t> imageData;
	};
	//for file data, do we want to have the file r/w functions as part of them, rather than a separate thing?
	struct TextureFileData {
		std::array<Image, MAX_TEXTURE_MAPS> maps;
	};

	struct SoundFileData {
		uint32_t sampleRate;
		uint8_t sampleCount, channelCount, bitDepth;
		std::vector<uint8_t> data;
	};//Note: unused right now, don't forget about it!

	struct MeshFileData {
		size_t vertexDataBlockSize;
		std::vector<Vertex> vertexData;
		size_t indexDataBlockSize;
		std::vector<uint32_t> indexData;
	};

	struct ShaderFileData {
		char numShaders;
		SAGEVertexType vertInputs;
		std::string name;
		std::vector<std::vector<DescriptorGenerationData>> descriptorSetsLayout;
		std::vector<PushConstantRangeGenerationData> PCRData;
		std::vector<VkShaderStageFlagBits> shaderStages;
		std::vector<std::string> shaderEntryPoints;
		std::vector<size_t> shaderDataSizes;
		std::vector<std::vector<uint32_t>> shaderData;
	};

	struct MaterialFileData {
		uint32_t shaderId;
		std::vector<uint8_t> pbrBindingSlots;
		std::vector<std::string> pipelineConfigNames;
		//std::vector<char> pipelineCacheData; defunct (for now?)
	};

	enum External_WaveFormats {
		EXTERNAL_WAVE_FORMAT_3COM_NBX                   = 0x7000, /* 3COM Corp. */

		EXTERNAL_WAVE_FORMAT_PROSODY_1612               = 0x0027, /* Aculab plc */
		EXTERNAL_WAVE_FORMAT_PROSODY_8KBPS              = 0x0094, /* Aculab plc */
		EXTERNAL_WAVE_FORMAT_AMR_NB                     = 0x7361, /* AMR Narrowband */
		EXTERNAL_WAVE_FORMAT_AMR_WB                     = 0x7362, /* AMR Wideband */
		EXTERNAL_WAVE_FORMAT_AMR_WP                     = 0x7363, /* AMR Wideband Plus */
		EXTERNAL_WAVE_FORMAT_G723_ADPCM                 = 0x0014, /* Antex Electronics Corporation */
		EXTERNAL_WAVE_FORMAT_ANTEX_ADPCME               = 0x0033, /* Antex Electronics Corporation */
		EXTERNAL_WAVE_FORMAT_G721_ADPCM                 = 0x0040, /* Antex Electronics Corporation */
		EXTERNAL_WAVE_FORMAT_G728_CELP                  = 0x0041, /* Antex Electronics Corporation */
		EXTERNAL_WAVE_FORMAT_G726_ADPCM                 = 0x0064, /* APICOM */
		EXTERNAL_WAVE_FORMAT_G722_ADPCM                 = 0x0065, /* APICOM */
		EXTERNAL_WAVE_FORMAT_ALAC                       = 0x6C61, /* Apple Lossless */
		EXTERNAL_WAVE_FORMAT_G729A                      = 0x0083, /* AT&T Labs, Inc. */
		EXTERNAL_WAVE_FORMAT_VME_VMPCM                  = 0x0680, /* AT&T Labs, Inc. */
		EXTERNAL_WAVE_FORMAT_TPC                        = 0x0681, /* AT&T Labs, Inc. */
		EXTERNAL_WAVE_FORMAT_SOUNDSPACE_MUSICOMPRESS    = 0x1500, /* AT&T Labs, Inc. */
		EXTERNAL_WAVE_FORMAT_APTX                       = 0x0025, /* Audio Processing Technology */
		EXTERNAL_WAVE_FORMAT_RAW_SPORT                  = 0x0240, /* Aureal Semiconductor */

		EXTERNAL_WAVE_FORMAT_IRAT                       = 0x0101, /* BeCubed Software Inc. */
		EXTERNAL_WAVE_FORMAT_BTV_DIGITAL                = 0x0400, /* Brooktree Corporation */

		EXTERNAL_WAVE_FORMAT_CANOPUS_ATRAC              = 0x0063, /* Canopus, co., Ltd. */
		EXTERNAL_WAVE_FORMAT_CIRRUS                     = 0x0060, /* Cirrus Logic */
		EXTERNAL_WAVE_FORMAT_LIGHTWAVE_LOSSLESS         = 0x08AE, /* Clearjump */
		EXTERNAL_WAVE_FORMAT_CODIAN                     = 0xA124, /* CODIAN */
		EXTERNAL_WAVE_FORMAT_VSELP                      = 0x0004, /* Compaq Computer Corp. */
		EXTERNAL_WAVE_FORMAT_COMVERSE_INFOSYS_G723_1    = 0xA100, /* Comverse Infosys */
		EXTERNAL_WAVE_FORMAT_COMVERSE_INFOSYS_AVQSBC    = 0xA101, /* Comverse Infosys */
		EXTERNAL_WAVE_FORMAT_COMVERSE_INFOSYS_SBC       = 0xA102, /* Comverse Infosys */
		EXTERNAL_WAVE_FORMAT_CONGRUENCY                 = 0x008D, /* Congruency Inc. */
		EXTERNAL_WAVE_FORMAT_CS2                        = 0x0260, /* Consistent Software */
		EXTERNAL_WAVE_FORMAT_CONTROL_RES_VQLPC          = 0x0034, /* Control Resources Limited */
		EXTERNAL_WAVE_FORMAT_CONTROL_RES_CR10           = 0x0037, /* Control Resources Limited */
		EXTERNAL_WAVE_FORMAT_CONVEDIA_G729              = 0x008C, /* Convedia Corp. */
		EXTERNAL_WAVE_FORMAT_CREATIVE_ADPCM             = 0x0200, /* Creative Labs, Inc */
		EXTERNAL_WAVE_FORMAT_CREATIVE_FASTSPEECH8       = 0x0202, /* Creative Labs, Inc */
		EXTERNAL_WAVE_FORMAT_CREATIVE_FASTSPEECH10      = 0x0203, /* Creative Labs, Inc */
		EXTERNAL_WAVE_FORMAT_CS_IMAADPCM                = 0x0039, /* Crystal Semiconductor IMA ADPCM */
		EXTERNAL_WAVE_FORMAT_CUSEEME                    = 0x1F03, /* CUSeeMe */

		EXTERNAL_WAVE_FORMAT_DF_G726                    = 0x0085, /* DataFusion Systems (Pty) (Ltd) */
		EXTERNAL_WAVE_FORMAT_DF_GSM610                  = 0x0086, /* DataFusion Systems (Pty) (Ltd) */
		EXTERNAL_WAVE_FORMAT_DIALOGIC_OKI_ADPCM         = 0x0017, /* Dialogic Corporation */
		EXTERNAL_WAVE_FORMAT_G726ADPCM                  = 0x0140, /* Dictaphone Corporation */
		EXTERNAL_WAVE_FORMAT_DICTAPHONE_CELP68          = 0x0141, /* Dictaphone Corporation */
		EXTERNAL_WAVE_FORMAT_DICTAPHONE_CELP54          = 0x0142, /* Dictaphone Corporation */
		EXTERNAL_WAVE_FORMAT_DIGITAL_G723               = 0x0123, /* Digital Equipment Corporation */
		EXTERNAL_WAVE_FORMAT_DTS_DS                     = 0x0190, /* Digital Theatre Systems, Inc. */
		EXTERNAL_WAVE_FORMAT_DIVIO_MPEG4_AAC            = 0x4143, /* Divio, Inc. */
		EXTERNAL_WAVE_FORMAT_DIVIO_G726                 = 0x4243, /* Divio, Inc. */
		EXTERNAL_WAVE_FORMAT_DOLBY_AC2                  = 0x0030, /* Dolby Laboratories */
		EXTERNAL_WAVE_FORMAT_DOLBY_AC4                  = 0xAC40, /* Dolby AC-4 */
		EXTERNAL_WAVE_FORMAT_DIGISTD                    = 0x0015, /* DSP Solutions, Inc. */
		EXTERNAL_WAVE_FORMAT_DIGIFIX                    = 0x0016, /* DSP Solutions, Inc. */
		EXTERNAL_WAVE_FORMAT_DSPGROUP_TRUESPEECH        = 0x0022, /* DSP Group, Inc */
		EXTERNAL_WAVE_FORMAT_DIGIREAL                   = 0x0035, /* DSP Solutions, Inc. */
		EXTERNAL_WAVE_FORMAT_DIGIADPCM                  = 0x0036, /* DSP Solutions, Inc. */

		EXTERNAL_WAVE_FORMAT_ECHOSC1                    = 0x0023, /* Echo Speech Corporation */
		EXTERNAL_WAVE_FORMAT_ECHOSC3                    = 0x003A, /* Echo Speech Corporation */
		EXTERNAL_WAVE_FORMAT_ENCORE_G726                = 0xA107, /* Encore Software */
		EXTERNAL_WAVE_FORMAT_ESPCM                      = 0x0061, /* ESS Technology */
		EXTERNAL_WAVE_FORMAT_ESST_AC3                   = 0x0241, /* ESS Technology, Inc. */

		EXTERNAL_WAVE_FORMAT_DVM                        = 0x2000, /* FAST Multimedia AG */
		EXTERNAL_WAVE_FORMAT_FLAC                       = 0xF1AC, /* flac.sourceforge.net */
		EXTERNAL_WAVE_FORMAT_FRACE_TELECOM_G729         = 0xA123, /* France Telecom */
		EXTERNAL_WAVE_FORMAT_FRAUNHOFER_IIS_MPEG2_AAC   = 0x0180, /* Fraunhofer IIS */
		EXTERNAL_WAVE_FORMAT_FM_TOWNS_SND               = 0x0300, /* Fujitsu Corp. */

		EXTERNAL_WAVE_FORMAT_GLOBAL_IP_ILBC             = 0xA116, /* Global IP */
		EXTERNAL_WAVE_FORMAT_GSM_AMR_CBR                = 0x7A21, /* GSMA/3GPP */
		EXTERNAL_WAVE_FORMAT_GSM_AMR_VBR_SID            = 0x7A22, /* GSMA/3GPP */

		EXTERNAL_WAVE_FORMAT_CU_CODEC                   = 0x0019, /* Hewlett-Packard Company */
		EXTERNAL_WAVE_FORMAT_HP_DYN_VOICE               = 0x001A, /* Hewlett-Packard Company */
		
		EXTERNAL_WAVE_FORMAT_ILINK_VC                   = 0x0230, /* I-link Worldwide */
		EXTERNAL_WAVE_FORMAT_IBM_CVSD                   = 0x0005, /* IBM Corporation */
		EXTERNAL_WAVE_FORMAT_INFOCOM_ITS_G721_ADPCM     = 0x008B, /* Infocom */
		EXTERNAL_WAVE_FORMAT_OLIGSM                     = 0x1000, /* Ing C. Olivetti & C., S.p.A. */
		EXTERNAL_WAVE_FORMAT_OLIADPCM                   = 0x1001, /* Ing C. Olivetti & C., S.p.A. */
		EXTERNAL_WAVE_FORMAT_OLICELP                    = 0x1002, /* Ing C. Olivetti & C., S.p.A. */
		EXTERNAL_WAVE_FORMAT_OLISBC                     = 0x1003, /* Ing C. Olivetti & C., S.p.A. */
		EXTERNAL_WAVE_FORMAT_OLIOPR                     = 0x1004, /* Ing C. Olivetti & C., S.p.A. */
		EXTERNAL_WAVE_FORMAT_INGENIENT_G726             = 0xA105, /* Ingenient Technologies, Inc. */
		EXTERNAL_WAVE_FORMAT_INNINGS_TELECOM_ADPCM      = 0x1979, /* Innings Telecom Inc. */
		EXTERNAL_WAVE_FORMAT_RT24                       = 0x0052, /* InSoft, Inc. */
		EXTERNAL_WAVE_FORMAT_PAC                        = 0x0053, /* InSoft, Inc. */
		EXTERNAL_WAVE_FORMAT_DVI_ADPCM                  = 0x0011, /* Intel Corporation */
		EXTERNAL_WAVE_FORMAT_IMA_ADPCM                  = 0x0011, /* Intel Corporation */
		EXTERNAL_WAVE_FORMAT_INTEL_G723_1               = 0x0043, /* Intel Corp. */
		EXTERNAL_WAVE_FORMAT_INTEL_G729                 = 0x0044, /* Intel Corp. */
		EXTERNAL_WAVE_FORMAT_INTEL_MUSIC_CODER          = 0x0401, /* Intel Corp. */
		EXTERNAL_WAVE_FORMAT_IPI_HSX                    = 0x0250, /* Interactive Products, Inc. */
		EXTERNAL_WAVE_FORMAT_IPI_RPELP                  = 0x0251, /* Interactive Products, Inc. */
		EXTERNAL_WAVE_FORMAT_ISIAUDIO_2                 = 0x1401, /* ISIAudio */
		EXTERNAL_WAVE_FORMAT_MPEGLAYER3                 = 0x0055, /* ISO/MPEG Layer3 Format Tag */
		EXTERNAL_WAVE_FORMAT_MPEG4_AAC                  = 0xA106, /* ISO/MPEG-4 */
		EXTERNAL_WAVE_FORMAT_ISIAUDIO                   = 0x0088, /* Iterated Systems, Inc. */

		EXTERNAL_WAVE_FORMAT_KNOWLEDGE_ADVENTURE_ADPCM  = 0x0178, /* Knowledge Adventure, Inc. */
		
		EXTERNAL_WAVE_FORMAT_LEAD_SPEECH                = 0x434C, /* LEAD Technologies */
		EXTERNAL_WAVE_FORMAT_LEAD_VORBIS                = 0x564C, /* LEAD Technologies */
		EXTERNAL_WAVE_FORMAT_LH_CODEC                   = 0x1100, /* Lernout & Hauspie */
		EXTERNAL_WAVE_FORMAT_LH_CODEC_CELP              = 0x1101, /* Lernout & Hauspie */
		EXTERNAL_WAVE_FORMAT_LH_CODEC_SBC8              = 0x1102, /* Lernout & Hauspie */
		EXTERNAL_WAVE_FORMAT_LH_CODEC_SBC12             = 0x1103, /* Lernout & Hauspie */
		EXTERNAL_WAVE_FORMAT_LH_CODEC_SBC16             = 0x1104, /* Lernout & Hauspie */
		EXTERNAL_WAVE_FORMAT_INDEO_AUDIO                = 0x0402, /* Ligos */
		EXTERNAL_WAVE_FORMAT_LUCENT_G723                = 0x0059, /* Lucent Technologies */
		EXTERNAL_WAVE_FORMAT_LUCENT_SX8300P             = 0x1C07, /* Lucent Technologies */
		EXTERNAL_WAVE_FORMAT_LUCENT_SX5363S             = 0x1C0C, /* Lucent Technologies */
	
		EXTERNAL_WAVE_FORMAT_MALDEN_PHONYTALK           = 0x00A0, /* Malden Electronics Ltd. */
		EXTERNAL_WAVE_FORMAT_MEDIAVISION_ADPCM          = 0x0018, /* Media Vision, Inc. */
		EXTERNAL_WAVE_FORMAT_MEDIASONIC_G723            = 0x0093, /* MediaSonic */
		EXTERNAL_WAVE_FORMAT_LRC                        = 0x0028, /* Merging Technologies S.A. */
		EXTERNAL_WAVE_FORMAT_MICRONAS                   = 0x0350, /* Micronas Semiconductors, Inc. */
		EXTERNAL_WAVE_FORMAT_MICRONAS_CELP833           = 0x0351, /* Micronas Semiconductors, Inc. */
		EXTERNAL_WAVE_FORMAT_UNKNOWN                    = 0x0000, /* Microsoft Corporation */
		EXTERNAL_WAVE_FORMAT_PCM                        = 0x0001, // Ayyyy lmao
		EXTERNAL_WAVE_FORMAT_ADPCM                      = 0x0002, /* Microsoft Corporation */
		EXTERNAL_WAVE_FORMAT_IEEE_FLOAT                 = 0x0003, /* Microsoft Corporation */
		EXTERNAL_WAVE_FORMAT_ALAW                       = 0x0006, /* Microsoft Corporation */
		EXTERNAL_WAVE_FORMAT_MULAW                      = 0x0007, /* Microsoft Corporation */
		EXTERNAL_WAVE_FORMAT_DTS                        = 0x0008, /* Microsoft Corporation */
		EXTERNAL_WAVE_FORMAT_DRM                        = 0x0009, /* Microsoft Corporation */
		EXTERNAL_WAVE_FORMAT_WMAVOICE9                  = 0x000A, /* Microsoft Corporation */
		EXTERNAL_WAVE_FORMAT_WMAVOICE10                 = 0x000B, /* Microsoft Corporation */
		EXTERNAL_WAVE_FORMAT_GSM610                     = 0x0031, /* Microsoft Corporation */
		EXTERNAL_WAVE_FORMAT_MSNAUDIO                   = 0x0032, /* Microsoft Corporation */
		EXTERNAL_WAVE_FORMAT_MSG723                     = 0x0042, /* Microsoft Corporation */
		EXTERNAL_WAVE_FORMAT_MPEG                       = 0x0050, /* Microsoft Corporation */
		EXTERNAL_WAVE_FORMAT_DSAT                       = 0x0066, /* Microsoft Corporation */
		EXTERNAL_WAVE_FORMAT_DSAT_DISPLAY               = 0x0067, /* Microsoft Corporation */
		EXTERNAL_WAVE_FORMAT_MSRT24                     = 0x0082, /* Microsoft Corporation */
		EXTERNAL_WAVE_FORMAT_MSAUDIO1                   = 0x0160, /* Microsoft Corporation */
		EXTERNAL_WAVE_FORMAT_WMAUDIO2                   = 0x0161, /* Microsoft Corporation */
		EXTERNAL_WAVE_FORMAT_WMAUDIO3                   = 0x0162, /* Microsoft Corporation */
		EXTERNAL_WAVE_FORMAT_WMAUDIO_LOSSLESS           = 0x0163, /* Microsoft Corporation */
		EXTERNAL_WAVE_FORMAT_WMASPDIF                   = 0x0164, /* Microsoft Corporation */
		EXTERNAL_WAVE_FORMAT_MPEG_ADTS_AAC              = 0x1600, /* Microsoft Corporation */
		EXTERNAL_WAVE_FORMAT_MPEG_RAW_AAC               = 0x1601, /* Microsoft Corporation */
		EXTERNAL_WAVE_FORMAT_MPEG_LOAS                  = 0x1602, /* Microsoft Corporation (MPEG-4 Audio Transport Streams (LOAS/LATM) */
		EXTERNAL_WAVE_FORMAT_NOKIA_MPEG_ADTS_AAC        = 0x1608, /* Microsoft Corporation */
		EXTERNAL_WAVE_FORMAT_NOKIA_MPEG_RAW_AAC         = 0x1609, /* Microsoft Corporation */
		EXTERNAL_WAVE_FORMAT_VODAFONE_MPEG_ADTS_AAC     = 0x160A, /* Microsoft Corporation */
		EXTERNAL_WAVE_FORMAT_VODAFONE_MPEG_RAW_AAC      = 0x160B, /* Microsoft Corporation */
		EXTERNAL_WAVE_FORMAT_MPEG_HEAAC                 = 0x1610, /* Microsoft Corporation
		(MPEG-2 AAC or MPEG-4 HE-AAC v1/v2 streams with any payload	(ADTS, ADIF, LOAS/LATM, RAW).
		Format block includes MP4 AudioSpecificConfig() -- see HEAACWAVEFORMAT below */
		EXTERNAL_WAVE_FORMAT_WM9_SPECTRUM_ANALYZER      = 0xA10B, /* Microsoft */
		EXTERNAL_WAVE_FORMAT_WMF_SPECTRUM_ANAYZER       = 0xA10C, /* Microsoft */
		EXTERNAL_WAVE_FORMAT_MVI_MVI2                   = 0x0084, /* Motion Pixels */
		EXTERNAL_WAVE_FORMAT_MULTITUDE_FT_SX20          = 0x008A, /* Multitude Inc. */
		
		EXTERNAL_WAVE_FORMAT_NMS_VBXADPCM               = 0x0038, /* Natural MicroSystems */
		EXTERNAL_WAVE_FORMAT_NEC_AAC                    = 0x00B0, /* NEC Corp. */
		EXTERNAL_WAVE_FORMAT_NICE_ACA                   = 0xA118, /* Nice Systems */
		EXTERNAL_WAVE_FORMAT_NICE_ADPCM                 = 0xA119, /* Nice Systems */
		EXTERNAL_WAVE_FORMAT_NICE_G728                  = 0xA122, /* Nice Systems */
		EXTERNAL_WAVE_FORMAT_NOKIA_ADAPTIVE_MULTIRATE   = 0x4201, /* Nokia */
		EXTERNAL_WAVE_FORMAT_NORCOM_VOICE_SYSTEMS_ADPCM = 0x0285, /* Norcom Electronics Corp. */
		EXTERNAL_WAVE_FORMAT_NORRIS                     = 0x1400, /* Norris Communications, Inc. */
		EXTERNAL_WAVE_FORMAT_NTCSOFT_ALF2CM_ACM         = 0x1FC4, /* NTCSoft */
		
		EXTERNAL_WAVE_FORMAT_OGG_VORBIS_MODE_1          = 0x674F, /* Ogg Vorbis */
		EXTERNAL_WAVE_FORMAT_OGG_VORBIS_MODE_2          = 0x6750, /* Ogg Vorbis */
		EXTERNAL_WAVE_FORMAT_OGG_VORBIS_MODE_3          = 0x6751, /* Ogg Vorbis */
		EXTERNAL_WAVE_FORMAT_OGG_VORBIS_MODE_1_PLUS     = 0x676F, /* Ogg Vorbis */
		EXTERNAL_WAVE_FORMAT_OGG_VORBIS_MODE_2_PLUS     = 0x6770, /* Ogg Vorbis */
		EXTERNAL_WAVE_FORMAT_OGG_VORBIS_MODE_3_PLUS     = 0x6771, /* Ogg Vorbis */
		EXTERNAL_WAVE_FORMAT_OKI_ADPCM                  = 0x0010, /* OKI */
		EXTERNAL_WAVE_FORMAT_ON2_VP7_AUDIO              = 0x0500, /* On2 Technologies */
		EXTERNAL_WAVE_FORMAT_ON2_VP6_AUDIO              = 0x0501, /* On2 Technologies */
		EXTERNAL_WAVE_FORMAT_ONLIVE                     = 0x0089, /* OnLive! Technologies, Inc. */
		EXTERNAL_WAVE_FORMAT_OPUS                       = 0x704F, /* Opus */

		EXTERNAL_WAVE_FORMAT_PHILIPS_LPCBB              = 0x0098, /* Philips Speech Processing */
		EXTERNAL_WAVE_FORMAT_PHILIPS_CELP               = 0x0120, /* Philips Speech Processing */
		EXTERNAL_WAVE_FORMAT_PHILIPS_GRUNDIG            = 0x0121, /* Philips Speech Processing */
		EXTERNAL_WAVE_FORMAT_POLYCOM_G722               = 0xA112, /* Polycom */
		EXTERNAL_WAVE_FORMAT_POLYCOM_G728               = 0xA113, /* Polycom */
		EXTERNAL_WAVE_FORMAT_POLYCOM_G729_A             = 0xA114, /* Polycom */
		EXTERNAL_WAVE_FORMAT_POLYCOM_SIREN              = 0xA115, /* Polycom */

		EXTERNAL_WAVE_FORMAT_QDESIGN_MUSIC              = 0x0450, /* QDesign Corporation */
		EXTERNAL_WAVE_FORMAT_QUALCOMM_PUREVOICE         = 0x0150, /* Qualcomm, Inc. */
		EXTERNAL_WAVE_FORMAT_QUALCOMM_HALFRATE          = 0x0151, /* Qualcomm, Inc. */
		EXTERNAL_WAVE_FORMAT_QUARTERDECK                = 0x0220, /* Quarterdeck Corporation */

		EXTERNAL_WAVE_FORMAT_RACAL_RECORDER_GSM         = 0x00A1, /* Racal recorders */
		EXTERNAL_WAVE_FORMAT_RACAL_RECORDER_G720_A      = 0x00A2, /* Racal recorders */
		EXTERNAL_WAVE_FORMAT_RACAL_RECORDER_G723_1      = 0x00A3, /* Racal recorders */
		EXTERNAL_WAVE_FORMAT_RACAL_RECORDER_TETRA_ACELP = 0x00A4, /* Racal recorders */
		EXTERNAL_WAVE_FORMAT_RADIOTIME_TIME_SHIFT_RADIO = 0xA117, /* RadioTime */
		EXTERNAL_WAVE_FORMAT_RAW_AAC1                   = 0x00FF, /* Raw AAC, with format block AudioSpecificConfig() (as defined by MPEG-4), that follows WAVEFORMATEX */
		EXTERNAL_WAVE_FORMAT_RHETOREX_ADPCM             = 0x0100, /* Rhetorex Inc. */
		EXTERNAL_WAVE_FORMAT_TUBGSM                     = 0x0155, /* Ring Zero Systems, Inc. */
		EXTERNAL_WAVE_FORMAT_ROCKWELL_ADPCM             = 0x003B, /* Rockwell International */
		EXTERNAL_WAVE_FORMAT_ROCKWELL_DIGITALK          = 0x003C, /* Rockwell International */
		
		EXTERNAL_WAVE_FORMAT_SANYO_LD_ADPCM             = 0x0125, /* Sanyo Electric Co., Ltd. */
		EXTERNAL_WAVE_FORMAT_SHARP_G726                 = 0x0045, /* Sharp */
		EXTERNAL_WAVE_FORMAT_SBC24                      = 0x0091, /* Siemens Business Communications Sys */
		EXTERNAL_WAVE_FORMAT_SIERRA_ADPCM               = 0x0013, /* Sierra Semiconductor Corp */
		EXTERNAL_WAVE_FORMAT_SIPROLAB_ACEPLNET          = 0x0130, /* Sipro Lab Telecom Inc. */
		EXTERNAL_WAVE_FORMAT_SIPROLAB_ACELP4800         = 0x0131, /* Sipro Lab Telecom Inc. */
		EXTERNAL_WAVE_FORMAT_SIPROLAB_ACELP8V3          = 0x0132, /* Sipro Lab Telecom Inc. */
		EXTERNAL_WAVE_FORMAT_SIPROLAB_G729              = 0x0133, /* Sipro Lab Telecom Inc. */
		EXTERNAL_WAVE_FORMAT_SIPROLAB_G729A             = 0x0134, /* Sipro Lab Telecom Inc. */
		EXTERNAL_WAVE_FORMAT_SIPROLAB_KELVIN            = 0x0135, /* Sipro Lab Telecom Inc. */
		EXTERNAL_WAVE_FORMAT_SOFTSOUND                  = 0x0080, /* Softsound, Ltd. */
		EXTERNAL_WAVE_FORMAT_DOLBY_AC3_SPDIF            = 0x0092, /* Sonic Foundry */
		EXTERNAL_WAVE_FORMAT_SONICFOUNDRY_LOSSLESS      = 0x1971, /* Sonic Foundry */
		EXTERNAL_WAVE_FORMAT_SONY_SCX                   = 0x0270, /* Sony Corp. */
		EXTERNAL_WAVE_FORMAT_SONY_SCY                   = 0x0271, /* Sony Corp. */
		EXTERNAL_WAVE_FORMAT_SONY_ATRAC3                = 0x0272, /* Sony Corp. */
		EXTERNAL_WAVE_FORMAT_SONY_SPC                   = 0x0273, /* Sony Corp. */
		EXTERNAL_WAVE_FORMAT_SONARC                     = 0x0021, /* Speech Compression */		
		EXTERNAL_WAVE_FORMAT_PACKED                     = 0x0099, /* Studer Professional Audio AG */
		EXTERNAL_WAVE_FORMAT_SYCOM_ACM_SYC008           = 0x0174, /* SyCom Technologies */
		EXTERNAL_WAVE_FORMAT_SYCOM_ACM_SYC701_G726L     = 0x0175, /* SyCom Technologies */
		EXTERNAL_WAVE_FORMAT_SYCOM_ACM_SYC701_CELP54    = 0x0176, /* SyCom Technologies */
		EXTERNAL_WAVE_FORMAT_SYCOM_ACM_SYC701_CELP68    = 0x0177, /* SyCom Technologies */
		EXTERNAL_WAVE_FORMAT_SYMBOL_G729_A              = 0xA103, /* Symbol Technologies */
		
		EXTERNAL_WAVE_FORMAT_TELUM_AUDIO                = 0x0280, /* Telum Inc. */
		EXTERNAL_WAVE_FORMAT_TELUM_IA_AUDIO             = 0x0281, /* Telum Inc. */

		EXTERNAL_WAVE_FORMAT_UHER_ADPCM                 = 0x0210, /* UHER informatic GmbH */
		EXTERNAL_WAVE_FORMAT_ULEAD_DV_AUDIO             = 0x0215, /* Ulead Systems, Inc. */
		EXTERNAL_WAVE_FORMAT_ULEAD_DV_AUDIO_1           = 0x0216, /* Ulead Systems, Inc. */
		EXTERNAL_WAVE_FORMAT_UNISYS_NAP_ADPCM           = 0x0170, /* Unisys Corp. */
		EXTERNAL_WAVE_FORMAT_UNISYS_NAP_ULAW            = 0x0171, /* Unisys Corp. */
		EXTERNAL_WAVE_FORMAT_UNISYS_NAP_ALAW            = 0x0172, /* Unisys Corp. */
		EXTERNAL_WAVE_FORMAT_UNISYS_NAP_16K             = 0x0173, /* Unisys Corp. */
		
		EXTERNAL_WAVE_FORMAT_VIANIX_MASC                = 0xA10A, /* Vianix LLC */
		EXTERNAL_WAVE_FORMAT_MEDIASPACE_ADPCM           = 0x0012, /* Videologic */
		EXTERNAL_WAVE_FORMAT_AUDIOFILE_AF36             = 0x0024, /* Virtual Music, Inc. */
		EXTERNAL_WAVE_FORMAT_AUDIOFILE_AF10             = 0x0026, /* Virtual Music, Inc. */		
		EXTERNAL_WAVE_FORMAT_VIVO_G723                  = 0x0111, /* Vivo Software */
		EXTERNAL_WAVE_FORMAT_VIVO_SIREN                 = 0x0112, /* Vivo Software */
		EXTERNAL_WAVE_FORMAT_VOCORD_G721                = 0xA11A, /* Vocord Telecom */
		EXTERNAL_WAVE_FORMAT_VOCORD_G726                = 0xA11B, /* Vocord Telecom */
		EXTERNAL_WAVE_FORMAT_VOCORD_G722_1              = 0xA11C, /* Vocord Telecom */
		EXTERNAL_WAVE_FORMAT_VOCORD_G728                = 0xA11D, /* Vocord Telecom */
		EXTERNAL_WAVE_FORMAT_VOCORD_G729                = 0xA11E, /* Vocord Telecom */
		EXTERNAL_WAVE_FORMAT_VOCORD_G729_A              = 0xA11F, /* Vocord Telecom */
		EXTERNAL_WAVE_FORMAT_VOCORD_G723_1              = 0xA120, /* Vocord Telecom */
		EXTERNAL_WAVE_FORMAT_VOCORD_LBC                 = 0xA121, /* Vocord Telecom */
		EXTERNAL_WAVE_FORMAT_VOICEAGE_AMR               = 0x0136, /* VoiceAge Corp. */
		EXTERNAL_WAVE_FORMAT_VOICEAGE_AMR_WB            = 0xA104, /* VoiceAge Corp. */
		EXTERNAL_WAVE_FORMAT_VOXWARE                    = 0x0062, /* Voxware Inc */
		EXTERNAL_WAVE_FORMAT_VOXWARE_BYTE_ALIGNED       = 0x0069, /* Voxware Inc */
		EXTERNAL_WAVE_FORMAT_VOXWARE_AC8                = 0x0070, /* Voxware Inc */
		EXTERNAL_WAVE_FORMAT_VOXWARE_AC10               = 0x0071, /* Voxware Inc */
		EXTERNAL_WAVE_FORMAT_VOXWARE_AC16               = 0x0072, /* Voxware Inc */
		EXTERNAL_WAVE_FORMAT_VOXWARE_AC20               = 0x0073, /* Voxware Inc */
		EXTERNAL_WAVE_FORMAT_VOXWARE_RT24               = 0x0074, /* Voxware Inc */
		EXTERNAL_WAVE_FORMAT_VOXWARE_RT29               = 0x0075, /* Voxware Inc */
		EXTERNAL_WAVE_FORMAT_VOXWARE_RT29HW             = 0x0076, /* Voxware Inc */
		EXTERNAL_WAVE_FORMAT_VOXWARE_VR12               = 0x0077, /* Voxware Inc */
		EXTERNAL_WAVE_FORMAT_VOXWARE_VR18               = 0x0078, /* Voxware Inc */
		EXTERNAL_WAVE_FORMAT_VOXWARE_TQ40               = 0x0079, /* Voxware Inc */
		EXTERNAL_WAVE_FORMAT_VOXWARE_SC3                = 0x007A, /* Voxware Inc */
		EXTERNAL_WAVE_FORMAT_VOXWARE_SC3_1              = 0x007B, /* Voxware Inc */
		EXTERNAL_WAVE_FORMAT_VOXWARE_TQ60               = 0x0081, /* Voxware Inc */
		EXTERNAL_WAVE_FORMAT_VOXWARE_RT24_SPEECH        = 0x181C, /* Voxware Inc. */

		EXTERNAL_WAVE_FORMAT_XEBEC                      = 0x003D, /* Xebec Multimedia Solutions Limited */
		EXTERNAL_WAVE_FORMAT_WAVPACK_AUDIO              = 0x5756, /* xiph.org */
		EXTERNAL_WAVE_FORMAT_SPEEX_VOICE                = 0xA109, /* xiph.org */

		EXTERNAL_WAVE_FORMAT_YAMAHA_ADPCM               = 0x0020, /* Yamaha Corporation of America */

		EXTERNAL_WAVE_FORMAT_ZOLL_ASAO                  = 0xA108, /* ZOLL Medical Corp. */
		EXTERNAL_WAVE_FORMAT_ZYXEL_ADPCM                = 0x0097, /* ZyXEL Communications, Inc. */

		EXTERNAL_WAVE_FORMAT_GENERIC_PASSTHRU           = 0x0249, //?????
		
		EXTERNAL_WAVE_FORMAT_DTS2                       = 0x2001, 
		EXTERNAL_WAVE_FORMAT_MAKEAVIS                   = 0x3313, 
		
		EXTERNAL_WAVE_FORMAT_FAAD_AAC                   = 0x706D,
		
		EXTERNAL_WAVE_FORMAT_GSM_610                    = 0xA10D,
		EXTERNAL_WAVE_FORMAT_GSM_620                    = 0xA10E, 
		EXTERNAL_WAVE_FORMAT_GSM_660                    = 0xA10F, 
		EXTERNAL_WAVE_FORMAT_GSM_690                    = 0xA110, 
		EXTERNAL_WAVE_FORMAT_GSM_ADAPTIVE_MULTIRATE_WB  = 0xA111, 
		
		

		WAVE_FORMAT_EXTENSIBLE = 0xFFFE,
		WAVE_FORMAT_DEVELOPMENT = UINT16_MAX
	};//note: from mmreg.h ([Pfiles]/x86/[Winkits]/[OS]/include/[Ver]/shared)

	enum External_SpeakerPositionMaskBits {
		EXTERNAL_SPEAKER_FRONT_LEFT            = 0x1,
		EXTERNAL_SPEAKER_FRONT_RIGHT           = 0x2,
		EXTERNAL_SPEAKER_FRONT_CENTER          = 0x4,
		EXTERNAL_SPEAKER_LOW_FREQUENCY         = 0x8,
		EXTERNAL_SPEAKER_BACK_LEFT             = 0x10,
		EXTERNAL_SPEAKER_BACK_RIGHT            = 0x20,
		EXTERNAL_SPEAKER_FRONT_LEFT_OF_CENTER  = 0x40,
		EXTERNAL_SPEAKER_FRONT_RIGHT_OF_CENTER = 0x80,
		EXTERNAL_SPEAKER_BACK_CENTER           = 0x100,
		EXTERNAL_SPEAKER_SIDE_LEFT             = 0x200,
		EXTERNAL_SPEAKER_SIDE_RIGHT            = 0x400,
		EXTERNAL_SPEAKER_TOP_CENTER            = 0x800,
		EXTERNAL_SPEAKER_TOP_FRONT_LEFT        = 0x1000,
		EXTERNAL_SPEAKER_TOP_FRONT_CENTER      = 0x2000,
		EXTERNAL_SPEAKER_TOP_FRONT_RIGHT       = 0x4000,
		EXTERNAL_SPEAKER_TOP_BACK_LEFT         = 0x8000,
		EXTERNAL_SPEAKER_TOP_BACK_CENTER       = 0x10000,
		EXTERNAL_SPEAKER_TOP_BACK_RIGHT        = 0x20000,
		
		// Bit mask locations reserved for future use
		SPEAKER_RESERVED              = 0x7FFC0000,

        // Used to specify that any possible permutation of speaker configurations
		SPEAKER_ALL                   = 0x80000000
	};//note: from mmreg.h ([Pfiles]/x86/[Winkits]/[OS]/include/[Ver]/shared)

	struct External_WaveSubformat {
		External_WaveFormats subFmtType;
		char constant[14]; //they make it with some sort of macro, idk
	};

	struct External_WaveFileData {
		bool isValidFile; //should always check before playing
		size_t amtRead; //helps with error detection

		char riffMark[5];             //"RIFF"
		uint32_t chunkSize;           //file size, 4 + n
		char waveMark[5];             //"WAVE"
									                 
		char fmtMark[5];              //"fmt "
		uint32_t fmtLength;           //following length, either 16(PCM only), 18, or 40
		External_WaveFormats fmtType; //uint16_t
		uint16_t numChannels;         //interleaved
		uint32_t sampleRate;          //blocks per second, Hz
		uint32_t byteRate;            //avg bytes per second (sampleRate * bitsPerSample * numChannels)/8
		uint16_t blockAlign;          //data block byte stride, divide by numChannels to get sample byte stride
		uint16_t bitDepth;            //bits per sample, divide by 8 for sample byte stride
									                 
		uint16_t extensionSize;       //either 0 or 22, checked if fmtLength > 16
		uint16_t validBitsPerSample;  //max of bitDepth, but could be less
		uint32_t speakerPosMask;      //use External_SpeakerPositionMaskBits
		External_WaveSubformat sfmt;  //GUID...?  Idk why this is here.
		/*
		from the doc:
		SubFormat (16 bytes):
		A GUID that specifies the audio format to use for recording data.
		This field MUST be set to KSDATAFORMAT_SUBTYPE_PCM
		({00000001-0000-0010-8000-00aa00389b71}).
		*/

		char dataMark[5];             //"data"
		uint32_t dataSize;            //in bytes
		std::vector<char> data;       //the good stuff
		//if dataSize is odd, there is a padding byte here,
		//represented by whether or not we'll delete a byte from data (resize to size - 1)
									           
		char factMark[5];             //only present for non-PCM data (but if it's extension data that's PCM it could be optionally present)
		uint32_t factChunkSize;       //guess it's just protocol
		uint32_t factNumSamplesPerChannel;//idk why not derive this but there you go (could totally leave this chunk out)

		/*
		other possible chunks: check out http://www.piclist.com/techref/io/serial/midi/wave.html
		We'll keep things simple for now though, I have a feeling we only care about the data in a wav file.
		We'll use our own format when we want to do "extra" things
		and maybe then we'll convert between extra chunks and our format as need be
		*/
	};//thanks to https://www.mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html

	struct External_PNGFileData {
		char ping[5] = { 'P',' ','N','G', '\0' };
	};
}