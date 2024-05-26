#include "SolarSystemComponent.h"
#include <random>

SolarSystemComponent::SolarSystemComponent() 
{
	Init();
}

void SolarSystemComponent::Init()
{
	Ball = new TriangleComponent(CreateSphere(1.0, L"../Textures/test.dds"));

	/*Components.push_back(new TriangleComponent(CreateMesh(1.0f, "../Models/Rat2.obj", Ball, L"../Textures/test.dds")));
	Components.push_back(new TriangleComponent(CreateMesh(1.0f, "../Models/Cheems.obj", Ball, L"../Textures/Cheems.dds")));
	Components.push_back(new TriangleComponent(CreateMesh(1.0f, "../Models/Livesey.obj", Ball, L"../Textures/Livesey.dds")));
	Components.push_back(new TriangleComponent(CreateMesh(1.0f, "../Models/Skull.obj", Ball, L"../Textures/Livesey.dds")));
	Components.push_back(new TriangleComponent(CreateMesh(1.0f, "../Models/Rom.obj", Ball, L"../Textures/Livesey.dds")));*/
	Components.push_back(Ball);

	std::random_device rd;
	std::mt19937 rng(rd());
	std::uniform_int_distribution<int> ang_uni(0, 314);
	std::uniform_int_distribution<int> rad_uni(3, 20);

	for (int i = 0; i < Components.size(); i++)
	{
		/*int radius = rad_uni(rng);
		float angle = (float)ang_uni(rng);
		Components[i]->SetPos({ radius * 1.0f, radius * 1.0f / 1.5f, radius * 1.0f - 10 });
		Components[i]->SetRot({ -(1.57), angle, 0.0});*/
		Components[i]->SetPos({ 2.0f * i , 10.0f * (Components.size() - i) / Components.size(), 1.0f * i * (float)pow(-1.0f, i) });
	}

	floor = new TriangleComponent(CreateFloor(L"../Textures/ground.dds", 1.0f));
	floor->pos = { 0.0, 0.0, 0.0 };
	Components.push_back(floor);

	floor = new TriangleComponent(CreateFloor(L"../Textures/ground.dds", 0.1f));
	floor->pos = { 25.0f, 20.0f, 0 };
	Components.push_back(floor);

	//Components.push_back(Ball);
	Ball->pos = { 5.0f, -5.0f, 0.0f };

	auto cameraInstance = new Camera(&Ball->pos);
	cameraInstance->Initialize(
		DirectX::SimpleMath::Vector3(0.0f, 5.0f, 0.0f),
		(1.57 / 2),
		(1.57 / 2),
		display.getScreenWidth(),
		display.getScreenHeight(),
		&inputDevice
	);

	camera.push_back(cameraInstance);

	inputDeviceInstance.Init(display.getHWND());

	cameraInstanceP = cameraInstance;

}

void SolarSystemComponent::Run() 
{
	Game::Run();
}

void SolarSystemComponent::Update() 
{
	Game::camera.at(0)->Update(
		Game::deltaTime, 
		Game::display.getScreenWidth(), 
		Game::display.getScreenHeight()
	);

	angle += 0.1f;
	
	for (int i = 0; i < Components.size(); i++) Components[i]->Update(Game::context, Game::camera.at(0));

	DirectX::SimpleMath::Vector3 deltaPos = DirectX::SimpleMath::Vector3::Zero;

	deltaPos.Normalize();
	deltaPos /= 20;

	Ball->pos += deltaPos;

	float distance = deltaPos.Length();
	float ang = distance;
	if (distance) 
	{
		DirectX::SimpleMath::Vector3 deltaRotationAxis = DirectX::SimpleMath::Vector3(0.0f, 0.0f, 1.0f).Cross(deltaPos);
		deltaRotationAxis.Normalize();

		Ball->rotate *= DirectX::SimpleMath::Quaternion::CreateFromAxisAngle(deltaRotationAxis, ang);
	}
}


TriangleComponent SolarSystemComponent::CreateSphere(float radius, TriangleComponent* _parent, const wchar_t* _texturePath)
{
	TriangleComponentParameters sphere;
	int parallels = 50;
	int meridians = 50;
	
	sphere.numPoints = parallels * meridians * 4 - 1 * 2 * meridians;
	TriangleComponentParameters::Vertex* vertices = new TriangleComponentParameters::Vertex[sphere.numPoints];
	
	sphere.numIndeces = parallels * meridians * 6 - 3 * 2 * meridians;
	sphere.numPoints *= 2;
	sphere.points = new TriangleComponentParameters::Vertex[sphere.numPoints];
	sphere.indeces = new int[sphere.numIndeces];
	
	int tempPos = 0;
	int tempInd = 0;
	
	TriangleComponentParameters::Vertex vertex1, vertex2, vertex3, vertex4;
	
	for (int i = 0; i < parallels; i++)
	{
		float teta1 = ((float)(i) / parallels) * PI;
		float teta2 = ((float)(i + 1) / parallels) * PI;
		
		for (int j = 0; j < meridians; j++)
		{
			float fi1 = ((float)(j) / meridians) * 2 * PI;
			float fi2 = ((float)(j + 1) / meridians) * 2 * PI;

			float x;
			float y;
			float z;
			
			x = radius * sin(teta1) * cos(fi1);
			y = radius * sin(teta1) * sin(fi1);
			z = radius * cos(teta1);
			vertex1.position = DirectX::SimpleMath::Vector4(x, y, z, 1.0f);
			vertex1.uv.x = teta1 / PI;
			vertex1.uv.y = fi1 / (2 * PI);
			
			x = radius * sin(teta1) * cos(fi2);
			y = radius * sin(teta1) * sin(fi2);
			z = radius * cos(teta1);
			vertex2.position = DirectX::SimpleMath::Vector4(x, y, z, 1.0f);
			vertex2.uv.x = teta1 / PI;
			vertex2.uv.y = fi2 / (2 * PI);

			x = radius * sin(teta2) * cos(fi2);
			y = radius * sin(teta2) * sin(fi2);
			z = radius * cos(teta2);
			vertex3.position = DirectX::SimpleMath::Vector4(x, y, z, 1.0f);
			vertex3.uv.x = teta2 / PI;
			vertex3.uv.y = fi2 / (2 * PI);

			x = radius * sin(teta2) * cos(fi1);
			y = radius * sin(teta2) * sin(fi1);
			z = radius * cos(teta2);
			vertex4.position = DirectX::SimpleMath::Vector4(x, y, z, 1.0f);
			vertex4.uv.x = teta2 / PI;
			vertex4.uv.y = fi1 / (2 * PI);

			if (i == 0)
			{
				vertices[tempPos] = vertex2;
				sphere.indeces[tempInd] = tempPos;
				tempPos++;
				tempInd++;
				vertices[tempPos] = vertex3;
				sphere.indeces[tempInd] = tempPos;
				tempPos++;
				tempInd++;
				vertices[tempPos] = vertex4;
				sphere.indeces[tempInd] = tempPos;
				tempPos++;
				tempInd++;
			}
			else if (i == parallels - 1)
			{
				vertices[tempPos] = vertex4;
				sphere.indeces[tempInd] = tempPos;
				tempPos++;
				tempInd++;
				vertices[tempPos] = vertex1;
				sphere.indeces[tempInd] = tempPos;
				tempPos++;
				tempInd++;
				vertices[tempPos] = vertex2;
				sphere.indeces[tempInd] = tempPos;
				tempPos++;
				tempInd++;
			}
			else
			{
				vertices[tempPos] = vertex1;
				tempPos++;
				vertices[tempPos] = vertex2;
				tempPos++;
				vertices[tempPos] = vertex3;
				tempPos++;
				vertices[tempPos] = vertex4;
				tempPos++;

				sphere.indeces[tempInd++] = tempPos - 1;
				sphere.indeces[tempInd++] = tempPos - 4;
				sphere.indeces[tempInd++] = tempPos - 3;
				sphere.indeces[tempInd++] = tempPos - 3;
				sphere.indeces[tempInd++] = tempPos - 2;
				sphere.indeces[tempInd++] = tempPos - 1;
			}
		}
	}

	for (int temp = 0; temp < sphere.numPoints; temp++)
	{
		sphere.points[temp] = vertices[temp];
	}
	
	TriangleComponent Sphere1(sphere, _texturePath);

	//Sphere1.orbit = _orbit;
	Sphere1.parent = _parent;
	
	return Sphere1;

}

TriangleComponent SolarSystemComponent::CreateSphere(float radius, const wchar_t* _texturePath)
{
	TriangleComponentParameters sphere;
	int parallels = 50;
	int meridians = 50;

	sphere.numPoints = parallels * meridians * 4 - 1 * 2 * meridians;
	TriangleComponentParameters::Vertex* vertices = new TriangleComponentParameters::Vertex[sphere.numPoints];

	sphere.numIndeces = parallels * meridians * 6 - 3 * 2 * meridians;
	sphere.points = new TriangleComponentParameters::Vertex[sphere.numPoints];
	sphere.indeces = new int[sphere.numIndeces];

	int tempPos = 0;
	int tempInd = 0;

	TriangleComponentParameters::Vertex vertex1, vertex2, vertex3, vertex4;

	for (int i = 0; i < parallels; i++)
	{
		float teta1 = ((float)(i) / parallels) * PI;
		float teta2 = ((float)(i + 1) / parallels) * PI;

		for (int j = 0; j < meridians; j++)
		{
			float fi1 = ((float)(j) / meridians) * 2 * PI;
			float fi2 = ((float)(j + 1) / meridians) * 2 * PI;

			float x;
			float y;
			float z;

			x = radius * sin(teta1) * cos(fi1);
			y = radius * sin(teta1) * sin(fi1);
			z = radius * cos(teta1);
			vertex1.position = DirectX::SimpleMath::Vector4(x, y, z, 1.0f);
			vertex1.uv.x = teta1 / PI;
			vertex1.uv.y = fi1 / (2 * PI);
			vertex1.normal = vertex1.position;

			x = radius * sin(teta1) * cos(fi2);
			y = radius * sin(teta1) * sin(fi2);
			z = radius * cos(teta1);
			vertex2.position = DirectX::SimpleMath::Vector4(x, y, z, 1.0f);
			vertex2.uv.x = teta1 / PI;
			vertex2.uv.y = fi2 / (2 * PI);
			vertex2.normal = vertex2.position;

			x = radius * sin(teta2) * cos(fi2);
			y = radius * sin(teta2) * sin(fi2);
			z = radius * cos(teta2);
			vertex3.position = DirectX::SimpleMath::Vector4(x, y, z, 1.0f);
			vertex3.uv.x = teta2 / PI;
			vertex3.uv.y = fi2 / (2 * PI);
			vertex3.normal = vertex3.position;

			x = radius * sin(teta2) * cos(fi1);
			y = radius * sin(teta2) * sin(fi1);
			z = radius * cos(teta2);
			vertex4.position = DirectX::SimpleMath::Vector4(x, y, z, 1.0f);
			vertex4.uv.x = teta2 / PI;
			vertex4.uv.y = fi1 / (2 * PI);
			vertex4.normal = vertex4.position;

			if (i == 0)
			{
				vertices[tempPos] = vertex2;
				sphere.indeces[tempInd] = tempPos;
				tempPos++;
				tempInd++;
				vertices[tempPos] = vertex3;
				sphere.indeces[tempInd] = tempPos;
				tempPos++;
				tempInd++;
				vertices[tempPos] = vertex4;
				sphere.indeces[tempInd] = tempPos;
				tempPos++;
				tempInd++;
			}
			else if (i == parallels - 1)
			{
				vertices[tempPos] = vertex4;
				sphere.indeces[tempInd] = tempPos;
				tempPos++;
				tempInd++;
				vertices[tempPos] = vertex1;
				sphere.indeces[tempInd] = tempPos;
				tempPos++;
				tempInd++;
				vertices[tempPos] = vertex2;
				sphere.indeces[tempInd] = tempPos;
				tempPos++;
				tempInd++;
			}
			else
			{
				vertices[tempPos] = vertex1;
				tempPos++;
				vertices[tempPos] = vertex2;
				tempPos++;
				vertices[tempPos] = vertex3;
				tempPos++;
				vertices[tempPos] = vertex4;
				tempPos++;

				sphere.indeces[tempInd++] = tempPos - 1;
				sphere.indeces[tempInd++] = tempPos - 4;
				sphere.indeces[tempInd++] = tempPos - 3;
				sphere.indeces[tempInd++] = tempPos - 3;
				sphere.indeces[tempInd++] = tempPos - 2;
				sphere.indeces[tempInd++] = tempPos - 1;
			}
		}
	}

	for (int temp = 0; temp < sphere.numPoints; temp++)
	{
		sphere.points[temp] = vertices[temp];
	}

	TriangleComponent Sphere1(sphere, _texturePath);

	return Sphere1;

}

TriangleComponent SolarSystemComponent::CreateFloor(const wchar_t* _texturePath, float scale)
{
	TriangleComponentParameters _floor;

	_floor.numPoints = 16;
	_floor.numIndeces = 36;

	_floor.points = new TriangleComponentParameters::Vertex[_floor.numPoints]{
		DirectX::SimpleMath::Vector4(20.0 * scale, 0.0, 20.0 * scale, 1.0),		DirectX::SimpleMath::Vector2(1.0, 1.0),		DirectX::SimpleMath::Vector4(0.0, 1.0, 0.0, 1.0),
		DirectX::SimpleMath::Vector4(20.0 * scale, 0.0, -20.0 * scale, 1.0),	DirectX::SimpleMath::Vector2(1.0, -1.0),	DirectX::SimpleMath::Vector4(0.0, 1.0, 0.0, 1.0),
		DirectX::SimpleMath::Vector4(-20.0 * scale, 0.0, -20.0 * scale, 1.0),	DirectX::SimpleMath::Vector2(-1.0, -1.0),	DirectX::SimpleMath::Vector4(0.0, 1.0, 0.0, 1.0),
		DirectX::SimpleMath::Vector4(-20.0 * scale, 0.0, 20.0 * scale, 1.0),	DirectX::SimpleMath::Vector2(-1.0, 1.0),	DirectX::SimpleMath::Vector4(0.0, 1.0, 0.0, 1.0),
		DirectX::SimpleMath::Vector4(20.0 * scale, -10.0 * scale, 20.0 * scale, 1.0),	DirectX::SimpleMath::Vector2(1.0, 1.0),		DirectX::SimpleMath::Vector4(0.0, -1.0, 0.0, 1.0),
		DirectX::SimpleMath::Vector4(20.0 * scale, -10.0 * scale, -20.0 * scale, 1.0),	DirectX::SimpleMath::Vector2(1.0, 1.0),		DirectX::SimpleMath::Vector4(0.0, -1.0, 0.0, 1.0),
		DirectX::SimpleMath::Vector4(-20.0 * scale, -10.0 * scale, -20.0 * scale, 1.0), DirectX::SimpleMath::Vector2(1.0, 1.0),		DirectX::SimpleMath::Vector4(0.0, -1.0, 0.0, 1.0),
		DirectX::SimpleMath::Vector4(-20.0 * scale, -10.0 * scale, 20.0 * scale, 1.0),	DirectX::SimpleMath::Vector2(1.0, 1.0),		DirectX::SimpleMath::Vector4(0.0, -1.0, 0.0, 1.0)
	};
	_floor.indeces = new int[_floor.numIndeces] {
		0, 1, 2, 2, 3, 0,
		4, 5, 6, 6, 7, 4,
		0, 4, 5, 5, 1, 0,
		1, 5, 6, 6, 2, 1,
		2, 6, 7, 7, 3, 2,
		3, 7, 4, 4, 0, 3
	};

	TriangleComponent Floor(_floor, _texturePath);

	return Floor;
}

TriangleComponent SolarSystemComponent::CreateMesh(float _radius, const std::string& _modelPath, TriangleComponent* _parent, const wchar_t* _texturePath)
{
	TriangleComponentParameters _model;
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(_modelPath, aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);
	std::vector<TriangleComponentParameters::Vertex> _points;
	std::vector<int> _indeces;

	ReadNode(scene->mRootNode, scene, &_points, &_indeces);

	_model.numPoints = _points.size();
	_model.numIndeces = _indeces.size();

	_model.points = new TriangleComponentParameters::Vertex[_model.numPoints];
	_model.indeces = new int[_model.numIndeces];

	for (int i = 0; i < _model.numPoints; i++) _model.points[i] = _points[i];
	for (int i = 0; i < _model.numIndeces; i++) _model.indeces[i] = _indeces[i];

	TriangleComponent Mesh(_model, _texturePath);
	Mesh.parent = _parent;
	Mesh.radius = _radius;
	return Mesh;
}

void SolarSystemComponent::ReadNode(aiNode* node, const aiScene* scene, std::vector<TriangleComponentParameters::Vertex>* _points, std::vector<int>* _indeces)
{
	if (!node->mNumMeshes) std::cout << node->mName.data << " loaded" << std::endl;
	for (int j = 0; j < node->mNumMeshes; j++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[j]];
		for (int i = 0; i < mesh->mNumVertices; i++)
		{
			TriangleComponentParameters::Vertex point;

			point.position.x = mesh->mVertices[i].x;
			point.position.y = mesh->mVertices[i].y;
			point.position.z = mesh->mVertices[i].z;
			point.position.w = 1.0f;

			if (mesh->mTextureCoords[0])
			{
				point.uv.y = mesh->mTextureCoords[0][i].x;
				point.uv.x = mesh->mTextureCoords[0][i].y;
			}

			if (mesh->HasNormals())
			{
				point.normal.x = mesh->mNormals[i].x;
				point.normal.y = mesh->mNormals[i].y;
				point.normal.z = mesh->mNormals[i].z;
			}

			_points->push_back(point);
		}

		for (int i = 0; i < mesh->mNumFaces; i++) for (int k = 0; k < mesh->mFaces[i].mNumIndices; k++) _indeces->push_back(mesh->mFaces[i].mIndices[k]);
	}

	for (int i = 0; i < node->mNumChildren; i++) ReadNode(node->mChildren[i], scene, _points, _indeces);
}