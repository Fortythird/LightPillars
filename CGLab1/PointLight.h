#pragma once
#include "export.h"

#define PI 3.1415926535

class PointLight
{
private:
	int textureResolution = 1024;

	D3D11_VIEWPORT shadowViewport;

public:
	
	DirectX::SimpleMath::Vector3 position;
	DirectX::SimpleMath::Vector3 color;

	DirectX::SimpleMath::Matrix views[6];
	DirectX::SimpleMath::Matrix projectionMtrx;

	float intensity;
	float nearDistance;
	float farDistance;

	ID3D11Texture2D* depthTextures[6] = {};
	ID3D11ShaderResourceView* depthView[6] = {};
	ID3D11SamplerState* samplerState;

	PointLight();
	PointLight(DirectX::SimpleMath::Vector3 pos, DirectX::SimpleMath::Vector3 col);
	PointLight(DirectX::SimpleMath::Vector3 pos, DirectX::SimpleMath::Vector3 col, float _intensity);
	PointLight(DirectX::SimpleMath::Vector3 pos, DirectX::SimpleMath::Vector3 col, float _intensity, float nearDist, float farDist);

	void Update(ID3D11DeviceContext* context, DirectX::SimpleMath::Vector3 viewerPos);
	void PrepareResources(Microsoft::WRL::ComPtr<ID3D11Device> device, DirectX::SimpleMath::Vector3 viewerPos);
	void DestroyResources();
};

