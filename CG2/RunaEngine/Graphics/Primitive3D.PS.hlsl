#include "object3d.hlsli"
static const int32_t LIGHTING_NONE = 0;
static const int32_t LIGHTING_HALF_LAMBERT = 1;
static const int32_t LIGHTING_LAMBERT = 2;

struct Material {
    float32_t4 color;
    int32_t lightingMode;
    float32_t4x4 uvTransform;
};

struct DirectionalLight {
    float32_t4 color;
    float32_t3 direction;
    float intensity;
};

struct PixelShaderOutput {
    float32_t4 color : SV_TARGET0;
};

ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);

PixelShaderOutput main(VertexShaderOutput input) {
    PixelShaderOutput output;

   

    float32_t4 baseColor = gMaterial.color;

    if (gMaterial.lightingMode == LIGHTING_NONE) {
        output.color = baseColor;
        return output;
    }

    float32_t NdotL =
        dot(normalize(input.normal), -gDirectionalLight.direction);

    float32_t diffuse = 1.0f;

    if (gMaterial.lightingMode == LIGHTING_HALF_LAMBERT) {
        diffuse = pow(saturate(NdotL * 0.5f + 0.5f), 2.0f);
    } else if (gMaterial.lightingMode == LIGHTING_LAMBERT) {
        diffuse = saturate(NdotL);
    }

    output.color.rgb =
        baseColor.rgb *
        gDirectionalLight.color.rgb *
        diffuse *
        gDirectionalLight.intensity;

    output.color.a = baseColor.a;
    return output;
}