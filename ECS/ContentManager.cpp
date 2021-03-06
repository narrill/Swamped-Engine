#include "ContentManager.h"
#include <cstdarg>

using namespace std;

ContentManager::ContentManager()
{
}

ContentManager::~ContentManager()
{
	for (auto i = m_meshStores.begin(); i != m_meshStores.end(); i++) {
		Mesh& m = i->second.m_m;
		m.vertexBuffer->Release();
		m.indexBuffer->Release();
	}

	for (auto i = m_samplers.begin(); i != m_samplers.end(); i++)
	{
		if (i->second != nullptr)
			i->second->Release();
	}
	for (auto i = m_textures.begin(); i != m_textures.end(); i++)
	{
		if (i->second != nullptr)
			i->second->Release();
	}
	for (auto i = m_cubemaps.begin(); i != m_cubemaps.end(); i++)
	{
		if (i->second != nullptr)
			i->second->Release();
	}
	for (auto i = m_vshaders.begin(); i != m_vshaders.end(); i++)
	{
		if (i->second != nullptr)
			delete i->second;
	}
	for (auto i = m_pshaders.begin(); i != m_pshaders.end(); i++)
	{
		if (i->second != nullptr)
			delete i->second;
	}
	
	m_context->Release();
	m_device->Release();
}

void ContentManager::Init(ID3D11Device * device, ID3D11DeviceContext * context)
{
	device->AddRef();
	context->AddRef();

	m_device = device;
	m_context = context;

	m_materials = std::unordered_map<std::string, Material>();
	m_meshStores = std::unordered_map<std::string, MeshStore>();
	m_samplers = std::unordered_map<std::string, ID3D11SamplerState*>();
	m_vshaders = std::unordered_map<std::string, SimpleVertexShader*>();
	m_textures = std::unordered_map<std::string, ID3D11ShaderResourceView*>();
	m_cubemaps = std::unordered_map<std::string, ID3D11ShaderResourceView*>();
	
	std::vector<std::wstring> vshaderNames;
	std::vector<std::wstring> pshaderNames;
	std::vector<std::wstring> gshaderNames;
	std::vector<std::wstring> textures;
	std::vector<std::wstring>	cubemaps;
	std::vector<std::string> models;

	FindFilesInFolderWSTR(L"VertexShaders", vshaderNames);
	FindFilesInFolderWSTR(L"PixelShaders", pshaderNames);
	FindFilesInFolderWSTR(L"GeometryShaders", gshaderNames);
	FindFilesInFolderWSTR(L"assets/Textures", textures);
	FindFilesInFolderWSTR(L"assets/CubeMaps", cubemaps);
	FindFilesInFolder(L"assets/Models", models);

	CreateSamplers("sampler", D3D11_TEXTURE_ADDRESS_WRAP, D3D11_DEFAULT_MAX_ANISOTROPY);
	CreateSamplers("borderSampler", D3D11_TEXTURE_ADDRESS_BORDER,D3D11_DEFAULT_MAX_ANISOTROPY);
	CreateSamplers("fxaaSampler", D3D11_TEXTURE_ADDRESS_BORDER, 4);

	for (unsigned int i = 0; i < vshaderNames.size(); i++)
	{
		CreateVShader(vshaderNames[i]);
	}
	for (unsigned int i = 0; i < pshaderNames.size(); i++)
	{
		CreatePShader(pshaderNames[i]);
	}
	for (unsigned int i = 0; i < gshaderNames.size(); i++)
	{
		CreateGShader(gshaderNames[i]);
	}
	for (unsigned int i = 0; i < textures.size(); i++)
	{
		CreateTexture(textures[i]);
	}
	for (unsigned int i = 0; i < cubemaps.size(); i++)
	{
		CreateCubeMap(cubemaps[i]);
	}
	for (unsigned int i = 0; i < models.size(); i++)
	{
		CreateMeshStore(models[i]);
	}

	LoadMaterial("brickLightingNormalMap", "sampler", "vsLighting.cso", "psLighting.cso", "bricks.png", "bricksNM.png");
	LoadMaterial("BrightPixels", "sampler", "vsPostProcessing.cso", "psBrightPixels.cso", "null", "null");
	LoadMaterial("Blur", "sampler", "vsPostProcessing.cso", "psBlur.cso", "null", "null");
	LoadMaterial("Bloom", "sampler", "vsPostProcessing.cso", "psBloom.cso", "null", "null");
	LoadMaterial("Ground", "sampler", "vsLighting.cso", "psLighting.cso", "hi res dirt.png", "hi res dirt NM.png");
	LoadSkyBoxMaterial("skyMap", "sampler", "SkyVS.cso", "SkyPS.cso", "Ni.dds");
	LoadParticleMaterial("snowflake", "borderSampler", "ParticleVS.cso", "BillboardGS.cso", "ParticlePS.cso", "brightSpot.png");
}

Material ContentManager::LoadMaterial(std::string name, std::string samplerName, std::string vs, std::string ps, std::string textureName, std::string normalMapName)
{
	SimpleVertexShader* vshader = m_vshaders[vs];
	SimplePixelShader* pshader = m_pshaders[ps];
	ID3D11SamplerState*  sampler = m_samplers[samplerName];
	ID3D11ShaderResourceView* texture = (textureName != "null") ? m_textures[textureName] : nullptr;
	ID3D11ShaderResourceView * normalMap = (normalMapName != "null") ? m_textures[normalMapName] : nullptr;

	Material mat = { vshader, pshader, texture, normalMap, sampler };//new Material(vshader, pshader, texture, sampler);
	m_materials[name] = mat;
	return mat;
}

ParticleMaterial ContentManager::LoadParticleMaterial(std::string name, std::string samplerName, std::string vs, std::string gs, std::string ps, std::string textureName) {
	SimpleVertexShader* vshader = m_vshaders[vs];
	SimpleGeometryShader * gShader = m_gshaders[gs];
	SimplePixelShader* pshader = m_pshaders[ps];
	ID3D11SamplerState*  sampler = m_samplers[samplerName];
	ID3D11ShaderResourceView* texture = m_textures[textureName];

	ParticleMaterial pMat = { vshader, gShader, pshader, texture, sampler };
	m_particleMaterials[name] = pMat;
	return pMat;
}

SkyBoxMaterial ContentManager::LoadSkyBoxMaterial(std::string name, std::string samplerName, std::string vs, std::string ps, std::string textureName) {
	SimpleVertexShader* vshader = m_vshaders[vs];
	SimplePixelShader* pshader = m_pshaders[ps];
	ID3D11SamplerState*  sampler = m_samplers[samplerName];
	ID3D11ShaderResourceView* texture = m_cubemaps[textureName];

	SkyBoxMaterial sbMat = { vshader, pshader, texture, sampler };
	m_skyBoxMaterials[name] = sbMat;
	return sbMat;
}

MeshStore ContentManager::GetMeshStore(std::string mesh)
{
	return m_meshStores[mesh];
}

Material ContentManager::GetMaterial(std::string name)
{
	return m_materials[name];
}

ParticleMaterial ContentManager::GetParticleMaterial(std::string name) {
	return m_particleMaterials[name];
}

SkyBoxMaterial ContentManager::GetSkyBoxMaterial(std::string name) {
	return m_skyBoxMaterials[name];
}

SimpleGeometryShader* ContentManager::GetGShader(std::string name) {
	return m_gshaders[name];
}

SimpleVertexShader* ContentManager::GetVShader(std::string name) {
	return m_vshaders[name];
}

SimplePixelShader* ContentManager::GetPShader(std::string name) {
	return m_pshaders[name];
}

ID3D11SamplerState* ContentManager::GetSampler(std::string name) {
	return m_samplers[name];
}

//took from Chris Cascioli's code
void ContentManager::CalculateTangents(Vertex* verts, int numVerts, unsigned int* indices, int numIndices)
{
	// Reset tangents
	for (int i = 0; i < numVerts; i++)
	{
		verts[i].Tangent = DirectX::XMFLOAT3(0, 0, 0);
	}

	// Calculate tangents one whole triangle at a time
	for (int i = 0; i < numVerts;)
	{
		// Grab indices and vertices of first triangle
		unsigned int i1 = indices[i++];
		unsigned int i2 = indices[i++];
		unsigned int i3 = indices[i++];
		Vertex* v1 = &verts[i1];
		Vertex* v2 = &verts[i2];
		Vertex* v3 = &verts[i3];

		// Calculate vectors relative to triangle positions
		float x1 = v2->Position.x - v1->Position.x;
		float y1 = v2->Position.y - v1->Position.y;
		float z1 = v2->Position.z - v1->Position.z;

		float x2 = v3->Position.x - v1->Position.x;
		float y2 = v3->Position.y - v1->Position.y;
		float z2 = v3->Position.z - v1->Position.z;

		// Do the same for vectors relative to triangle uv's
		float s1 = v2->UV.x - v1->UV.x;
		float t1 = v2->UV.y - v1->UV.y;

		float s2 = v3->UV.x - v1->UV.x;
		float t2 = v3->UV.y - v1->UV.y;

		// Create vectors for tangent calculation
		float r = 1.0f / (s1 * t2 - s2 * t1);

		float tx = (t2 * x1 - t1 * x2) * r;
		float ty = (t2 * y1 - t1 * y2) * r;
		float tz = (t2 * z1 - t1 * z2) * r;

		// Adjust tangents of each vert of the triangle
		v1->Tangent.x += tx;
		v1->Tangent.y += ty;
		v1->Tangent.z += tz;

		v2->Tangent.x += tx;
		v2->Tangent.y += ty;
		v2->Tangent.z += tz;

		v3->Tangent.x += tx;
		v3->Tangent.y += ty;
		v3->Tangent.z += tz;
	}

	// Ensure all of the tangents are orthogonal to the normals
	for (int i = 0; i < numVerts; i++)
	{
		// Grab the two vectors
		DirectX::XMVECTOR normal = XMLoadFloat3(&verts[i].Normal);
		DirectX::XMVECTOR tangent = XMLoadFloat3(&verts[i].Tangent);

		//DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(tangent1, DirectX::XMVectorMultiply(normal1, DirectX::XMVector3Dot(normal1, tangent1))));

		// Use Gram-Schmidt orthogonalize
		tangent = DirectX::XMVector3Normalize(
			DirectX::XMVectorSubtract(tangent, DirectX::XMVectorMultiply(normal, DirectX::XMVector3Dot(normal, tangent))));

		// Store the tangent
		DirectX::XMStoreFloat3(&verts[i].Tangent, tangent);
	}
}

void ContentManager::CreateMeshStore(std::string objFile)
{
	std::string releasePath = "Assets/Models/";
	releasePath = releasePath + objFile;

	// File input object
	std::ifstream obj(releasePath.c_str());

	// Check for successful open
	if (!obj.is_open())
		return;

	// Variables used while reading the file
	std::vector<DirectX::XMFLOAT3> positions;     // Positions from the file
	std::vector<DirectX::XMFLOAT3> normals;       // Normals from the file
	std::vector<DirectX::XMFLOAT2> uvs;           // UVs from the file
	std::vector<Vertex> verts;           // Verts we're assembling
	std::vector<UINT> indices;           // Indices of these verts
	unsigned int vertCounter = 0;        // Count of vertices/indices
	char chars[100];                     // String for line reading
	
	
										 // Still good?
	while (obj.good())
	{
		// Get the line (100 characters should be more than enough)
		obj.getline(chars, 100);

		// Check the type of line
		if (chars[0] == 'v' && chars[1] == 'n')
		{
			// Read the 3 numbers directly into an XMFLOAT3
			DirectX::XMFLOAT3 norm;
			sscanf_s(
				chars,
				"vn %f %f %f",
				&norm.x, &norm.y, &norm.z);

			// Add to the list of normals
			normals.push_back(norm);
		}
		else if (chars[0] == 'v' && chars[1] == 't')
		{
			// Read the 2 numbers directly into an XMFLOAT2
			DirectX::XMFLOAT2 uv;
			sscanf_s(
				chars,
				"vt %f %f",
				&uv.x, &uv.y);

			// Add to the list of uv's
			uvs.push_back(uv);
		}
		else if (chars[0] == 'v')
		{
			// Read the 3 numbers directly into an XMFLOAT3
			DirectX::XMFLOAT3 pos;
			sscanf_s(
				chars,
				"v %f %f %f",
				&pos.x, &pos.y, &pos.z);
			
			// Add to the positions
			positions.push_back(pos);
		}
		else if (chars[0] == 'f')
		{
			// Read the face indices into an array
			unsigned int i[12];
			int facesRead = sscanf_s(
				chars,
				"f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d",
				&i[0], &i[1], &i[2],
				&i[3], &i[4], &i[5],
				&i[6], &i[7], &i[8],
				&i[9], &i[10], &i[11]);

			// - Create the verts by looking up
			//    corresponding data from vectors
			// - OBJ File indices are 1-based, so
			//    they need to be adusted
			Vertex v1;
			v1.Position = positions[i[0] - 1];
			v1.UV = uvs[i[1] - 1];
			v1.Normal = normals[i[2] - 1];

			Vertex v2;
			v2.Position = positions[i[3] - 1];
			v2.UV = uvs[i[4] - 1];
			v2.Normal = normals[i[5] - 1];

			Vertex v3;
			v3.Position = positions[i[6] - 1];
			v3.UV = uvs[i[7] - 1];
			v3.Normal = normals[i[8] - 1];

			// Flip the UV's since they're probably "upside down"
			v1.UV.y = 1.0f - v1.UV.y;
			v2.UV.y = 1.0f - v2.UV.y;
			v3.UV.y = 1.0f - v3.UV.y;

			// Add the verts to the vector
			verts.push_back(v1);
			verts.push_back(v2);
			verts.push_back(v3);

			// Add three more indices
			indices.push_back(vertCounter); vertCounter += 1;
			indices.push_back(vertCounter); vertCounter += 1;
			indices.push_back(vertCounter); vertCounter += 1;

			// Was there a 4th face?
			if (facesRead == 12)
			{
				// Make the last vertex
				Vertex v4;
				v4.Position = positions[i[9] - 1];
				v4.UV = uvs[i[10] - 1];
				v4.Normal = normals[i[11] - 1];

				// Flip the y
				v4.UV.y = 1.0f - v4.UV.y;

				// Add a whole triangle
				verts.push_back(v1);
				verts.push_back(v3);
				verts.push_back(v4);

				// Add three more indices
				indices.push_back(vertCounter); vertCounter += 1;
				indices.push_back(vertCounter); vertCounter += 1;
				indices.push_back(vertCounter); vertCounter += 1;
			}
		}
	}

	// Close the file and create the actual buffers
	obj.close();

	CalculateTangents(&verts[0], verts.size(), &indices[0], indices.size());

	//m_meshes[objFile] = new Mesh(&verts[0], &indices[0], verts.size(), indices.size(), m_device);
	size_t vertCount = verts.size();
	size_t indCount = indices.size();

	DirectX::XMVECTOR current;
	DirectX::XMVECTOR max = DirectX::XMLoadFloat3(&DirectX::XMFLOAT3(-FLT_MAX, -FLT_MAX, -FLT_MAX));
	DirectX::XMVECTOR min = DirectX::XMLoadFloat3(&DirectX::XMFLOAT3(FLT_MAX, FLT_MAX, FLT_MAX));
	for (unsigned int c = 0; c < vertCount; c++) {
		current = DirectX::XMLoadFloat3(&verts[c].Position);
		max = DirectX::XMVectorMax(max, DirectX::XMVectorScale(current,-1.0f));
		min = DirectX::XMVectorMin(min, DirectX::XMVectorScale(current,-1.0f));		
	}

	ID3D11Buffer * vertexBuffer = 0;
	ID3D11Buffer * indexBuffer = 0;
	DirectX::XMFLOAT3 maxFloat;
	DirectX::XMFLOAT3 minFloat;
	DirectX::XMStoreFloat3(&maxFloat, max);
	DirectX::XMStoreFloat3(&minFloat, min);
	DirectX::XMFLOAT3 bb[8] = {
		{maxFloat.x,maxFloat.y,maxFloat.z},
		{ maxFloat.x,maxFloat.y,minFloat.z },
		{ maxFloat.x,minFloat.y,maxFloat.z },
		{ maxFloat.x,minFloat.y,minFloat.z },
		{ minFloat.x,maxFloat.y,maxFloat.z },
		{ minFloat.x,maxFloat.y,minFloat.z },
		{ minFloat.x,minFloat.y,maxFloat.z },
		{ minFloat.x,minFloat.y,minFloat.z }
	};
	
	// Create the VERTEX BUFFER description
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex) * vertCount;       // 3 = number of vertices in the buffer
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER; // Tells DirectX this is a vertex buffer
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	// Create the proper struct to hold the initial vertex data
	D3D11_SUBRESOURCE_DATA initialVertexData;
	initialVertexData.pSysMem = &verts[0];

	// Actually create the buffer with the initial data
	m_device->CreateBuffer(&vbd, &initialVertexData, &vertexBuffer);

	// Create the INDEX BUFFER description
	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(unsigned int) * indCount;         // 3 = number of indices in the buffer
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER; // Tells DirectX this is an index buffer
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;

	// Create the proper struct to hold the initial index data
	D3D11_SUBRESOURCE_DATA initialIndexData;
	initialIndexData.pSysMem = &indices[0];

	// Actually create the buffer with the initial data
	m_device->CreateBuffer(&ibd, &initialIndexData, &indexBuffer);
	Mesh m = { vertexBuffer, indexBuffer, indCount };
	MeshStore ms = { m,{
			{
				{ maxFloat.x,maxFloat.y,maxFloat.z },
				{ maxFloat.x,maxFloat.y,-maxFloat.z },
				{ maxFloat.x,-maxFloat.y,maxFloat.z },
				{ maxFloat.x,-maxFloat.y,-maxFloat.z },
				{ -maxFloat.x,maxFloat.y,maxFloat.z },
				{ -maxFloat.x,maxFloat.y,-maxFloat.z },
				{ -maxFloat.x,-maxFloat.y,maxFloat.z },
				{ -maxFloat.x,-maxFloat.y,-maxFloat.z }
			} 
		}
	};
	m_meshStores[objFile] = ms;
}

void ContentManager::CreateSamplers(std::string name, D3D11_TEXTURE_ADDRESS_MODE addressMode, UINT maxAnisotropy)
{
	ID3D11SamplerState*  sampler;

	D3D11_SAMPLER_DESC samplerDes;
	memset(&samplerDes, 0, sizeof(D3D11_SAMPLER_DESC));
	//Address U, V, W
	samplerDes.AddressU = addressMode;
	samplerDes.AddressV = addressMode;
	samplerDes.AddressW = addressMode;
	//Filter
	samplerDes.Filter = D3D11_FILTER_ANISOTROPIC;	//trilinear filtering
															//MaxLOD
	samplerDes.MaxLOD = D3D11_FLOAT32_MAX;
	samplerDes.MaxAnisotropy = maxAnisotropy;

	HRESULT result = m_device->CreateSamplerState(&samplerDes, &sampler);
	if (result != S_OK)
		printf("ERROR: Failed to create Sampler State.");
	else
		m_samplers[name] = sampler;
}

void ContentManager::CreateTexture(std::wstring textureName)
{
	std::wstring path = L"Assets/Textures/";
	path = path + textureName;

	ID3D11ShaderResourceView* texture;

	HRESULT result = DirectX::CreateWICTextureFromFile(m_device, m_context, path.c_str(), 0, &texture);
	if (result != S_OK)
		printf("ERROR: Failed to Load Texture.");
	std::string name(textureName.begin(), textureName.end());
	m_textures[name] = texture;
}

void ContentManager::CreateCubeMap(std::wstring cubeName)
{
	std::wstring debugPath = L"assets/CubeMaps/";
	debugPath += cubeName;

	ID3D11ShaderResourceView* cubemap;

	DirectX::CreateDDSTextureFromFile(m_device, debugPath.c_str(), 0, &cubemap);

	std::string name(cubeName.begin(), cubeName.end());
	m_cubemaps[name] = cubemap;
}

void ContentManager::CreateVShader(std::wstring shader)
{
	std::wstring releasePath = L"VertexShaders/";
	releasePath = releasePath + shader;

	SimpleVertexShader* vertexShader = new SimpleVertexShader(m_device, m_context);
	if (!vertexShader->LoadShaderFile(releasePath.c_str()))
		vertexShader->LoadShaderFile(shader.c_str());

	std::string shaderString(shader.begin(), shader.end());
	m_vshaders[shaderString] = vertexShader;
}

void ContentManager::CreatePShader(std::wstring shader)
{
	std::wstring releasePath = L"PixelShaders/";
	releasePath = releasePath + shader;

	SimplePixelShader* pixelShader = new SimplePixelShader(m_device, m_context);
	if (!pixelShader->LoadShaderFile(releasePath.c_str()))
		pixelShader->LoadShaderFile(shader.c_str());

	std::string shaderString(shader.begin(), shader.end());
	m_pshaders[shaderString] = pixelShader;
}

void ContentManager::CreateGShader(std::wstring shader)
{
	std::wstring releasePath = L"GeometryShaders/";
	releasePath = releasePath + shader;

	SimpleGeometryShader* geometryShader = new SimpleGeometryShader(m_device, m_context, true, true);
	if (!geometryShader->LoadShaderFile(releasePath.c_str()))
		geometryShader->LoadShaderFile(shader.c_str());

	std::string shaderString(shader.begin(), shader.end());
	m_gshaders[shaderString] = geometryShader;
}

//I took the basic code from below and modified it to work for both UNICODE and non-UNICODE character sets
//Got it from: http://stackoverflow.com/questions/612097/how-can-i-get-the-list-of-files-in-a-directory-using-c-or-c
void ContentManager::FindFilesInFolder(std::wstring folder, std::vector<std::string>& listOfFiles)
{
#ifdef UNICODE
	std::wstring path = folder + L"/*.*";
	
	WIN32_FIND_DATA fd;
	//for the below to work they need to run with findfilefirstW not A
	HANDLE hFind = ::FindFirstFile(path.c_str(), &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			// read all (real) files in current folder
			// , delete '!' read other 2 default folder . and ..
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				std::wstring temp1(fd.cFileName);
				std::string temp2(temp1.begin(), temp1.end());
				listOfFiles.push_back(temp2);
			}
		} while (::FindNextFile(hFind, &fd));
		::FindClose(hFind);
	}

#else
	
	std::wstring path = folder + L"/*.*";
	std::string pathSTR(path.begin(), path.end());
	WIN32_FIND_DATA fd;

	HANDLE hFind = ::FindFirstFile(pathSTR.c_str(), &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			// read all (real) files in current folder
			// , delete '!' read other 2 default folder . and ..
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {

				std::string temp(fd.cFileName);
				listOfFiles.push_back(temp);
			}
		} while (::FindNextFile(hFind, &fd));
		::FindClose(hFind);
	}
#endif
}

//I took the basic code from below and modified it to work for both UNICODE and non-UNICODE character sets
//Got it from: http://stackoverflow.com/questions/612097/how-can-i-get-the-list-of-files-in-a-directory-using-c-or-c
void ContentManager::FindFilesInFolderWSTR(std::wstring folder, std::vector<std::wstring>& listOfFiles)
{
#ifdef UNICODE
	std::wstring path = folder + L"/*.*";
	WIN32_FIND_DATA fd;

	HANDLE hFind = ::FindFirstFile(path.c_str(), &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			// read all (real) files in current folder
			// , delete '!' read other 2 default folder . and ..
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				std::wstring temp(fd.cFileName);
				listOfFiles.push_back(temp);
			}
		} while (::FindNextFile(hFind, &fd));
		::FindClose(hFind);
	}

#else

	std::wstring path = folder + L"/*.*";
	std::string pathSTR(path.begin(), path.end());
	WIN32_FIND_DATA fd;

	HANDLE hFind = ::FindFirstFile(pathSTR.c_str(), &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			// read all (real) files in current folder
			// , delete '!' read other 2 default folder . and ..
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {

				std::string temp1(fd.cFileName);
				std::wstring temp2(temp1.begin(), temp1.end());
				listOfFiles.push_back(temp2);
			}
		} while (::FindNextFile(hFind, &fd));
		::FindClose(hFind);
	}
#endif
}
