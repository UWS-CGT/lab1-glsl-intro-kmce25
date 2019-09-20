#pragma once
#include <vector>
#include <GL/glew.h>
#include <SDL.h>
#include <iostream>
#include <glm/glm.hpp>

bool loadOBJ(
	const char * path,
	std::vector<glm::vec3> & out_vertices,
	std::vector<glm::vec2> & out_uvs,
	std::vector<glm::vec3> & out_normals
);