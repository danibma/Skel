#include "EditorApp.h"

namespace Skel
{
	void EditorApp::Start()
	{
		OnInit();
		SK_LOG(LogLevel::Warning, LogModule::System, "Skel Editor Initialized");
		Run();
	}

	void EditorApp::Run()
	{
		double lastTime = ImGui::GetTime();
		int nbFrames = 0;

		while (!m_window->IsClosed())
		{
			double currentTime = ImGui::GetTime();
			nbFrames++;
			if (currentTime - lastTime >= 1.0) {
				nbFrames = 0;
				lastTime += 1.0;
			
			}
			OnTick(lastTime);
		}

		OnDestroy();
	}

	void EditorApp::OnInit()
	{
		m_window = new Window(1280, 720, "Skel Engine");
		shader = new Shader("../Engine/Content/Shaders/basic.vert", "../Engine/Content/Shaders/basic.frag");
		skyboxShader = new Shader("../Engine/Content/Shaders/cubemap.vert", "../Engine/Content/Shaders/cubemap.frag");

		shader->enable();

		camera = new Camera(0.2f, m_window, shader);

		light = new DirectionalLight(shader, camera);
		glm::vec3 lightPos(0.0f, -5.0f, -2.0f);
		light->setLightDirection(lightPos);
		light->setIntensity(1.0f);

		skybox = new Skybox(skyboxShader);

		crysisModel = new Model("../Engine/Content/models/nanosuit/nanosuit.obj");
		garroshModel = new Model("../Engine/Content/models/floor.obj");
		sponzaModel = new Model("../Engine/Content/models/knob/mitsuba.obj");
		knobModel = new Model("../Engine/Content/models/knob/mitsuba.obj");

		sponza = new Entity(sponzaModel, shader);
		sponza->setPosition(-50.0f, -5.0f, 30.0f);
		sponza->setSize(4.0f, 4.0f, 4.0f);

		garrosh = new Entity(garroshModel, shader);
		garrosh->setPosition(7.0f, -9.0f, -2.0f);
		garrosh->setSize(50.0f, 4.0f, 50.0f);

		crysis = new Entity(crysisModel, shader);
		crysis->setPosition(0.0f, -5.0f, -2.0f);
		crysis->setSize(1.0f, 1.0f, 1.0f);
		crysis->setRotation(180.0f, false, true, false);

		knob = new Entity(knobModel, shader);
		knob->setPosition(0.0f, -5.0f, 30.0f);
		knob->setSize(4.0f, 4.0f, 4.0f);
#if 0
		float garroshRotationY = garrosh->getTransform().getRotation().y;
		float garroshLocationX = garrosh->getTransform().getPosition().x;
		float garroshSizeX = garrosh->getTransform().getSize().x;
		float garroshRotationX = garrosh->getTransform().getRotation().x;
		float garroshLocationY = garrosh->getTransform().getPosition().y;
		float garroshSizeY = garrosh->getTransform().getSize().y;
		float garroshRotationZ = garrosh->getTransform().getRotation().z;
		float garroshLocationZ = garrosh->getTransform().getPosition().z;
		float garroshSizeZ = garrosh->getTransform().getSize().z;
#endif

		frameBuffer = new FrameBuffer();
		// Setup Dear ImGui binding
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.Fonts->AddFontFromFileTTF("../Engine/Content/fonts/Roboto-Regular.ttf", 17.0f);
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

		ImGui_ImplGlfw_InitForOpenGL(m_window->getGLFWwindow(), true);
		ImGui_ImplOpenGL3_Init("#version 130");

		// Setup style
		ImGui::StyleColorsSkel();

		ImGui::InitDock();


		
		// Initialize Bullet. This strictly follows http://bulletphysics.org/mediawiki-1.5.8/index.php/Hello_World, 
		// even though we won't use most of this stuff.

		// Build the broadphase
		btBroadphaseInterface* broadphase = new btDbvtBroadphase();

		// Set up the collision configuration and dispatcher
		btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
		btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);

		// The actual physics solver
		btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;

		// The world.
		dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
		dynamicsWorld->setGravity(btVector3(0, -9.81f, 0));

		btCollisionShape* boxCollisionShape = new btBoxShape(btVector3(1.0f, 1.0f, 1.0f));

		glm::quat quat = glm::normalize(glm::toQuat(glm::orientate3(crysis->getTransform().getRotation())));
	
		btDefaultMotionState* motionstate = new btDefaultMotionState(btTransform(
			btQuaternion(quat.x, quat.y, quat.z, quat.w),
			btVector3(crysis->getTransform().getPosition().x, crysis->getTransform().getPosition().y, crysis->getTransform().getPosition().z)
		));

		btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(
			0,                  // mass, in kg. 0 -> Static object, will never move.
			motionstate,
			boxCollisionShape,  // collision shape of body
			btVector3(0, 0, 0)    // local inertia
		);
		btRigidBody *rigidBody = new btRigidBody(rigidBodyCI);

		double mouseX, mouseY;
		float screenWidth, screenHeight;

		SKInput::GetMousePosition(m_window, mouseX, mouseY);

		screenWidth = m_window->GetWidth();
		screenHeight = m_window->GetHeight();
		
		// The ray Start and End positions, in Normalized Device Coordinates (Have you read Tutorial 4 ?)
		glm::vec4 lRayStart_NDC(
			((float)mouseX / (float)screenWidth - 0.5f) * 2.0f, // [0,1024] -> [-1,1]
			((float)mouseY / (float)screenHeight - 0.5f) * 2.0f, // [0, 768] -> [-1,1]
			-1.0, // The near plane maps to Z=-1 in Normalized Device Coordinates
			1.0f
		);
		glm::vec4 lRayEnd_NDC(
			((float)mouseX / (float)screenWidth - 0.5f) * 2.0f,
			((float)mouseY / (float)screenHeight - 0.5f) * 2.0f,
			0.0,
			1.0f
		);

		// Faster way (just one inverse)
		glm::mat4 M = glm::inverse(camera->getProjection() * camera->getView());
		lRayStart_world = M * lRayStart_NDC; lRayStart_world/=lRayStart_world.w;
		lRayEnd_world   = M * lRayEnd_NDC  ; lRayEnd_world  /=lRayEnd_world.w;

		lRayDir_world = glm::vec3(lRayEnd_world - lRayStart_world);
		lRayDir_world = glm::normalize(lRayDir_world);


	}

	void EditorApp::OnTick(float DeltaTime)
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		//Render
		frameBuffer->bind(m_window->GetWidth(), m_window->GetHeight());
		
		GLCall(glClearColor(0.2f, 0.3f, 0.5f, 1.0f));
		GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
		GLCall(glEnable(GL_DEPTH_TEST));
		GLCall(glEnable(GL_BLEND));
		GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));


		light->update();
		skybox->Update(camera, camera->getProjection());

		shader->enable();

		//crysis->setTransform(glm::vec3(0.0f, -5.0f, -2.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), 180.0f);
		crysis->draw();

		garrosh->draw();

		sponza->draw();
		knob->draw();

		frameBuffer->unbind();
		GLCall(glClearColor(0.2f, 0.3f, 0.5f, 1.0f));
		GLCall(glClear(GL_COLOR_BUFFER_BIT));

		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Edit"))
			{
				if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
				if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
				ImGui::Separator();
				if (ImGui::MenuItem("Cut", "CTRL+X")) {}
				if (ImGui::MenuItem("Copy", "CTRL+C")) {}
				if (ImGui::MenuItem("Paste", "CTRL+V")) {}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		if (ImGui::Begin("Engine", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus))
		{
			ImGui::SetWindowPos("Engine", ImVec2(0.0f, 20.0f));
			ImGui::SetWindowSize("Engine", ImVec2(m_window->GetWidth(), m_window->GetHeight()));
			// dock layout by hard-coded or .ini file
			ImGui::BeginDockspace();
	
			if (ImGui::BeginDock("Game"), NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus)
			{
				if (m_window->IsPaused())
				{
					if (ImGui::Button("Play", ImVec2(50, 0)))
						m_window->SetGameState(PLAY);
				}
				else
				{
					if (ImGui::Button("Pause"))
						m_window->SetGameState(PAUSED);
				}
			}
			ImGui::EndDock();

			if (ImGui::BeginDock("Game")) 
			{
				size = ImGui::GetWindowSize();

				ImGui::GetWindowDrawList()->AddImage(
					(void *)frameBuffer->GetTexture(), ImVec2(ImGui::GetCursorScreenPos()),
					ImVec2((ImGui::GetCursorScreenPos().x + size.x) - 2.0f, (ImGui::GetCursorScreenPos().y + size.y) - 15.0f), ImVec2(0, 1), ImVec2(1, 0));

				/*GUIZMO*/
				//ImGuizmo::BeginFrame(ImGui::GetCursorScreenPos(), ImVec2((ImGui::GetCursorScreenPos().x + size.x) - 2.0f, (ImGui::GetCursorScreenPos().y + size.y) - 15.0f));
				ImGuizmo::SetDrawlist();

				static const float identityMatrix[16] =
				{ 1.f, 0.f, 0.f, 0.f,
					0.f, 1.f, 0.f, 0.f,
					0.f, 0.f, 1.f, 0.f,
					0.f, 0.f, 0.f, 1.f };

				//static float ng = 0.f;
				//ng += 0.01f;
				//ng = 1.f;
				//rotationY(ng, objectMatrix);
				//objectMatrix[12] = 5.f;
				// debug
				//ImGuizmo::DrawCube(glm::value_ptr(camera->getView()), glm::value_ptr(camera->getProjection()), objectMatrix);
				objectMatrix = glm::value_ptr(crysis->getTransform().getTransformMatrice());


				static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);
				static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::LOCAL);
				static bool useSnap = false;
				static float snap[3] = { 1.f, 1.f, 1.f };
				static float bounds[] = { -0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f };
				static float boundsSnap[] = { 0.1f, 0.1f, 0.1f };
				static bool boundSizing = false;
				static bool boundSizingSnap = false;

				if (SKInput::isKeyPressed(m_window, KEY_W))
					mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
				if (SKInput::isKeyPressed(m_window, KEY_E))
					mCurrentGizmoOperation = ImGuizmo::ROTATE;
				if (SKInput::isKeyPressed(m_window, KEY_R)) // r Key
					mCurrentGizmoOperation = ImGuizmo::SCALE;
				if (mCurrentGizmoOperation != ImGuizmo::SCALE)
				{
					mCurrentGizmoMode = ImGuizmo::WORLD;
				}
				else
				{
					mCurrentGizmoMode = ImGuizmo::LOCAL;
				}

				ImGuiIO& io = ImGui::GetIO();
				ImGuizmo::SetRect(ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y, (ImGui::GetCursorScreenPos().x + size.x) - 2.0f, (ImGui::GetCursorScreenPos().y + size.y) - 15.0f);
				ImGuizmo::Manipulate(glm::value_ptr(camera->getView()), glm::value_ptr(camera->getProjection()), mCurrentGizmoOperation, mCurrentGizmoMode, objectMatrix, NULL, useSnap ? &snap[0] : NULL, boundSizing ? bounds : NULL, boundSizingSnap ? boundsSnap : NULL);

				crysis->SetTransformMatrix(glm::make_mat4(objectMatrix));

			}
			ImGui::EndDock();

			if (ImGui::BeginDock("Debug")) 
			{
				ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			}
			ImGui::EndDock();

			if (ImGui::BeginDock("Transform")) 
			{
				ImGui::Text("TODO");
			}
			ImGui::EndDock();

			if (ImGui::BeginDock("Project Settings"))
			{
			
				ImGui::InputText("Project Name", name, sizeof(name));
				ImGui::Text(path.c_str());
				ImGui::SameLine();
				if (ImGui::Button("Choose Directory"))
					dialog = true;
				if (ImGui::Button("Create Project"))
				{
					BuildTool::CreateProject(name, path.c_str());
				}
				if (ImGui::Button("Load Project"))
				{
					//First user will locate the .skproject file
					//Load the project DLL
				}

			}
			ImGui::EndDock();

			ImGui::EndDockspace();
		}	
		ImGui::End();

		if (dialog)
		{
			if (ImGuiFileDialog::Instance()->FileDialog("Choose Directory", ".cpp\0.h\0.hpp\0\0", ".", ""))
			{
				if (ImGuiFileDialog::Instance()->IsOk == true)
				{
					path = ImGuiFileDialog::Instance()->GetCurrentPath();
				}
				else
				{
					path = "";
				}
				dialog = false;
			}
		}
	

		/*Bullet*/
		if (SKInput::isMouseButtonPressed(m_window, MOUSE_BUTTON_LEFT))
		{
			glm::vec3 out_origin;
			glm::vec3 out_direction;

			out_origin = glm::vec3(lRayStart_world);
			out_direction = glm::normalize(lRayDir_world);

			glm::vec3 out_end = out_origin + out_direction * 1000.0f;

			btCollisionWorld::ClosestRayResultCallback RayCallback(
				btVector3(out_origin.x, out_origin.y, out_origin.z),
				btVector3(out_end.x, out_end.y, out_end.z)
			);
			dynamicsWorld->rayTest(
				btVector3(out_origin.x, out_origin.y, out_origin.z),
				btVector3(out_end.x, out_end.y, out_end.z),
				RayCallback
			);

			std::string message;
			

			DebugDrawer debugDrawer = DebugDrawer();
			debugDrawer.setDebugMode(btIDebugDraw::DBG_DrawWireframe); 
			debugDrawer.setDebugMode(1); 

			dynamicsWorld->setDebugDrawer(&debugDrawer);

			dynamicsWorld->getDebugDrawer()->drawLine(btVector3(out_origin.x, out_origin.y, out_origin.z), btVector3(out_end.x, out_end.y, out_end.z), btVector3(1.0f, 0.0f, 0.0f));

			if (RayCallback.hasHit()) {
				std::ostringstream oss;
				oss << "mesh " << (int)RayCallback.m_collisionObject->getUserPointer();
				message = oss.str();
				SK_LOGP(Error, System, "%s", message.c_str());
			}
			else {
				message = "background";
			}


		}
		
		/*Bullet*/


		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


		//Render
		if (!m_window->IsPaused())
			camera->update();

			
		m_window->Update();
	}

	void EditorApp::OnDestroy()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}
}

#include <json.hpp>
#include <iomanip>

using namespace Skel;
using namespace nlohmann;

int main()
{
	EditorApp app;
	app.Start();
}