#pragma once
#include "config.h"
#include <vector>
#include "glm/glm.hpp"
namespace vkMesh {

	class ObjMesh {
	public:
		std::vector<float> vertices;
		std::vector<glm::vec3> vert;
		std::vector<uint32_t> indices;
		std::vector<glm::vec3> v, vn;
		std::vector<glm::vec2> vt;
		std::unordered_map<std::string, uint32_t> history;
		std::unordered_map<std::string, glm::vec3> colorLookup;
		glm::vec3 brushColor;
		glm::mat4 preTransform;
		std::vector<float> normals;
		std::vector<float> uv;
		ObjMesh(const char* objFilepath, const char* mtlFilepath, glm::mat4 preTransform);

		void read_vertex_data(const std::vector<std::string>& words);

		void read_texcoord_data(const std::vector<std::string>& words);

		void read_normal_data(const std::vector<std::string>& words);

		void read_face_data(const std::vector<std::string>& words);

		void read_corner(const std::string& vertex_description);
	};
	std::vector<std::string> split(std::string line, std::string delimiter);
}
