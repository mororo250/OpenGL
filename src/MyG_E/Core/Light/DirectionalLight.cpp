#include "DirectionalLight.h"

#include "Core/Shader.h"
#include "Core/ShadowMap.h"

#include "imgui.h"

unsigned short DirectionalLight::s_count = 0;
unsigned short DirectionalLight::s_count_shadow_caster = 0;

DirectionalLight::DirectionalLight(Vector3f const& position, Vector3f const& color, Vector3f const& direction, ShadowMap* shadow_map)
	: m_direction(direction)
	, m_shadow_map(shadow_map)
	, Light(position, color)
{
	if (m_shadow_map)
		s_count_shadow_caster++;
	else
		s_count++;
}

DirectionalLight::DirectionalLight(DirectionalLight const& other)
	: Light(other)
{
	s_count++;
	copy_other(other);
}

DirectionalLight& DirectionalLight::operator=(DirectionalLight const& other)
{
	Light::operator=(other);
	if (this != &other)
	{
		copy_other(other);
	}
	return *this;
}

DirectionalLight::~DirectionalLight()
{
	if (m_shadow_map)
		s_count_shadow_caster--;
	else
		s_count--;
}

Matrix4x4f DirectionalLight::get_light_persp()
{
	// Create assert if m_shadow_map is nullptr

	static constexpr float SHADOW_RADIUS = ShadowMap::get_shadow_radius(); // distance from center to borders

	return Matrix4x4f::make_orthographic_matrix(
		-SHADOW_RADIUS, SHADOW_RADIUS, -SHADOW_RADIUS, SHADOW_RADIUS, 0.0f, 1000.0f);
}

Matrix4x4f DirectionalLight::get_light_view()
{
	// Create assert if m_shadow_map is nullptr

	return  Matrix4x4f::make_look_at(m_shadow_map->get_central_pos() - m_direction * 25.0f,
		m_shadow_map->get_central_pos(), Z_AXIS);
}

void DirectionalLight::imgui_renderer()
{
	Light::imgui_renderer();
	ImGui::DragFloat3("Direction", &m_direction[0], 0.05f, -1.0f, 1.0f);
	if (is_shadow_caster())
	{
		if (ImGui::CollapsingHeader("Shadows", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
			m_shadow_map->imgui_renderer();
	}
}

void DirectionalLight::set_uniform(Shader const* shader)
{
	static unsigned int count = 0;
	static unsigned int id = 0;
	static unsigned int id_shadow_caster = 0;

	count++;
	std::string light;
	if (m_shadow_map)
	{
		light = "u_shadow_caster_directional_light[" + std::to_string(id_shadow_caster) + "]";
		shader->set_uniform1i(shader->get_uniform_location("u_shadow_map[" + std::to_string(id_shadow_caster) + "]"), 3 + id_shadow_caster);
		shader->set_uniformMatrix4f(shader->get_uniform_location("u_light_view[" + std::to_string(id_shadow_caster) + "]"), get_light_view());
		shader->set_uniformMatrix4f(shader->get_uniform_location("u_light_persp[" + std::to_string(id_shadow_caster) + "]"), get_light_persp());
		m_shadow_map->bind(3 + id_shadow_caster); // + 3 because of normal map diffuse map and specular map // NEED IMPROVIMENT
		id_shadow_caster++;
	}
	else
	{
		light = "u_directional_light[" + std::to_string(id++) + "]";
	}
	
	set_general_uniform(shader, light);
	shader->set_uniform3f(shader->get_uniform_location(light + ".directional"), m_direction.get_normalized());
	if (count >= s_count)
	{
		id = 0;
		id_shadow_caster = 0;
		count = 0;
	}
}


void DirectionalLight::copy_other(DirectionalLight const& other)
{
	m_direction = other.m_direction;
	if(other.m_shadow_map)
		m_shadow_map.reset(new ShadowMap(other.m_shadow_map->get_camera_view()));
}
