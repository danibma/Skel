/*
 * @module Graphics
 * @project Skel Engine(https://github.com/dani9bma/SkelEngine)
 * @author Daniel Assun��o
 * @Github https://github.com/dani9bma
 */

#pragma once

#include "../camera/camera.h"
#include "../opengl/shader.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Skel { namespace graphics {

	class DirectionalLight
	{
	public:
		DirectionalLight(Shader* shader, Camera camera);
		~DirectionalLight();
		void update();
		void setLightDirection(glm::vec3 direction);
		void setIntensity(float value);
	private:
		Shader* m_shader;
		Camera m_camera;
		glm::vec3 m_direction = glm::vec3(1.0f, 1.0f, 1.0f);
		glm::vec3 m_intensity = glm::vec3(1.0f, 1.0f, 1.0f);
	};
} }