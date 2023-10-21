#include <Dragonfly/editor.h>		 //inlcludes most features
#include <Dragonfly/detail/buffer.h> //will be replaced
#include <Dragonfly/detail/vao.h>	 //will be replaced

#include <glm/glm.hpp>

#include <Dragonfly/detail/Buffer/Buffer.h>

#include <iostream>

#include "application.h"

void test1();
void test_transforms();
void raytrace_demo();

int main(int argc, char* args[])
{
	//main2(argc,args);
	//test_transforms();
	raytrace_demo();
	return 0;
}

void raytrace_demo()
{
	glm::vec2 iResolution{ 1024,768 };
	df::Sample sam("Ray Tracing Demo", iResolution.x, iResolution.y, df::Sample::FLAGS::DEFAULT);
	// df::Sample simplifies OpenGL, SDL, ImGui, RenderDoc in the render loop, and handles user input via callback member functions in priority queues
	df::Camera cam;								// Implements a camera event class with handles
	sam.AddHandlerClass(cam, 5);				// class callbacks will be called to change its state
	sam.AddHandlerClass<df::ImGuiHandler>(10);	// static handle functions only

	eltecg::ogl::ArrayBuffer MyVBO;	MyVBO.constructMutable(std::vector<glm::vec2>{ {-1, -1}, { 3, -1 }, { -1, 3 }}, GL_STATIC_DRAW);
	eltecg::ogl::VertexArray MyVAO;	MyVAO.addVBO<glm::vec2>(MyVBO);
	df::VaoArrays demoVao((GLuint)MyVAO, GL_TRIANGLE_STRIP, 3, 0u); // temporary solution that wraps an ID

	df::TextureCube<> testCubemap("Assets/xpos.png", "Assets/xneg.png", "Assets/ypos.png", "Assets/yneg.png", "Assets/zpos.png", "Assets/zneg.png");
	df::Texture2D<> testTex = testCubemap[df::X_POS]; // 2D view of a cubemap face

	df::ShaderProgramEditorVF raytraceProgram = "Ray tracing demo shader program";
	raytraceProgram << "Shaders/raytrace.vert"_vert << "Shaders/raytrace.frag"_frag << df::LinkProgram;

	GL_CHECK; //extra opengl error checking in GPU Debug build configuration

	cam.SetSpeed(10.0);
	//cam.LookAt(glm::vec3(12, 8, 8));
	cam.LookAt(glm::vec3(4, 4, -3));

	auto begin = std::chrono::high_resolution_clock::now();

	Application_Initialize();

	sam.Run([&](float deltaTime) //delta time in ms
		{
			cam.Update();

			auto end = std::chrono::high_resolution_clock::now();
			auto dur = end - begin;
			float ms = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count() / 1000.0;

			//if (ms < 7.)
			Application_Frame(ms);

			df::Backbuffer << df::Clear() << raytraceProgram << "iResolution" << iResolution
				<< "iTime" << ms
				<< "texImg" << testTex
				<< "cameraAt" << cam.GetAt()
				<< "cameraPos" << cam.GetEye()
				<< "cameraView" << cam.GetView();
			//raytraceProgram << df::NoVao(GL_TRIANGLES, 3); // Rendering a pixel shader
			raytraceProgram << demoVao;

			GL_CHECK;
			raytraceProgram.Render();
		}
	);

	Application_Finalize();
}

void test_transforms()
{
	df::Sample sam("Dragonfly Demo", 1024, 768, df::Sample::FLAGS::DEFAULT);
	// df::Sample simplifies OpenGL, SDL, ImGui, RenderDoc in the render loop, and handles user input via callback member functions in priority queues
	df::Camera cam;								// Implements a camera event class with handles
	sam.AddHandlerClass(cam, 5);				// class callbacks will be called to change its state
	sam.AddHandlerClass<df::ImGuiHandler>(10);	// static handle functions only

	eltecg::ogl::ArrayBuffer MyVBO;	MyVBO.constructMutable(std::vector<glm::vec2>{ {-1, -1}, { 1, -1 }, { 0, 1 }}, GL_STATIC_DRAW);
	eltecg::ogl::VertexArray MyVAO;	MyVAO.addVBO<glm::vec2>(MyVBO);
	df::VaoArrays demoVao((GLuint)MyVAO, GL_TRIANGLE_STRIP, 3, 0u); // temporary solution that wraps an ID

	df::ShaderProgramEditorVF testProgram = "Transformations-test shader program";
	testProgram << "Shaders/test_transforms.vert"_vert << "Shaders/test_transforms.frag"_frag << df::LinkProgram;

	GL_CHECK; //extra opengl error checking in GPU Debug build configuration

	sam.Run([&](float deltaTime) //delta time in ms
		{
			cam.Update();

			glm::mat4 mvp;
			df::Backbuffer << df::Clear() << testProgram << "mvp" << mvp;
			testProgram << demoVao; // Rendering a pixel shader

			GL_CHECK;
			testProgram.Render();
		}
	);
}

void test1()
{
	df::Sample sam("Dragonfly Demo", 1024, 768, df::Sample::FLAGS::DEFAULT);
	// df::Sample simplifies OpenGL, SDL, ImGui, RenderDoc in the render loop, and handles user input via callback member functions in priority queues
	df::Camera cam;								// Implements a camera event class with handles
	sam.AddHandlerClass(cam, 5);				// class callbacks will be called to change its state
	sam.AddHandlerClass<df::ImGuiHandler>(10);	// static handle functions only

	eltecg::ogl::ArrayBuffer MyVBO;	MyVBO.constructMutable(std::vector<glm::vec2>{ {-1, -1}, { 1, -1 }, { 0, 1 }}, GL_STATIC_DRAW);
	eltecg::ogl::VertexArray MyVAO;	MyVAO.addVBO<glm::vec2>(MyVBO);
	df::VaoArrays demoVao((GLuint)MyVAO, GL_TRIANGLE_STRIP, 3, 0u); // temporary solution that wraps an ID

	df::ShaderProgramEditorVF testProgram = "Test shader program";
	testProgram << "Shaders/test.vert"_vert << "Shaders/test.frag"_frag << df::LinkProgram;

	GL_CHECK; //extra opengl error checking in GPU Debug build configuration

	sam.Run([&](float deltaTime) //delta time in ms
		{


			cam.Update();
			df::Backbuffer << df::Clear() << testProgram;
			testProgram << demoVao; // Rendering a pixel shader

			GL_CHECK;
			testProgram.Render();
		}
	);
}


int main2(int argc, char* args[])
{
	df::Sample sam("Dragonfly Demo", 720, 480, df::Sample::FLAGS::DEFAULT);
	// df::Sample simplifies OpenGL, SDL, ImGui, RenderDoc in the render loop, and handles user input via callback member functions in priority queues
	df::Camera cam;								// Implements a camera event class with handles
	sam.AddHandlerClass(cam, 5);				// class callbacks will be called to change its state
	sam.AddHandlerClass<df::ImGuiHandler>(10);	// static handle functions only

	eltecg::ogl::ArrayBuffer MyVBO;	MyVBO.constructMutable(std::vector<glm::vec2>{ {-1, -1}, { 1, -1 }, { 0, 1 }}, GL_STATIC_DRAW);
	eltecg::ogl::VertexArray MyVAO;	MyVAO.addVBO<glm::vec2>(MyVBO);		//these two classes will be removed from Dragonfly as soon as we have the replacement ready

	df::VaoArrays demoVao((GLuint)MyVAO, GL_TRIANGLE_STRIP, 3, 0u); // temporary solution that wraps an ID

	df::TextureCube<> testCubemap("Assets/xpos.png", "Assets/xneg.png", "Assets/ypos.png", "Assets/yneg.png", "Assets/zpos.png", "Assets/zneg.png");
	df::Texture2D<> testTex = testCubemap[df::X_POS]; // 2D view of a cubemap face

	df::ShaderProgramEditorVF program = "MyShaderProgram";
	program << "Shaders/vert.vert"_vert << "Shaders/frag.frag"_frag << df::LinkProgram;
	df::ShaderProgramEditorVF postprocess = "Postprocess shader program";
	postprocess << "Shaders/postprocess.vert"_vert << "Shaders/postprocess.frag"_frag << df::LinkProgram;

	int w = df::Backbuffer.getWidth(), h = df::Backbuffer.getHeight();
	auto frameBuff = df::Renderbuffer<df::depth24>(w, h) + df::Texture2D<>(w, h, 1);

	sam.AddResize([&](int w, int h) {frameBuff = frameBuff.MakeResized(w, h); });

	GL_CHECK; //extra opengl error checking in GPU Debug build configuration

	sam.Run([&](float deltaTime) //delta time in ms
		{
			cam.Update();

			frameBuff << df::Clear() << program << "texImg" << testTex;
			program << demoVao;	//Rendering: Ensures that both the vao and program is attached

			df::Backbuffer << df::Clear() << postprocess << "texFrame" << frameBuff.get<glm::u8vec3>();
			postprocess << df::NoVao(GL_TRIANGLES, 3); // Rendering a pixel shader

			GL_CHECK;
			program.Render(); postprocess.Render(); //only the UI!!
		}
	);
	return 0;
}
