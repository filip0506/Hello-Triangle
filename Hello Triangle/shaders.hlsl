/* vertex attributes go here to input to the vertex shader */

cbuffer cBuf : register(b1)
{
    row_major matrix WVP;
    row_major matrix world;
    row_major matrix view;
    row_major matrix projection;
};

struct Light
{
    float3 position;
    float3 ambientColor;
    float4 diffuse;
    float3 direction;
    float3 attenuation;
    
    float3 cameraPosition;
    float3 specularColor;

 };

cbuffer light : register(b2)
{
    Light light;
};

struct VS_INPUT
{
    float3 vPosition : POSITION;
    float2 vTexture : TEXTURE; // Inputs to vertex shader (from vertex buffer)
    float3 vNormal : NORMAL;
};


/* outputs from vertex shader go here. can be interpolated to pixel shader */
struct VS_OUTPUT
{
    float4 position : SV_POSITION; // required output of VS
    float2 textureVar : TEXTURE; // Outputs to vertex shader 
    float3 normal : NORMAL;
    float3 worldPos : WORLDPOSITION;

    

};


VS_OUTPUT VS_Main(VS_INPUT input) //Vertex shader
{
    VS_OUTPUT outPut;
    
    outPut = (VS_OUTPUT) 0; // zero the memory first
    outPut.position = mul(float4(input.vPosition, 1.0f), WVP);
    

    outPut.textureVar = input.vTexture;
    outPut.normal = mul(float4(input.vNormal, 0.0f), world).xyz;
    outPut.worldPos = mul(float4(input.vPosition, 1.0f), world).xyz;

 
    return outPut;
}

Texture2D tex : TEXTURE;
SamplerState samplerState : SAMPLER;

float3 diffuseFunc(Light light, float3 L, float3 N)
{
    // Calculate lighting intensity based on the angle of the surface
    float LdotN = saturate(dot(L, N));
    
    // Multiply the lighting intensity by the light's actual RGB color
    // This scales the light brightness from full intensity (facing the light) to zero (shadow)
    float3 diffuse = LdotN * light.diffuse.rgb;
    
    return diffuse;
}

float3 specularFunc(Light light, float3 position, float3 L, float3 N, float specularExponent)
{
    if (dot(N, L) <= 0.0f)
    {
        return float3(0.0f, 0.0f, 0.0f);

    }
    
    // 1. Calculate View Direction (Surface to Camera)
        float3 V = normalize(light.cameraPosition - position);
    
    // 2. Calculate Reflection Vector (Note: HLSL reflect() expects incoming direction -L)
    float3 R = normalize(reflect(-L, N));

    // 3. Dot product clamped between 0 and 1 to protect the pow() function
    float RdotV = saturate(dot(V, R));
    
    // 4. Raise to the exponent to determine highlight sharpness
    float specularIntensity = pow(RdotV, specularExponent);

    // 5. Final color blending
    float3 spec = light.specularColor * specularIntensity;
    
    return spec;

}

float attenuation(Light light, float3 worldPosition)
{
    float distanceToLight = length(light.position - worldPosition);
    float attenuation = 1.0f / (light.attenuation.x + (light.attenuation.y * distanceToLight) + (light.attenuation.z * (distanceToLight * distanceToLight)));

    return attenuation;
}



float4 PS_Main(VS_OUTPUT input) : SV_TARGET //Pixel shader
{
    
    float3 finalColor = float3(0, 0, 0);

    //Nrmalize normal
    float3 N = normalize(input.normal);
    
    // Direction from surface to light
    float3 L = normalize(light.position - input.worldPos);


    float3 diffuse = diffuseFunc(light, L, N);
    float3 specular = specularFunc(light, input.worldPos, L, N, 32.0f);
    float3 ambient = light.ambientColor;
        
    // Apply texture to ambient and diffuse components
    float4 texColor = tex.Sample(samplerState, input.textureVar);
    float att = attenuation(light, input.worldPos);
    
    finalColor = ambient * texColor.rgb + att * (diffuse * texColor.rgb + specular);
    return float4(saturate(finalColor), texColor.a);
}