#include "Mainframe.hpp"

/*
namespace std {
	template<>
	struct hash<SAGE::Vertex> {
		size_t operator()(SAGE::Vertex const& vert) const {
			size_t seed = 0;
			hashCombine(seed, vert.position, vert.color, vert.texturePosition);
			return seed;
		}
	};
}*/

namespace SAGE {

	VkExtent2D getWindowVkExtent(VideoSettings* vs) {
		SDL_Vulkan_GetDrawableSize(vs->mainWindow, &vs->windowWidth, &vs->windowHeight);
		return VkExtent2D{ static_cast<uint32_t>(vs->windowWidth), static_cast<uint32_t>(vs->windowHeight) };
	}

	bool isCompatible(Mesh* mesh, Material* mat) {
		return mat->shader->getVertexType() == mesh->vertexType;
	}

	Texture::Texture(VideoSettings* vs, TextureFileData& assetData, Asset* parent) {
		settings = vs;
		isPBR = true;

		UUID = 0;
		if (parent) {
			UUID = parent->UUID;
			listPosition = parent->listPosition;
			numReferences = parent->numReferences;
			delete parent;
		}

		for (size_t i = 0; i < assetData.maps.size(); i++) {
			VkFormat format = assetData.maps[i].channels == 1 ? VK_FORMAT_R8_UNORM : VK_FORMAT_R8G8B8A8_UNORM;
			if (i == DIFFUSE_TEXTURE_MAP || i == ALBEDO_TEXTURE_MAP) format = VK_FORMAT_R8G8B8A8_SRGB;
			samplers[i] = settings->videoCard->getSampler(assetData.maps[i].samplerName);
			images[i] = new VulkanTensor(settings->videoCard->getState(), format, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT,
				VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				VK_IMAGE_TYPE_2D, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, 1,
				static_cast<uint32_t>(assetData.maps[i].width), static_cast<uint32_t>(assetData.maps[i].height));
			VulkanStagingBuffer tStager(images[i], assetData.maps[i].imageData.data());

			settings->videoCard->prepareNewTensorAsStagingTarget(images[i]);
			settings->videoCard->stage(&tStager, images[i]);
			//now have to use the present command buffer to transition the layout to be shader-readable
			settings->videoCard->prepareStagingTargetAsTexture(images[i]);
		}
	}
	Texture::Texture(VideoSettings* vs, uint32_t w, uint32_t h) {
		settings = vs;
		isPBR = false;

		UUID = 0;

		for (VulkanTensor* p : images) {
			p = nullptr;
		}

		samplers[DIFFUSE_TEXTURE_MAP] = settings->videoCard->getSampler("text");
		images[DIFFUSE_TEXTURE_MAP] = new VulkanTensor(
			settings->videoCard->getState(),
			VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
			VK_SAMPLE_COUNT_1_BIT,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_IMAGE_TYPE_2D, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, 1,
			roundPow2(w), roundPow2(h)
		);
	}
	Texture::~Texture() {
		if (isPBR) {
			for (auto p : images) {
				delete p;
			}
		}
		else delete images[DIFFUSE_TEXTURE_MAP];
	}

	TextureLibrary::TextureLibrary(VideoSettings* vs) {
		settings = vs;

		TextureFileData temp{};
		for (size_t i = 0; i < temp.maps.size(); i++) {
			temp.maps[i].samplerName = "default";
			textureDefaultHandler(i, temp.maps[i]);
		}
		defaultTexture = new Texture(settings, temp);
	}
	TextureLibrary::~TextureLibrary() {
		delete defaultTexture;
		for (auto& kv : textures) {
			settings->assetSystem->returnAsset(kv.second);
		}
	}
	Texture* TextureLibrary::getTexture(uint32_t UUID) {
		UUIDinfo checker(UUID);
		if (checker.getAssetType() != SAGE_TEXTURE_ASSET) return defaultTexture;
		if (!textures.contains(UUID)) {
			TextureFileData rData{};
			Asset* loan = settings->assetSystem->getAsset(UUID,&rData);
			if(!loan) return defaultTexture;
			loan = new Texture(settings, rData, loan);
			textures[UUID] = (Texture*)loan;
		}
		Texture* toReturn = textures[UUID];
		toReturn->numReferences++;
		return toReturn;
	}
	void TextureLibrary::returnTexture(Texture* texture) {
		if (texture->UUID == 0) return;
		if (--texture->numReferences == 0) {
			textures.erase(texture->UUID);
			settings->assetSystem->returnAsset(texture);
		}
	}

	Cubemap::Cubemap(VideoSettings* vs, Image& assetData, Asset* parent) {
		settings = vs;

		if (parent) {
			UUID = parent->UUID;
			listPosition = parent->listPosition;
			numReferences = parent->numReferences;
			delete parent;
		}

		mapSampler = settings->videoCard->getSampler(assetData.samplerName);
		map = new VulkanTensor(settings->videoCard->getState(), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT,
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_TYPE_2D,
			VK_IMAGE_VIEW_TYPE_CUBE, VK_IMAGE_ASPECT_COLOR_BIT, 6,
			static_cast<uint32_t>(assetData.width), static_cast<uint32_t>(assetData.height), 1,
			VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);
		VulkanStagingBuffer cStager(map, assetData.imageData.data());

		settings->videoCard->prepareNewTensorAsStagingTarget(map);
		settings->videoCard->stage(&cStager, map);
		settings->videoCard->prepareStagingTargetAsTexture(map);
	}
	Cubemap::~Cubemap() {
		delete map;
	}

	CubemapLibrary::CubemapLibrary(VideoSettings* vs) {
		settings = vs;

		Image temp{};
		temp.samplerName = "default";
		temp.width = 1;
		temp.height = 1;
		temp.channels = 4;
		temp.imageDataSize = 4 * 6;
		temp.imageData = {
			UINT8_MAX, 0, 0, 0,
			0, UINT8_MAX, 0, 0,
			0, 0, UINT8_MAX, 0,
			UINT8_MAX, UINT8_MAX, 0, 0,
			UINT8_MAX, 0, UINT8_MAX, 0,
			0, UINT8_MAX, UINT8_MAX, 0
		};
		defaultCubemap = new Cubemap(settings, temp);
	}
	CubemapLibrary::~CubemapLibrary() {
		delete defaultCubemap;
		for (auto& kv : maps) {
			settings->assetSystem->returnAsset(kv.second);
		}
	}
	Cubemap* CubemapLibrary::getCubemap(uint32_t UUID) {
		UUIDinfo checker(UUID);
		if (checker.getAssetType() != SAGE_CUBEMAP_ASSET) return defaultCubemap;
		if (!maps.contains(UUID)) {
			Image rData{};
			Asset* loan = settings->assetSystem->getAsset(UUID, &rData);
			if (!loan) return defaultCubemap;
			loan = new Cubemap(settings, rData, loan);
			maps[UUID] = (Cubemap*)loan;
		}
		Cubemap* toReturn = maps[UUID];
		toReturn->numReferences++;
		return toReturn;
	}
	void CubemapLibrary::returnCubemap(Cubemap* map) {
		if (map == defaultCubemap)return;
		if (--map->numReferences == 0) {
			maps.erase(map->UUID);
			settings->assetSystem->returnAsset(map);
		}
	}

	Mesh::Mesh(VideoSettings* vs, MeshFileData& data, Asset* parent) {
		settings = vs;
		UUID = 0;

		if (parent) {
			UUID = parent->UUID;
			listPosition = parent->listPosition;
			numReferences = parent->numReferences;
			delete parent;
		}

		numVertices = data.vertexData.size();
		numIndices = data.indexData.size();

		vertices = new VulkanArray(settings->videoCard->getState(),
			data.vertexDataBlockSize,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		indices = new VulkanArray(settings->videoCard->getState(),
			data.indexDataBlockSize,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		VulkanStagingBuffer vStager(vertices, data.vertexData.data());
		VulkanStagingBuffer iStager(indices, data.indexData.data());

		settings->videoCard->stage(&vStager, vertices);
		settings->videoCard->stage(&iStager, indices);

		vertexType = SAGE_DEFAULT_VERTEX;
	}
	Mesh::Mesh(VideoSettings* vs, uint32_t pW, uint32_t pH) {
		settings = vs;
		UUID = 0;

		numVertices = 4;
		numIndices = 6;

		float normalPixW = (float)pW/settings->windowWidth * 2.0;
		float normalPixH = (float)pH/settings->windowHeight * 2.0;

		std::array<SpriteVertex, 4> verts = {
			glm::vec4(-1.f,-1.f,0.f,0.f),
			glm::vec4(normalPixW - 1.f, -1.f, 1.f, 0.f),
			glm::vec4(-1.f, normalPixH - 1.f, 0.f, 1.f),
			glm::vec4(normalPixW - 1.f, normalPixH - 1.f, 1.f, 1.f)
		};

		vertices = new VulkanArray(settings->videoCard->getState(), sizeof(SpriteVertex) * numVertices,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);//do we actually want sprites stored on the device or to be host-accesible?
		VulkanStagingBuffer vStager(vertices, verts.data());
		settings->videoCard->stage(&vStager, vertices);
		indices = settings->defaultQuadIndices;

		vertexType = SAGE_SPRITE_VERTEX;
	}
	Mesh::~Mesh() {
		delete vertices;
		if (vertexType == SAGE_DEFAULT_VERTEX) delete indices;
	}

	MeshLibrary::MeshLibrary(VideoSettings* vs) {
		settings = vs;

		MeshFileData temp{};
		glm::vec3 defNorm =  { 0.f,0.f,1.f };
		glm::vec3 defBit =   { 0.f,1.f,0.f };
		glm::vec3 defTan =   { 1.f,0.f,0.f };
		glm::vec4 defColor = { 1.f,1.f,1.f,1.f };
		temp.vertexData = {
			{ {-0.5f, -0.5f, 0.f}, defNorm, defTan, defBit, defColor, {0.f, 0.f} },//top left
			{ {0.5f, -0.5f, 0.f},  defNorm, defTan, defBit, defColor, {1.f, 0.f} },//top right
			{ {-0.5f, 0.5f, 0.f},  defNorm, defTan, defBit, defColor, {0.f, 1.f} },//bottom left
			{ {0.5f, 0.5f, 0.f},   defNorm, defTan, defBit, defColor, {1.f, 1.f} } //bottom right
		};
		temp.vertexDataBlockSize = sizeof(Vertex) * temp.vertexData.size();
		temp.indexData = { 0,1,3,3,2,0 };
		temp.indexDataBlockSize = sizeof(uint32_t) * temp.indexData.size();
		defaultMesh = new Mesh(settings, temp);
	}
	MeshLibrary::~MeshLibrary() {
		delete defaultMesh;
		for (auto& kv : meshes) {
			settings->assetSystem->returnAsset(kv.second);
		}
	}
	Mesh* MeshLibrary::getMesh(uint32_t UUID) {
		UUIDinfo checker(UUID);
		if (checker.getAssetType() != SAGE_MESH_ASSET) return defaultMesh;
		if (!meshes.contains(UUID)) {
			MeshFileData rData{};
			Asset* loan = settings->assetSystem->getAsset(UUID, &rData);
			if (!loan) return defaultMesh;
			loan = new Mesh(settings, rData, loan);
			meshes[UUID] = (Mesh*)loan;
		}
		Mesh* toReturn = meshes[UUID];
		toReturn->numReferences++;
		return toReturn;
	}
	void MeshLibrary::returnMesh(Mesh* mesh) {
		if (mesh->UUID == 0)return;
		if (--mesh->numReferences == 0) {
			meshes.erase(mesh->UUID);
			settings->assetSystem->returnAsset(mesh);
		}
	}

	Material::Material(VideoSettings* vs, MaterialFileData& data, Asset* parent) {
		settings = vs;

		if (parent) {
			UUID = parent->UUID;
			listPosition = parent->listPosition;
			numReferences = parent->numReferences;
			delete parent;
		}
		for (auto x : data.pbrBindingSlots) {
			texMaps.push_back((SAGETextureMapType)x);
		}
		shader = settings->videoCard->getStateManager()->getProgram(data.shaderId);
		pipelines = settings->videoCard->getStateManager()->getPipelines(UUID, shader->UUID, data.pipelineConfigNames);
		activePipeline = pipelines[0];
	}
	Material::~Material() {
		settings->videoCard->getStateManager()->returnProgram(shader);
	}
	
	MaterialLibrary::MaterialLibrary(VideoSettings* vs) {
		settings = vs;
		configurator = settings->videoCard->getStateManager();
		defaultMaterial = getMaterial(settings->assetSystem->getAssetUUID("material:default"));
	}
	MaterialLibrary::~MaterialLibrary() {
		for (auto& m : materials) {
			settings->assetSystem->returnAsset(m.second);
		}
	}
	Material* MaterialLibrary::getMaterial(uint32_t UUID) {
		UUIDinfo checker(UUID);
		if (checker.getAssetType() != SAGE_MATERIAL_ASSET) return defaultMaterial;
		if (!materials.contains(UUID)) {
			MaterialFileData rData{};
			Asset* loan = settings->assetSystem->getAsset(UUID, &rData);
			if (!loan)return defaultMaterial;
			loan = new Material(settings, rData, loan);
			materials[UUID] = (Material*)loan;
		}
		materials[UUID]->numReferences++;
		return materials[UUID];
	}
	void MaterialLibrary::returnMaterial(Material* mat) {
		if (mat == defaultMaterial) return;
		if (--mat->numReferences == 0) {
			materials.erase(mat->UUID);
			settings->assetSystem->returnAsset(mat);
		}
	}

	Camera::Camera(VideoSettings* vs) {
		settings = vs;

		entityID = settings->otherSystems->getEntitySystem()->makeEntity();
		settings->otherSystems->getEntitySystem()->addComponent(TRANSFORM_COMPONENT, entityID);
		settings->otherSystems->getEntitySystem()->addComponent(VISION_COMPONENT, entityID);
		settings->otherSystems->getEntitySystem()->addComponent(CONTROLLABILITY_COMPONENT, entityID);
		//will add vision component here
		transforms = settings->otherSystems->getEntitySystem()->getTransformComponent(entityID);
		viewData = settings->otherSystems->getEntitySystem()->getVisionComponent(entityID);
		control = settings->otherSystems->getEntitySystem()->getControllabilityComponent(entityID);
		transforms->size.x = (float)settings->windowWidth;//aspect ratio numerator
		transforms->size.y = (float)settings->windowHeight;//aspect ratio denomenator
		transforms->size.z = 0.1f;//nearplane
		transforms->size.w = 1000.f;//farplane

		std::function<void(InputSystem*)> oink = std::bind(&Camera::camCtrl, this, std::placeholders::_1);
		control->setUpdateFunction(oink);

		moveSpeed = 0.01f;
		rotSpeed = 0.0005f;
		zoomSpeed = 0.001f;
		fov = M_PI / 4;

		mouseX = 0.f;
		mouseY = 0.f;
	}
	Camera::~Camera() {
		settings->otherSystems->getEntitySystem()->returnEntity(entityID);
	}
	glm::vec3 Camera::getRight()   { return glm::normalize(glm::rotate(transforms->orientation, glm::vec3(1.f, 0.f, 0.f))); }
	glm::vec3 Camera::getUp()      { return glm::normalize(glm::rotate(transforms->orientation, glm::vec3(0.f, 1.f, 0.f))); }
	glm::vec3 Camera::getForward() { return glm::normalize(glm::rotate(transforms->orientation, glm::vec3(0.f, 0.f, -1.f))); }
	void Camera::moveUp()       { transforms->translate(getUp()      *  moveSpeed);}
	void Camera::moveDown()     { transforms->translate(getUp()      * -moveSpeed);}
	void Camera::moveRight()    { transforms->translate(getRight()   *  moveSpeed);}
	void Camera::moveLeft()     { transforms->translate(getRight()   * -moveSpeed);}
	void Camera::moveForward()  { transforms->translate(getForward() *  moveSpeed);}
	void Camera::moveBackward() { transforms->translate(getForward() * -moveSpeed);}
	void Camera::panUp()        { transforms->rotate( rotSpeed, glm::vec3(1.f,0.f,0.f)); }
	void Camera::panDown()      { transforms->rotate(-rotSpeed, glm::vec3(1.f,0.f,0.f)); }
	void Camera::panLeft()      { transforms->rotate( rotSpeed, glm::vec3(0.f,1.f,0.f)); }
	void Camera::panRight()     { transforms->rotate(-rotSpeed, glm::vec3(0.f,1.f,0.f)); }
	void Camera::rollRight()    { transforms->rotate( rotSpeed, glm::vec3(0.f,0.f,-1.f));}
	void Camera::rollLeft()     { transforms->rotate(-rotSpeed, glm::vec3(0.f,0.f,-1.f));}
	void Camera::zoomIn() {
		if (fov <= 0.0001f) return;
		fov -= zoomSpeed;
	}
	void Camera::zoomOut() {
		if (fov >= 3.14) return;
		fov += zoomSpeed;
	}
	void Camera::changeFarCutoff(float x) { transforms->size.w += x; }
	void Camera::changeNearCutoff(float x) { transforms->size.z += x; }
	void Camera::changeFrameWidth(float x) { transforms->size.x += x; }
	void Camera::changeFrameHeight(float x) { transforms->size.y += x; }
	void Camera::followMouse(float x, float y) {
		float xDiff = x - mouseX;
		float yDiff = y - mouseY;
		mouseX = x;
		mouseY = y;
		transforms->orientation = glm::rotate(transforms->orientation, xDiff, glm::vec3(1.f, 0.f, 0.f));
		transforms->orientation = glm::rotate(transforms->orientation, yDiff, glm::vec3(0.f, 1.f, 0.f));
		//def too simple but will have to get back to it when we actually end up using it.
	}
	void Camera::camCtrl(InputSystem* input) {
		if (input->isPressed(W_KEY)) moveForward();
		if (input->isPressed(A_KEY)) moveLeft();
		if (input->isPressed(S_KEY)) moveBackward();
		if (input->isPressed(D_KEY)) moveRight();
		if (input->isPressed(LSHIFT_KEY) || input->isPressed(SPACE_BAR)) moveUp();
		if (input->isPressed(LCTRL_KEY)) moveDown();

		if (input->isPressed(I_KEY)) panUp();
		if (input->isPressed(J_KEY)) panLeft();
		if (input->isPressed(K_KEY)) panDown();
		if (input->isPressed(L_KEY)) panRight();
		if (input->isPressed(RSHIFT_KEY)) rollLeft();
		if (input->isPressed(RCTRL_KEY))  rollRight();
	}
	void Camera::updateData() {
		viewData->data.view = glm::lookAt(
			glm::vec3(transforms->position),
			glm::vec3(transforms->position) + getForward(),
			getUp()
		);
		viewData->data.proj = glm::perspective(
			fov,
			transforms->size.x / transforms->size.y,
			transforms->size.z,
			transforms->size.w
		);
		viewData->data.proj[1][1] *= -1;//compensation for disparity between GL & VK
		viewData->data.projview = viewData->data.proj * viewData->data.view;
		viewData->data.forward = glm::vec4(getForward(), 0.0);
		viewData->data.right = glm::vec4(getRight(), 0.0);
		viewData->data.up = glm::vec4(getUp(), 0.0);
	}

	Light::Light(VideoSettings* vs) {
		settings = vs;
		entityID = settings->otherSystems->getEntitySystem()->makeEntity();
		settings->otherSystems->getEntitySystem()->addComponent(TRANSFORM_COMPONENT, entityID);
		settings->otherSystems->getEntitySystem()->addComponent(RENDERABILITY_COMPONENT, entityID);
		RenderabilityComponent* hand = settings->otherSystems->getEntitySystem()->getRenderabilityComponent(entityID);
		hand->changeMaterial(STRINGtoUUID("material:lite"));
		hand->changeMesh(0);
		hand->changeTexture(0);
	}
	Light::~Light() {
		settings->otherSystems->getEntitySystem()->returnEntity(entityID);
	}

	RenderabilityComponent::RenderabilityComponent(VideoSettings* vs) {
		settings = vs;
		material = settings->matLib->getMaterial(0);
		mesh = settings->meshLib->getMesh(0);
		texture = settings->texLib->getTexture(0);
	}
	RenderabilityComponent::~RenderabilityComponent() {
		if (texture) settings->texLib->returnTexture(texture);
		if (material) settings->matLib->returnMaterial(material);
		if (mesh) settings->meshLib->returnMesh(mesh);
	}
	void RenderabilityComponent::draw(VkCommandBuffer& buffer) {//this'll take some work
		//may be that per-component draws will only ever push through indices
		PushIndices pushy{};
		pushy.modelIndex = myComponentID;

		VkDeviceSize vertOffsets[] = { 0 };
		vkCmdBindVertexBuffers(buffer, 0, 1, &mesh->vertices->handle(), vertOffsets);
		vkCmdBindIndexBuffer(buffer, mesh->indices->handle(), 0, VK_INDEX_TYPE_UINT32);
		//turn into for loop iterating over the push constant data in the AP
		if (material->shader->hasPushConstants()) vkCmdPushConstants(buffer, material->shader->getPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushIndices), &pushy);
		//will have to figure out how to do push constant stuff from reflection
		vkCmdDrawIndexed(buffer, mesh->numIndices, 1, 0, 0, 0);
	}
	void RenderabilityComponent::updateModel() {
		TransformComponent* myTransform = settings->otherSystems->getEntitySystem()->getTransformComponent(parentEntityID);
		renderData.modelMatrix = glm::translate(glm::mat4(1.f),glm::vec3(myTransform->position));
		renderData.modelMatrix *= glm::toMat4(myTransform->orientation);
		renderData.modelMatrix = glm::scale(renderData.modelMatrix, glm::vec3(myTransform->size));
		renderData.normalMatrix = glm::transpose(glm::inverse(renderData.modelMatrix));
	}
	void RenderabilityComponent::changeMaterial(uint32_t UUID) {
		if (material) settings->matLib->returnMaterial(material);
		material = settings->matLib->getMaterial(UUID);
	}
	void RenderabilityComponent::changeMesh(uint32_t UUID) {
		if (mesh) settings->meshLib->returnMesh(mesh);
		mesh = settings->meshLib->getMesh(UUID);
	}
	void RenderabilityComponent::changeTexture(uint32_t UUID) {
		if (texture) settings->texLib->returnTexture(texture);
		texture = settings->texLib->getTexture(UUID);
	}

	RenderBatch::RenderBatch(VideoSettings* vs, RenderabilityComponent* basis) {
		settings = vs;
		commonMaterial = basis->material;
		descriptorHandler = settings->videoCard->getDescriptorSystem();
		descriptorHandler->registerProgram(commonMaterial->shader);
		objectListDescriptor = descriptorHandler->createDescriptorSet(commonMaterial->shader->getID(), 0)[0];
		descriptorHandler->connectBufferDescriptors({ objectListDescriptor });
		add(basis);
	}
	void RenderBatch::add(RenderabilityComponent* member) {
		if (member->material != commonMaterial) return;//should also report this but...low priority error
		memberList.push_back(member);

		std::vector<size_t> temp = descriptorHandler->createDescriptorSet(commonMaterial->shader->getID(), 2);
		//if the PBR set is ever not to be set #2 then we'll have to think of something else.
		//vCombine(memberTextureDecriptors, temp);//this is if we need every descriptor individually
		if (temp[0] != 0) memberTextureDecriptors.push_back(temp[0]);//because we only need to hold on to this to update the whole set
		else temp.clear();
		for (size_t i = 0; i < temp.size(); i++) {
			descriptorHandler->connectImageDescriptor(
				temp[i],
				member->texture->getImage(commonMaterial->texMaps[i])->sliceHandle(),
				member->texture->getSampler(commonMaterial->texMaps[i])
			);
		}

		memberVertices.push_back(&member->mesh->vertices->handle());
		memberVertexListOffsets.push_back(member->mesh->vertices->size());
	}
	void RenderBatch::remove(RenderabilityComponent* member) {
		size_t index = 0;//will need work when we actually get to removing members but...just a reminder for now :)
		for (index; index < memberList.size(); index++) {
			if (memberList[index] == member)break;
		}
		memberList.erase(memberList.begin() + index);
		memberVertices.erase(memberVertices.begin() + index);
		memberVertexListOffsets.erase(memberVertexListOffsets.begin() + index);
	}
	void RenderBatch::render(VkCommandBuffer& buffer, RenderFrameData& frameData) {
		if (commonMaterial->shader->getSetSize(1) > 0) {
			if (frameData.frameNumber >= frameRelevantDescriptors.size() / commonMaterial->shader->getSetSize(1)) {
				std::vector<size_t> frameSet = descriptorHandler->createDescriptorSet(commonMaterial->shader->getID(), 1);
				vCombine(frameRelevantDescriptors, frameSet);
				descriptorHandler->connectBufferDescriptors(frameSet);
			}
		}
		else {
			frameRelevantDescriptors.resize(1);
			frameRelevantDescriptors[0] = 0;
		}
		VkPipelineLayout pip = commonMaterial->shader->getPipelineLayout();
		size_t magiDex = commonMaterial->shader->getSetSize(1) * frameData.frameNumber;

		for (auto c : memberList) {
			c->updateModel();
			descriptorHandler->updateGlobalDescriptor(c->myComponentID, c->getData());
		}
		for (size_t i = 0; i < commonMaterial->shader->getSetSize(1); i++) {//must find out how to accomodate any set configuration?
			switch (i)
			{
			case 0:
				descriptorHandler->updateUBODescriptor(frameRelevantDescriptors[magiDex], &frameData.mainCamera->getData());
				break;
			case 1:
				descriptorHandler->updateUBODescriptor(frameRelevantDescriptors[magiDex + 1], &frameData.sceneData);
				break;
			default:
				std::cout << "not supported yet!";
				break;
			}
		}
		
		//should we have a pipeline abstraction containing the layout, createInfo, and handle?
		vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, commonMaterial->activePipeline);
		VkViewport dynamicViewport;//want dynamism to depend on the actual dynamism of the pipeline.
		VkRect2D dynamicScissor;
		dynamicScissor.offset = { 0,0 };
		dynamicScissor.extent = getWindowVkExtent(settings);
		dynamicViewport.x = 0.f;
		dynamicViewport.y = 0.f;
		dynamicViewport.width = settings->windowWidth;
		dynamicViewport.height = settings->windowHeight;
		dynamicViewport.minDepth = 0.f;
		dynamicViewport.maxDepth = 1.f;
		vkCmdSetViewport(buffer, 0, 1, &dynamicViewport);
		vkCmdSetScissor(buffer, 0, 1, &dynamicScissor);
		descriptorHandler->bindGlobalSet(objectListDescriptor, buffer, pip);
		descriptorHandler->bindPerFrameSet(frameRelevantDescriptors[magiDex], buffer, pip);

		for (size_t i = 0; i < memberList.size(); i++) {//want to change this to batch draw?  What of object-level descriptors then?
			if (memberTextureDecriptors.size() > i) descriptorHandler->bindPerDrawSet(memberTextureDecriptors[i], buffer, pip);
			memberList[i]->draw(buffer);
		}
	}

	Swapchain::Swapchain(VideoSettings* vs, Swapchain* old) {
		settings = vs;

		VkSwapchainCreateInfoKHR sci;
		sci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		sci.pNext = nullptr;
		sci.flags = 0;
		sci.surface = settings->videoCard->getState()->vulkanSurface;
		sci.minImageCount = settings->numFramesToFly;
		sci.imageFormat = settings->videoCard->getState()->chosenSurfaceFormat.format;
		sci.imageColorSpace = settings->videoCard->getState()->chosenSurfaceFormat.colorSpace;
		sci.imageExtent = getWindowVkExtent(settings);
		width = sci.imageExtent.width;
		height = sci.imageExtent.height;
		sci.imageArrayLayers = 1;
		sci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		sci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		sci.queueFamilyIndexCount = 0;
		sci.pQueueFamilyIndices = nullptr;
		sci.preTransform = settings->videoCard->getState()->vulkanSurfaceCapabilities.currentTransform;
		sci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		sci.presentMode = settings->videoCard->getState()->chosenPresentMode;
		sci.clipped = VK_TRUE;
		sci.oldSwapchain = old == nullptr ? VK_NULL_HANDLE : old->getVkSwapchain();
		vkCreateSwapchainKHR(*settings->videoCard->getDevice(), &sci, nullptr, &swapchain);
		delete old;


		uint32_t numImages;

		
		vkGetSwapchainImagesKHR(*settings->videoCard->getDevice(), swapchain, &numImages, nullptr);
		images.resize(numImages);
		vkGetSwapchainImagesKHR(*settings->videoCard->getDevice(), swapchain, &numImages, images.data());
		imageViews.resize(numImages);
		imageAttachmentDescriptions.resize(numImages);

		for (size_t i = 0; i < numImages; i++) {
			VkImageViewCreateInfo temp;

			temp.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			temp.pNext = nullptr;
			temp.flags = 0;
			temp.image = images[i];
			temp.viewType = VK_IMAGE_VIEW_TYPE_2D;
			temp.format = settings->videoCard->getState()->chosenSurfaceFormat.format;
			temp.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			temp.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			temp.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			temp.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			temp.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			temp.subresourceRange.baseMipLevel = 0;
			temp.subresourceRange.levelCount = 1;
			temp.subresourceRange.baseArrayLayer = 0;
			temp.subresourceRange.layerCount = 1;

			vkCreateImageView(*settings->videoCard->getDevice(), &temp, nullptr, &imageViews[i]);

			imageAttachmentDescriptions[i].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			imageAttachmentDescriptions[i].pNext = nullptr;
			imageAttachmentDescriptions[i].imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
			imageAttachmentDescriptions[i].resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;//probably?
			imageAttachmentDescriptions[i].resolveImageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;//check if this is okay
			imageAttachmentDescriptions[i].resolveImageView = imageViews[i];
			imageAttachmentDescriptions[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			imageAttachmentDescriptions[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			imageAttachmentDescriptions[i].clearValue = { 0.f,0.f,0.f,0.f };//be suspicious
		}

		VulkanTensor* msaaResource = new VulkanTensor(settings->videoCard->getState(),
			settings->videoCard->getState()->chosenSurfaceFormat.format,
			VK_IMAGE_TILING_OPTIMAL,
			settings->chosenSampleCount,
			VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_IMAGE_TYPE_2D, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, 1, width, height);
		for (auto& r : imageAttachmentDescriptions) {
			r.imageView = msaaResource->sliceHandle();
		}
		VulkanTensor* depthResource = new VulkanTensor(settings->videoCard->getState(),
			VK_FORMAT_D32_SFLOAT,
			VK_IMAGE_TILING_OPTIMAL,
			settings->chosenSampleCount,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_IMAGE_TYPE_2D, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_DEPTH_BIT, 1, width, height);

		renderingResources.push_back(msaaResource);
		renderingResources.push_back(depthResource);
		//consider adding stencil resource

		depthAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		depthAttachmentInfo.pNext = nullptr;
		depthAttachmentInfo.imageView = depthResource->sliceHandle();
		depthAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
		depthAttachmentInfo.resolveMode = VK_RESOLVE_MODE_NONE;//will ignore the next value
		depthAttachmentInfo.resolveImageView = VK_NULL_HANDLE;//ignored thanks to the above
		depthAttachmentInfo.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;//this ignored?
		depthAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachmentInfo.clearValue = { 1.f,0 };

	}
	Swapchain::~Swapchain() {
		for (VulkanTensor* r : renderingResources) {
			delete r;
		}
		for (VkImageView iv : imageViews) {
			vkDestroyImageView(*settings->videoCard->getDevice(), iv, nullptr);
		}
		vkDestroySwapchainKHR(*settings->videoCard->getDevice(), swapchain, nullptr);
	}
	//should we leave this preparation to the swapchain or move it to the accelerator with the other transition functions?
	void Swapchain::prepareImage(size_t index, bool toRender, VkCommandBuffer& buff) {
		VkImageMemoryBarrier2 tempBarrier;
		tempBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
		tempBarrier.pNext = nullptr;
		tempBarrier.image = images[index];
		tempBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		tempBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		tempBarrier.subresourceRange = { 0,0,1,0,1 };
		tempBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

		if (toRender) {
			tempBarrier.srcAccessMask = 0;
			tempBarrier.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;

			tempBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			tempBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			tempBarrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
			tempBarrier.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
		}
		else {
			tempBarrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
			tempBarrier.dstAccessMask = 0;

			tempBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			tempBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			tempBarrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
			tempBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		}

		VkDependencyInfo dinfo{};
		dinfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		dinfo.imageMemoryBarrierCount = 1;
		dinfo.pImageMemoryBarriers = &tempBarrier;

		vkCmdPipelineBarrier2(buff, &dinfo);

	}

	RenderSystem::RenderSystem(VideoSettings* vs, Camera* startingCam) {
		settings = vs;
		ds = settings->videoCard->getDescriptorSystem();
		////////////////////////////////////////////////////////////////////////
		pointThatsProbablyAHackDontSayIDidntWarnYou = new Light(settings);
		pointThatsProbablyAHackDontSayIDidntWarnYou->data.lightColor = glm::vec4(0.25f, 1.f, 0.25f, 1000.f);
		pointThatsProbablyAHackDontSayIDidntWarnYou->data.lightPos = glm::vec4(0.f,0.f,0.f,1.f);
		pointThatsProbablyAHackDontSayIDidntWarnYou->data.attenuationData = glm::vec4(0.f, 0.f, 1.f, 0.f);
		pointThatsProbablyAHackDontSayIDidntWarnYou->data.spotData = glm::vec4(0.f);
		lightingBatchNoteThisIsAHackDoNotKeep = new RenderBatch(
			settings,
			settings->otherSystems->getEntitySystem()->getRenderabilityComponent(
				pointThatsProbablyAHackDontSayIDidntWarnYou->entityID
			)
		);
		///////////////////////////////////////////////////////////////////////////
		presentingSwapchain = new Swapchain(settings);
		mainCamera = startingCam;
		skybox = settings->cbmLib->getCubemap(0);
		skyboxMat = settings->matLib->getMaterial(STRINGtoUUID("material:skybox"));
		ds->registerProgram(skyboxMat->shader);
		skyboxDescriptor = ds->createDescriptorSet(skyboxMat->shader->getID(), 2)[0];
		ds->connectImageDescriptor(skyboxDescriptor, skybox->getMap()->sliceHandle(), skybox->getSampler());

		frameData.resize(settings->numFramesToFly);

		VkFenceCreateInfo fci;
		fci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fci.pNext = nullptr;
		fci.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		VkSemaphoreCreateInfo sci;
		sci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		sci.pNext = nullptr;
		sci.flags = 0;

		for (size_t i = 0; i < frameData.size(); i++) {
			frameData[i].frameNumber = i;
			frameData[i].sceneData = {};
			frameData[i].mainCamera = mainCamera;
			vkCreateFence(*settings->videoCard->getDevice(), &fci, nullptr, &frameData[i].isFrameInFlight);
			vkCreateSemaphore(*settings->videoCard->getDevice(), &sci, nullptr, &frameData[i].isImageAvailable);
			vkCreateSemaphore(*settings->videoCard->getDevice(), &sci, nullptr, &frameData[i].isRenderingFinished);
			skyboxCamDescriptors.push_back(ds->createDescriptorSet(skyboxMat->shader->getID(), 1)[0]);
		}
		ds->connectBufferDescriptors(skyboxCamDescriptors);

		renderingFrame = 0;
		windowResize = false;
		suspended = false;
		windowCallbackForEventSystem = std::bind(&RenderSystem::handleWindowEvent, this, std::placeholders::_1);
		settings->otherSystems->data.eventSystem->subscribe(windowCallbackForEventSystem);
	}
	RenderSystem::~RenderSystem() {
		settings->matLib->returnMaterial(skyboxMat);
		settings->cbmLib->returnCubemap(skybox);
		for (auto& fd : frameData) {
			vkDestroyFence(*settings->videoCard->getDevice(), fd.isFrameInFlight, nullptr);
			vkDestroySemaphore(*settings->videoCard->getDevice(), fd.isImageAvailable, nullptr);
			vkDestroySemaphore(*settings->videoCard->getDevice(), fd.isRenderingFinished, nullptr);
		}
		///////////////////////////////////////////////////
		delete lightingBatchNoteThisIsAHackDoNotKeep;
		delete pointThatsProbablyAHackDontSayIDidntWarnYou;
		////////////////////////////////////////////////////
		delete presentingSwapchain;
	}
	void RenderSystem::rebuildSwapchain() {
		settings->videoCard->idle();
		presentingSwapchain = new Swapchain(settings, presentingSwapchain);
	}
	void RenderSystem::render(std::vector<RenderBatch>& scene) {
		if (suspended) return;
		SDL_SetWindowTitle(settings->mainWindow, settings->windowName.data());
		vkWaitForFences(*settings->videoCard->getDevice(), 1, &frameData[renderingFrame].isFrameInFlight, VK_TRUE, UINT64_MAX);

		uint32_t swapchainImageIndex;
		trackedResult = vkAcquireNextImageKHR
		(
			*settings->videoCard->getDevice(),
			presentingSwapchain->getVkSwapchain(),
			UINT64_MAX,
			frameData[renderingFrame].isImageAvailable,
			VK_NULL_HANDLE,
			&swapchainImageIndex
		);

		if (trackedResult == VK_ERROR_OUT_OF_DATE_KHR) {
			rebuildSwapchain();
			return;
		}

		vkResetFences(*settings->videoCard->getDevice(), 1, &frameData[renderingFrame].isFrameInFlight);
		mainCamera->updateData();

		settings->videoCard->getDescriptorSystem()->updateUBODescriptor(skyboxCamDescriptors[renderingFrame], &mainCamera->getData());
		glm::mat4 hackhock = frameData[renderingFrame].mainCamera->getData().view;
		
		TransformComponent* loight = 
			settings->otherSystems->getEntitySystem()->getTransformComponent(pointThatsProbablyAHackDontSayIDidntWarnYou->entityID);

		pointThatsProbablyAHackDontSayIDidntWarnYou->data.lightPos = hackhock * loight->position;

		frameData[renderingFrame].sceneData.ambientColor = { 1.f, 1.f, 1.f, .01f };
		frameData[renderingFrame].sceneData.fogColor = glm::vec4(glm::vec3(mainCamera->transforms->position), 512.f);//passing in camera position + shininess
		frameData[renderingFrame].sceneData.sceneLights[0].lightColor = glm::vec4(0.25f, 0.25f, 1.f, 1.f);//directional
		frameData[renderingFrame].sceneData.sceneLights[0].lightPos = hackhock * glm::vec4(-0.2f, -1.f, -0.3f, 0.f);
		frameData[renderingFrame].sceneData.sceneLights[0].spotData = glm::vec4(0.f);
		frameData[renderingFrame].sceneData.sceneLights[1] = pointThatsProbablyAHackDontSayIDidntWarnYou->data;
		frameData[renderingFrame].sceneData.sceneLights[2].lightColor = glm::vec4(1.f, 0.25f, 0.25f, 1.f);//spot
		frameData[renderingFrame].sceneData.sceneLights[2].lightPos = hackhock * mainCamera->transforms->position;
		frameData[renderingFrame].sceneData.sceneLights[2].attenuationData = glm::vec4(1.f, 0.007f, 0.00045f, glm::cos(glm::radians(12.5f)));
		frameData[renderingFrame].sceneData.sceneLights[2].spotData = glm::vec4(glm::normalize(glm::mat3(hackhock) * mainCamera->getForward()), glm::cos(glm::radians(17.5f)));
		
		std::vector<VkSemaphoreSubmitInfo> waits{}, signals{};

		VkSemaphoreSubmitInfo waitInfo, signalInfo;
		waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
		waitInfo.pNext = nullptr;
		waitInfo.semaphore = frameData[renderingFrame].isImageAvailable;
		waitInfo.value = 0;
		waitInfo.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
		waitInfo.deviceIndex = 0;
		waits.push_back(waitInfo);

		signalInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
		signalInfo.pNext = nullptr;
		signalInfo.semaphore = frameData[renderingFrame].isRenderingFinished;
		signalInfo.value = 0;
		signalInfo.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
		signalInfo.deviceIndex = 0;
		signals.push_back(signalInfo);

		VkCommandBuffer customCommandBuffer = settings->videoCard->getAndStartCustomCommandBuffer(PRESENT_FAMILY, renderingFrame);
		presentingSwapchain->prepareImage(swapchainImageIndex, true, customCommandBuffer);//transition to renderable image layout
		//Should the swapchain contain the dynamic thing?
		VkRenderingInfo rinfo;
		rinfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		rinfo.pNext = nullptr;
		rinfo.flags = 0;
		rinfo.renderArea.offset = { 0,0 };
		rinfo.renderArea.extent = presentingSwapchain->dimensions();
		rinfo.layerCount = 1;
		rinfo.viewMask = 0;
		rinfo.colorAttachmentCount = 1;
		rinfo.pColorAttachments = &presentingSwapchain->getImageAttachmentInfo(swapchainImageIndex);
		rinfo.pDepthAttachment = &presentingSwapchain->getDepthAttachmentInfo();
		rinfo.pStencilAttachment = nullptr;//maybe?
		vkCmdBeginRendering(customCommandBuffer, &rinfo);

		lightingBatchNoteThisIsAHackDoNotKeep->render(customCommandBuffer, frameData[renderingFrame]);

		//and this is where my shadow pass would go.  IF I HAD ONE!

		for (RenderBatch& rc : scene) {
			rc.render(customCommandBuffer, frameData[renderingFrame]);
		}

		vkCmdBindPipeline(customCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, skyboxMat->activePipeline);
		VkViewport dynamicViewport;
		VkRect2D dynamicScissor;
		dynamicScissor.offset = { 0,0 };
		dynamicScissor.extent = getWindowVkExtent(settings);
		dynamicViewport.x = 0.f;
		dynamicViewport.y = 0.f;
		dynamicViewport.width = settings->windowWidth;
		dynamicViewport.height = settings->windowHeight;
		dynamicViewport.minDepth = 0.f;
		dynamicViewport.maxDepth = 1.f;
		vkCmdSetViewport(customCommandBuffer, 0, 1, &dynamicViewport);
		vkCmdSetScissor(customCommandBuffer, 0, 1, &dynamicScissor);
		ds->bindPerFrameSet(skyboxCamDescriptors[renderingFrame], customCommandBuffer, skyboxMat->shader->getPipelineLayout());
		ds->bindPerDrawSet(skyboxDescriptor, customCommandBuffer, skyboxMat->shader->getPipelineLayout());
		vkCmdDraw(customCommandBuffer, 6, 1, 0, 0);

		vkCmdEndRendering(customCommandBuffer);
		presentingSwapchain->prepareImage(swapchainImageIndex, false, customCommandBuffer);//transition to presentable image layout

		VkQueue customQueue = settings->videoCard->endAndSubmitCustomCommandBuffer(PRESENT_FAMILY, renderingFrame, frameData[renderingFrame].isFrameInFlight, waits, signals);

		VkSwapchainKHR chains[] = { presentingSwapchain->getVkSwapchain() };
		VkPresentInfoKHR pInfo;
		pInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		pInfo.pNext = nullptr;
		pInfo.waitSemaphoreCount = 1;
		pInfo.pWaitSemaphores = &signalInfo.semaphore;
		pInfo.swapchainCount = 1;
		pInfo.pSwapchains = chains;
		pInfo.pImageIndices = &swapchainImageIndex;
		pInfo.pResults = nullptr;

		trackedResult = vkQueuePresentKHR(customQueue, &pInfo);

		if (trackedResult == VK_ERROR_OUT_OF_DATE_KHR || trackedResult == VK_SUBOPTIMAL_KHR || windowResize) {
			rebuildSwapchain();
			windowResize = false;
		}

		SDL_UpdateWindowSurface(settings->mainWindow);
		++renderingFrame %= settings->numFramesToFly;
	}
	void RenderSystem::handleWindowEvent(SDL_WindowEvent evt) {
		switch (evt.event) {
		case SDL_WINDOWEVENT_MINIMIZED:
			//suspended = true;
		case SDL_WINDOWEVENT_RESIZED:
			SDL_Vulkan_GetDrawableSize(settings->mainWindow, &settings->windowWidth, &settings->windowHeight);
			windowResize = true;
			suspended = settings->windowWidth == 0 || settings->windowHeight == 0;
			break;
		case SDL_WINDOWEVENT_RESTORED:
			suspended = false;
			break;
		case SDL_WINDOWEVENT_CLOSE:
			suspended = true;
			break;
		}
	}
	void RenderSystem::changeSkybox(uint32_t UUID) {
		settings->cbmLib->returnCubemap(skybox);
		skybox = settings->cbmLib->getCubemap(UUID);
		ds->connectImageDescriptor(skyboxDescriptor, skybox->getMap()->sliceHandle(), skybox->getSampler());
	}

	VideoManager::VideoManager() {
		vs.assetSystem = AssetManager::getManager();

		SDL_Init(SDL_INIT_VIDEO);

		//SDL_Vulkan_LoadLibrary(NULL);
		//auto bots = SDL_Vulkan_GetVkGetInstanceProcAddr();
		vs.windowName = "Test";
		vs.mainWindow = SDL_CreateWindow(vs.windowName.data(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, vs.windowWidth, vs.windowHeight, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

		vulkanImplementation = new Accelerator(vs.mainWindow);
		vs.videoCard = vulkanImplementation;
		vs.chosenSampleCount = hardwareInfo.graphicsCardInfo.maxMSAA;
		vs.numFramesToFly = hardwareInfo.graphicsCardInfo.numFrames;

		vs.meshLib = new MeshLibrary(&vs);
		vs.defaultQuadIndices = vs.meshLib->defaultMesh->indices;
		vs.texLib = new TextureLibrary(&vs);
		vs.cbmLib = new CubemapLibrary(&vs);
		vs.matLib = new MaterialLibrary(&vs);
	}
	VideoManager::~VideoManager() {
		delete vs.matLib;
		delete vs.texLib;
		delete vs.cbmLib;
		delete vs.meshLib;
		vs.assetSystem->release();
		delete vulkanImplementation;
		//SDL_Vulkan_UnloadLibrary();
		SDL_DestroyWindow(vs.mainWindow);
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
	}
}