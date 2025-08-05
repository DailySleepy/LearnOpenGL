#version 460 core

uniform sampler2D screenTexture;

in vec2 texCoord;

out vec4 FragColor;

const float xoffset = 1.0 / 800.0;  
const float yoffset = 1.0 / 800.0; 

void main()
{
    vec2 offsets[9] = vec2[](
        vec2(-xoffset,  yoffset), // 左上
        vec2( 0.0f,     yoffset), // 正上
        vec2( xoffset,  yoffset), // 右上
        vec2(-xoffset,  0.0f),    // 左
        vec2( 0.0f,     0.0f),    // 中
        vec2( xoffset,  0.0f),    // 右
        vec2(-xoffset, -yoffset), // 左下
        vec2( 0.0f,    -yoffset), // 正下
        vec2( xoffset, -yoffset)  // 右下
    );

    float kernel[9] = float[](
        -1, -1, -1,
        -1,  9, -1,
        -1, -1, -1
    );

    vec3 col = vec3(0);
    for(int i = 0; i < 9; i++) {
        col += kernel[i] * vec3(texture(screenTexture, texCoord + offsets[i]));
    }

    FragColor = vec4(col, 1.0);
}
