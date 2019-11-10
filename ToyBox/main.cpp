#include "PreInclude.h"

// global settings
const char *title = "MoonEngine - v0.01 WIP";
Vector2 MOON_WndSize = Vector2(1600, 900);
Vector2 MOON_OutputSize = Vector2(200, 100);
ImVec4 clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

// global parameters
Matrix4x4 view;
Matrix4x4 projection;
ImVec4 grdLineColor = ImVec4(0.8f, 0.8f, 0.8f, 1.0f);

// scene objects


int main() {
	// glfw window creation
	GLFWwindow* window = InitWnd();
	if (window == NULL) return -1;

	// local parameters
	ImGuiIO& io = ImGui::GetIO();

	// do some things at init
	if (!MainUI::LoadImages()) return -1;

	// compile and link shaders
	SceneManager::ShaderManager::CreateDefaultShaders();

	// render loop
	while (!glfwWindowShouldClose(window)) {
		// per-frame time logic
		float currentFrame = glfwGetTime();
		MOON_Clock::deltaTime = currentFrame - MOON_Clock::lastFrame;
		MOON_Clock::lastFrame = currentFrame;

		// openGL input
		processInput(window);

		// clear background
		glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
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
			if (MainUI::show_control_window) MainUI::ControlPanel(io, clearColor);
			if (MainUI::show_preference_window) MainUI::PreferencesWnd();
			if (MainUI::show_VFB_window) MainUI::ShowVFB(io);
			if (MainUI::show_demo_window) ImGui::ShowDemoWindow(&MainUI::show_demo_window);
			if (MainUI::show_about_window) MainUI::AboutWnd();
			if (MainUI::show_scene_window) MainUI::SceneWnd();
			if (MainUI::show_inspector_window) MainUI::InspectorWnd();
			if (MainUI::show_explorer_window) MainUI::ExplorerWnd();
			if (MainUI::show_console_window) MainUI::ConsoleWnd();
			if (MainUI::show_project_window) MainUI::ProjectWnd();
			if (MainUI::show_create_window) MainUI::CreateWnd();
		}

		// Rendering objects
		view = MOON_SceneCamera->GetViewMatrix();
		projection = Matrix4x4::Perspective(MOON_SceneCamera->Zoom, MOON_WndSize.x / MOON_WndSize.y, 0.1f, 100.0f);

		DrawGround(1.0, 5, SceneManager::ShaderManager::GetItem("pointShader"));

		// imgui input
		if (!io.WantCaptureMouse) {

		}

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

	ImGuiIO& io = ImGui::GetIO();
	auto fonts = io.Fonts;
	fonts->AddFontFromFileTTF(
		"./Resources/msyh.ttc",
		16.0f,
		NULL,
		fonts->GetGlyphRangesChineseSimplifiedCommon()
	);
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigWindowsMoveFromTitleBarOnly = true;

	return window;
}

// glfw callbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		MOON_SceneCamera->ProcessKeyboard(FORWARD, MOON_Clock::deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		MOON_SceneCamera->ProcessKeyboard(BACKWARD, MOON_Clock::deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		MOON_SceneCamera->ProcessKeyboard(LEFT, MOON_Clock::deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		MOON_SceneCamera->ProcessKeyboard(RIGHT, MOON_Clock::deltaTime);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (MOON_MousePos[0] == -2.0f) {
		MOON_MousePos.x = xpos;
		MOON_MousePos.y = ypos;
	}

	// reversed since y-coordinates go from bottom to top
	float xoffset = xpos - MOON_MousePos.x;
	float yoffset = MOON_MousePos.y - ypos;

	MOON_MousePos.x = xpos;
	MOON_MousePos.y = ypos;

	MOON_SceneCamera->ProcessMouseMovement(xoffset, yoffset);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (action == GLFW_PRESS) switch (button) {
		case GLFW_MOUSE_BUTTON_LEFT:
			std::cout << "Mosue left button pressed!" << std::endl;
			break;
		case GLFW_MOUSE_BUTTON_MIDDLE:
			std::cout << "Mosue middle button pressed!" << std::endl;
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			std::cout << "Mosue right button pressed!" << std::endl;
			break;
		default:
			return;
	} else if (action == GLFW_RELEASE) switch (button) {
		case GLFW_MOUSE_BUTTON_LEFT:
			std::cout << "Mosue left button released!" << std::endl;
			break;
		case GLFW_MOUSE_BUTTON_MIDDLE:
			std::cout << "Mosue middle button released!" << std::endl;
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			std::cout << "Mosue right button released!" << std::endl;
			break;
		default:
			return;
	}
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	MOON_SceneCamera->ProcessMouseScroll(yoffset);
}

// moon: clean up and draw commands
void MOON_CleanUp() {
	SceneManager::LightManager::Clear();
	SceneManager::MaterialManager::Clear();
	SceneManager::ShaderManager::Clear();
	SceneManager::TextureManager::Clear();
	SceneManager::ModelManager::Clear();
	SceneManager::CameraManager::Clear();

	delete MOON_SceneCamera;
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