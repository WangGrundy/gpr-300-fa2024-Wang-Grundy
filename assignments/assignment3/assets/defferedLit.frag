#version 450 core
out vec4 FragColor; 
in vec2 UV; //From fsTriangle.vert

////////////////////////////////////////////////////////////////////////////
//All your material and lighting uniforms go here!

uniform sampler2D _MainTex; 
uniform vec3 _EyePos;
uniform vec3 _LightDirection = vec3(0.0,-1.0,0.0); //expose this at some point
uniform vec3 _LightColor = vec3(1.0);
uniform vec3 _AmbientColor = vec3(0.3,0.4,0.46);

in vec4 LightSpacePos;
uniform sampler2D _ShadowMap; 
uniform float bias;

struct Material{
	float Ka; //Ambient coefficient (0-1)
	float Kd; //Diffuse coefficient (0-1)
	float Ks; //Specular coefficient (0-1)
	float Shininess; //Affects size of specular highlight
};
uniform Material _Material;

uniform sampler2D _MainTex_Normal;  

////////////////////////////////////////////////////////////////////////////

//layout(binding = i) can be used as an alternative to shader.setInt()
//Each sampler will always be bound to a specific texture unit
uniform layout(binding = 0) sampler2D _gPositions;
uniform layout(binding = 1) sampler2D _gNormals;
uniform layout(binding = 2) sampler2D _gAlbedo;

vec3 calculateLighting(vec3 normal, vec3 worldPos, vec3 albedo)
{
	//Light pointing straight do
	vec3 toLight = -_LightDirection;
	float diffuseFactor = max(dot(normal,toLight),0.0);
	//Calculate specularly reflected light
	vec3 toEye = normalize(_EyePos - worldPos);
	//Blinn-phong uses half angle
	vec3 h = normalize(toLight + toEye);
	float specularFactor = pow(max(dot(normal,h),0.0),_Material.Shininess);
	//Combination of specular and diffuse reflection
	return (_Material.Kd * diffuseFactor + _Material.Ks * specularFactor) * _LightColor;
}

void main(){
	//Sample surface properties for this screen pixel
	vec3 normal = texture(_gNormals,UV).xyz;
	vec3 worldPos = texture(_gPositions,UV).xyz;
	vec3 albedo = texture(_gAlbedo,UV).xyz;

	//Worldspace lighting calculations, same as in forward shading
	vec3 lightColor = calculateLighting(normal,worldPos,albedo);
	FragColor = vec4(albedo * lightColor,1.0);
}
