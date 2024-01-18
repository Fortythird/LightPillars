static const int DEPTH_CALC_ENABLED = 0;

struct ConstData
{
    float4x4 invertedCamViewProjection;
    float4x4 camViewProjection;
    float3 viewerPos;
    float dummy;
};

struct PointLightData
{
    float4 lightSourcePosition;
    float4 lightColor;
    float4x4 frontFaceViewProjection;
    float4x4 upperFaceViewProjection;
};

struct Plane
{
    // Ax + By + Cz + D = 0
    
    float A;
    float B;
    float C;
    float D;
};

cbuffer ConstBuf : register(b0)
{
    ConstData constData;
};

cbuffer LightConstBuf : register(b1)
{
    PointLightData pointLightData;
};

Texture2D camDepthTexture : register(t0);
Texture2D lightFrontFaceDepthTexture : register(t1);
Texture2D lightUpperFaceDepthTexture : register(t2);

SamplerState camDepthSampler : register(s0);
SamplerState lightDepthSampler : register(s1);

static const float CRIT_ANGLE_RAD = radians(5.0f);
static const float CRIT_ANGLE_DEG = 5.0f;
static const float PI = 3.14159265359f;
static const float ABSORPTION_PARAMETER = 0.02f;
static const float DEPTH_READING_FREQUENCY = 100.0f;
static const float PILLAR_WIDTH_FACTOR = 10.0f;

float CalculateGaussProt(float d)
{
    return (2.0f * (903168.0f * pow(d, 10.0f) - 24613120.0f * pow(d, 9.0f) + 275587200.0f * pow(d, 8.0f) - 1540512000.0f * pow(d, 7.0f)
		+ 3869040000.0f * pow(d, 6.0f) - 1775340000.0f * pow(d, 5.0f) + 11280150000.0f * pow(d, 4.0f) - 126215250000.0f * pow(d, 3.0f)
		+ 5906250000.0f * pow(d, 2.0f) + 1928925534375.0f * d) / (8064.0f * pow(10.0f, 9.0f)));
}

float CalculateGaussIntegral(float lowerBorder, float upperBorder)
{
    return (CalculateGaussProt(upperBorder) - CalculateGaussProt(lowerBorder));
}

float CalculateRefractionProt(float d)
{
    return (0.15f * sin(4.0f * d) + 3.68f * sin(2.0f * d) - 0.2f * pow(sin(d), 5.0f)
        + 3.93f * pow(sin(d), 3.0f) - 15.71f * sin(d) + 8.73 * d) * 5.14f;
}

float CalculateRefractionIntegral(float lowerBorder, float upperBorder)
{
    return (CalculateRefractionProt(upperBorder) - CalculateRefractionProt(lowerBorder));
}

float4 VSMain(uint vI : SV_VERTEXID) : SV_POSITION
{
	float2 texcoord = float2(vI & 1, vI >> 1);
    return float4((texcoord.x - 0.5f) * 2.0f, -(texcoord.y - 0.5f) * 2.0f, 0.0f, 1.0f);
}

float4 PSMain(float4 pos : SV_POSITION) : SV_TARGET
{
    float4 NDC = float4(pos.x * 2.0f / 800.0f - 1.0f, -pos.y * 2.0f / 800.0f + 1.0f, 0.0f, 1.0f);
    float4 worldPoint = mul(NDC, constData.invertedCamViewProjection);
    worldPoint /= worldPoint.w;
    
    float3 viewDir = normalize(worldPoint.xyz - constData.viewerPos);
    
    if (viewDir.y == 0 || viewDir.x == 0 && viewDir.z == 0) return float4(0, 0, 0, 1);
    else 
    {
        float shiftY = (pointLightData.lightSourcePosition.y - constData.viewerPos.y) / viewDir.y;
    
        float R0x = (pointLightData.lightSourcePosition.x + constData.viewerPos.x + viewDir.x * shiftY) / 2.0f;
        float R0z = (pointLightData.lightSourcePosition.z + constData.viewerPos.z + viewDir.z * shiftY) / 2.0f;
        float3 R0 = float3(
            R0x,
            viewDir.x == 0 ? constData.viewerPos.y + viewDir.y * (R0z - constData.viewerPos.z) / viewDir.z 
                : constData.viewerPos.y + viewDir.y * (R0x - constData.viewerPos.x) / viewDir.x,
            R0z
        );
        

    
        float3 reflectionDir = normalize(pointLightData.lightSourcePosition.xyz - R0);
    
        float3 normal0 = normalize(viewDir - reflectionDir);
    
        if (normal0.y < cos(CRIT_ANGLE_RAD)) return float4(0, 0, 0, 1); // abs() to normal0.y to calculate lower pillar
        else
        {
            Plane workingPlane;
            
            workingPlane.A = viewDir.y * normal0.z - viewDir.z * normal0.y;
            workingPlane.B = -(viewDir.x * normal0.z - viewDir.z * normal0.x);
            workingPlane.C = viewDir.x * normal0.y - viewDir.y * normal0.x;
            workingPlane.D = -constData.viewerPos.x * workingPlane.A - constData.viewerPos.y * workingPlane.B - constData.viewerPos.z * workingPlane.C;
            
            float3 n1;
            
            float3 n2;
            
            if (workingPlane.A != 0)
            {
                float a = pow(workingPlane.A, 2) + pow(workingPlane.C, 2);
                float b = 2 * cos(CRIT_ANGLE_RAD) * workingPlane.B * workingPlane.C;
                float c = pow(cos(CRIT_ANGLE_RAD), 2) * pow(workingPlane.B, 2) - pow(sin(CRIT_ANGLE_RAD), 2) * pow(workingPlane.A, 2);
                float D = pow(b, 2) - 4 * a * c;

                float n1z = (-b + sqrt(D)) / 2.0f / a;
                n1 = normalize(float3(
                    (-cos(CRIT_ANGLE_RAD) * workingPlane.B - n1z * workingPlane.C) / workingPlane.A,
                    cos(CRIT_ANGLE_RAD),
                    n1z
                ));
                
                float n2z = (-b - sqrt(D)) / 2.0f / a;
                n2 = normalize(float3(
                    (-cos(CRIT_ANGLE_RAD) * workingPlane.B - n2z * workingPlane.C) / workingPlane.A,
                    cos(CRIT_ANGLE_RAD),
                    n2z
                ));
            }
            else
            {
                float a = pow(workingPlane.A, 2) + pow(workingPlane.C, 2);
                float b = 2 * cos(CRIT_ANGLE_RAD) * workingPlane.A * workingPlane.B;
                float c = pow(cos(CRIT_ANGLE_RAD), 2) * pow(workingPlane.B, 2) - pow(sin(CRIT_ANGLE_RAD), 2) * pow(workingPlane.C, 2);
                float D = pow(b, 2) - 4 * a * c;

                float n1x = (-b + sqrt(D)) / 2.0f / a;
                n1 = normalize(float3(
                    n1x,
                    cos(CRIT_ANGLE_RAD),
                    (-cos(CRIT_ANGLE_RAD) * workingPlane.B - n1x * workingPlane.A) / workingPlane.C
                ));
                
                float n2x = (-b - sqrt(D)) / 2.0f / a;
                n2 = normalize(float3(
                    n2x,
                    cos(CRIT_ANGLE_RAD),
                    (-cos(CRIT_ANGLE_RAD) * workingPlane.B - n2x * workingPlane.A) / workingPlane.C
                ));
            }
            
            /*float a = pow(viewDir.x * reflectionDir.y - viewDir.y * reflectionDir.x, 2) + pow(viewDir.y * reflectionDir.z - viewDir.z * reflectionDir.y, 2);
            
            float b;
            
            float c;
            
            float3 n1;
            
            float3 n2;
            
            if (viewDir.z != 0 || reflectionDir.z != 0)
            {
                b = -2 * cos(CRIT_ANGLE_RAD) * (viewDir.x * reflectionDir.z - viewDir.z * reflectionDir.x) * (viewDir.x * reflectionDir.y - viewDir.y * reflectionDir.x);
        
                c = pow(cos(CRIT_ANGLE_RAD), 2) * pow(viewDir.x * reflectionDir.z - viewDir.z * reflectionDir.x, 2)
                    - pow(sin(CRIT_ANGLE_RAD), 2) * pow(viewDir.y * reflectionDir.z - viewDir.z * reflectionDir.y, 2);
                
                float discriminant = pow(b, 2) - 4 * a * c;
        
                float n1z = (-b + sqrt(discriminant)) / 2.0f / a;
        
                n1 = normalize(float3(
                    (cos(CRIT_ANGLE_RAD) * (viewDir.x * reflectionDir.z - viewDir.z * reflectionDir.x) - n1z * (viewDir.x * reflectionDir.y - viewDir.y * reflectionDir.x))
                        / (viewDir.y * reflectionDir.z - viewDir.z * reflectionDir.y),
                    cos(CRIT_ANGLE_RAD),
                    n1z
                ));
        
                float n2z = (-b - sqrt(discriminant)) / 2.0f / a;
            
                n2 = normalize(float3(
                    (cos(CRIT_ANGLE_RAD) * (viewDir.x * reflectionDir.z - viewDir.z * reflectionDir.x) - n2z * (viewDir.x * reflectionDir.y - viewDir.y * reflectionDir.x))
                        / (viewDir.y * reflectionDir.z - viewDir.z * reflectionDir.y),
                    cos(CRIT_ANGLE_RAD),
                    n2z
                ));
            }
            else
            {
                b = -2 * cos(CRIT_ANGLE_RAD) * (viewDir.x * reflectionDir.z - viewDir.z * reflectionDir.x) * (viewDir.y * reflectionDir.z - viewDir.z * reflectionDir.y);
        
                c = pow(cos(CRIT_ANGLE_RAD), 2) * pow(viewDir.x * reflectionDir.z - viewDir.z * reflectionDir.x, 2)
                    - pow(sin(CRIT_ANGLE_RAD), 2) * pow(viewDir.x * reflectionDir.y - viewDir.y * reflectionDir.x, 2);
                
                float discriminant = pow(b, 2) - 4 * a * c;
        
                float n1x = (-b + sqrt(discriminant)) / 2.0f / a;
        
                n1 = normalize(float3(
                    n1x,
                    cos(CRIT_ANGLE_RAD),
                    (cos(CRIT_ANGLE_RAD) * (viewDir.x * reflectionDir.z - viewDir.z * reflectionDir.x) - n1x * (viewDir.y * reflectionDir.z - viewDir.z * reflectionDir.y))
                        / (viewDir.x * reflectionDir.y - viewDir.y * reflectionDir.x)
                ));
        
                float n2x = (-b - sqrt(discriminant)) / 2.0f / a;
            
                n2 = normalize(float3(
                    n2x,
                    cos(CRIT_ANGLE_RAD),
                    (cos(CRIT_ANGLE_RAD) * (viewDir.x * reflectionDir.z - viewDir.z * reflectionDir.x) - n2x * (viewDir.y * reflectionDir.z - viewDir.z * reflectionDir.y))
                        / (viewDir.x * reflectionDir.y - viewDir.y * reflectionDir.x)
                ));
            }*/
        
            /*float t1 = (-n1.x * viewDir.x - n1.y * viewDir.y - n1.z * viewDir.z) / (pow(n1.x, 2) + pow(n1.y, 2) + pow(n1.z, 2));
            float t2 = (-n2.x * viewDir.x - n2.y * viewDir.y - n2.z * viewDir.z) / (pow(n2.x, 2) + pow(n2.y, 2) + pow(n2.z, 2));
            
            float3 reflectionDir1 = normalize(float3(
                2 * n1.x * t1 + viewDir.x,
                2 * n1.y * t1 + viewDir.y,
                2 * n1.z * t1 + viewDir.z
            ));
        
            float3 reflectionDir2 = normalize(float3(
                2 * n2.x * t2 + viewDir.x,
                2 * n2.y * t2 + viewDir.y,
                2 * n2.z * t2 + viewDir.z
            ));*/
            
            float3 reflectionDir1 = reflect(viewDir, n1);
            float3 reflectionDir2 = reflect(viewDir, n2);
        
            float k = 0;
        
            if (viewDir.x != 0)
            {
                if (viewDir.x / viewDir.y == constData.viewerPos.x / constData.viewerPos.y 
                && reflectionDir1.x / reflectionDir1.y == pointLightData.lightSourcePosition.x / pointLightData.lightSourcePosition.y)
                    k = (viewDir.x * constData.viewerPos.z + viewDir.z * pointLightData.lightSourcePosition.x 
                    - viewDir.z * constData.viewerPos.x - viewDir.x * pointLightData.lightSourcePosition.z) / (viewDir.x * reflectionDir1.z - viewDir.z * reflectionDir1.x);
                else
                    k = (viewDir.x * constData.viewerPos.y + viewDir.y * pointLightData.lightSourcePosition.x 
                    - viewDir.y * constData.viewerPos.x - viewDir.x * pointLightData.lightSourcePosition.y) / (viewDir.x * reflectionDir1.y - viewDir.y * reflectionDir1.x);
            }
            else if (viewDir.y != 0)
            {
                if (viewDir.x / viewDir.y == constData.viewerPos.x / constData.viewerPos.y 
                && reflectionDir1.x / reflectionDir1.y == pointLightData.lightSourcePosition.x / pointLightData.lightSourcePosition.y)
                    k = (viewDir.y * constData.viewerPos.z + viewDir.z * pointLightData.lightSourcePosition.y 
                    - viewDir.z * constData.viewerPos.y - viewDir.y * pointLightData.lightSourcePosition.z) / (viewDir.y * reflectionDir1.z - viewDir.z * reflectionDir1.y);
                else
                    k = (viewDir.y * constData.viewerPos.x + viewDir.x * pointLightData.lightSourcePosition.y 
                    - viewDir.x * constData.viewerPos.y - viewDir.y * pointLightData.lightSourcePosition.x) / (viewDir.y * reflectionDir1.x - viewDir.x * reflectionDir1.y);
            }
            else if (viewDir.z != 0)
            {
                if (viewDir.x / viewDir.z == constData.viewerPos.x / constData.viewerPos.z
                && reflectionDir1.x / reflectionDir1.z == pointLightData.lightSourcePosition.x / pointLightData.lightSourcePosition.z)
                    k = (viewDir.z * constData.viewerPos.y + viewDir.y * pointLightData.lightSourcePosition.z 
                    - viewDir.y * constData.viewerPos.z - viewDir.z * pointLightData.lightSourcePosition.y) / (viewDir.z * reflectionDir1.y - viewDir.y * reflectionDir1.z);
                else
                    k = (viewDir.z * constData.viewerPos.x + viewDir.x * pointLightData.lightSourcePosition.z 
                    - viewDir.x * constData.viewerPos.z - viewDir.z * pointLightData.lightSourcePosition.x) / (viewDir.z * reflectionDir1.x - viewDir.x * reflectionDir1.z);
            }
        
            float3 R1 = pointLightData.lightSourcePosition.xyz + k * reflectionDir1;
            
            if (viewDir.x != 0)
            {
                if (viewDir.x / viewDir.y == constData.viewerPos.x / constData.viewerPos.y 
                && reflectionDir2.x / reflectionDir2.y == pointLightData.lightSourcePosition.x / pointLightData.lightSourcePosition.y)
                    k = (viewDir.x * constData.viewerPos.z + viewDir.z * pointLightData.lightSourcePosition.x 
                    - viewDir.z * constData.viewerPos.x - viewDir.x * pointLightData.lightSourcePosition.z) / (viewDir.x * reflectionDir2.z - viewDir.z * reflectionDir2.x);
                else
                    k = (viewDir.x * constData.viewerPos.y + viewDir.y * pointLightData.lightSourcePosition.x 
                    - viewDir.y * constData.viewerPos.x - viewDir.x * pointLightData.lightSourcePosition.y) / (viewDir.x * reflectionDir2.y - viewDir.y * reflectionDir2.x);
            }
            else if (viewDir.y != 0)
            {
                if (viewDir.x / viewDir.y == constData.viewerPos.x / constData.viewerPos.y 
                && reflectionDir2.x / reflectionDir2.y == pointLightData.lightSourcePosition.x / pointLightData.lightSourcePosition.y)
                    k = (viewDir.y * constData.viewerPos.z + viewDir.z * pointLightData.lightSourcePosition.y 
                    - viewDir.z * constData.viewerPos.y - viewDir.y * pointLightData.lightSourcePosition.z) / (viewDir.y * reflectionDir2.z - viewDir.z * reflectionDir2.y);
                else
                    k = (viewDir.y * constData.viewerPos.x + viewDir.x * pointLightData.lightSourcePosition.y 
                    - viewDir.x * constData.viewerPos.y - viewDir.y * pointLightData.lightSourcePosition.x) / (viewDir.y * reflectionDir2.x - viewDir.x * reflectionDir2.y);
            }
            else if (viewDir.z != 0)
            {
                if (viewDir.x / viewDir.z == constData.viewerPos.x / constData.viewerPos.z
                && reflectionDir2.x / reflectionDir2.z == pointLightData.lightSourcePosition.x / pointLightData.lightSourcePosition.z)
                    k = (viewDir.z * constData.viewerPos.y + viewDir.y * pointLightData.lightSourcePosition.z 
                    - viewDir.y * constData.viewerPos.z - viewDir.z * pointLightData.lightSourcePosition.y) / (viewDir.z * reflectionDir2.y - viewDir.y * reflectionDir2.z);
                else
                    k = (viewDir.z * constData.viewerPos.x + viewDir.x * pointLightData.lightSourcePosition.z 
                    - viewDir.x * constData.viewerPos.z - viewDir.z * pointLightData.lightSourcePosition.x) / (viewDir.z * reflectionDir2.x - viewDir.x * reflectionDir2.z);
            }
        
            float3 R2 = pointLightData.lightSourcePosition.xyz + k * reflectionDir2;
            
            if (R1.x == R2.x && R1.y == R2.y && R1.z == R2.z) return float4(0, 0, 0, 1);
               
            float3 Rn = R1, Rf = R1;
            if (length(R1 - constData.viewerPos) > length(R2 - constData.viewerPos))
                Rn = R2;
            else
                Rf = R2;
            
            if (DEPTH_CALC_ENABLED)
            {   
                float4 camSpacePoint = mul(constData.camViewProjection, float4(Rn.xyz, 0));
                float depthValue = camDepthTexture.SampleLevel(camDepthSampler, float2((NDC.x + 1.0f) / 2, -(NDC.y + 1.0f) / 2), 0).x;
                if (depthValue <= camSpacePoint.z / camSpacePoint.w && depthValue != 1.0f)
                    return float4(0, 0, 0, 1);
            
                float3 step = (Rf - Rn) / DEPTH_READING_FREQUENCY;
            
                float3 currentPoint = Rn, startPoint = Rn;
            
                float3 currentVector = currentPoint - pointLightData.lightSourcePosition.xyz;
            
                bool isDepthTestPassed = true;
            
                float intensity = 0, L = 0;
            
                if (currentVector.y < sqrt(pow(currentVector.x, 2) + pow(currentVector.z, 2)))
                {
                    float4 lightSpacePos = mul(pointLightData.frontFaceViewProjection, float4(currentPoint.xyz, 1.0f));
                    float depthValue = lightFrontFaceDepthTexture.SampleLevel(lightDepthSampler,
		            float2(lightSpacePos.x / lightSpacePos.w, -lightSpacePos.y / lightSpacePos.w) * 0.5f + 0.5f, 0).x;
                
                    if (depthValue <= lightSpacePos.z / lightSpacePos.w && depthValue != 1.0f)
                        isDepthTestPassed = false;
                    else
                        isDepthTestPassed = true;
                }
                else
                {
                    float4 lightSpacePos = mul(pointLightData.upperFaceViewProjection, float4(currentPoint.xyz, 1.0f));
                    float depthValue = lightUpperFaceDepthTexture.SampleLevel(lightDepthSampler,
		            float2(lightSpacePos.x / lightSpacePos.w, -lightSpacePos.y / lightSpacePos.w) * 0.5f + 0.5f, 0).x;
                
                    if (depthValue <= lightSpacePos.z / lightSpacePos.w && depthValue != 1.0f)
                        isDepthTestPassed = false;
                    else
                        isDepthTestPassed = true;
                }
            
                bool testDepthPass = true;
                
                isDepthTestPassed = true;
                
                for (int i = 1; i <= DEPTH_READING_FREQUENCY; i++)
                {
                    currentPoint = Rn + step * i;
                    currentVector = normalize(currentPoint - pointLightData.lightSourcePosition.xyz);
                    
                    float depthValue;
                    
                    if (currentVector.y < sqrt(pow(currentVector.x, 2) + pow(currentVector.z, 2)))
                    {
                        float4 lightSpacePos = mul(pointLightData.frontFaceViewProjection, float4(currentPoint.xyz, 1.0f));
                        depthValue = lightFrontFaceDepthTexture.SampleLevel(lightDepthSampler,
		                    float2(lightSpacePos.x / lightSpacePos.w, -lightSpacePos.y / lightSpacePos.w) * 0.5f + 0.5f, 0).x;
                    }
                    else
                    {
                        float4 lightSpacePos = mul(pointLightData.upperFaceViewProjection, float4(currentPoint.xyz, 1.0f));
                        depthValue = lightUpperFaceDepthTexture.SampleLevel(lightDepthSampler,
		                    float2(lightSpacePos.x / lightSpacePos.w, -lightSpacePos.y / lightSpacePos.w) * 0.5f + 0.5f, 0).x;
                    }
                    
                    if ( /*depthValue <= lightSpacePos.z / lightSpacePos.w && depthValue != 1.0f*/!testDepthPass)
                    {
                        if (isDepthTestPassed)
                        {
                            isDepthTestPassed = false;
                            
                            n1 = normalize(viewDir + normalize(startPoint - pointLightData.lightSourcePosition.xyz));
                            n2 = normalize(viewDir + currentVector);
                            float e1 = acos(n1.x * viewDir.x + n1.y * viewDir.y + n1.z * viewDir.z);
                            float e2 = acos(n2.x * viewDir.x + n2.y * viewDir.y + n2.z * viewDir.z);
                            
                            float3 nR0 = R0 - startPoint;
                            float3 nf = currentPoint - startPoint;
                            float N;
                                
                            if (length(nR0) < length(nf) && nf.x / nR0.x >= 0) 
                                N = abs(CalculateGaussIntegral(acos(n1.y) / PI * 180.0f, acos(normal0.y) / PI * 180.0f))
                                    + abs(CalculateGaussIntegral(acos(n2.y) / PI * 180.0f, acos(normal0.y) / PI * 180.0f));
                            else
                                N = abs(CalculateGaussIntegral(acos(n1.y) / PI * 180.0f, acos(n2.y) / PI * 180.0f));
                            
                            intensity = intensity + N * abs(CalculateRefractionIntegral(e1, e2))
                                    * pow(2.71f, -ABSORPTION_PARAMETER * length((startPoint + currentPoint) / PILLAR_WIDTH_FACTOR - constData.viewerPos));
                            
                            L += length(currentPoint - startPoint);
                        }
                    }
                    else
                    {
                        if (!isDepthTestPassed)
                        {
                            isDepthTestPassed = true;
                            startPoint = currentPoint;
                        }
                    }
                    
                    if (i < 99)
                        testDepthPass = true;
                    else
                        testDepthPass = false;
                }
                
                //TODO: add post-loop calculation

                intensity *= clamp(L / 2.0f, 0, 1);
                return float4(pointLightData.lightColor.xyz * intensity, 1.0f);
            }
            else
            {
                float4 camSpacePoint = mul(constData.camViewProjection, float4(Rn.xyz, 0));
                float depthValue = camDepthTexture.SampleLevel(camDepthSampler, float2((NDC.x + 1.0f) / 2, -(NDC.y + 1.0f) / 2), 0).x;
                if (depthValue <= camSpacePoint.z / camSpacePoint.w && depthValue != 1.0f)
                    return float4(0, 0, 0, 1);
                
                float N = abs(CalculateGaussIntegral(acos(n1.y) / PI * 180.0f, acos(normal0.y) / PI * 180.0f))
                + abs(CalculateGaussIntegral(acos(n2.y) / PI * 180.0f, acos(normal0.y) / PI * 180.0f));
            
                float e0 = acos(normal0.x * viewDir.x + normal0.y * viewDir.y + normal0.z * viewDir.z);
                float e1 = acos(n1.x * viewDir.x + n1.y * viewDir.y + n1.z * viewDir.z);
                float e2 = acos(n2.x * viewDir.x + n2.y * viewDir.y + n2.z * viewDir.z);
            
                N *= (abs(CalculateRefractionIntegral(e1, e2)));
            
                N *= pow(2.71f, -ABSORPTION_PARAMETER * length(R0 - constData.viewerPos)) * clamp(length(R2 - R1) / PILLAR_WIDTH_FACTOR, 0, 1);
            
                return float4(pointLightData.lightColor.xyz * N, 1.0f);
            }
        }
    }
}