#shader vertex
#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

layout(std140) uniform Matrices{
	mat4 projection;
	mat4 view;
};
uniform mat4 model;

out vec3 Normal;
out vec3 fragPos;
out vec2 texCoord;

void main() {
	gl_Position = projection * view * model * vec4(aPos, 1);
	Normal = mat3(transpose(inverse(model))) * aNormal;
	fragPos = vec3(model * vec4(aPos, 1));
	vec2 texScale = vec2(20);
    texCoord = aTexCoord * texScale;
}




#shader fragment
#version 460 core

struct Material {
    sampler2D diffuse1;
    sampler2D specular1;
};

in vec3 Normal;
in vec3 fragPos;
in vec2 texCoord;

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BrightColor;

uniform Material material;
uniform vec3 viewPos;
uniform vec3 lightPos[4];
uniform vec3 lightColor[4];
uniform float bloomThreshold;

void main() {
    vec3 ambient = vec3(0.0);
    vec3 diffuse = vec3(0.0);

    vec3 normal = normalize(Normal);

    for (int i = 0; i < 4; ++i) {
        float d = length(lightPos[i] - fragPos);
        float attenuation = 1.0 / (1.0 + 0.045 * d + 0.0075 * d * d);

        ambient += 0.3 * lightColor[i] * vec3(texture(material.diffuse1, texCoord)) * attenuation;

        vec3 lightDir = normalize(lightPos[i] - fragPos);
        float diff = max(dot(normal, lightDir), 0.0);
        diffuse += 0.8 * lightColor[i] * diff * vec3(texture(material.diffuse1, texCoord)) * attenuation;
    }

    vec3 result = ambient + diffuse;

    FragColor = vec4(result, 1.0);

    float brightness = dot(result, vec3(0.2126, 0.7152, 0.0722));

    if(brightness > bloomThreshold) 
        BrightColor = FragColor;
    //else
    //    BrightColor = vec4(0.0, 0.0, 0.0, 1.0); // 如果启用了blend, 这样避免提取到被遮挡的部分
}
