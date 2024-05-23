#pragma once

#include "export.h"
#include "DisplayWin32.h"
#include "GameComponent.h"
#include "TriangleComponent.h"
#include "Camera.h"
#include "PointLight.h"

struct ConstPillarData
{
	DirectX::SimpleMath::Matrix invertedCamViewProjection;
	DirectX::SimpleMath::Matrix invertedCamTransform;
	DirectX::SimpleMath::Matrix camViewProjection;
	DirectX::SimpleMath::Vector3 viewerPos;
	float dummy = 1.0f;
};

struct PointLightData
{
	DirectX::SimpleMath::Vector4 lightSourcePosition;
	DirectX::SimpleMath::Vector4 lightColor;
	DirectX::SimpleMath::Matrix transformMtrx;
	DirectX::SimpleMath::Matrix frontFaceViewProjection;
	DirectX::SimpleMath::Matrix upperFaceViewProjection;
};

class Game 
{
private:
	int perf_FrameCount = 0;
	float perf_timer = 0;

public:
	Microsoft::WRL::ComPtr<ID3D11Device> device;

	DisplayWin32 display;
	D3D11_VIEWPORT viewport;
	D3D11_VIEWPORT shadowViewport;
	IDXGISwapChain* swapChain;
	ID3D11RenderTargetView* rtv;
	TriangleComponent triangleComponent;
	ID3D11Debug* debug;
	ID3D11Texture2D* shadowDepthTexture;
	ID3D11Texture2D* sceneDepthTexture;
	ID3D11DepthStencilView* depthView;
	ID3D11DepthStencilView* shadowDepthView;
	ID3D11ShaderResourceView* resView;
	ID3D11ShaderResourceView* camDepthView;
	ID3D11DepthStencilState* depthState;
	ID3D11SamplerState* samplerState;
	ID3D11RasterizerState* drawRastState;
	ID3D11RasterizerState* shadowRastState;
	ID3D11Buffer* lightViewProjBuffer;

	std::chrono::time_point<std::chrono::steady_clock> prevTime;

	float deltaTime;
	float totalTime = 0;
	unsigned int frameCount = 0;

	void Init();
	int PrepareResources();
	void DestroyResources();
	void PrepareFrame();
	void Draw();
	void DrawShadows();
	void DrawPillars();

	float CalculateGaussProt(float d);

	float* BGcolor;

	ID3D11VertexShader* vertexPillarsShader;
	ID3D11PixelShader* pixelPillarsShader;

	ID3D11Buffer* constPillarsBuffer;
	ConstPillarData constPillarsData;

	ID3D11Buffer* pointLightBuffer;
	PointLightData pointLightData;

	ID3D11DepthStencilState* depthStencilState;

	std::vector <Camera*> camera;

	InputDevice inputDevice;
	std::vector <GameComponent*> Components;
	ID3D11DeviceContext* context;

	std::vector <PointLight*> pointLights;

	bool IsKeyDown(Keys key) { inputDevice.IsKeyDown(key); };
	virtual void Update();

	Game();
	void Run();

	void SetBackgroundColor(float* color) { BGcolor = color; }
};