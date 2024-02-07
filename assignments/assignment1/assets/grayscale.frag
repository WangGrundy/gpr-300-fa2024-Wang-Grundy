#version 450
out vec4 FragColor;
in vec2 UV;
uniform sampler2D _ColorBuffer;

void main() {
    vec3 color = texture(_ColorBuffer, UV).rgb;
    
    //get gray colour
    float gray = (color.r + color.g + color.b) / 3.0;

    //set frag colour top this colour
    FragColor = vec4(gray, gray, gray, 1.0);
}