#version 450 core

#define MAX_NUM_POINT_LIGHT 10
#define MAX_NUM_SPOT_LIGHT 10
#define MAX_NUM_DIRECTIONAL_LIGHT 5
#define MAX_NUM_SHADOW_CASTER_DIRECTIONAL_LIGTH 5

layout(location = 0) out vec4 Frag_color;


struct Material 
{
	vec3 diffuse;
	vec3 specular;
	float shininess;
};

struct AmbientLight
{
	vec3 color;
	float strength;	
};

struct Light
{
	vec3 color;
	float diffuse_strength;
	float specular_strength;
};

struct PointLight 
{
	vec3 position;
	Light light;

	// Attenuation.
	float constant;
	float linear;
	float quadratic;
};

struct SpotLight
{
	vec3 position;
	Light light;

	float in_angle;
	float out_angle;
	vec3 direction;
};

struct DirectionalLight 
{
	Light light;
	vec3 directional;
};

in vec4 v_position;
in vec3 v_normal;
in vec2 v_tex_coord;
in mat3 v_world_normals;
in vec4 v_pos_light_space[MAX_NUM_DIRECTIONAL_LIGHT];

// Material
uniform sampler2D u_texture;
uniform sampler2D u_specular_map;
uniform sampler2D u_nomal_map;
uniform sampler2D u_shadow_map[MAX_NUM_SHADOW_CASTER_DIRECTIONAL_LIGTH];
uniform bool u_is_using_normal_map;
uniform float u_shininess;
Material material;

uniform vec3 u_view_pos;

// Lights:
uniform uint u_NUM_POINT_LIGHT;
uniform uint u_NUM_SPOT_LIGHT;
uniform uint u_NUM_DIRECTIONAL_LIGHT;
uniform uint u_NUM_SHADOW_CASTER_DIRECTIONAL_LIGTH; 

uniform AmbientLight u_ambient_light;
uniform PointLight u_point_light[MAX_NUM_POINT_LIGHT];
uniform SpotLight u_spot_light[MAX_NUM_SPOT_LIGHT];
uniform DirectionalLight u_directional_light[MAX_NUM_DIRECTIONAL_LIGHT];
uniform DirectionalLight u_shadow_caster_directional_light[MAX_NUM_SHADOW_CASTER_DIRECTIONAL_LIGTH];

vec3 light(Light light, vec3 ray_direction)
{
	vec3 norm = normalize(v_normal);
	if (u_is_using_normal_map)
	{
		norm = texture(u_nomal_map, v_tex_coord).rgb;
		// transform normal vector to range [-1,1]
		norm = normalize(norm * 2.0f - 1.0f);
		norm = normalize(v_world_normals * norm);
	}
	vec3 view_direction = normalize(u_view_pos - v_position.xyz);
	vec3 reflect_direction = reflect(ray_direction, norm);

// Difuse
	float diff = max(dot(norm, -ray_direction), 0.0f);
	vec3 diffuse = diff * light.diffuse_strength * light.color * material.diffuse;

// Specular
	float spec = pow(max(dot(view_direction, reflect_direction), 0.0f), material.shininess * 32);
	vec3 specular = light.specular_strength * spec * light.color * material.specular;

	return diffuse + specular;
}

vec3 point_light(const uint i)
{
	vec3 light_direction = normalize(u_point_light[i].position - v_position.xyz);

// Attenuation
	float distance = length(u_point_light[i].position - v_position.xyz);
	float attenuation = 1.0f / (u_point_light[i].constant + u_point_light[i].linear * distance + u_point_light[i].quadratic * distance * distance);
	
	vec3 light = light(u_point_light[i].light, -light_direction);
	return light * attenuation;
}

vec3 spot_light(const uint i)
{
	vec3 light_direction = normalize(u_spot_light[i].position - v_position.xyz);
	
	float theta = dot(u_spot_light[i].direction, -light_direction);
	float epsilon = (u_spot_light[i].in_angle - u_spot_light[i].out_angle);
	float intensity = clamp((theta - u_spot_light[i].out_angle)/ epsilon , 0.0f, 1.0f);
	
	vec3 light = light(u_spot_light[i].light, -light_direction);
	return light * intensity;
}

vec3 directional_light(const uint i)
{
	vec3 light = light(u_directional_light[i].light, u_directional_light[i].directional);
	return light;
}

float shadow_calculation(const uint i)
{
	// shadow map coords
	vec3 proj_coords = v_pos_light_space[i].xyz / v_pos_light_space[i].w;
	proj_coords = proj_coords * 0.5f + 0.5f; // Set coordinates to the range {0, 1}

	float bias = 0.01f * tan(acos(clamp(dot( v_normal, u_shadow_caster_directional_light[i].directional ), 0.0f, 1.0f)));
	bias = clamp(bias, 0.0f, 0.01f);

	if(proj_coords.z <= texture(u_shadow_map[i], proj_coords.xy).r + 0.05f)
		return 1.0f;
	else 
		return 0.0f;
}

vec3 shadow_caster_directional_light(const uint i)
{
	vec3 light = light(u_shadow_caster_directional_light[i].light, u_shadow_caster_directional_light[i].directional);
	return light * shadow_calculation(i);
}

void main()
{
	material.diffuse = texture(u_texture, v_tex_coord).xyz;
	material.specular = texture(u_specular_map, v_tex_coord).xyz;
	material.shininess = u_shininess;

	vec3 result = vec3(0.0f);
	for(uint i = 0u; i < u_NUM_POINT_LIGHT; i++)
		result += point_light(i);
	for(uint i = 0u; i < u_NUM_DIRECTIONAL_LIGHT; i++)
		result += directional_light(i);
	for(uint i = 0u; i < u_NUM_SHADOW_CASTER_DIRECTIONAL_LIGTH; i++)
		result += shadow_caster_directional_light(i);
	for(uint i = 0u; i < u_NUM_SPOT_LIGHT; i++)
		result += spot_light(i);
	result += u_ambient_light.strength * u_ambient_light.color * material.diffuse; 

	Frag_color = vec4(result, 1.0f);
};
