#include "export.h"
#include "Game.h"
#include "TriangleComponent.h"

Game::Game() 
{
	context = nullptr;
	swapChain = nullptr;
	rtv = nullptr;
	debug = nullptr;
	BGcolor = new float[4] { 0.0f, 0.0f, 0.0f, 0.0f };

	shadowDepthTexture = nullptr;
	sceneDepthTexture = nullptr;
	depthView = nullptr;
	camDepthView = nullptr;
	shadowDepthView = nullptr;
	resView = nullptr;
	shadowRastState = nullptr;
	depthStencilState = nullptr;

	vertexPillarsShader = nullptr;
	pixelPillarsShader = nullptr;
	constPillarsBuffer = nullptr;
}

void Game::Init() 
{
	inputDevice.Init(display.getHWND());

	display.CreateDisplay(&inputDevice);

	pointLights.push_back(new PointLight(
		DirectX::SimpleMath::Vector3(5.0f, 5.0f, 0),
		DirectX::SimpleMath::Vector3(0.5f, 0.2f, 0.8f)
	));

	PrepareResources();
}

void Game::Run() 
{
	Init();

	bool isExitRequested = false;
	MSG msg = {};

	while (!isExitRequested) 
	{
		// Handle the windows messages.
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			// If windows signals to end the application then exit out.
			if (msg.message == WM_QUIT) isExitRequested = true;
		}

		PrepareFrame();

		Update();
		DrawShadows();
		Draw();
		DrawPillars();
		
		swapChain->Present(1, 0);
	}

	DestroyResources();
}

int Game::PrepareResources() 
{
	D3D_FEATURE_LEVEL featureLevel[] = {D3D_FEATURE_LEVEL_11_1};

	DXGI_SWAP_CHAIN_DESC swapDesc = {};
	swapDesc.BufferCount = 2;
	swapDesc.BufferDesc.Width = display.getScreenWidth();
	swapDesc.BufferDesc.Height = display.getScreenHeight();
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.OutputWindow = display.getHWND();
	swapDesc.Windowed = true;
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	swapDesc.SampleDesc.Count = 1;
	swapDesc.SampleDesc.Quality = 0;

	viewport = {};
	viewport.Width = static_cast<float>(display.getScreenWidth());
	viewport.Height = static_cast<float>(display.getScreenHeight());
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1.0f;

	shadowViewport = {};
	shadowViewport.Width = 5000;
	shadowViewport.Height = 5000;
	shadowViewport.TopLeftX = 0;
	shadowViewport.TopLeftY = 0;
	shadowViewport.MinDepth = 0;
	shadowViewport.MaxDepth = 1.0f;

	HRESULT res = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		D3D11_CREATE_DEVICE_DEBUG,
		featureLevel,
		1,
		D3D11_SDK_VERSION,
		&swapDesc,
		&swapChain,
		&device,
		nullptr,
		&context
	);

	if (FAILED(res))
	{
		std::cout << "Error while create device and swap chain" << std::endl;
	}

	D3D11_TEXTURE2D_DESC depthTexDesc = {};
	depthTexDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	depthTexDesc.ArraySize = 1; 
	depthTexDesc.MipLevels = 1;
	depthTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
	depthTexDesc.CPUAccessFlags = 0;
	depthTexDesc.MiscFlags = 0;
	depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
	depthTexDesc.Width = 5000;
	depthTexDesc.Height = 5000;
	depthTexDesc.SampleDesc = { 1, 0 };
	res = device->CreateTexture2D(&depthTexDesc, nullptr, &shadowDepthTexture);
	depthTexDesc.Width = display.getScreenWidth();
	depthTexDesc.Height = display.getScreenHeight();
	res = device->CreateTexture2D(&depthTexDesc, nullptr, &sceneDepthTexture);

	if (FAILED(res))
	{
		std::cout << "Error while creating texture 2D" << std::endl;
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStenDesc = {};
	depthStenDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStenDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStenDesc.Texture2D.MipSlice = 0;
	res = device->CreateDepthStencilView(sceneDepthTexture, &depthStenDesc, &depthView);

	if (FAILED(res))
	{
		std::cout << "Error while creating scene depth stencil view" << std::endl;
	}

	res = device->CreateDepthStencilView(shadowDepthTexture, &depthStenDesc, &shadowDepthView);

	if (FAILED(res))
	{
		std::cout << "Error while creating shadow depth stencil view" << std::endl;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResViewDesc = {};
	shaderResViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResViewDesc.Format = DXGI_FORMAT_R32_FLOAT;
	shaderResViewDesc.Texture2D.MipLevels = 1;
	res = device->CreateShaderResourceView(shadowDepthTexture, &shaderResViewDesc, &resView);
	res = device->CreateShaderResourceView(sceneDepthTexture, &shaderResViewDesc, &camDepthView);

	if (FAILED(res))
	{
		std::cout << "Error while creating shader resource view" << std::endl;
	}

	D3D11_RASTERIZER_DESC drawRenderStateDesc = {};
	drawRenderStateDesc.CullMode = D3D11_CULL_BACK;
	drawRenderStateDesc.FillMode = D3D11_FILL_WIREFRAME;
	drawRenderStateDesc.DepthClipEnable = true;
	res = device->CreateRasterizerState(&drawRenderStateDesc, &drawRastState);

	ID3D11Texture2D* backTexture;
	res = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backTexture);
	res = device->CreateRenderTargetView(backTexture, nullptr, &rtv);

	for (int i = 0; i < Components.size(); i++)
	{
		Components[i]->Init(device, display, res);
	}

	ID3DBlob* vertexBC = nullptr;
	ID3DBlob* errorVertexCode = nullptr;

	res = D3DCompileFromFile(
		L"../Shaders/PillarsShader.hlsl",
		nullptr,
		nullptr,
		"VSMain",
		"vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&vertexBC,
		&errorVertexCode
	);

	if (FAILED(res)) {
		if (errorVertexCode)
		{
			char* compileErrors = (char*)(errorVertexCode->GetBufferPointer());
			std::cout << compileErrors << std::endl;
		}
		else
		{
			MessageBox(display.getHWND(), L"../Shaders/ThirdExampleShader.hlsl", L"Missing Shader File", MB_OK);
		}
	}

	ID3DBlob* pixelBC = nullptr;
	ID3DBlob* errorPixelCode = nullptr;

	res = D3DCompileFromFile(
		L"../Shaders/PillarsShader.hlsl",
		nullptr,
		nullptr,
		"PSMain",
		"ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&pixelBC,
		&errorPixelCode
	);

	if (FAILED(res)) {
		if (errorPixelCode)
		{
			char* compileErrors = (char*)(errorPixelCode->GetBufferPointer());
			std::cout << compileErrors << std::endl;
		}
		else
		{
			MessageBox(display.getHWND(), L"../Shaders/ThirdExampleShader.hlsl", L"Missing Shader File", MB_OK);
		}
	}

	device->CreateVertexShader(
		vertexBC->GetBufferPointer(),
		vertexBC->GetBufferSize(),
		nullptr,
		&vertexPillarsShader
	);

	device->CreatePixelShader(
		pixelBC->GetBufferPointer(),
		pixelBC->GetBufferSize(),
		nullptr,
		&pixelPillarsShader
	);

	D3D11_BUFFER_DESC constPillarsBufDesc = {};
	constPillarsBufDesc.Usage = D3D11_USAGE_DYNAMIC;
	constPillarsBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constPillarsBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constPillarsBufDesc.MiscFlags = 0;
	constPillarsBufDesc.StructureByteStride = 0;
	constPillarsBufDesc.ByteWidth = sizeof(constPillarsData);
	HRESULT result = device->CreateBuffer(&constPillarsBufDesc, nullptr, &constPillarsBuffer);

	if (FAILED(result)) 
	{
		std::cout << "Error while pillars' const buffer creating...";
	}

	D3D11_BUFFER_DESC pointLightBufDesc = {};
	pointLightBufDesc.Usage = D3D11_USAGE_DYNAMIC;
	pointLightBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	pointLightBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	pointLightBufDesc.MiscFlags = 0;
	pointLightBufDesc.StructureByteStride = 0;
	pointLightBufDesc.ByteWidth = sizeof(PointLightData);
	result = device->CreateBuffer(&pointLightBufDesc, nullptr, &pointLightBuffer);

	if (FAILED(result))
	{
		std::cout << "Error while point light buffer creating...";
	}

	for (int i = 0; i < pointLights.size(); i++)
	{
		pointLights[i]->PrepareResources(device, camera.at(0)->position);
	}

	return 0;
}

void Game::DestroyResources() 
{
	for (int i = 0; i < Components.size(); i++) 
	{
		Components[i]->DestroyResources();
	}

	if (context != nullptr) 
	{
		context->ClearState();
		context->Release();
	}

	if (swapChain != nullptr) 
	{
		swapChain->Release();
	}

	if (device != nullptr) 
	{
		device->Release();
	}

	if (debug != nullptr) 
	{
		debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
	}

	if (shadowDepthTexture) 
	{
		shadowDepthTexture->Release();
	}

	if (sceneDepthTexture)
	{
		sceneDepthTexture->Release();
	}

	if (depthView != nullptr) 
	{
		depthView->Release();
	}

	if (camDepthView != nullptr)
	{
		camDepthView->Release();
	}
	
	if (shadowDepthView != nullptr) 
	{
		shadowDepthView->Release();
	}

	if (vertexPillarsShader != nullptr)
	{
		vertexPillarsShader->Release();
	}

	if (pixelPillarsShader != nullptr)
	{
		pixelPillarsShader->Release();
	}

	if (depthStencilState != nullptr)
	{
		depthStencilState->Release();
	}

	if (constPillarsBuffer != nullptr)
	{
		constPillarsBuffer->Release();
	}

	if (pointLightBuffer != nullptr)
	{
		pointLightBuffer->Release();
	}

	for (int i = 0; i < pointLights.size(); i++)
	{
		pointLights[i]->DestroyResources();
	}
}

void Game::PrepareFrame() 
{
	auto curTime = std::chrono::steady_clock::now();
	deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(curTime - prevTime).count() / 1000000.0f;
	prevTime = curTime;
	totalTime += deltaTime;
	frameCount++;

	if (totalTime > 1.0f) 
	{
		float fps = frameCount / totalTime;
		totalTime = 0.0f;
		WCHAR text[256];
		swprintf_s(text, TEXT("FPS: %f"), fps);
		SetWindowText(display.getHWND(), text);
		frameCount = 0;
	}

	context->ClearState();
	context->ClearDepthStencilView(depthView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	context->ClearDepthStencilView(shadowDepthView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	context->ClearRenderTargetView(rtv, BGcolor);
	context->OMSetBlendState(0, 0, 0xFFFFFF);
}

void Game::Update() 
{
	camera.at(0)->Update(
		deltaTime, 
		display.getScreenWidth(), 
		display.getScreenHeight()
	);

	for (int i = 0; i < Components.size(); i++) 
	{
		Components[i]->Update(context, camera.at(0));
	}
}

void Game::Draw() 
{
	camera.at(0)->Update(
		deltaTime,
		display.getScreenWidth(),
		display.getScreenHeight()
	);
	
	context->ClearRenderTargetView(rtv, BGcolor);

	context->RSSetViewports(1, &viewport);
	context->OMSetRenderTargets(1, &rtv, depthView);

	for (int i = 0; i < Components.size(); i++)
	{
		Components[i]->Draw(context, camera.at(0), resView);
	}
}

void Game::DrawShadows()
{
	context->RSSetViewports(1, &shadowViewport);
	ID3D11RenderTargetView* nullrtv[8] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
	context->OMSetRenderTargets(1, nullrtv, shadowDepthView);

	for (int i = 0; i < Components.size(); i++)
	{
		Components[i]->DrawShadow(context, device);
	}	
	
}

void Game::DrawPillars()
{

	for (int i = 0; i < pointLights.size(); i++)
	{
		pointLights[i]->Update(context, camera.at(0)->position);
	}

	constPillarsData.viewerPos = camera.at(0)->position;
	constPillarsData.invertedCamViewProjection = (camera.at(0)->viewMatrix * camera.at(0)->projectionMatrix).Transpose().Invert();
	constPillarsData.camViewProjection = camera.at(0)->projectionMatrix.Transpose();

	D3D11_MAPPED_SUBRESOURCE subresourse = {};
	context->Map(
		constPillarsBuffer,
		0,
		D3D11_MAP_WRITE_DISCARD,
		0,
		&subresourse
	);

	memcpy(
		reinterpret_cast<float*>(subresourse.pData),
		&constPillarsData,
		sizeof(ConstPillarData)
	);
	context->Unmap(constPillarsBuffer, 0);

	pointLightData.lightSourcePosition = DirectX::SimpleMath::Vector4(pointLights[0]->position.x, pointLights[0]->position.y, pointLights[0]->position.z, 1.0f);
	pointLightData.lightColor = DirectX::SimpleMath::Vector4(pointLights[0]->color.x, pointLights[0]->color.y, pointLights[0]->color.z, 1.0f);

	D3D11_MAPPED_SUBRESOURCE subresourse2 = {};
	context->Map(
		pointLightBuffer,
		0,
		D3D11_MAP_WRITE_DISCARD,
		0,
		&subresourse2
	);

	memcpy(
		reinterpret_cast<float*>(subresourse2.pData),
		&pointLightData,
		sizeof(PointLightData)
	);
	context->Unmap(pointLightBuffer, 0);

	ID3D11BlendState* blendState = nullptr;
	D3D11_BLEND_DESC blendDesc = {};

	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_COLOR;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_BLEND_FACTOR;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	HRESULT res = device->CreateBlendState(
		&blendDesc,
		&blendState
	);

	if (FAILED(res)) {
		std::cout << "Failed creating BlendState" << std::endl;
	}

	float blendFactor[] = { 1.0f, 1.0f, 1.0f, 1.0f };

	D3D11_DEPTH_STENCIL_DESC depthStencilStateDesc;
	depthStencilStateDesc.DepthEnable = false;
	depthStencilStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilStateDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthStencilStateDesc.StencilEnable = true;
	depthStencilStateDesc.StencilReadMask = 0xFF;
	depthStencilStateDesc.StencilWriteMask = 0xFF;
	depthStencilStateDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilStateDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilStateDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilStateDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthStencilStateDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilStateDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilStateDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilStateDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	device->CreateDepthStencilState(
		&depthStencilStateDesc,
		&depthStencilState
	);

	D3D11_SAMPLER_DESC SamplerDesc = {};
	SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	device->CreateSamplerState(&SamplerDesc, &samplerState);

	//context->OMSetDepthStencilState(depthStencilState, 1);
	context->OMSetRenderTargets(1, &rtv, nullptr);
	context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	context->VSSetShader(vertexPillarsShader, nullptr, 0);
	context->PSSetShader(pixelPillarsShader, nullptr, 0);

	context->PSSetConstantBuffers(0, 1, &constPillarsBuffer);
	context->PSSetConstantBuffers(1, 1, &pointLightBuffer);

	context->PSSetShaderResources(0, 1, &camDepthView);
	context->PSSetShaderResources(1, 1, &(pointLights[0]->depthView[2]));
	context->PSSetShaderResources(2, 1, &(pointLights[0]->depthView[4]));
	context->PSSetSamplers(0, 1, &samplerState);
	context->PSSetSamplers(1, 1, &(pointLights[0]->samplerStates[2]));
	context->PSSetSamplers(2, 1, &(pointLights[0]->samplerStates[4]));

	context->OMSetBlendState(blendState, blendFactor, 0xFFFFFF);
	context->Draw(4, 0);
}