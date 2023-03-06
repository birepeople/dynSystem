
#include "PointVisualModule.h"
#include "Utility.h"

#include "../Engine/OpenGL/backend/Vulkan/VulkanBuffer.h"
#include <glad/glad.h>

namespace dyno
{
	PointVisualModule::PointVisualModule()
	{
		mNumPoints = 0;
		this->setName("point_renderer");
		// buffers
		mVertexBuffer = new gl::VulkanBuffer;
		mColorBuffer = new gl::VulkanBuffer;
		mInstanceBuffer = new gl::VulkanBuffer;
	}

	PointVisualModule::~PointVisualModule()
	{
		// TODO: release resource within OpenGL context
		delete mVertexBuffer;
		delete mColorBuffer;
		delete mInstanceBuffer;
	}

	std::string PointVisualModule::caption()
	{
		return "Point Visual Module";
	}

	void  PointVisualModule::setColorMapMode(ColorMapMode mode) {
		mColorMode = mode;
	}


	bool PointVisualModule::initializeGL()
	{
		// create vertex buffer and vertex array object
		mVAO.create();

		mVertexBuffer->create(GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW);
		mColorBuffer->create(GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW);
		
		mVAO.bindVertexBuffer(mVertexBuffer, 0, 4, GL_FLOAT, 0, 0, 0);
		mVAO.bindVertexBuffer(mColorBuffer, 1, 3, GL_FLOAT, 0, 0, 0);
	

		// create transform buffer for instances, we should bind it to VAO later if necessary
		mInstanceBuffer->create(GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW);

		// create shader program
		mShaderProgram = gl::ShaderFactory::createShaderProgram("point.vert", "point.frag");

		gl::glCheckError();

		return true;

	}

	
	void PointVisualModule::updateGL()
	{
		
		auto& xyz = this->inPosition()->getData();
		mNumPoints = xyz.size() * 3;
		printf("%d\n", mNumPoints);
		mVertexBuffer->load(xyz.buffer(), xyz.bufferSize());


		mVAO.bind();

		glDisableVertexAttribArray(1);
		gl::glCheckError();
		
		mVAO.unbind();
		
	}

	void PointVisualModule::paintGL(GLRenderPass pass)
	{
		if (mNumPoints == 0)
			return;

		mShaderProgram->use();
		mShaderProgram->setFloat("uPointSize", this->varPointSize()->getData());

		unsigned int subroutine;
		if (pass == GLRenderPass::COLOR)
		{
			mShaderProgram->setFloat("uMetallic", this->varMetallic()->getData());
			mShaderProgram->setFloat("uRoughness", this->varRoughness()->getData());
			mShaderProgram->setFloat("uAlpha", this->varAlpha()->getData());

			subroutine = 0;
			glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &subroutine);
		}
		else if (pass == GLRenderPass::SHADOW)
		{
			subroutine = 1;
			glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &subroutine);
		}
		else if (pass == GLRenderPass::TRANSPARENCY)
		{
			printf("WARNING: PointVisualModule does not support transparency!\n");
			return;
		}
		else
		{
			printf("Unknown render pass!\n");
			return;
		}

		// per-object color color
		auto color = this->varBaseColor()->getData();
		glVertexAttrib3f(1, color[0], color[1], color[2]);

		mVAO.bind();
		glDrawArrays(GL_POINTS, 0, mNumPoints);
		gl::glCheckError();
	}
}