#version 450

out vec4 FragColor; //out
in vec2 UV; //in
uniform sampler2D _ColorBuffer; //sampler
uniform float gammaValue = 5;

void main(){
	vec3 color = texture(_ColorBuffer,UV).rgb;
	color = pow(color,vec3(gammaValue/2.2));
    FragColor = vec4(color, 1.0);
}