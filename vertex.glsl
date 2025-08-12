#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;

uniform mat4 lightSpaceMatrix;
out vec4 FragPosLightSpace;

uniform mat4 model, view, projection;

out vec2 TexCoord;
out vec3 FragPos;
out vec3 Normal;

void main() {
    vec4 worldPos = model * vec4(aPos, 1.0);
    FragPosLightSpace = lightSpaceMatrix * worldPos;
    FragPos = worldPos.xyz;

    mat3 normalMatrix = transpose(inverse(mat3(model)));
    Normal = normalize(normalMatrix * aNormal);

    TexCoord = aTexCoord;
    gl_Position = projection * view * worldPos;
}
