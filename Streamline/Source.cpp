#include <iostream>
#include <algorithm>  
#include <cmath>  
#include <iomanip>  
#include <iostream>  
#include <vector> 
#include <map> 
#include <fstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader_s.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "stb_image.h"
using namespace std;

// settings
unsigned int SCR_WIDTH = 700;
unsigned int SCR_HEIGHT = 700;

int total = 0, xsize, ysize;
double h = 0.1;//步長
double gap = 1;
float max_speed = 0;//最大流速
float min_speed = FLT_MAX;//最小流速
float user_mod_thick = 0.25;
int line_max_limit = 1000;
int line_min_limit = 50;
vector<vector<glm::dvec2>> vecdata;
vector<vector<int>> flag;
vector<double>vertices;
const char* filename_list[] = {"1.vec", "2.vec", "3.vec", "4.vec", "5.vec", "6.vec", "7.vec", "8.vec", "9.vec", "10.vec", "11.vec", "12.vec", "13.vec", "14.vec", "15.vec", "16.vec"
                                , "19.vec", "20.vec", "21.vec", "22.vec", "23.vec", "rect2.vec", "step5_velocity.vec", "test_not_unit.vec", "test_unit.vec"};
int filename_current = 0;
unsigned int VBO, VAO;

unsigned int texName;
unsigned char texture_map[256][4];

bool re0 = false;

// camera
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 100.0f);
glm::mat3 cameraU = glm::mat3(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
//教學的
//glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 300.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraRight = glm::vec3(1.0f, 0.0f, 0.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
float yaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch = 0.0f;
// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

float windows[6] = { -10.0, 300.0, -10.0, 300.0, 5.0, 310.0 };//left, right, bottom, top, nearVal, farVal

/*-----Translation and rotations of eye coordinate system---*/
#define  PI   3.1415926535
float   eyeDx = 0.0, eyeDy = 0.0, eyeDz = 0.0;
float   eyeAngx = 0.0, eyeAngy = 0.0, eyeAngz = 0.0;
float   cv = cos(1.0 * PI / 180.0), sv = sin(1.0 * PI / 180.0); /* cos(5.0) and sin(5.0) */
float pre_eyeAngx = 0, pre_eyeAngy = 0, pre_eyeAngz = 0;
float pre_eyeDx = 0, pre_eyeDy = 0, pre_eyeDz = 0;

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
    SCR_HEIGHT = height;
    SCR_WIDTH = width;
    //windows[2] = windows[2] * (float)SCR_HEIGHT / (float)SCR_WIDTH;
    //windows[3] = windows[3] * (float)SCR_HEIGHT / (float)SCR_WIDTH;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

// This example can also compile and run with Emscripten! See 'Makefile.emscripten' for details.
#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

void init_vao_vbo() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(double), vertices.data(), GL_STATIC_DRAW);//vertices要 * sizeof(float)

    // position attribute
    glVertexAttribPointer(0, 2, GL_DOUBLE, GL_FALSE, 4 * sizeof(double), (void*)0);
    glEnableVertexAttribArray(0);
    
    // 粗細 attribute
    glVertexAttribPointer(1, 1, GL_DOUBLE, GL_FALSE, 4 * sizeof(double), (void*)(2 * sizeof(double)));
    glEnableVertexAttribArray(1);

    // speed attribute
    glVertexAttribPointer(2, 1, GL_DOUBLE, GL_FALSE, 4 * sizeof(double), (void*)(3 * sizeof(double)));
    glEnableVertexAttribArray(2);
}

void read_file() {
    vecdata.clear();
    string fc = filename_list[filename_current];
    ifstream file("vector\\" + fc);

    if (!file.is_open()) {
        cerr << "Unable to open file!\n";
        return;
    }

    total = 0;
    int temp = 0;
    
    string line;
    while (getline(file, line)) {
        istringstream iss(line);
        double val;
        while (iss >> val) {
            if (total == 0) xsize = val;
            else if (total == 1) ysize = val;
            else {
                if ((total - 2) % (2 * ysize) == 0) vecdata.push_back(vector<glm::dvec2>());
                if (temp == 0) {
                    vecdata.back().push_back({val, 0});
                    temp = 1;
                }
                else {
                    vecdata.back().back().y = val;
                    temp = 0;
                }
            }
            total++;
        }
    }
    file.close();

    return;
}

glm::dvec2 Bilinear_Interpolation(glm::dvec2 pos) {//雙線性內插

    glm::dvec2 vec;
    int x0 = (int)pos.x;
    int y0 = (int)pos.y;
    int x1 = x0 + 1;
    int y1 = y0 + 1;
    double dx = pos.x - x0;
    double dy = pos.y - y0;

    vec.x = vecdata[x0][y0].x * (1 - dx) * (1 - dy) + vecdata[x1][y0].x * dx * (1 - dy) + vecdata[x0][y1].x * (1 - dx) * dy + vecdata[x1][y1].x * dx * dy;
    vec.y = vecdata[x0][y0].y * (1 - dx) * (1 - dy) + vecdata[x1][y0].y * dx * (1 - dy) + vecdata[x0][y1].y * (1 - dx) * dy + vecdata[x1][y1].y * dx * dy;

    return vec;
}

void trace_streamline() {
    flag.clear();
    vertices.clear();
    flag.resize(xsize, std::vector<int>(ysize, 0));
    for (double x = 0; x < xsize; x += gap) {
        for (double y = 0; y < ysize; y += gap) {
            vector<pair<glm::dvec2, double>> streamline;//座標、速度
            glm::dvec2 pos = { x, y }, k1, k2, next_pos, re_pos;
            if (pos.x < 0 || pos.x > xsize - 2 || pos.y < 0 || pos.y > ysize - 2) continue;
            if (flag[(int)pos.x][(int)pos.y] == 1) continue;
            streamline.push_back({ pos, glm::length(Bilinear_Interpolation(pos)) });

            for (int it = 0; it < line_max_limit; it++) {//rk2
                k1 = Bilinear_Interpolation(pos);
                if (abs(k1.x - 0.0) <= 0.00000001 && abs(k1.y - 0.0) <= 0.00000001) break;
                k1 = glm::normalize(k1);

                next_pos = pos + h * k1;
                if (next_pos.x < 0 || next_pos.x > xsize - 2 || next_pos.y < 0 || next_pos.y > ysize - 2) break;
                k2 = Bilinear_Interpolation(next_pos);
                if (abs(k2.x - 0.0) <= 0.00000001 && abs(k2.y - 0.0) <= 0.00000001) break;
                k2 = glm::normalize(k2);

                re_pos = pos + h *  (k1 + k2) / 2.0;
                if (re_pos.x < 0 || re_pos.x > xsize - 2 || re_pos.y < 0 || re_pos.y > ysize - 2) break;
                if (flag[(int)re_pos.x][(int)re_pos.y] == 1) break;
                
                streamline.push_back({ re_pos, glm::length(Bilinear_Interpolation(re_pos)) });
                pos = re_pos;
            }
            
            if (streamline.size() > line_min_limit) {//其他太短不畫
                for (int k = 1; k < streamline.size(); k++) {
                    flag[(int)streamline[k - 1].first.x][(int)streamline[k - 1].first.y] = 1;
                    flag[(int)streamline[k].first.x][(int)streamline[k].first.y] = 1;
                    //座標
                    vertices.push_back(streamline[k - 1].first.x);
                    vertices.push_back(streamline[k - 1].first.y);
                    vertices.push_back((double)k / (double)streamline.size() * 3 + 1);//k / streamline.size():讓粗度歸一化到0~1，+1是怕有些太細
                    vertices.push_back(streamline[k - 1].second);

                    vertices.push_back(streamline[k].first.x);
                    vertices.push_back(streamline[k].first.y);
                    vertices.push_back((double)k / (double)streamline.size() * 3 + 1);
                    vertices.push_back(streamline[k].second);

                    max_speed = fmax(max_speed, streamline[k - 1].second);
                    min_speed = fmin(min_speed, streamline[k - 1].second);
                    max_speed = fmax(max_speed, streamline[k].second);
                    min_speed = fmin(min_speed, streamline[k].second);
                }
            }
        }
    }
    return;
}

void make_texture1d() {
    glGenTextures(1, &texName);

    for (int i = 0; i < 256; i++) {
        texture_map[i][0] = i;
        texture_map[i][1] = 0;
        texture_map[i][2] = 255 - i;
        texture_map[i][3] = 255;
    }

    //建立1d texture:(rgba)
    glBindTexture(GL_TEXTURE_1D, texName);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);//解壓縮像素時的對其方式，1表連續儲存，沒有空餘
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_map);
}

void gui() {
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("UI");

    if (ImGui::Combo("filename", &filename_current, filename_list, IM_ARRAYSIZE(filename_list))) {
        re0 = true;
    }

    ImGui::Text("xsize: %d", xsize);
    ImGui::Text("ysize: %d", ysize);

    ImGui::PushItemWidth(80);
    if (ImGui::InputInt("Line min Limit", &line_min_limit, 10)) {
        trace_streamline();
        init_vao_vbo();
    }
    
    if (ImGui::InputInt("Line max Limit", &line_max_limit, 100)) {
        trace_streamline();
        init_vao_vbo();
    }

    ImGui::Text("thickness: %f", user_mod_thick);
    ImGui::SameLine();
    if (ImGui::Button("-")) {
        if (user_mod_thick >= 0.03) {
            user_mod_thick -= 0.03;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("+")) {
        user_mod_thick += 0.03;
    }

    ImGui::Text("gap: %f", gap);
    ImGui::SameLine();
    if (ImGui::Button("gap-")) {
        if (gap >= 0.11) {
            gap -= 0.1;
            trace_streamline();
            init_vao_vbo();
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("gap+")) {
        gap += 0.5;
        trace_streamline();
        init_vao_vbo();
    }

    ImGui::Text("h_gap: %f", h);
    ImGui::SameLine();
    if (ImGui::Button("--")) {
        if (h >= 0.1) {
            h -= 0.1;
            trace_streamline();
            init_vao_vbo();
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("++")) {
        h += 0.1;
        trace_streamline();
        init_vao_vbo();
    }

    if (ImGui::CollapsingHeader("Camera")) {
        ImGui::Separator();
        ImGui::Text("Transform:");
        ImGui::Separator();
        ImGui::Indent();

        ImGui::PushItemWidth(80);
        float cameraSpeed = static_cast<float>(100.0 * deltaTime);
        if (ImGui::DragFloat("X", &eyeDx, 0.1f)) {
            if (eyeDx - pre_eyeDx >= 0) cameraPos += cameraSpeed * cameraU[0];//右移
            else cameraPos -= cameraSpeed * cameraU[0];//左移
            pre_eyeDx = eyeDx;
        }
        ImGui::SameLine();
        if (ImGui::DragFloat("Y", &eyeDy, 0.1f)) {
            if (eyeDy - pre_eyeDy >= 0) cameraPos += cameraSpeed * cameraU[1];//上移
            else cameraPos -= cameraSpeed * cameraU[1];//下移
            pre_eyeDy = eyeDy;
        }
        ImGui::SameLine();
        if (ImGui::DragFloat("Z", &eyeDz, 0.01f)) {
            if (eyeDz - pre_eyeDz >= 0) cameraPos += cameraSpeed * cameraU[2];//後退
            else cameraPos -= cameraSpeed * cameraU[2];//前進
            pre_eyeDz = eyeDz;
        }
        ImGui::Unindent();

        ImGui::Text("Rotation:");
        ImGui::Separator();
        ImGui::Indent();
        ImGui::PushItemWidth(240);
        //ImGui::DragFloat("Far", &mFar, 0.1f, 0.1f, 1000.0f);
        if (ImGui::DragFloat("Pitch-X", &eyeAngx, 0.01f)) {
            glm::vec3 ex, ey, ez;
            if (eyeAngx - pre_eyeAngx >= 0) {
                ey = cv * cameraU[1] - sv * cameraU[2];
                ez = sv * cameraU[1] + cv * cameraU[2];
                cameraU[1] = ey;
                cameraU[2] = ez;
            }
            else {
                ey = cv * cameraU[1] + sv * cameraU[2];
                ez = -sv * cameraU[1] + cv * cameraU[2];
                cameraU[1] = ey;
                cameraU[2] = ez;
            }
            pre_eyeAngx = eyeAngx;
        }
        if (ImGui::DragFloat("Yaw-Y", &eyeAngy, 0.01f)) {
            glm::vec3 ex, ey, ez;
            if (eyeAngy - pre_eyeAngy >= 0) {
                ex = cv * cameraU[0] - sv * cameraU[2];
                ez = sv * cameraU[0] + cv * cameraU[2];
                cameraU[0] = ex;
                cameraU[2] = ez;
            }
            else {
                ex = cv * cameraU[0] + sv * cameraU[2];
                ez = -sv * cameraU[0] + cv * cameraU[2];
                cameraU[0] = ex;
                cameraU[2] = ez;
            }
            pre_eyeAngy = eyeAngy;
        }
        if (ImGui::DragFloat("Roll-Z", &eyeAngz, 0.01f)) {
            glm::vec3 ex, ey, ez;
            if (eyeAngz - pre_eyeAngz >= 0) {
                ex = cv * cameraU[0] - sv * cameraU[1];
                ey = sv * cameraU[0] + cv * cameraU[1];
                cameraU[0] = ex;
                cameraU[1] = ey;
            }
            else {
                ex = cv * cameraU[0] + sv * cameraU[1];
                ey = -sv * cameraU[0] + cv * cameraU[1];
                cameraU[0] = ex;
                cameraU[1] = ey;
            }
            pre_eyeAngz = eyeAngz;
        }
        //camCtrl->updateCameraVectors();
        ImGui::Unindent();
    }

    ImGui::Spacing();//空行
    ImGui::Spacing();//空行
    ImGui::Spacing();//空行

    if (ImGui::Button("Reset Camera")) {

        cameraPos = glm::vec3(0.0f, 0.0f, 300.0f);
        cameraU = glm::mat3(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

        eyeDx = 0, eyeDy = 0, eyeDz = 0;
        pre_eyeDx = 0, pre_eyeDy, pre_eyeDz = 0;
        eyeAngx = 0, eyeAngy = 0, eyeAngz = 0;
        pre_eyeAngx = 0, pre_eyeAngy = 0, pre_eyeAngz = 0;
    }

    ImGui::End();

    ImGui::Render();
    //glutPostRedisplay();
}

int main()
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "mesh", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
#ifdef __EMSCRIPTEN__
    ImGui_ImplGlfw_InstallEmscriptenCanvasResizeCallback("#canvas");
#endif
    ImGui_ImplOpenGL3_Init(glsl_version);

    // glfw: initialize and configure
    // ------------------------------
    //glfwInit();
    //glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    //glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    //GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    //glfwSetScrollCallback(window, scroll_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    //glEnable(GL_DEPTH_TEST);

    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);
    //glFrontFace(GL_CCW);/*面剔除*/
    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // build and compile our shader program
    Shader ourShader("vex.vs", "fram.fs", "geometry.gs"); // you can name your shader files however you like

    read_file();
    trace_streamline();
    make_texture1d();
    init_vao_vbo();

    // Main loop
#ifdef __EMSCRIPTEN__
    // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
    // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
    io.IniFilename = nullptr;
    EMSCRIPTEN_MAINLOOP_BEGIN
#else
    while (!glfwWindowShouldClose(window))
#endif
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        // input
        // -----
        processInput(window);

        // render
        // ------
        gui();
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (re0) {
            cameraPos = glm::vec3(0.0f, 0.0f, 300.0f);
            cameraU = glm::mat3(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

            eyeDx = 0, eyeDy = 0, eyeDz = 0;
            pre_eyeDx = 0, pre_eyeDy, pre_eyeDz = 0;
            eyeAngx = 0, eyeAngy = 0, eyeAngz = 0;
            pre_eyeAngx = 0, pre_eyeAngy = 0, pre_eyeAngz = 0;

            line_max_limit = 1000;
            line_min_limit = 50;
            glDeleteVertexArrays(1, &VAO);
            glDeleteBuffers(1, &VBO);
            h = 0.1;//步長
            gap = 1;
            max_speed = 0;//最大流速
            min_speed = FLT_MAX;//最小流速
            user_mod_thick = 0.25;
            read_file();
            trace_streamline();
            init_vao_vbo();
            re0 = false;
        }

        ourShader.use();

        // be sure to activate shader when setting uniforms/drawing objects
        ourShader.use();
        ourShader.setInt("T0", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_1D, texName);
        ourShader.setFloat("maxSpeed", max_speed);
        ourShader.setFloat("minSpeed", min_speed);
        ourShader.setFloat("user_mod_thick", user_mod_thick);
        ourShader.setVec3("viewPos", cameraPos);

        // view/projection transformations
        glm::mat4 projection = glm::ortho(-10.0f, xsize + 10.0f, -10.0f * (float)SCR_HEIGHT / (float)SCR_WIDTH, ysize + 10.0f * (float)SCR_HEIGHT / (float)SCR_WIDTH, 0.1f, 1000.0f);
        //glm::mat4 projection = glm::ortho(0.0f, (float)SCR_WIDTH, (float)SCR_HEIGHT, 0.0f, -1.0f, 1.0f);
        glm::mat4 view;
        view = glm::lookAt(cameraPos, cameraPos - cameraU[2], cameraU[1]);
        glm::mat4 model = glm::mat4(1.0f);
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);
        
        ourShader.setMat4("model", model);

        ourShader.use();
        glBindVertexArray(VAO);
        glDrawArrays(GL_LINES, 0, vertices.size() / 4);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}