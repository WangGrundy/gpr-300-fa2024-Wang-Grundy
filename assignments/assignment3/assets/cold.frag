#version 450
out vec4 FragColor; //out
in vec2 UV; //in
uniform sampler2D _ColorBuffer; //sampler

void main(){
	vec3 color = texture(_ColorBuffer, UV).rgb;
    
    //make colour bluer
    float redToBlueRed = (color.r - color.b/2);
    float greenToBlueRed = (color.g - color.b/2);
    float blueIncrease = (color.b + (color.b/10));

    //set frag colour top this colour
    FragColor = vec4(redToBlueRed, greenToBlueRed, blueIncrease, 1.0);
}