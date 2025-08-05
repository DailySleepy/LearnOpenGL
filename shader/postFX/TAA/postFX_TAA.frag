#version 460 core
uniform sampler2D historyColor;
uniform sampler2D currentColor;
uniform sampler2D velocity;
uniform float mixRatio;

in vec2 texCoord;

out vec4 FragColor;

vec3 rgbToYCoCg(vec3 rgb) {
    float Y = 0.25 * rgb.r + 0.5 * rgb.g + 0.25 * rgb.b;
    float Co = 0.5 * rgb.r - 0.5 * rgb.b;
    float Cg = -0.25 * rgb.r + 0.5 * rgb.g - 0.25 * rgb.b;
    return vec3(Y, Co, Cg);
}

vec3 yCoCgToRGB(vec3 yCoCg) {
    float R = yCoCg.x + yCoCg.y - yCoCg.z;
    float G = yCoCg.x + yCoCg.z;
    float B = yCoCg.x - yCoCg.y - yCoCg.z;
    return vec3(R, G, B);
}

void main(){
    vec2 UV = texCoord;
    vec2 v = texture(velocity, UV).xy;
    vec2 historyUV = UV - v;
    
    if(historyUV.x < 0.0 || historyUV.x > 1.0 || historyUV.y < 0.0 || historyUV.y > 1.0){
        FragColor = texture(currentColor, UV);
        return;
    }
    
    vec3 historyRGB = texture(historyColor, historyUV).rgb;
    vec3 currentRGB = texture(currentColor, UV).rgb;

    vec3 historyYCoCg = rgbToYCoCg(historyRGB);
    vec3 currentYCoCg = rgbToYCoCg(currentRGB);
    
    vec3 minColor = currentYCoCg, maxColor = currentYCoCg;
    for(int i = -1; i <= 1; ++i){
        for(int j = -1; j <= 1; ++j){
            vec3 neighbor = rgbToYCoCg(texture(currentColor, UV + vec2(i,j) / textureSize(currentColor, 0)).rgb);
            minColor = min(minColor, neighbor);
            maxColor = max(maxColor, neighbor);
        }
    }
    historyYCoCg = clamp(historyYCoCg, minColor, maxColor);

    float ratio = clamp(mixRatio + length(v) * 10.0, 0.0, 1.0);
    
    vec3 colorYCoCg = mix(historyYCoCg, currentYCoCg, ratio);
    vec3 colorRGB = yCoCgToRGB(colorYCoCg);
    FragColor = vec4(colorRGB, 1);
}