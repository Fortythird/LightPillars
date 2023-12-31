#include "PointLight.h"

PointLight::PointLight()
{
	position = DirectX::SimpleMath::Vector3::Zero;
	color = DirectX::SimpleMath::Vector3::One;

	intensity = 10.0;

	nearDistance = 10.0;
	farDistance = 20.0;
}

PointLight::PointLight(DirectX::SimpleMath::Vector3 pos, DirectX::SimpleMath::Vector3 col)
{
	position = pos;
	color = col;

	intensity = 10.0;

	nearDistance = 10.0;
	farDistance = 20.0;
}

PointLight::PointLight(DirectX::SimpleMath::Vector3 pos, DirectX::SimpleMath::Vector3 col, float _intensity)
{
	position = pos;
	color = col;

	intensity = _intensity;

	nearDistance = 10.0;
	farDistance = 20.0;
}

PointLight::PointLight(DirectX::SimpleMath::Vector3 pos, DirectX::SimpleMath::Vector3 col, float _intensity, float nearDist, float farDist)
{
	position = pos;
	color = col;

	intensity = _intensity;

	nearDistance = nearDist;
	farDistance = nearDist < farDist ? farDist : nearDist;
}

void PointLight::Update(ID3D11DeviceContext* context, DirectX::SimpleMath::Vector3 viewerPos)
{
	// Right view
	// Left view
	views[2] = DirectX::SimpleMath::Matrix::CreateLookAt(
		position, 
		DirectX::SimpleMath::Vector3(viewerPos.x, position.y, viewerPos.z),
		DirectX::SimpleMath::Vector3::Up
		);
	// Down view
	views[4] = DirectX::SimpleMath::Matrix::CreateLookAt(
		position,
		DirectX::SimpleMath::Vector3::Up,
		-DirectX::SimpleMath::Vector3(viewerPos.x, position.y, viewerPos.z) - position
	);
	// Back view
}

void PointLight::DestroyResources()
{
	for (int i = 0; i < 6; i++)
	{
		if (i == 2 || i == 4) // Temporary for testing
		{
			depthTextures[i]->Release();
			depthView[i]->Release();
		}
	}
}

void PointLight::PrepareResources(Microsoft::WRL::ComPtr<ID3D11Device> device, DirectX::SimpleMath::Vector3 viewerPos)
{
	for (int i = 0; i < 6; i++)
	{
		depthTextures[i] = nullptr;
		depthView[i] = nullptr;
	}

	projectionMtrx = DirectX::SimpleMath::Matrix::CreatePerspectiveFieldOfView(
		((float)PI) / 2.0f,
		1.0f,
		0.1f,
		farDistance
	);

	// Right view
	// Left view
	views[2] = DirectX::SimpleMath::Matrix::CreateLookAt(
		position,
		DirectX::SimpleMath::Vector3(viewerPos.x, position.y, viewerPos.z),
		DirectX::SimpleMath::Vector3::Up
	);
	// Down view
	views[4] = DirectX::SimpleMath::Matrix::CreateLookAt(
		position,
		DirectX::SimpleMath::Vector3::Up,
		-DirectX::SimpleMath::Vector3(viewerPos.x, position.y, viewerPos.z) - position
	);
	// Back view

	D3D11_TEXTURE2D_DESC depthTexDesc = {};
	depthTexDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	depthTexDesc.ArraySize = 1;
	depthTexDesc.MipLevels = 1;
	depthTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
	depthTexDesc.CPUAccessFlags = 0;
	depthTexDesc.MiscFlags = 0;
	depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
	depthTexDesc.Width = textureResolution;
	depthTexDesc.Height = textureResolution;
	depthTexDesc.SampleDesc = { 1, 0 };
	device->CreateTexture2D(&depthTexDesc, nullptr, &(depthTextures[2]));
	device->CreateTexture2D(&depthTexDesc, nullptr, &(depthTextures[4]));

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResViewDesc = {};
	shaderResViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResViewDesc.Format = DXGI_FORMAT_R32_FLOAT;
	shaderResViewDesc.Texture2D.MipLevels = 1;
	device->CreateShaderResourceView(depthTextures[2], &shaderResViewDesc, &(depthView[2]));
	device->CreateShaderResourceView(depthTextures[4], &shaderResViewDesc, &(depthView[4]));

	D3D11_SAMPLER_DESC SamplerDesc = {};
	SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	for (int i = 0; i < 6; i++) device->CreateSamplerState(&SamplerDesc, &(samplerStates[i]));
}