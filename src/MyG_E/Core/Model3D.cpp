// Heap file.
#include "Model3D.h"

// Intern files.
#include "Foundation\Math\Quaternion.h"
#include "Foundation/UI/FileBrowser.h"

// Third Parties.
#include "imgui.h"

unsigned int Model3D::m_number_of_objects = 0;

Model3D::Model3D(Mesh* mesh)
	: m_position({ 0.0f, 0.0f, 0.0f })
	, m_trans_matrix(0.0f, 0.0f, 0.0f)
	, m_scale_matrix(1.0f, 1.0f, 1.0f)
	, m_scale({ 1.0f, 1.0f, 1.0f })
	, m_rotation_matrix(Matrix<float, 4, 4>::make_identity())
	, m_mesh(new Mesh(*mesh))
	, m_is_visible(true)
{
	m_object_name = "Object " + std::to_string(m_number_of_objects);
	m_number_of_objects++;
}

Model3D::Model3D(Mesh* mesh, std::string& const name)
	: m_position({ 0.0f, 0.0f, 0.0f }),
	m_trans_matrix(0.0f, 0.0f, 0.0f),
	m_scale_matrix(1.0f, 1.0f, 1.0f),
	m_scale({1.0f, 1.0f, 1.0f}),
	m_rotation_matrix(),
	m_mesh(new Mesh(*mesh)),
	m_object_name(name),
	m_is_visible(true)
{
	m_number_of_objects++;
}

Model3D::Model3D(Model3D const& other)
	:m_trans_matrix(0.0f, 0.0f, 0.0f),
	m_scale_matrix(1.0f, 1.0f, 1.0f),
	m_rotation_matrix()
{
	copy_other(other);
}

Model3D& Model3D::operator=(Model3D const& other)
{
	if (this != &other)
	{
		delete m_mesh;
		copy_other(other);
	}
	return *this;
}

Model3D::~Model3D()
{
	delete m_mesh;
}

void Model3D::set_translation(Vector<float, 3> const& trans)
{
	m_position = trans;
	m_trans_matrix.SetTranX(m_position[0]);
	m_trans_matrix.SetTranY(m_position[1]);
	m_trans_matrix.SetTranZ(m_position[2]);
}

void Model3D::set_scale(Vector<float, 3> const& scale)
{
	m_scale = scale;
	m_scale_matrix.SetScaleX(m_scale[0]);
	m_scale_matrix.SetScaleY(m_scale[1]);
	m_scale_matrix.SetScaleZ(m_scale[2]);
}

void Model3D::set_rotation(Vector<float, 3> const& rotation)
{
	m_rotate = rotation;
	Quaternion quatx = Quaternion::MakeRotate(m_rotate[0], { 1.0f, 0.0f, 0.0f });
	Quaternion quaty = Quaternion::MakeRotate(m_rotate[1], { 0.0f, 1.0f, 0.0f });
	Quaternion quatz = Quaternion::MakeRotate(m_rotate[2], { 0.0f, 0.0f, 1.0f });
	Quaternion quat = quatx * quaty * quatz;

	m_rotation_matrix = Quaternion::CreateRotationMatrix(quat);
}

void Model3D::set_material(Vector<float, 3>const& diffuse, Vector<float, 3> const& specular, float shininess)
{
	m_material.diffuse = Texture(diffuse);
	m_material.specular = specular;
	m_material.shininess = shininess;
}

void Model3D::set_material(std::string const& filepath, Vector<float, 3> const& specular, float shininess)
{
	m_material.diffuse = Texture(filepath);
	m_material.specular = specular;
	m_material.shininess = shininess;
}


void Model3D::ImGuiRenderer()
{
	ImGui::DragFloat3("Translate", &m_position[0], 0.1f);
	ImGui::DragFloat3("Scale", &m_scale[0], 0.1f, 0.0f, 100.0f);
	ImGui::SliderFloat3("Rotate", &m_rotate[0], -6.28f, 6.28f);
	ImGui::Separator();

	if (ImGui::Button("import texture"))
	{
		m_material.diffuse.change_texture(open_file_browser("(*.png) Project File\0*.png\0"));
	}
	ImGui::Text("Current texture: %s", m_material.diffuse.get_filepath().c_str());
	
	float const* color = m_material.diffuse.get_color();
	if (m_material.diffuse.is_unitary())
	{
		Vector<float, 3> diffuse{ color[0], color[1], color[2] };
		ImGui::ColorEdit3("Diffuse", &diffuse[0]);
		m_material.diffuse.change_texture(diffuse);
	}
	else
		if (ImGui::Button("remove texture"))
			m_material.diffuse.change_texture(Vector<float, 3>{});

	ImGui::ColorEdit3("Specular", &m_material.specular[0]);
	ImGui::DragFloat("shininess", &m_material.shininess, 0.05f, 0.0f, 1.0f);
	ImGui::Separator();
	
	ImGui::Checkbox("Visibility", &m_is_visible);

	set_translation(m_position);
	set_scale(m_scale);
	set_rotation(m_rotate);
}

void Model3D::copy_other(Model3D const& other)
{
	m_mesh = new Mesh(*other.m_mesh);
	m_material = other.m_material;

	//matrices
	set_translation(other.m_position);
	set_scale(other.m_scale);
	set_rotation(other.m_rotate);

	m_object_name = other.m_object_name;
	m_is_visible = other.m_is_visible;
	
	m_number_of_objects++;
}

