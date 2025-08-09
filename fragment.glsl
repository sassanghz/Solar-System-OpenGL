#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;
in vec4 FragPosLightSpace;   // from vertex.glsl

uniform sampler2D texture1;
uniform sampler2D shadowMap; // bound to texture unit 1
uniform vec3 viewPos;
uniform bool isSun;   // true only when drawing the Sun

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
struct PointLight {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float quadratic;
};

uniform DirLight  sun;
uniform PointLight earthLight;
uniform bool isEarth;  // set true only while drawing Earth (optional; defaults false)

vec3 CalcDirLight(DirLight light, vec3 N, vec3 V, vec3 albedo) {
    vec3 L = normalize(-light.direction);
    float diff = max(dot(N, L), 0.0);
    vec3 R = reflect(-L, N);
    float spec = pow(max(dot(V, R), 0.0), 32.0);
    vec3 ambient  = light.ambient * albedo;
    vec3 diffuse  = light.diffuse * diff * albedo;
    vec3 specular = light.specular * spec;
    return ambient + diffuse + specular;
}

vec3 CalcPointLight(PointLight L, vec3 N, vec3 P, vec3 V, vec3 albedo) {
    vec3 toL = L.position - P;
    float d  = length(toL);
    vec3  l  = toL / max(d, 1e-6);

    float diff = max(dot(N, l), 0.0);
    vec3  R    = reflect(-l, N);
    float spec = pow(max(dot(V, R), 0.0), 32.0);

    float att = 1.0 / (L.constant + L.linear * d + L.quadratic * d * d);

    vec3 ambient  = L.ambient  * albedo;
    vec3 diffuse  = L.diffuse  * diff * albedo;
    vec3 specular = L.specular * spec;
    return (ambient + diffuse + specular) * att;
}

float ShadowFactor(vec4 fragPosLightSpace, vec3 N, vec3 Lsun)
{
    // project to [0,1]
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.z > 1.0) return 0.0; // outside light frustum

    // bias (reduce acne)
    float ndotl = max(dot(N, Lsun), 0.0);
    float bias  = max(0.0005, 0.005 * (1.0 - ndotl));

    // 3x3 PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for (int x = -1; x <= 1; ++x)
    for (int y = -1; y <= 1; ++y) {
        float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x,y) * texelSize).r;
        shadow += (projCoords.z - bias > pcfDepth) ? 1.0 : 0.0;
    }
    shadow /= 9.0;
    return shadow;
}

void main()
{
    vec3 albedo = texture(texture1, TexCoord).rgb;
    vec3 N = normalize(Normal);
    vec3 V = normalize(viewPos - FragPos);

    // Sun (directional) + its shadow
    vec3 Lsun = normalize(-sun.direction);
    vec3 sunTerm = CalcDirLight(sun, N, V, albedo);
    float shadow = isSun ? 0.0 : ShadowFactor(FragPosLightSpace, N, Lsun);

    // emissive for the Sun so it looks self-lit
    vec3 outColor = sunTerm * (1.0 - shadow);
    if (isSun) {
    outColor += albedo * vec3(2.0); // emissive boost; tweak to taste
    }

    // Earth point light (optionally make Earth uniformly lit by itself)
    vec3 earthTerm = CalcPointLight(earthLight, N, FragPos, V, albedo);
    if (isEarth) {
        earthTerm = earthLight.diffuse * albedo + earthLight.ambient * albedo;
    }
    outColor += earthTerm;

    // tonemap + gamma
    outColor = outColor / (outColor + vec3(1.0));
    outColor = pow(outColor, vec3(1.0/2.2));
    FragColor = vec4(outColor, 1.0);
}
