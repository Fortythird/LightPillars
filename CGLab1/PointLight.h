#pragma once
#include "export.h"

class PointLight
{
public:
	
	DirectX::SimpleMath::Vector3 position;
	DirectX::SimpleMath::Vector3 color;

	float intensity;
	float nearDistance;
	float farDistance;

	PointLight();
	PointLight(DirectX::SimpleMath::Vector3 pos, DirectX::SimpleMath::Vector3 col);
	PointLight(DirectX::SimpleMath::Vector3 pos, DirectX::SimpleMath::Vector3 col, float _intensity);
	PointLight(DirectX::SimpleMath::Vector3 pos, DirectX::SimpleMath::Vector3 col, float _intensity, float nearDist, float farDist);
};

