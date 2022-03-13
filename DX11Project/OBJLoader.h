#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <iostream>

#include "Mesh.h"
#include "Model.h"
#include "Texture.h"

#include <DirectXMath.h>
namespace dx = DirectX;

namespace OBJLoader
{

	inline std::vector<std::string> Split(std::string str, char delimeter)
	{
		std::vector<std::string>  strs;
		size_t pos = str.find(delimeter);
		size_t initialPos = 0;
		strs.clear();

		while (pos != std::string::npos) {
			strs.push_back(str.substr(initialPos, pos - initialPos));
			initialPos = pos + 1;
			pos = str.find(delimeter, initialPos);
		}

		size_t min = (pos < str.size()) ? pos : str.size();
		strs.push_back(str.substr(initialPos, min - initialPos + 1));
		return strs;
	}

	inline std::unordered_map<std::string, Material*> LoadMaterialLibrary(std::string file, Shader* shader, DX11* dx11)
	{
		std::unordered_map<std::string, Material*> m_loadedmats;
		Material* cMat = nullptr;

		std::string matname = "";
		std::string matkey = "";

		std::fstream fs;
		fs.open(file);

		if (fs.is_open())
		{
			std::string line;
			std::vector<std::string> strs;

			while (std::getline(fs, line))
			{
				if (line[0] == '#')
					continue;

				strs = Split(line, ' ');

				if (strs[0] == "newmtl")
				{
					// spara den som redan är inläst om någon
					if (cMat != nullptr)
					{
						m_loadedmats.insert({ matname, cMat });
					}

					cMat = new Material(shader, dx11);
					matname = strs[1];
					std::cout << "Creating material " << matname << std::endl;
				}
				else if (strs[0] == "Ka")
				{
					dx::XMFLOAT4 ambientColor = dx::XMFLOAT4(
						std::stof(strs[1]),
						std::stof(strs[2]),
						std::stof(strs[3]),
						1.0f
					);
					cMat->properties.ambient = ambientColor;
				}
				else if (strs[0] == "Kd")
				{
					dx::XMFLOAT4 diffuseColor = dx::XMFLOAT4(
						std::stof(strs[1]),
						std::stof(strs[2]),
						std::stof(strs[3]),
						1.0f
					);
					cMat->properties.diffuse = diffuseColor;
				}
				else
				{
					std::cout << "unused: \"" << strs[0] << "\"" << std::endl;
				}
			}

			m_loadedmats.insert({ matname, cMat });
		}
		else
		{
			std::cout << "can't open " << file << std::endl;
		}

		return m_loadedmats;
	}

	inline dx::XMFLOAT3 m_calculateTangent(std::string path, VERTEX vertex1, VERTEX vertex2, VERTEX vertex3)
	{
		float vector1[3], vector2[3];
		float tuVector[2], tvVector[2];
		float den;
		float length;

		vector1[0] = vertex2.position.x - vertex1.position.x;
		vector1[1] = vertex2.position.y - vertex1.position.y;
		vector1[2] = vertex2.position.z - vertex1.position.z;

		vector2[0] = vertex3.position.x - vertex1.position.x;
		vector2[1] = vertex3.position.y - vertex1.position.y;
		vector2[2] = vertex3.position.z - vertex1.position.z;

		// Calculate the tu and tv texture space vectors.
		tuVector[0] = vertex2.uv.x - vertex1.uv.x;
		tvVector[0] = vertex2.uv.y - vertex1.uv.y;

		tuVector[1] = vertex3.uv.x - vertex1.uv.x;
		tvVector[1] = vertex3.uv.y - vertex1.uv.y;

		// Calculate the denominator of the tangent/binormal equation.
		den = 1.0f / (tuVector[0] * tvVector[1] - tuVector[1] * tvVector[0]);

		dx::XMFLOAT3 tangent;

		tangent.x = (tvVector[1] * vector1[0] - tvVector[0] * vector2[0]) * den;
		tangent.y = (tvVector[1] * vector1[1] - tvVector[0] * vector2[1]) * den;
		tangent.z = (tvVector[1] * vector1[2] - tvVector[0] * vector2[2]) * den;

		length = sqrt((tangent.x * tangent.x) + (tangent.y * tangent.y) + (tangent.z * tangent.z));
		tangent.x = tangent.x / length;
		tangent.y = tangent.y / length;
		tangent.z = tangent.z / length;

		return tangent;
	}

	inline Mesh CreateMesh(std::vector<VERTEX> vertexes, std::vector<unsigned int> indices, ID3D11Device* device)
	{
		Mesh mesh;
		mesh.vertexBuffer = nullptr;
		mesh.indexBuffer = nullptr;
		mesh.vertexes = vertexes;
		mesh.indicies = indices;

		D3D11_SUBRESOURCE_DATA subresourceData;
		D3D11_BUFFER_DESC bufferDescription;
		ZeroMemory(&bufferDescription, sizeof(D3D11_BUFFER_DESC));
		ZeroMemory(&subresourceData, sizeof(D3D11_SUBRESOURCE_DATA));

		if (vertexes.size() > 0)
		{
			// MIN MAX
			dx::XMVECTOR min = dx::XMLoadFloat3(&vertexes[0].position);
			dx::XMVECTOR max = dx::XMLoadFloat3(&vertexes[0].position);

			for (size_t i = 1; i < vertexes.size(); i++)
			{
				min = dx::XMVectorMin(min, dx::XMLoadFloat3(&vertexes[i].position));
				max = dx::XMVectorMax(max, dx::XMLoadFloat3(&vertexes[i].position));
			}

			dx::XMVECTOR center = dx::XMVectorLerp(min, max, 0.5f);
			mesh.radius = dx::XMVectorGetByIndex(dx::XMVector3Length(dx::XMVectorSubtract(max, center)), 0);

			// Skapar vertex buffer
			bufferDescription.Usage = D3D11_USAGE_DEFAULT;
			bufferDescription.ByteWidth = sizeof(VERTEX) * vertexes.size();
			bufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;

			subresourceData.pSysMem = vertexes.data();

			HRESULT vertexBufferResult = device->CreateBuffer(&bufferDescription, &subresourceData, &mesh.vertexBuffer);
			assert(SUCCEEDED(vertexBufferResult));
		}

		// skapar index buffer
		ZeroMemory(&bufferDescription, sizeof(D3D11_BUFFER_DESC));
		bufferDescription.Usage = D3D11_USAGE_DEFAULT;
		bufferDescription.ByteWidth = sizeof(unsigned int) * indices.size();
		//bufferDescription.BindFlags = D3D11_BIND_INDEX_BUFFER | D3D11_BIND_STREAM_OUTPUT;
		bufferDescription.BindFlags = D3D11_BIND_INDEX_BUFFER;


		ZeroMemory(&subresourceData, sizeof(D3D11_SUBRESOURCE_DATA));
		subresourceData.pSysMem = indices.data();

		HRESULT indexBufferResult = device->CreateBuffer(&bufferDescription, &subresourceData, &mesh.indexBuffer);
		assert(SUCCEEDED(indexBufferResult));
		return mesh;
	}

	inline Model* LoadOBJ(std::string path, Shader* shader, DX11* dx11)
	{
		std::unordered_map<std::string, Material*> m_loadedmats;
		std::string matlib = "";
		Model* model = nullptr;
		Material* mat = nullptr;

		std::fstream fs;
		fs.open(path);

		if (fs.is_open())
		{
			std::vector<VERTEX> vertexes;
			std::vector<unsigned int> indices;
			std::vector<dx::XMFLOAT2> uvs;
			std::vector<dx::XMFLOAT3> normals;

			std::string line;
			std::vector<std::string> strs;
			std::string key;

			while (getline(fs, line))
			{
				if (line[0] != '#' && line[0] != 'o' && line[0] != 's')
				{
					float x, y, z;
					strs = Split(line, ' ');
					key = strs[0];

					if (key == "vt")	// UV
					{
						uvs.push_back(dx::XMFLOAT2(
							std::stof(strs[1]),
							std::stof(strs[2])
						));
					}
					else if (key == "vn")	// NORMAL
					{
						normals.push_back(dx::XMFLOAT3(
							std::stof(strs[1]),
							std::stof(strs[2]),
							std::stof(strs[3])
						));
					}
					else if (key == "v")	// VERTEX
					{
						x = std::stof(strs[1]);
						y = std::stof(strs[2]);
						z = std::stof(strs[3]);

						VERTEX v;
						v.position = dx::XMFLOAT3(x, y, z);
						vertexes.push_back(v);
					}
					else if (key == "f")	//FACE
					{
						std::vector<std::string> subd;

						for (size_t i = 0; i < 3; i++)
						{
							subd = Split(strs[i + 1], '/');
							int j = std::stoi(subd[0]) - 1;
							indices.push_back(j);

							dx::XMVECTOR d = dx::XMVectorAdd(
								dx::XMLoadFloat3(&vertexes[j].normal),
								dx::XMLoadFloat3(&normals[std::stoi(subd[2]) - 1])
							);
							//d = dx::XMVector3Normalize(d);

							dx::XMStoreFloat3(&vertexes[j].normal, d);
							vertexes[j].uv = uvs[std::stoi(subd[1]) - 1];
						}
					}
					else if (strs[0] == "mtllib")
					{
						matlib = strs[0];

						std::cout << "loading matlib: " << strs[1] << std::endl;
						m_loadedmats = LoadMaterialLibrary("Models/" + strs[1], shader, dx11);
					}
					else if (strs[0] == "usemtl")
					{
						std::cout << "using mat: " << strs[1] << " with key: " << strs[1] << std::endl;
						mat = m_loadedmats[strs[1]];

						std::cout << "has a fucking value: " << (mat != nullptr ? "true" : "false") << std::endl;


					}

				}
			}

			// normalisera adderade normalerna
			for (size_t j = 0; j < vertexes.size(); j++)
			{
				dx::XMVECTOR n = dx::XMVector3Normalize(dx::XMLoadFloat3(&vertexes[j].normal));
				dx::XMStoreFloat3(&vertexes[j].normal, n);
			}

			dx::XMFLOAT3 tangent;
			size_t triangleCount = indices.size() / 3;

			for (size_t i = 0; i < triangleCount; i++)
			{
				tangent = m_calculateTangent(path, vertexes[indices[i * 3 + 0]], vertexes[indices[i * 3 + 1]], vertexes[indices[i * 3 + 2]]);
				vertexes[indices[i * 3 + 0]].tangent = tangent;
				vertexes[indices[i * 3 + 1]].tangent = tangent;
				vertexes[indices[i * 3 + 2]].tangent = tangent;
			}

			for (size_t j = 0; j < vertexes.size(); j++)
			{
				dx::XMVECTOR n = dx::XMVector3Normalize(dx::XMLoadFloat3(&vertexes[j].tangent));
				dx::XMStoreFloat3(&vertexes[j].tangent, n);
			}

			std::cout << "loaded " << path << "\tvertices: " << std::to_string(vertexes.size()) << std::endl << std::endl << std::endl;
			model = new Model(CreateMesh(vertexes, indices, dx11->device), mat);
		}
		else
		{
			std::cout << "Can't open " << path << std::endl;
		}

		return model;
	}
}