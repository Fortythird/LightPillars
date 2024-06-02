#include "export.h"
#include "Camera.h"

Camera::Camera(DirectX::SimpleMath::Vector3* _parentPos) 
{
	parentPos = _parentPos;
}

void Camera::Initialize(DirectX::SimpleMath::Vector3 pos, float yawVal, float pitchVal, int screenWidth, int screenHeight, InputDevice* inputeDeviceInstance) 
{
	inputDeviceCameraInstance = inputeDeviceInstance;
	viewMatrix = DirectX::SimpleMath::Matrix::Identity;

	projectionMatrix = DirectX::SimpleMath::Matrix::CreatePerspectiveFieldOfView(
		((float) PI) / 2.0f,
		(float) screenWidth / screenHeight, 
		0.1f,
		500.0f
	); 
	
	yawAxis = yawVal;
	pitchAxis = pitchVal;
	position = pos;

	position = { 30.0f, 1.0f, 2.0f };
	yawAxis = 1.57f;
	pitchAxis = .7f;
	
	if (inputDeviceCameraInstance != nullptr) {
		inputDeviceCameraInstance->MouseMove.AddRaw(this, &Camera::OnMouseMove);
	}
}

void Camera::Update(float deltaTime, int screenWidth, int screenHeight) 
{	
	auto rotation = DirectX::SimpleMath::Matrix::CreateFromYawPitchRoll(yawAxis, pitchAxis, 0);

	if (inputDeviceCameraInstance != nullptr) {
		auto velDirection = DirectX::SimpleMath::Vector3::Zero;

		if (inputDeviceCameraInstance->IsKeyDown(Keys::W)) velDirection += DirectX::SimpleMath::Vector3(5.0f, 0.0f, 0.0f);
		if (inputDeviceCameraInstance->IsKeyDown(Keys::S)) velDirection += DirectX::SimpleMath::Vector3(-5.0f, 0.0f, 0.0f);
		if (inputDeviceCameraInstance->IsKeyDown(Keys::A)) velDirection += DirectX::SimpleMath::Vector3(0.0f, 0.0f, -5.0f);
		if (inputDeviceCameraInstance->IsKeyDown(Keys::D)) velDirection += DirectX::SimpleMath::Vector3(0.0f, 0.0f, 5.0f);
		if (inputDeviceCameraInstance->IsKeyDown(Keys::E)) velDirection += DirectX::SimpleMath::Vector3(0.0f, 5.0f, 0.0f);
		if (inputDeviceCameraInstance->IsKeyDown(Keys::Q)) velDirection += DirectX::SimpleMath::Vector3(0.0f, -5.0f, 0.0f);

		velDirection.Normalize();

		auto velDir = rotation.Forward() * velDirection.x + DirectX::SimpleMath::Vector3::Up * velDirection.y + rotation.Right() * velDirection.z;
		if (velDir.Length() != 0) {
			velDir.Normalize();
		}

		position += velDir * velocityMagnitude * deltaTime;
	}

	viewMatrix = DirectX::SimpleMath::Matrix::CreateLookAt(position, position + rotation.Forward(), rotation.Up());
	projectionMatrix = DirectX::SimpleMath::Matrix::CreatePerspectiveFieldOfView(
		(float)PI / 2.0f,
		(float)screenWidth / screenHeight,
		0.1f,
		100.0f
	);
	transformMatrix = DirectX::SimpleMath::Matrix::CreateWorld(position, position + rotation.Forward(), rotation.Up());
}

void Camera::OnMouseMove(const MouseMoveEventArgs& args) 
{
	if (inputDeviceCameraInstance == nullptr) {
		return;
	}

	yawAxis -= args.Offset.x * 0.003f * mouseSensetivity;
	pitchAxis -= args.Offset.y * 0.003f * mouseSensetivity;
	if (pitchAxis < -1.57f) pitchAxis = -1.57f;
	else if (pitchAxis > 1.57f) pitchAxis = 1.57f;

	if (args.WheelDelta == 0) return;
	if (args.WheelDelta > 0) velocityMagnitude += 1;
	if (args.WheelDelta < 0) velocityMagnitude -= 1;
}

DirectX::SimpleMath::Matrix Camera::GetModelMatrix()
{
	DirectX::SimpleMath::Matrix model = DirectX::SimpleMath::Matrix::Identity;
	model *= DirectX::SimpleMath::Matrix::CreateFromQuaternion(DirectX::SimpleMath::Quaternion::CreateFromYawPitchRoll(yawAxis, pitchAxis, 0));
	model *= DirectX::SimpleMath::Matrix::CreateTranslation(position);

	return model;
}