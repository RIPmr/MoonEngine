#include "PreInclude.h"

//#define MOON_DEBUG_MODE

// global settings
const char *title = "MoonEngine - v0.01 WIP";
Vector2 MOON_WndSize = Vector2(1600, 900);
Vector2 MOON_OutputSize = Vector2(200, 100);
ImVec4 clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

// global parameters
Matrix4x4 view;
Matrix4x4 projection;
ImVec4 grdLineColor = ImVec4(0.8f, 0.8f, 0.8f, 1.0f);
unsigned int Renderer::samplingRate = 5;
unsigned int Renderer::maxReflectionDepth = 5;

Vector3 movePos = Vector3::ZERO();


int main() {
	std::cout << "starting moon engine... ..." << std::endl;

	// glfw window creation
	GLFWwindow* window = InitWnd();
	if (window == NULL) return -1;

	// configure global opengl state
	glEnable(GL_DEPTH_TEST);

	// engine resources initialization
	if (!MOON_TextureManager::LoadImagesForUI()) return -1;
	std::cout << "- Images For UI Loaded." << std::endl;
	MOON_ShaderManager::LoadDefaultShaders();
	std::cout << "- Default Shaders Loaded." << std::endl;
	MOON_MaterialManager::PrepareMatBall();
	std::cout << "- Material Ball Created." << std::endl;
	MOON_MaterialManager::CreateDefaultMats();
	std::cout << "- Default Materials Created." << std::endl;
	MOON_CameraManager::LoadSceneCamera();
	std::cout << "- Scene Camera Created." << std::endl;
	std::cout << "Finished." << std::endl;

	// scene objects
	Model* teapot = MOON_ModelManager::LoadModel("Resources/box_stack.obj");

	// render loop
	while (!glfwWindowShouldClose(window)) {
		// per-frame time logic
		MOON_UpdateClock();

		// clear background
		glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClear(GL_COLOR_BUFFER_BIT);

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// update output image realtime while rendering
		if (Renderer::progress != 0) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, MOON_OutputSize.x, MOON_OutputSize.y, 0, GL_RGB, GL_UNSIGNED_BYTE, Renderer::outputImage);
			if (Renderer::progress < 0) {
				Renderer::progress = 0;
				glBindTexture(GL_TEXTURE_2D, 0);
			}
		}

		{ // UI window
			if (ImGui::BeginMainMenuBar()) MainUI::MainMenu();
			if (MainUI::show_control_window) MainUI::ControlPanel(MainUI::io, clearColor);
			if (MainUI::show_preference_window) MainUI::PreferencesWnd();
			if (MainUI::show_VFB_window) MainUI::ShowVFB();
			if (MainUI::show_demo_window) ImGui::ShowDemoWindow(&MainUI::show_demo_window);
			if (MainUI::show_about_window) MainUI::AboutWnd();
			if (MainUI::show_scene_window) MainUI::SceneWnd();
			if (MainUI::show_inspector_window) MainUI::InspectorWnd();
			if (MainUI::show_explorer_window) MainUI::ExplorerWnd();
			if (MainUI::show_console_window) MainUI::ConsoleWnd();
			if (MainUI::show_project_window) MainUI::ProjectWnd();
			if (MainUI::show_create_window) MainUI::CreateWnd();
		}

		// calculate matrix
		view = MOON_SceneCamera->GetViewMatrix();
		projection = Matrix4x4::Perspective(MOON_SceneCamera->fov, MOON_WndSize.x / MOON_WndSize.y, 0.1f, 100.0f);

		// Rendering objects
		DrawGround(1.0, 5, SceneManager::ShaderManager::GetItem("ConstColor"));

		Shader* ts = SceneManager::ShaderManager::GetItem("SimplePhong");
		ts->use();
		Matrix4x4 move = Matrix4x4::TranslateMat(movePos);
		Matrix4x4 model = Matrix4x4::Scale(move, 1.0);

		ts->setMat4("model", model);
		ts->setMat4("view", view);
		ts->setMat4("projection", projection);

		ts->setVec3("lightColor", Vector3(1.0, 1.0, 1.0));
		ts->setVec3("objectColor", Vector3(0.8f, 0.6f, 0.2f));
		ts->setVec3("lightPos", MOON_SceneCamera->transform.position);
		ts->setVec3("viewPos", MOON_SceneCamera->transform.position);

		teapot->Draw();

		// process user input
		MOON_InputManager::isHoverUI = MainUI::io->WantCaptureMouse;
		MOON_InputProcessor(window);

		// Rendering UI
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	MOON_CleanUp();

	// glfw: terminate, clearing all previously allocated GLFW resources.
	glfwTerminate();
	return 0;
}

// glfw: initialize and configure
GLFWwindow* InitWnd() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(MOON_WndSize.x, MOON_WndSize.y, title, NULL, NULL);

	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		window = NULL;
	}
	glfwMakeContextCurrent(window);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// glad: load all OpenGL function pointers
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		window = NULL;
	}

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	// Setup Dear ImGui style
	ImGui::StyleBlack();
	//ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	// glsl_version can be replaced with string "#version 150"
	ImGui_ImplOpenGL3_Init("#version 150");

	MainUI::io = &ImGui::GetIO();
	auto fonts = MainUI::io->Fonts;
	fonts->AddFontFromFileTTF(
		"./Resources/msyh.ttc",
		16.0f,
		NULL,
		fonts->GetGlyphRangesChineseSimplifiedCommon()
	);
	MainUI::io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	MainUI::io->ConfigWindowsMoveFromTitleBarOnly = true;

	return window;
}

// glfw callbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (MOON_MousePos[0] == -2.0f) MOON_MousePos.setValue(xpos, ypos);

	float xoffset = xpos - MOON_MousePos.x;
	float yoffset = MOON_MousePos.y - ypos;

	MOON_MousePos.setValue(xpos, ypos);
	MOON_InputManager::mouseOffset.setValue(xoffset, yoffset);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	MOON_InputManager::mouseButton = button;
	MOON_InputManager::mouseAction = action;
	MOON_InputManager::mouseMods = mods;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	MOON_InputManager::mouseScrollOffset.setValue(xoffset, yoffset);
}

// moon: clean up, input processor and draw commands
void MOON_CleanUp() {
	MOON_LightManager::Clear();
	MOON_MaterialManager::Clear();
	MOON_ShaderManager::Clear();
	MOON_TextureManager::Clear();
	MOON_ModelManager::Clear();
	MOON_CameraManager::Clear();
	MOON_InputManager::Clear();
	SceneManager::Clear();

	delete MOON_MaterialManager::matBall;
	delete MOON_SceneCamera;
}

Vector3 unProjectMouse() {
	if (NULL == MOON_SceneCamera) {
		std::cout << "camera failed! failed to un-project mouse" << std::endl;
	} else {
		GLfloat winZ;
		Vector3 screenPos(MOON_MousePos.x, MOON_WndSize.y - MOON_MousePos.y - 1, 0.0f);

		//glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glReadPixels(screenPos.x, screenPos.y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		screenPos.z = winZ;

		Vector4 viewport = Vector4(0.0f, 0.0f, MOON_WndSize.x, MOON_WndSize.y);
		Vector3 worldPos = Matrix4x4::UnProject(screenPos, view, projection, viewport);
		//movePos.setValue(worldPos.x, worldPos.y, worldPos.z);

		return worldPos;
	}
}

void MOON_UpdateClock() {
	float currentFrame = glfwGetTime();
	MOON_Clock::deltaTime = currentFrame - MOON_Clock::lastFrame;
	MOON_Clock::lastFrame = currentFrame;
}

void DrawGround(const float &space, const int &gridCnt, const Shader* groundShader) {
	std::vector<float> grid;

	// configure shader
	groundShader->use();
	groundShader->setVec4("lineColor", grdLineColor);
	groundShader->setMat4("model", Matrix4x4());
	groundShader->setMat4("view", view);
	groundShader->setMat4("projection", projection);

	// generate grid points
	for (int i = -gridCnt; i <= gridCnt; i++) {
		grid.push_back(i * space);
		grid.push_back(0.0);
		grid.push_back(gridCnt * -space);

		grid.push_back(i * space);
		grid.push_back(0.0);
		grid.push_back(gridCnt * space);

		grid.push_back(gridCnt * -space);
		grid.push_back(0.0);
		grid.push_back(i * space);

		grid.push_back(gridCnt * space);
		grid.push_back(0.0);
		grid.push_back(i * space);
	}

	// enable color blend and anti-aliasing
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	// vertex array object
	unsigned int VAO;
	glGenVertexArrays(1, &VAO);
	// vertex buffer object
	unsigned int VBO;
	glGenBuffers(1, &VBO);
	// bind buffers
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// line width
	glLineWidth(1.0);
	// copy data
	glBufferData(GL_ARRAY_BUFFER, grid.size() * sizeof(float), &grid[0], GL_STATIC_DRAW);
	// vertex data format
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);
	// unbind buffers
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDrawArrays(GL_LINES, 0, (gridCnt << 3) + 4);
	glBindVertexArray(0);
	// delete buffer object
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

	// disable anti-aliasing
	glDisable(GL_LINE_SMOOTH);
	glDisable(GL_BLEND);
}

void MOON_InputProcessor(GLFWwindow *window) {
	if (MOON_InputManager::mouseAction == GLFW_PRESS)
		switch (MOON_InputManager::mouseButton) {
			case GLFW_MOUSE_BUTTON_LEFT:
				MOON_InputManager::mouse_left_hold = true;
				//unProjectMouse();
#ifdef MOON_DEBUG_MODE
				std::cout << "Mosue left button pressed!" << std::endl;
#endif // MOON_DEBUG_MODE
				break;
			case GLFW_MOUSE_BUTTON_MIDDLE:
				MOON_InputManager::mouse_middle_hold = true;
#ifdef MOON_DEBUG_MODE
				std::cout << "Mosue middle button pressed!" << std::endl;
#endif // MOON_DEBUG_MODE
				break;
			case GLFW_MOUSE_BUTTON_RIGHT:
				MOON_InputManager::mouse_right_hold = true;
#ifdef MOON_DEBUG_MODE
				std::cout << "Mosue right button pressed!" << std::endl;
#endif // MOON_DEBUG_MODE
				break;
			default:
				return;
		} else 
	if (MOON_InputManager::mouseAction == GLFW_RELEASE)
		switch (MOON_InputManager::mouseButton) {
			case GLFW_MOUSE_BUTTON_LEFT:
				MOON_InputManager::mouse_left_hold = false;
#ifdef MOON_DEBUG_MODE
				std::cout << "Mosue left button released!" << std::endl;
#endif // MOON_DEBUG_MODE
				break;
			case GLFW_MOUSE_BUTTON_MIDDLE:
				MOON_InputManager::mouse_middle_hold = false;
#ifdef MOON_DEBUG_MODE
				std::cout << "Mosue middle button released!" << std::endl;
#endif // MOON_DEBUG_MODE
				break;
			case GLFW_MOUSE_BUTTON_RIGHT:
				MOON_InputManager::mouse_right_hold = false;
#ifdef MOON_DEBUG_MODE
				std::cout << "Mosue right button released!" << std::endl;
#endif // MOON_DEBUG_MODE
				break;
			default:
				return;
		}

		if (!MOON_InputManager::isHoverUI) {
			if (MOON_InputManager::mouseScrollOffset.magnitude() > 0) {
				MOON_SceneCamera->PushCamera(MOON_InputManager::mouseScrollOffset);
			}

			if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS) {
				if (MOON_InputManager::mouse_right_hold) {
					MOON_SceneCamera->ZoomCamera(MOON_InputManager::mouseOffset);
				}
			}

			if (MOON_InputManager::mouse_middle_hold) {
				if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS) {
					MOON_SceneCamera->RotateCamera(MOON_InputManager::mouseOffset);
				} else if (MOON_InputManager::mouseOffset.magnitude() > 0) {
					MOON_SceneCamera->PanCamera(MOON_InputManager::mouseOffset);
				}
			}
		} else {

		}

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);

		// reset offset
		MOON_InputManager::mouseOffset.setValue(0.0f, 0.0f);
		MOON_InputManager::mouseScrollOffset.setValue(0.0f, 0.0f);
}