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