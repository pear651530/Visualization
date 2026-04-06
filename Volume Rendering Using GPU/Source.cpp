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
unsigned int SCR_WIDTH = 500;
unsigned int SCR_HEIGHT = 500;

const int SIZEx = 256, SIZEy = 256, SIZEz = 256;
//const int MAXN = 20000000;
map<int, int> cnt;
vector<float> cnt_vec;
int integerData[SIZEx][SIZEy][SIZEz];
const char* filename_list[] = {"Carp.raw", "foot.raw", "skull.raw", "aneurism.raw", "golfball.raw", "bonsai.raw", "mrt16_angio2.raw", "testing_engine.raw"};//, "BluntFin.raw"
int filename_current = 0;
vector<int> isolevel;//110, 230
unsigned int VBO, VAO;
map<int, pair<glm::vec3, float>> each_iso;

unsigned char gradient[SIZEx * SIZEy * SIZEz][4];
unsigned char texture_map[256][4];
unsigned int texName[2];
float plotData[4][256];

bool re0 = false;

// camera
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 300.0f);
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

int intValue = 0, delete_intValue = 0;
bool check_phong = false;

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
    float vertices[] = {
        //前
        127.5f, -127.5f, 127.5f, 1.0f, 0.0f, 1.0f,
        127.5f, 127.5f, 127.5f, 1.0f, 1.0f, 1.0f,
        -127.5f, -127.5f, 127.5f, 0.0f, 0.0f, 1.0f,
        
        -127.5f, -127.5f, 127.5f, 0.0f, 0.0f, 1.0f,
        127.5f, 127.5f, 127.5f, 1.0f, 1.0f, 1.0f,
        -127.5f, 127.5f, 127.5f, 0.0f, 1.0f, 1.0f,
        //後
        127.5f, -127.5f, -127.5f, 1.0f, 0.0f, 0.0f,
        -127.5f, -127.5f, -127.5f, 0.0f, 0.0f, 0.0f,
        127.5f, 127.5f, -127.5f, 1.0f, 1.0f, 0.0f,
        
        127.5f, 127.5f, -127.5f, 1.0f, 1.0f, 0.0f,
        -127.5f, -127.5f, -127.5f, 0.0f, 0.0f, 0.0f,
        -127.5f, 127.5f, -127.5f, 0.0f, 1.0f, 0.0f,
        //左
        -127.5f, -127.5f, -127.5f, 0.0f, 0.0f, 0.0f,
        -127.5f, -127.5f, 127.5f, 0.0f, 0.0f, 1.0f,
        -127.5f, 127.5f, -127.5f, 0.0f, 1.0f, 0.0f,
        
        -127.5f, 127.5f, -127.5f, 0.0f, 1.0f, 0.0f,
        -127.5f, -127.5f, 127.5f, 0.0f, 0.0f, 1.0f,
        -127.5f, 127.5f, 127.5f, 0.0f, 1.0f, 1.0f,
        //右
        127.5f, -127.5f, -127.5f, 1.0f, 0.0f, 0.0f,
        127.5f, 127.5f, -127.5f, 1.0f, 1.0f, 0.0f,
        127.5f, -127.5f, 127.5f, 1.0f, 0.0f, 1.0f,

        127.5f, -127.5f, 127.5f, 1.0f, 0.0f, 1.0f,
        127.5f, 127.5f, -127.5f, 1.0f, 1.0f, 0.0f,
        127.5f, 127.5f, 127.5f, 1.0f, 1.0f, 1.0f,
        // 上
        127.5f, 127.5f, 127.5f, 1.0f, 1.0f, 1.0f,
        127.5f, 127.5f, -127.5f, 1.0f, 1.0f, 0.0f,
        -127.5f, 127.5f, 127.5f, 0.0f, 1.0f, 1.0f,

        -127.5f, 127.5f, 127.5f, 0.0f, 1.0f, 1.0f,
        127.5f, 127.5f, -127.5f, 1.0f, 1.0f, 0.0f,
        -127.5f, 127.5f, -127.5f, 0.0f, 1.0f, 0.0f,
        // 下
        127.5f, -127.5f, 127.5f, 1.0f, 0.0f, 1.0f,
        -127.5f, -127.5f, 127.5f, 0.0f, 0.0f, 1.0f,
        127.5f, -127.5f, -127.5f, 1.0f, 0.0f, 0.0f,

        127.5f, -127.5f, -127.5f, 1.0f, 0.0f, 0.0f,
        -127.5f, -127.5f, -127.5f, 0.0f, 0.0f, 0.0f,
        -127.5f, -127.5f, -127.5f, 0.0f, 0.0f, 0.0f
    };
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // textcoord attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

void make_texture3d() {
    glGenTextures(2, texName);

    //建立3d texture:(gradient + iso-value)
    glBindTexture(GL_TEXTURE_3D, texName[0]);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);//解壓縮像素時的對其方式，1表連續儲存，沒有空餘
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, SIZEx, SIZEy, SIZEz, 0, GL_RGBA, GL_UNSIGNED_BYTE, gradient);
}

void make_texture1d() {
    for (int i = 0; i < 256; i++) {
        plotData[0][i] = 0;
        plotData[1][i] = 0;
        plotData[2][i] = 0;
        plotData[3][i] = 0;
        texture_map[i][0] = 0;
        texture_map[i][1] = 0;
        texture_map[i][2] = 0;
        texture_map[i][3] = 0;
    }
    for (auto it : each_iso) {//目標與其旁邊的a成尖角狀
        plotData[0][it.first] = it.second.first[0];
        plotData[1][it.first] = it.second.first[1];
        plotData[2][it.first] = it.second.first[2];
        plotData[3][it.first] = it.second.second;
        texture_map[it.first][0] = it.second.first[0];
        texture_map[it.first][1] = it.second.first[1];
        texture_map[it.first][2] = it.second.first[2];
        texture_map[it.first][3] = it.second.second;
        for (int i = it.second.second - 10, j = it.first - 1; i >= 1 && j >= 0; i -= 10, j--) {
            plotData[0][j] = it.second.first[0];
            plotData[1][j] = it.second.first[1];
            plotData[2][j] = it.second.first[2];
            plotData[3][j] = i;
            texture_map[j][0] = it.second.first[0];
            texture_map[j][1] = it.second.first[1];
            texture_map[j][2] = it.second.first[2];
            texture_map[j][3] = i;
        }
        for (int i = it.second.second - 10, j = it.first + 1; i >= 1 && j < 256; i -= 10, j++) {
            plotData[0][j] = it.second.first[0];
            plotData[1][j] = it.second.first[1];
            plotData[2][j] = it.second.first[2];
            plotData[3][j] = i;
            texture_map[j][0] = it.second.first[0];
            texture_map[j][1] = it.second.first[1];
            texture_map[j][2] = it.second.first[2];
            texture_map[j][3] = i;
        }
    }

    //建立1d texture:(rgba)
    glBindTexture(GL_TEXTURE_1D, texName[1]);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);//解壓縮像素時的對其方式，1表連續儲存，沒有空餘
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_map);
}

void add_iso(int target, glm::vec3 now_color, float now_alpha) {
    isolevel.push_back(target);
    each_iso[target] = { now_color, now_alpha };
    return;
}

void delete_iso(int target) {
    each_iso.erase(target);
    make_texture1d();
    return;
}

void read_file() {
    int total = 0;
    // 開啟.raw檔案
    ifstream inputFile(filename_list[filename_current], ios::binary);
    // 檢查檔案是否成功打開
    if (!inputFile.is_open()) {
        cout << "無法打開檔案！" << endl;
        return;
    }

    // 創建一個用於存儲值的向量
    vector<unsigned char> rawData(SIZEx * SIZEy * SIZEz);
    // 讀取.raw檔案的數據到向量中
    inputFile.read(reinterpret_cast<char*>(rawData.data()), rawData.size());
    // 關閉檔案
    inputFile.close();
    // 將unsigned char的值轉換為整數並存儲到三維陣列中
    int index = 0;
    for (int i = 0; i < SIZEx; i++) {
        for (int j = 0; j < SIZEy; j++) {
            for (int k = 0; k < SIZEz; k++) {
                integerData[i][j][k] = static_cast<int>(rawData[index]);

                cnt[integerData[i][j][k]]++;

                glm::vec3 dxyz;
                if (i == 0) dxyz.x = integerData[i + 1][j][k] - integerData[i][j][k];//後微
                else if (i == SIZEx - 1) dxyz.x = integerData[i][j][k] - integerData[i - 1][j][k];//前微
                else dxyz.x = (integerData[i + 1][j][k] - integerData[i - 1][j][k]) / 2;//中微

                if (j == 0) dxyz.y = integerData[i][j + 1][k] - integerData[i][j][k];//後微
                else if (j == SIZEy - 1) dxyz.y = integerData[i][j][k] - integerData[i][j - 1][k];//前微
                else dxyz.y = (integerData[i][j + 1][k] - integerData[i][j - 1][k]) / 2;//中微

                if (k == 0) dxyz.z = integerData[i][j][k + 1] - integerData[i][j][k];//後微
                else if (k == SIZEz - 1) dxyz.z = integerData[i][j][k] - integerData[i][j][k - 1];//前微
                else dxyz.z = (integerData[i][j][k + 1] - integerData[i][j][k - 1]) / 2;//中微

                dxyz = glm::normalize(dxyz);//range:-1~1
                dxyz = (dxyz + 1.0f) / 2.0f;//range:0~1
                dxyz = 255.0f * dxyz;//range:0~255

                gradient[index][0] = dxyz.x;
                gradient[index][1] = dxyz.y;
                gradient[index][2] = dxyz.z;
                gradient[index][3] = integerData[i][j][k];
                index++;
            }
        }
    }
    for (auto it : cnt) {
        //cout << it.first << " " << it.second << '\n';
        total += it.second;
        cnt_vec.push_back(log10(it.second));
    }
    //if (total == SIZEx * SIZEy * SIZEz) cout << "same\n";
    return;
}

void camera_rotation() {
    cameraFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront.y = sin(glm::radians(pitch));
    cameraFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(cameraFront);
    cameraRight = glm::normalize(glm::cross(cameraFront, worldUp));
    cameraUp = glm::normalize(glm::cross(cameraRight, cameraFront));
}

void gui() {
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    static int counter = 0;

    ImGui::Begin("UI");

    if (ImGui::Combo("filename", &filename_current, filename_list, IM_ARRAYSIZE(filename_list))) {
        re0 = true;
    }

    ImGui::PlotHistogram("isovalue", cnt_vec.data(), cnt.size(), 0, NULL, FLT_MAX, FLT_MAX, ImVec2(200, 130));

    ImGui::Checkbox("phong", &check_phong);

    ImGui::SliderInt("Isovalue", &intValue, 0, 255);
    //ImGui::InputInt("Isovalue", &intValue, 1);
    ImGui::SameLine();
    if (ImGui::Button("-")) {
        intValue--;
    }
    ImGui::SameLine();
    if (ImGui::Button("+")) {
        intValue++;
    }
    static float color[4] = { 1.0f, 1.0f, 1.0f, 0.2f };
    ImGui::ColorEdit4("Color Picker", color);
    if (ImGui::Button("IN!")) {
        add_iso(intValue, glm::vec3(color[0] * 255, color[1] * 255, color[2] * 255), color[3] * 255);
    }

    ImGui::PlotLines("Alpha", plotData[3], IM_ARRAYSIZE(plotData[3]), 0, NULL, 0, 255, ImVec2(200, 130));

    if (ImGui::CollapsingHeader("Isovalue List")) {
        for (auto it : each_iso) {
            ImGui::Text("%d", it.first);
            ImGui::SameLine();
            glm::vec4 isocolor = glm::vec4(it.second.first, it.second.second) / 255.0f;
            ImGui::ColorButton("MyColor##3c", *(ImVec4*)&isocolor, ImGuiColorEditFlags_NoBorder);
        }
    }

    ImGui::SliderInt("Now_Isovalue", &delete_intValue, 0, 255);
    ImGui::SameLine();
    if (ImGui::Button("--")) {
        delete_intValue--;
    }
    ImGui::SameLine();
    if (ImGui::Button("++")) {
        delete_intValue++;
    }
    //ImGui::InputInt("Now_Isovalue", &delete_intValue, 1);
    if (ImGui::Button("delete")) {
        delete_iso(delete_intValue);
    }

    /*if (ImGui::CollapsingHeader("Camera")) {
        ImGui::Separator();
        ImGui::Text("Transform:");
        ImGui::Separator();
        ImGui::Indent();

        ImGui::PushItemWidth(80);
        float cameraSpeed = static_cast<float>(100.0 * deltaTime);
        if (ImGui::DragFloat("X", &eyeDx, 0.1f)) {
            if (eyeDx - pre_eyeDx >= 0) cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;//右移
            else cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;//左移
            pre_eyeDx = eyeDx;
        }
        ImGui::SameLine();
        if (ImGui::DragFloat("Y", &eyeDy, 0.1f)) {
            if (eyeDy - pre_eyeDy >= 0) cameraPos += cameraSpeed * cameraUp;//上移
            else cameraPos -= cameraSpeed * cameraUp;//下移
            pre_eyeDy = eyeDy;
        }
        ImGui::SameLine();
        if (ImGui::DragFloat("Z", &eyeDz, 0.01f)) {
            if (eyeDz - pre_eyeDz >= 0) cameraPos += cameraSpeed * cameraFront;//後退
            else cameraPos -= cameraSpeed * cameraFront;//前進
            pre_eyeDz = eyeDz;
        }

        ImGui::PushItemWidth(240);
        float cameraRotSpeed = static_cast<float>(50.0 * deltaTime);
        if (ImGui::DragFloat("Pitch-X", &eyeAngx, 0.01f)) {
            glm::vec3 ex, ey, ez;
            if (eyeAngx - pre_eyeAngx >= 0) {
                pitch += cameraRotSpeed;
            }
            else {
                pitch -= cameraRotSpeed;
            }
            // make sure that when pitch is out of bounds, screen doesn't get flipped
            //if (pitch > 89.0f)
                //pitch = 89.0f;
            //if (pitch < -89.0f)
                //pitch = -89.0f;
            pre_eyeAngx = eyeAngx;
            camera_rotation();
        }
        if (ImGui::DragFloat("Yaw-Y", &eyeAngy, 0.01f)) {
            glm::vec3 ex, ey, ez;
            if (eyeAngy - pre_eyeAngy >= 0) {
                yaw += cameraRotSpeed;
            }
            else {
                yaw -= cameraRotSpeed;
            }
            pre_eyeAngy = eyeAngy;
            camera_rotation();
        }
        if (ImGui::DragFloat("Roll-Z", &eyeAngz, 0.01f)) {
            glm::vec3 ex, ey, ez;
            if (eyeAngz - pre_eyeAngz >= 0) {
            }
            else {
            }
            pre_eyeAngz = eyeAngz;
            camera_rotation();
        }
        ImGui::Unindent();
    }*/

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

    if (ImGui::Button("Reset")) {

        cameraPos = glm::vec3(0.0f, 0.0f, 300.0f);
        cameraU = glm::mat3(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

        eyeDx = 0, eyeDy = 0, eyeDz = 0;
        pre_eyeDx = 0, pre_eyeDy, pre_eyeDz = 0;
        eyeAngx = 0, eyeAngy = 0, eyeAngz = 0;
        pre_eyeAngx = 0, pre_eyeAngy = 0, pre_eyeAngz = 0;

        delete_intValue = 0;
        intValue = 0;
        check_phong = false;
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
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);/*面剔除*/
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // build and compile our shader program
    Shader ourShader("vex.vs", "fram.fs"); // you can name your shader files however you like

    read_file();

    init_vao_vbo();
    make_texture3d();
    make_texture1d();
    add_iso(200, glm::vec3(255, 0, 0), 50);

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
            intValue = 0, delete_intValue = 0;
            cnt.clear();
            cnt_vec.clear();
            each_iso.clear();
            read_file();
            make_texture3d();
            make_texture1d();
            add_iso(200, glm::vec3(255, 0, 0), 50);

            re0 = false;
        }

        make_texture1d();

        ourShader.use();
        ourShader.setInt("T0", 0);
        ourShader.setInt("T1", 1);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_3D, texName[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_1D, texName[1]);

        // be sure to activate shader when setting uniforms/drawing objects
        ourShader.use();
        ourShader.setInt("check_phong", (check_phong == true)? 1 : 0);
        ourShader.setVec3("light.direction", 300.0f, 300.0f, 300.0f);
        ourShader.setVec3("viewPos", cameraPos);

        // light properties
        ourShader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
        ourShader.setVec3("light.diffuse", 0.8f, 0.8f, 0.8f);
        ourShader.setVec3("light.specular", 0.1f, 0.1f, 0.1f);

        // material properties
        ourShader.setInt("material.diffuse", 0);
        ourShader.setVec3("material.specular", 0.5f, 0.5f, 0.5f);
        ourShader.setFloat("material.shininess", 89.599998f);

        // view/projection transformations
        glm::mat4 projection = glm::ortho(-250.0f, 250.0f, -250.0f * (float)SCR_HEIGHT / (float)SCR_WIDTH, 250.0f * (float)SCR_HEIGHT / (float)SCR_WIDTH, 0.1f, 1000.0f);
        //glm::mat4 projection = glm::ortho(0.0f, (float)SCR_WIDTH, (float)SCR_HEIGHT, 0.0f, -1.0f, 1.0f);
        glm::mat4 view;
        view = glm::lookAt(cameraPos, cameraPos - cameraU[2], cameraU[1]);
        //view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        //view = glm::lookAt(glm::vec3(0.0f, 0.0f, 300.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 model = glm::mat4(1.0f);
        ourShader.setMat4("projection", projection);
        ourShader.setVec3("raydir", -cameraU[2]);
        ourShader.setMat4("view", view);
        
        ourShader.setMat4("model", model);

        ourShader.use();
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

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