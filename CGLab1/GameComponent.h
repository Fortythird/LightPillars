#pragma once

#include "export.h"
#include "DisplayWin32.h"
#include "Camera.h"

using namespace DirectX;

class GameComponent {
	public:
		virtual int Init(Microsoft::WRL::ComPtr<ID3D11Device> device, DisplayWin32 display, HRESULT res) { return 0; };
	
		virtual void DestroyResources() {};

		virtual void Draw(ID3D11DeviceContext* context, Camera* camera, ID3D11ShaderResourceView* resView) {};

		virtual void DrawShadow(ID3D11DeviceContext* context) {};

		virtual void DrawDepth(ID3D11DeviceContext* context, DirectX::SimpleMath::Matrix viewProjMtrx) {};

		virtual void Update(ID3D11DeviceContext* context, Camera* camera) = 0;

		virtual void SetPos(DirectX::SimpleMath::Vector3 pos) = 0;
		virtual void SetRot(DirectX::SimpleMath::Vector3 rot) = 0;
};