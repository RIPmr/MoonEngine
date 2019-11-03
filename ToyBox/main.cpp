#include "PreInclude.h"

// global settings
Vector2 SceneManager::SCR_SIZE = Vector2(1600, 900);
Vector2 Renderer::OUTPUT_SIZE = Vector2(200, 100);
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

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


	// render loop
	while (!glfwWindowShouldClose(window)) {
		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// update output image realtime while rendering
		if (Renderer::progress != 0) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Renderer::OUTPUT_SIZE.x, Renderer::OUTPUT_SIZE.y, 0, GL_RGB, GL_UNSIGNED_BYTE, Renderer::outputImage);
			if (Renderer::progress < 0) {
				Renderer::progress = 0;
				glBindTexture(GL_TEXTURE_2D, 0);
			}
		}

		if (ImGui::BeginMainMenuBar()) MainUI::MainMenu();
		if (MainUI::show_control_window) MainUI::ControlPanel(io, clear_color);
		if (MainUI::show_preference_window) MainUI::PreferencesWnd();
		if (MainUI::show_VFB_window) MainUI::ShowVFB(io);
		if (MainUI::show_demo_window) ImGui::ShowDemoWindow(&MainUI::show_demo_window);
		if (MainUI::show_about_window) MainUI::AboutWnd();


		// openGL input
		processInput(window);

		// clear background
		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);

		// Rendering objects


		// imgui input
		if (io.WantCaptureMouse) {

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

	// glfw: terminate, clearing all previously allocated GLFW resources.
	glfwTerminate();
	return 0;
}

bool PrepareRendering() {
	Renderer::start = -1;
	Renderer::end = -1;
	if (Renderer::outputTexID != -1) glDeleteTextures(1, &Renderer::outputTexID);
	Renderer::PrepareVFB();

	return true;
}

// glfw: initialize and configure
GLFWwindow* InitWnd() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(SceneManager::SCR_SIZE.x, SceneManager::SCR_SIZE.y,
		"MoonEngine - v0.01 WIP", NULL, NULL);

	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		window = NULL;
	}
	glfwMakeContextCurrent(window);
	//glfwSetCursorPosCallback(window, mouse_callback);
	//glfwSetMouseButtonCallback(window, mouse_button_callback);
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

	auto fonts = ImGui::GetIO().Fonts;
	fonts->AddFontFromFileTTF(
		"./Resources/msyh.ttc",
		16.0f,
		NULL,
		fonts->GetGlyphRangesChineseSimplifiedCommon()
	);
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly = true;

	return window;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow *window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: mouse callbacks
/*
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (mousePos.x == -1) {
		mousePos.x = xpos;
		mousePos.y = ypos;
	}

	float xoffset = xpos - mousePos.x;
	// reversed since y-coordinates go from bottom to top
	float yoffset = mousePos.y - ypos;

	mousePos.x = xpos;
	mousePos.y = ypos;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (action == GLFW_PRESS) switch (button) {
		case GLFW_MOUSE_BUTTON_LEFT:
			mouse_left_hold = true;
			cout << "Mosue left button pressed!" << endl;
			break;
		case GLFW_MOUSE_BUTTON_MIDDLE:
			cout << "Mosue middle button pressed!" << endl;
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			cout << "Mosue right button pressed!" << endl;
			break;
		default:
			return;
	} else if (action == GLFW_RELEASE) switch (button) {
		case GLFW_MOUSE_BUTTON_LEFT:
			mouse_left_hold = false;
			cout << "Mosue left button released!" << endl;
			break;
		case GLFW_MOUSE_BUTTON_MIDDLE:
			cout << "Mosue middle button released!" << endl;
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			cout << "Mosue right button released!" << endl;
			break;
		default:
			return;
	}
}
*/