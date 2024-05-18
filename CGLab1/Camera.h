#pragma once

#include "export.h"
#include "InputDevice.h"

#define PI 3.1415926535

class Camera {
	private:
		

	public:
		InputDevice* inputDeviceCameraInstance;
		DirectX::SimpleMath::Matrix viewMatrix;
		DirectX::SimpleMath::Matrix projectionMatrix;
		DirectX::SimpleMath::Matrix transformMatrix;
		DirectX::SimpleMath::Vector3 position;
		DirectX::SimpleMath::Vector3 camDirection;

		float yawAxis;
		float pitchAxis;
		float velocityMagnitude = 2.0f;
		float mouseSensetivity = 5.0f;
		float orbitPos = 0;
		DirectX::SimpleMath::Vector3* parentPos;

		Camera(DirectX::SimpleMath::Vector3* _parentPos);

		void Initialize(
			DirectX::SimpleMath::Vector3 pos, 
			float yawVal, 
			float pitchVal, 
			int screenWidth, 
			int screenHeight, 
			InputDevice* inputeDeviceInstance);

		void Update(float deltaTime, int screenWidth, int screenHeight);
		void OnMouseMove(const MouseMoveEventArgs& args);

		DirectX::SimpleMath::Matrix GetModelMatrix();
};