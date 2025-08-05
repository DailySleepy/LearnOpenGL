#version 460 core
in vec4 currPos;
in vec4 prevPos;

out vec4 velocity;

void main(){
	vec2 currDNC = (currPos.xy / currPos.w) * 0.5 + 0.5;
	vec2 prevDNC = (prevPos.xy / prevPos.w) * 0.5 + 0.5;
	velocity = vec4(currDNC - prevDNC, 0, 1);
}