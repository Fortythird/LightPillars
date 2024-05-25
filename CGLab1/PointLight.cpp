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

void PointLight::Update(DirectX::SimpleMath::Vector3 viewerPos)
{
	// Right view
	// Left view
	viewMtrcs[2] = DirectX::SimpleMath::Matrix::CreateLookAt(
		position, 
		DirectX::SimpleMath::Vector3(viewerPos.x, position.y, viewerPos.z),
		DirectX::SimpleMath::Vector3::Up
		);
	// Down view
	viewMtrcs[4] = DirectX::SimpleMath::Matrix::CreateLookAt(
		position,
		position + DirectX::SimpleMath::Vector3::Up,
		DirectX::SimpleMath::Vector3(viewerPos.x, position.y, viewerPos.z) - position
	);
	// Back view

	/*context->RSSetViewports(1, &shadowViewport);
	ID3D11RenderTargetView* nullrtv[8] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
	context->OMSetRenderTargets(1, nullrtv, shadowViewport);*/
}

void PointLight::DestroyResources()
{
	for (int i = 0; i < 6; i++)
	{
		if (i == 2 || i == 4) // Temporary for testing
		{
			depthTextures[i]->Release();
			depthStencilViews[i]->Release();
			depthShaderRes[i]->Release();
		}
	}
}

void PointLight::PrepareResources(Microsoft::WRL::ComPtr<ID3D11Device> device, DirectX::SimpleMath::Vector3 viewerPos)
{
	HRESULT res;

	for (int i = 0; i < 6; i++)
	{
		depthTextures[i] = nullptr;
		depthStencilViews[i] = nullptr;
		depthShaderRes[i] = nullptr;
	}

	projectionMtrx = DirectX::SimpleMath::Matrix::CreatePerspectiveFieldOfView(
		((float)PI) / 2.0f,
		1.0f,
		0.1f,
		farDistance
	);

	// Right view
	// Left view
	// Front view
	viewMtrcs[2] = DirectX::SimpleMath::Matrix::CreateLookAt(
		position,
		DirectX::SimpleMath::Vector3(viewerPos.x, position.y, viewerPos.z),
		DirectX::SimpleMath::Vector3::Up
	);
	// Down view
	// Upper view
	viewMtrcs[4] = DirectX::SimpleMath::Matrix::CreateLookAt(
		position,
		DirectX::SimpleMath::Vector3::Up,
		DirectX::SimpleMath::Vector3(viewerPos.x, position.y, viewerPos.z) - position
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

	res = device->CreateTexture2D(&depthTexDesc, nullptr, &(depthTextures[2]));
	if (FAILED(res)) std::cout << "Failed creating cube map depth texture 2" << std::endl;

	res = device->CreateTexture2D(&depthTexDesc, nullptr, &(depthTextures[4]));
	if (FAILED(res)) std::cout << "Failed creating cube map depth texture 4" << std::endl;

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStenDesc = {};
	depthStenDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStenDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStenDesc.Texture2D.MipSlice = 0;

	res = device->CreateDepthStencilView(depthTextures[2], &depthStenDesc, &(depthStencilViews[2]));
	if (FAILED(res)) std::cout << "Failed creating cube map depth stencil view 2" << std::endl;

	res = device->CreateDepthStencilView(depthTextures[4], &depthStenDesc, &(depthStencilViews[4]));
	if (FAILED(res)) std::cout << "Failed creating cube map depth stencil view 4" << std::endl;

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResViewDesc = {};
	shaderResViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResViewDesc.Format = DXGI_FORMAT_R32_FLOAT;
	shaderResViewDesc.Texture2D.MipLevels = 1;
	res = device->CreateShaderResourceView(depthTextures[2], &shaderResViewDesc, &(depthShaderRes[2]));
	if (FAILED(res)) std::cout << "Failed creating cube map depth shader resource view 2" << std::endl;

	res = device->CreateShaderResourceView(depthTextures[4], &shaderResViewDesc, &(depthShaderRes[4]));
	if (FAILED(res)) std::cout << "Failed creating cube map depth shader resource view 4" << std::endl;

	D3D11_SAMPLER_DESC SamplerDesc = {};
	SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	res = device->CreateSamplerState(&SamplerDesc, &samplerState);
	if (FAILED(res)) std::cout << "Failed creating light depth texture Sampler state" << std::endl;

	shadowViewport = {};
	shadowViewport.Width = textureResolution;
	shadowViewport.Height = textureResolution;
	shadowViewport.TopLeftX = 0;
	shadowViewport.TopLeftY = 0;
	shadowViewport.MinDepth = 0;
	shadowViewport.MaxDepth = 1.0f;
}