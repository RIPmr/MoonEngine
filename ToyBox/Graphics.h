#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>

#include "MShader.h"
#include "Texture.h"
#include "MoonEnums.h"

namespace MOON {

	#define PostEffect Graphics::PostProcessing

	class Graphics {
	public:

		class PostProcessing {
		public:
			std::string name;

			bool enabled = true;
			bool opened = true;

			Shader* shader;

			PostProcessing(const std::string& name, Shader*& shader);
			PostProcessing(const std::string& name, const std::string& shaderPath);
			virtual ~PostProcessing() = default;

			virtual void ListProperties() {}
			virtual void ConfigureProps() {}
		};

		#pragma region parameters
		// system enums
		static PipelineMode pipeline;
		static ShadingMode shading;
		static SystemProcess process;
		static SceneView currDrawTarget;
		static int enviroment;

		// global parameters
		static std::vector<float> ground;
		static int edit_mode_point_size;

		// shadow parameters
		static float shadowDistance;

		// anti-aliasing parameters
		static bool antiAliasing;
		static int AAType;

		// post-processing stack
		static bool enablePP;
		static bool showList;
		static bool focusKey;
		static char	buf[64];
		static int	selection;
		static std::vector<PostProcessing*> postStack;
		static std::vector<std::string>	matchList;
		#pragma endregion

		// pipeline functions
		static void DrawSky();
		static void DrawIDLUT();
		static void DrawIDLUT_EditMode();
		static void DrawShadowMap();
		static void SetDrawTarget(SceneView view, const bool& depthTest);
		static void DrawSceneView(SceneView view);
		
		static void SetShadingMode(ShadingMode shading);
		static void DrawModels();
		static void DrawShapes();
		static void DrawHelpers();
		static void DrawCameras();
		static void HighlightSelection();

		// deferred functions
		static void Blit(FrameBuffer*& src, FrameBuffer*& dst, const Shader* shader);
		static void ApplyPostProcessing(FrameBuffer*& buffer, const Shader* shader);
		static void ApplyPostProcessing(FrameBuffer*& buffer, PostProcessing* renderer);
		static void ApplyPostStack(FrameBuffer*& buffer);

		// debug functions
		static void DrawPrototype();
		static void DrawMesh();

		static void SearchOps_Fuzzy(const char* typeName);
		static void DrawPostProcessingStack();
		static void ListEffects();

		static void Clear();

	private:
		static unsigned int quadVAO, quadVBO;

		// vertex attributes for a quad that fills the 
		// entire screen in Normalized Device Coordinates.
		static GLfloat quadVertices[20];

		// config screen quad VAO
		static void ConfigureScreenQuad();
	};

}

/*
*NOTE:
[模板测试]
启用：glEnable(GL_STENCIL_TEST);

void glStencilOp(GLenum sfail, GLenum zfail, GLenum zpass);
sfail 当蒙板测试失败时所执行的操作
zfail 当蒙板测试通过，深度测试失败时所执行的操作
zpass 当蒙板测试通过，深度测试通过时所执行的操作
GL_KEEP		保持当前的蒙板缓存区值
GL_ZERO		把当前的蒙板缓存区值设为0
GL_REPLACE	用glStencilFunc函数所指定的参考值替换蒙板参数值
GL_INCR		增加当前的蒙板缓存区值，但限制在允许的范围内
GL_DECR		减少当前的蒙板缓存区值，但限制在允许的范围内
GL_INVERT	将当前的蒙板缓存区值进行逐位的翻转

void glStencilFunc(GLenum func, GLint ref, GLuint mask);
func 决定模板测试的通过方式
ref	 为参考值
mask 为掩码值
GL_NEVER	从不通过蒙板测试
GL_ALWAYS	总是通过蒙板测试
GL_LESS		只有参考值<(蒙板缓存区的值&mask)时才通过
GL_LEQUAL	只有参考值<=(蒙板缓存区的值&mask)时才通过
GL_EQUAL	只有参考值=(蒙板缓存区的值&mask)时才通过
GL_GEQUAL	只有参考值>=(蒙板缓存区的值&mask)时才通过
GL_GREATER	只有参考值>(蒙板缓存区的值&mask)时才通过
GL_NOTEQUAL	只有参考值!=(蒙板缓存区的值&mask)时才通过


[混合]
启用：glEnable(GL_BLEND);

glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
GL_SRC_ALPHA 及 GL_ONE_MINUS_SRC_ALPHA 分别代表源和目标颜色在混合时所占比重的枚举常量

可取值包括：
GL_ZERO, 
GL_ONE, 
GL_DST_COLOR, 
GL_ONE_MINUS_DST_COLOR, 
GL_SRC_ALPHA, 
GL_ONE_MINUS_SRC_ALPHA, 
GL_DST_ALPHA, 
GL_ONE_MINUS_DST_ALPHA, 
GL_SRC_ALPHA_SATURATE

在计算混色时，首先是根据以上的枚举得出源颜色和目标颜色的系数，
然后分别与源和目标颜色相乘（乘积大于1时取值1），最后再把得出的结果相加。

假设根据以上枚举得出的RGBA四个成分上的系数分别为 
源 sfR, sfG, sfB, sfA， 
目标 dfR, dfG, dfB, dfA， 
源和目标的颜色成分值分别用 
sR, sG, sB, sA
dR, dG, dB, dA 
表示，最终结果色的 rR, rG, rB, rA 分别为
rR = sR * sfR + dR * dfR
rG = sG * sfG + dG * dfG
rB = sB * sfB + dB * dfB
rA = sA * sfA + dA * dfA

例：
glBlendFunc(GL_ONE, GL_ONE); 即源与目标颜色的RGBA分别相加。
glBlendFunc(GL_ONE, GL_ZERO); 即只取源颜色，这也是默认值。
glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA); 是比较典型的半透明效果，
如果源色alpha为0，则取目标色，如果源色alpha为1，则取源色，
否则视源色的alpha大小各取一部分。
源色的alpha越大，则源色取的越多，最终结果源色的表现更强；
源色的alpha越小，则目标色“透过”的越多。
*/
