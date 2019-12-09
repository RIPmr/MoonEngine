#include "PreInclude.h"
//#define MOON_DEBUG_MODE

// global settings ------------------------------------------------
const char *title = "MoonEngine - v0.02 WIP";

Vector2 MOON_WndSize = Vector2(1600, 900);
float SceneManager::aspect = MOON_WndSize.x / MOON_WndSize.y;
Vector2 MOON_OutputSize = Vector2(200, 100);
float Renderer::aspect = MOON_OutputSize.x / MOON_OutputSize.y;

Vector4 clearColor(0.45f, 0.55f, 0.60f, 1.00f);
Vector4 grdLineColor(0.8f, 0.8f, 0.8f, 1.0f);

unsigned int Renderer::samplingRate = 5;
unsigned int Renderer::maxReflectionDepth = 5;
// ----------------------------------------------------------------

int main() {
	std::cout << "starting moon engine... ..." << std::endl;

	// glfw window creation
	GLFWwindow* window = InitWnd();
	if (window == NULL) return -1;

	// engine resources initialization
	MOON_InitEngine();

	// test objects ------------------------------------------------------------------------
	//Model* teapot = MOON_ModelManager::LoadModel("Resources/teapot.obj");
	Model* boxes = MOON_ModelManager::LoadModel("Resources/box_stack.obj");
	//teapot->transform.Scale(Vector3(0.1f, 0.1f, 0.1f));
	boxes->transform.Translate(Vector3(0.0f, 1.0f, 0.0f));
	// -------------------------------------------------------------------------------------

	std::cout << "done." << std::endl;

	// main loop  --------------------------------------------------------------------------
	while (!glfwWindowShouldClose(window)) {
		// per-frame time logic
		MOON_UpdateClock();

		// configure global opengl state -------------------------------------------
		//glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		//glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

		if (SceneManager::wireMode) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else glPolygonMode(GL_FRONT, GL_FILL);

		// clear background of framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, MOON_TextureManager::IDLUT->fbo);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		// Start the Dear ImGui frame ---------------------------------------------
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		
		// UI controller
		MOON_DrawMainUI();

		// drawing ID LUT ---------------------------------------------------------
		MOON_ModelManager::DrawIDLUT();
		MOON_InputManager::GetIDFromLUT(MOON_MousePos);
		if (!MainUI::io->WantCaptureMouse && MainUI::io->MouseClicked[0] && !Gizmo::hoverGizmo)
			MOON_InputManager::Select(MOON_InputManager::hoverID);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// clear background of scene
		glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// rendering objects -------------------------------------------------------
		MOON_ModelManager::DrawModels();

		// drawing Gizmos ----------------------------------------------------------
		/// enable color blend and anti-aliasing
		glEnable(GL_BLEND);
		glEnable(GL_LINE_SMOOTH);
		glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		MOON_DrawGround(MOON_ShaderManager::lineShader);

		glDisable(GL_DEPTH_TEST);
		SceneManager::DrawGizmos();

		/// disable anti-aliasing
		glDisable(GL_LINE_SMOOTH);
		glDisable(GL_BLEND);

		// process user input ------------------------------------------------------
		MOON_InputManager::isHoverUI = MainUI::io->WantCaptureMouse;
		MOON_InputProcessor(window);

		// Rendering UI ------------------------------------------------------------
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Cleanup  ----------------------------------------------------------------------------
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	MOON_CleanUp();

	// glfw: terminate, clearing all previously allocated GLFW resources.
	glfwTerminate();
	return 0;
}

// glfw: initialize and configure ----------------------------------------------------------
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
	glfwSetKeyCallback(window, key_callback);
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

	// add fonts
	MainUI::io = &ImGui::GetIO();
	MainUI::style = &ImGui::GetStyle();
	//MainUI::io->Fonts->AddFontDefault();

	// add simplified Chinese
	MainUI::io->Fonts->AddFontFromFileTTF(
		"./Resources/msyh.ttc",
		16.0f,
		NULL,
		MainUI::io->Fonts->GetGlyphRangesChineseSimplifiedCommon()
	);

	// merge FontAwesome
	ImFontConfig config;
	config.MergeMode = true;
	config.GlyphMinAdvanceX = 13.0f; // Use if you want to make the icon monospaced
	static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
	MainUI::io->Fonts->AddFontFromFileTTF(
		"./Resources/fontawesome-webfont.ttf", 
		13.0f, 
		&config, 
		icon_ranges
	);

	MainUI::io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	MainUI::io->ConfigWindowsMoveFromTitleBarOnly = true;

	return window;
}

// glfw callbacks --------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	SceneManager::SetWndSize(width, height);
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

	if (MOON_InputManager::mouseAction == GLFW_PRESS)
		switch (MOON_InputManager::mouseButton) {
			case GLFW_MOUSE_BUTTON_LEFT:
				MOON_InputManager::mouse_left_hold = true;
				break;
			case GLFW_MOUSE_BUTTON_MIDDLE:
				MOON_InputManager::mouse_middle_hold = true;
				break;
			case GLFW_MOUSE_BUTTON_RIGHT:
				MOON_InputManager::mouse_right_hold = true;
				break;
			default:
				return;
		} else
	if (MOON_InputManager::mouseAction == GLFW_RELEASE)
		switch (MOON_InputManager::mouseButton) {
			case GLFW_MOUSE_BUTTON_LEFT:
				MOON_InputManager::mouse_left_hold = false;
				break;
			case GLFW_MOUSE_BUTTON_MIDDLE:
				MOON_InputManager::mouse_middle_hold = false;
				break;
			case GLFW_MOUSE_BUTTON_RIGHT:
				MOON_InputManager::mouse_right_hold = false;
				break;
			default:
				return;
		}
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	MOON_InputManager::mouseScrollOffset.setValue(xoffset, yoffset);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_M && action == GLFW_PRESS)
		MainUI::show_material_editor = !MainUI::show_material_editor;

	// Gizmo --------------------------------------------------------------------------
	if (key == GLFW_KEY_Q) Gizmo::gizmoMode = GizmoMode::none;
	if (key == GLFW_KEY_W) Gizmo::gizmoMode = GizmoMode::translate;
	if (key == GLFW_KEY_E) Gizmo::gizmoMode = GizmoMode::rotate;
	if (key == GLFW_KEY_R) Gizmo::gizmoMode = GizmoMode::scale;

	// Escape -------------------------------------------------------------------------
	if (key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(window, true);
}

// moon callbacks --------------------------------------------------------------------------
void MOON_CleanUp() {
	MOON_LightManager::Clear();
	MOON_MaterialManager::Clear();
	MOON_ShaderManager::Clear();
	MOON_TextureManager::Clear();
	MOON_ModelManager::Clear();
	MOON_CameraManager::Clear();
	MOON_InputManager::Clear();
	MainUI::CleanUp();
	SceneManager::Clear();
	AssetLoader::CleanUp();
}

void MOON_UpdateClock() {
	float currentFrame = glfwGetTime();
	MOON_Clock::deltaTime = currentFrame - MOON_Clock::lastFrame;
	MOON_Clock::lastFrame = currentFrame;
}

void MOON_GenerateGround(const float &space, const int &gridCnt) {
	grid.clear();
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
}

void MOON_DrawGround(const Shader* groundShader) {
	// configure shader
	groundShader->use();
	groundShader->setVec4("lineColor", grdLineColor);
	groundShader->setMat4("model", Matrix4x4());
	groundShader->setMat4("view", MOON_CurrentCamera->view);
	groundShader->setMat4("projection", MOON_CurrentCamera->projection);
	
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
	glDrawArrays(GL_LINES, 0, grid.size() / 3);
	glBindVertexArray(0);
	// delete buffer object
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

	// disable anti-aliasing
	//glDisable(GL_LINE_SMOOTH);
	//glDisable(GL_BLEND);
}

void MOON_InputProcessor(GLFWwindow *window) {
	if (MOON_InputManager::isHoverUI) {

	} else {
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
		}

	// Keys ---------------------------------------------------------------------------
	MOON_InputManager::left_ctrl_hold = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS;
	MOON_InputManager::left_shift_hold = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
	MOON_InputManager::right_ctrl_hold = glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;
	MOON_InputManager::right_ctrl_hold = glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;

	// reset offset -------------------------------------------------------------------
	MOON_InputManager::mouseOffset.setValue(0.0f, 0.0f);
	MOON_InputManager::mouseScrollOffset.setValue(0.0f, 0.0f);
}

void MOON_DrawMainUI() {
	// update output image realtime while rendering
	if (Renderer::progress != 0) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, MOON_OutputSize.x, MOON_OutputSize.y, 0, GL_RGB, GL_UNSIGNED_BYTE, Renderer::outputImage);
		if (Renderer::progress < 0) {
			Renderer::progress = 0;
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}
	MainUI::QuadMenu();
	if (ImGui::BeginMainMenuBar())		MainUI::MainMenu();
	if (MainUI::show_control_window)	MainUI::ControlPanel(MainUI::io, clearColor);
	if (MainUI::show_preference_window)	MainUI::PreferencesWnd();
	if (MainUI::show_VFB_window)		MainUI::ShowVFB();
	if (MainUI::show_about_window)		MainUI::AboutWnd();
	if (MainUI::show_scene_window)		MainUI::SceneWnd();
	if (MainUI::show_inspector_window)	MainUI::InspectorWnd();
	if (MainUI::show_explorer_window)	MainUI::ExplorerWnd();
	if (MainUI::show_console_window)	MainUI::ConsoleWnd();
	if (MainUI::show_project_window)	MainUI::ProjectWnd();
	if (MainUI::show_create_window)		MainUI::CreateWnd();
	if (MainUI::show_ribbon)			MainUI::RibbonBar();
	if (MainUI::show_enviroment_editor) MainUI::EnviromentWnd();
	if (MainUI::show_codeEditor)		MainUI::CodeEditor();
	if (MainUI::show_timeline)			MainUI::ShowTimeline();
	if (MainUI::show_material_editor)	MainUI::MaterialEditorWnd();
	if (MainUI::show_demo_window)		ImGui::ShowDemoWindow(&MainUI::show_demo_window);
}

void MOON_InitEngine() {
	std::cout << "- Loading Assets..." << std::endl;
	AssetLoader::BuildDirTree(".\\Assets");
	std::cout << "- Dir Tree Created." << std::endl;
	SceneManager::Init();
	std::cout << "- Scene Manager Initialized." << std::endl;
	MOON_GenerateGround(1.0, 5);
	MOON_TextureManager::CreateIDLUT();
}
