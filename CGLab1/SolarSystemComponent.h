#pragma once

#include "export.h"
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include "Game.h"

class SolarSystemComponent : public Game
{
private: 
	void Init();
	
	TriangleComponentParameters PrepareRect(float xOff, float yOff);
	TriangleComponentParameters PrepareCircle(float xOff, float yOff);
	DirectX::SimpleMath::Vector3 rotationAxis = DirectX::SimpleMath::Vector3::Zero;

	InputDevice inputDeviceInstance;
	Camera* cameraInstanceP;

public:
	float angle = 0.0f;

	SolarSystemComponent();
	
	void Run();
	void Update() override;

	TriangleComponent* Ball;
	TriangleComponent* Planet;
	TriangleComponent* Rat;
	TriangleComponent* floor;
	TriangleComponent* Cheems;

	TriangleComponent CreateSphere(float _radius, const wchar_t* _texturePath);
	TriangleComponent CreateSphere(float _radius, TriangleComponent* _parent, const wchar_t* _texturePath);
	TriangleComponent CreateFloor(const wchar_t* _texturePath, float scale);
	TriangleComponent CreateSkybox(const wchar_t* _texturePath);
	TriangleComponent CreateMesh(float _radius, const std::string& _modelPath, TriangleComponent* _parent, const wchar_t* _texturePath);
	void ReadNode(aiNode* node, const aiScene* scene, std::vector<TriangleComponentParameters::Vertex>* _points, std::vector<int>* _indeces);

	std::vector <TriangleComponent*> PlanetComponents;
};