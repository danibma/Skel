/*
 * @module Graphics
 * @project Skel Engine(https://github.com/dani9bma/SkelEngine)
 * @author Daniel Assun��o
 * @Github https://github.com/dani9bma
 */

#pragma once

#include <GL/glew.h>
#include <iostream>
#include "shader.h"
#include "stb_image.h"

namespace Skel { namespace graphics {

	class Texture
	{
	public:
		Texture(const char* path, Shader shader);
		~Texture();
		void draw();
	private:
		unsigned int m_textureID;
	};

} }