#include "View/vkMesh/obj_mesh.h"
#include <string>


vkMesh::ObjMesh::ObjMesh(const char* objFilepath, const char* mtlFilepath, glm::mat4 preTransform) {

	this->preTransform = preTransform;

	std::ifstream file;
	file.open(mtlFilepath);
	std::string line;
	std::string materialName;
	std::vector<std::string> words;

	while (std::getline(file, line)) {

		words = split(line, " ");

		if (!words[0].compare("newmtl")) {
			materialName = words[1];
		}
		if (!words[0].compare("Kd")) {
			brushColor = glm::vec3(std::stof(words[1]), std::stof(words[2]), std::stof(words[3]));
			colorLookup.insert({ materialName,brushColor });
		}
	}

	file.close();

	file.open(objFilepath);

	while (std::getline(file, line)) {
		words = split(line, " ");

		if (!words[0].compare("v")) {
			read_vertex_data(words);
		}
		if (!words[0].compare("vt")) {
			read_texcoord_data(words);
		}
		if (!words[0].compare("vn")) {
			read_normal_data(words);
		}
		if (!words[0].compare("usemtl")) {
			if (colorLookup.contains(words[1])) {
				brushColor = colorLookup[words[1]];
			}
			else {
				brushColor = glm::vec3(1.0f);
			}
		}
		if (!words[0].compare("f")) {
			read_face_data(words);
		}
	}

	file.close();
}

void vkMesh::ObjMesh::read_vertex_data(const std::vector<std::string>& words) {
	glm::vec4 new_vertex = glm::vec4(std::stof(words[1]), std::stof(words[2]), std::stof(words[3]), 1.0f);
	glm::vec3 transformed_vertex = glm::vec3(preTransform * new_vertex);
	v.push_back(transformed_vertex);
}

void vkMesh::ObjMesh::read_texcoord_data(const std::vector<std::string>& words) {
	glm::vec2 new_texcoord = glm::vec2(std::stof(words[1]), std::stof(words[2]));
	vt.push_back(new_texcoord);
}

void vkMesh::ObjMesh::read_normal_data(const std::vector<std::string>& words) {
	glm::vec4 new_normal = glm::vec4(std::stof(words[1]), std::stof(words[2]), std::stof(words[3]), 0.0f);
	glm::vec3 transformed_normal = glm::vec3(preTransform * new_normal);
	vn.push_back(transformed_normal);
}

void vkMesh::ObjMesh::read_face_data(const std::vector<std::string>& words) {
	
	size_t triangleCount = words.size() - 3;

	for (int i = 0; i < triangleCount; ++i) {
		glm::vec3 tangent = readBtangent(words[1], words[2 + i], words[3 + i]);
		read_corner(words[1], tangent);
		read_corner(words[2 + i], tangent);
		read_corner(words[3 + i], tangent);
	}
}

glm::vec3 vkMesh::ObjMesh::readBtangent(const std::string& first_vertex_description, const std::string& seccond_vertex_description, const std::string& third_vertex_description)
{
	std::vector<std::string> firstVertexDescription = split(first_vertex_description, "/");
	glm::vec3 firstPos = v[std::stol(firstVertexDescription[0]) - 1];;

	glm::vec2 firstTexcoord = glm::vec2(0.0f, 0.0f);
	if (firstVertexDescription.size() == 3 && firstVertexDescription[1].size() > 0) {
		firstTexcoord = vt[std::stol(firstVertexDescription[1]) - 1];
	}

	std::vector<std::string> seccondVertexDescription = split(seccond_vertex_description, "/");
	glm::vec3 seccondPos = v[std::stol(seccondVertexDescription[0]) - 1];;

	glm::vec2 seccondTexcoord = glm::vec2(0.0f, 0.0f);
	if (seccondVertexDescription.size() == 3 && seccondVertexDescription[1].size() > 0) {
		seccondTexcoord = vt[std::stol(seccondVertexDescription[1]) - 1];
	}

	std::vector<std::string> thirdVertexDescription = split(third_vertex_description, "/");
	glm::vec3 thirdPos = v[std::stol(thirdVertexDescription[0]) - 1];;

	glm::vec2 thirdTexcoord = glm::vec2(0.0f, 0.0f);
	if (thirdVertexDescription.size() == 3 && thirdVertexDescription[1].size() > 0) {
		thirdTexcoord = vt[std::stol(thirdVertexDescription[1]) - 1];
	}


	glm::vec3 edge1 = seccondPos - firstPos;
	glm::vec3 edge2 = thirdPos - firstPos;

	glm::vec2 deltaUV1 = seccondTexcoord - firstTexcoord;
	glm::vec2 deltaUV2 = thirdTexcoord - firstTexcoord;

	float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

	glm::vec3 tangent;
	tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
	tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
	tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);



	tangent = tangent;
	return tangent;
}

void vkMesh::ObjMesh::read_corner(const std::string& vertex_description, glm::vec3 tangent) {

	if (history.contains(vertex_description)) {
		indices.push_back(history[vertex_description]);
		return;
	}

	uint32_t index = static_cast<uint32_t>(history.size());
	history.insert({ vertex_description, index });
	indices.push_back(index);


	std::vector<std::string> v_vt_vn = split(vertex_description, "/");

	//position
	glm::vec3 pos = v[std::stol(v_vt_vn[0]) - 1];
	vertices.push_back(pos[0]);
	vertices.push_back(pos[1]);
	vertices.push_back(pos[2]);

	vert.push_back(glm::vec3(pos[0],pos[1],pos[2]));

	glm::vec3 normal = vn[std::stol(v_vt_vn[2]) - 1];

	normals.push_back(normal.x);
	normals.push_back(normal.y);
	normals.push_back(normal.z);


	//texcoord
	glm::vec2 texcoord = glm::vec2(0.0f, 0.0f);
	if (v_vt_vn.size() == 3 && v_vt_vn[1].size() > 0) {
		texcoord = vt[std::stol(v_vt_vn[1]) - 1];
	}
	uv.push_back(texcoord.x);
	uv.push_back(texcoord.y);
	
	tangents.push_back(tangent.x);
	tangents.push_back(tangent.y);
	tangents.push_back(tangent.z);
	
}

std::vector<std::string> vkMesh::split(std::string line, std::string delimiter)
{
	std::vector<std::string> split_line;

	size_t pos = 0;
	std::string token;
	while ((pos = line.find(delimiter)) != std::string::npos)
	{
		token = line.substr(0, pos);
		split_line.push_back(token);
		line.erase(0, pos + delimiter.length());
	}
	split_line.push_back(line);
	return split_line;
}

