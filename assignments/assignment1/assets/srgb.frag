#version 450
out vec4 FragColor;
in vec2 UV;
uniform sampler2D _ColorBuffer;

void main(){
	vec3 diffuseColor = texture(_ColorBuffer, UV).rgb;
	diffuseColor = pow(diffuseColor, vec3(2.2));
	FragColor = vec4(diffuseColor, 1.0);
}