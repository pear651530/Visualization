#include <iostream>
#include <algorithm>  
#include <cmath>  
#include <iomanip>  
#include <iostream>  
#include <vector> 
#include <map> 
#include <fstream>
#include <random>
#include <set>
#include <cstdio>

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

vector<double> VaoData;

int num_records, num_features;
double num_min = INT_MAX, num_max = INT_MIN;
vector<vector<double>> dataarr;
vector<vector<vector<double>>> lattice;
double learning_rate = 0.01, learning_rate_start = 0.01, dis_range = 32;
int iteration = 500, iteration_limit = 200000, iteration_cnt = 0;

const char* filename_list[] = { "vaseSurface.txt", "bunnySurface.txt", "teapotSurface.txt" };
int filename_current = 1;
unsigned int VBO, VAO;
bool som_training = true, re0 = false;
bool mesh = true;

enum class Options {
    Cylindrical,
    Plane
};
Options mode = Options::Cylindrical;

//材質圖片
unsigned int bg;

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
float   cv = cos(5.0 * PI / 180.0), sv = sin(5.0 * PI / 180.0); /* cos(5.0) and sin(5.0) */
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

unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
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

glm::dvec3 gradient(pair<int, int> a, pair<int, int> b, pair<int, int> c) {
    glm::dvec3 A(lattice[a.first][a.second][0], lattice[a.first][a.second][1], lattice[a.first][a.second][2]);
    glm::dvec3 B(lattice[b.first][b.second][0], lattice[b.first][b.second][1], lattice[b.first][b.second][2]);
    glm::dvec3 C(lattice[c.first][c.second][0], lattice[c.first][c.second][1], lattice[c.first][c.second][2]);

    glm::dvec3 AB = B - A;
    glm::dvec3 AC = C - A;

    glm::dvec3 normal = glm::cross(AB, AC);

    normal = glm::normalize(normal);

    return normal;
}

void init_vao_vbo() {
    VaoData.clear();

    if (mesh) {
        for (int i = 0; i < 32; i++) {//x
            for (int j = 1; j < 32; j++) {//y
                if (mode == Options::Plane && i == 0) continue;
                glm::dvec3 dxyz0(0, 0, 0);

                VaoData.push_back(dxyz0.x);
                VaoData.push_back(dxyz0.y);
                VaoData.push_back(dxyz0.z);
                for (int k = 0; k < num_features; k++) {
                    VaoData.push_back(lattice[i][j][k]);
                }
                VaoData.push_back((double)i / (double)31);
                VaoData.push_back((double)j / (double)31);
                //該點與上點連線
                VaoData.push_back(dxyz0.x);
                VaoData.push_back(dxyz0.y);
                VaoData.push_back(dxyz0.z);
                for (int k = 0; k < num_features; k++) {
                    VaoData.push_back(lattice[i][j - 1][k]);
                }
                VaoData.push_back((double)i / (double)31);
                VaoData.push_back((double)(j - 1) / (double)31);

                VaoData.push_back(dxyz0.x);
                VaoData.push_back(dxyz0.y);
                VaoData.push_back(dxyz0.z);
                for (int k = 0; k < num_features; k++) {
                    VaoData.push_back(lattice[i][j][k]);
                }
                VaoData.push_back((double)i / (double)31);
                VaoData.push_back((double)j / (double)31);
                VaoData.push_back(dxyz0.x);
                VaoData.push_back(dxyz0.y);
                VaoData.push_back(dxyz0.z);
                if (i != 0) {//該點與左點點連線
                    for (int k = 0; k < num_features; k++) {
                        VaoData.push_back(lattice[i - 1][j][k]);
                    }
                    VaoData.push_back((double)(i - 1) / (double)31);
                    VaoData.push_back((double)j / (double)31);
                }
                else if (i == 0) {//該點因為圓柱與左點點連線
                    for (int k = 0; k < num_features; k++) {
                        VaoData.push_back(lattice[31][j][k]);
                    }
                    VaoData.push_back((double)31 / (double)31);
                    VaoData.push_back((double)j / (double)31);
                }
            }
        }
    }
    else {
        for (int i = 0; i < 32; i++) {//x
            for (int j = 1; j < 32; j++) {//y
                if (mode == Options::Plane && i == 0) continue;
                glm::dvec3 dxyz0, dxyz1;

                //下三角
                if (i != 0) dxyz0 = gradient({ i, j }, { i, j - 1 }, { i - 1, j });//逆時針
                else if (i == 0) dxyz0 = gradient({ i, j }, { i, j - 1 }, { 31, j });

                VaoData.push_back(dxyz0.x);
                VaoData.push_back(dxyz0.y);
                VaoData.push_back(dxyz0.z);
                for (int k = 0; k < num_features; k++) {
                    VaoData.push_back(lattice[i][j][k]);
                }
                VaoData.push_back((double)i / (double)31);
                VaoData.push_back((double)j / (double)31);

                //該點與上點連線
                VaoData.push_back(dxyz0.x);
                VaoData.push_back(dxyz0.y);
                VaoData.push_back(dxyz0.z);
                for (int k = 0; k < num_features; k++) {
                    VaoData.push_back(lattice[i][j - 1][k]);
                }
                VaoData.push_back((double)i / (double)31);
                VaoData.push_back((double)(j - 1) / (double)31);

                VaoData.push_back(dxyz0.x);
                VaoData.push_back(dxyz0.y);
                VaoData.push_back(dxyz0.z);
                if (i != 0) {//該點與左點點連線
                    for (int k = 0; k < num_features; k++) {
                        VaoData.push_back(lattice[i - 1][j][k]);
                    }
                    VaoData.push_back((double)(i - 1) / (double)31);
                    VaoData.push_back((double)j / (double)31);
                }
                else if (i == 0) {//該點因為圓柱與左點點連線
                    for (int k = 0; k < num_features; k++) {
                        VaoData.push_back(lattice[31][j][k]);
                    }
                    VaoData.push_back((double)31 / (double)31);
                    VaoData.push_back((double)j / (double)31);
                }

                //上三角
                if (i != 0) dxyz1 = gradient({ i - 1, j - 1 }, { i - 1, j }, { i, j - 1 });//逆時針
                else if (i == 0) dxyz1 = gradient({ 31, j - 1 }, { 31, j }, { i, j - 1 });

                VaoData.push_back(dxyz1.x);
                VaoData.push_back(dxyz1.y);
                VaoData.push_back(dxyz1.z);
                if (i != 0) {
                    for (int k = 0; k < num_features; k++) {
                        VaoData.push_back(lattice[i - 1][j - 1][k]);
                    }
                    VaoData.push_back((double)(i - 1) / (double)31);
                    VaoData.push_back((double)(j - 1) / (double)31);
                }
                else {
                    for (int k = 0; k < num_features; k++) {
                        VaoData.push_back(lattice[31][j - 1][k]);
                    }
                    VaoData.push_back((double)31 / (double)31);
                    VaoData.push_back((double)(j - 1) / (double)31);
                }

                //該點與下點連線
                VaoData.push_back(dxyz1.x);
                VaoData.push_back(dxyz1.y);
                VaoData.push_back(dxyz1.z);
                if (i != 0) {
                    for (int k = 0; k < num_features; k++) {
                        VaoData.push_back(lattice[i - 1][j][k]);
                    }
                    VaoData.push_back((double)(i - 1) / (double)31);
                    VaoData.push_back((double)j / (double)31);
                }
                else {
                    for (int k = 0; k < num_features; k++) {
                        VaoData.push_back(lattice[31][j][k]);
                    }
                    VaoData.push_back((double)31 / (double)31);
                    VaoData.push_back((double)j / (double)31);
                }

                //該點與右點點連線
                VaoData.push_back(dxyz1.x);
                VaoData.push_back(dxyz1.y);
                VaoData.push_back(dxyz1.z);
                for (int k = 0; k < num_features; k++) {
                    VaoData.push_back(lattice[i][j - 1][k]);
                }
                VaoData.push_back((double)i / (double)31);
                VaoData.push_back((double)(j - 1) / (double)31);
            }
        }
    }

    /*for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            cout << lattice[i][j][0] << " " << lattice[i][j][1] << " " << lattice[i][j][2] << '\n';
        }
    }*/
    
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, VaoData.size() * sizeof(double), VaoData.data(), GL_STATIC_DRAW);//vertices要 * sizeof(float)

    // normal attribute
    glVertexAttribPointer(0, 3, GL_DOUBLE, GL_FALSE, 8 * sizeof(double), (void*)0);
    glEnableVertexAttribArray(0);

    // position attribute
    glVertexAttribPointer(1, 3, GL_DOUBLE, GL_FALSE, 8 * sizeof(double), (void*)(3 * sizeof(double)));
    glEnableVertexAttribArray(1);

    // texture attribute
    glVertexAttribPointer(2, 2, GL_DOUBLE, GL_FALSE, 8 * sizeof(double), (void*)(6 * sizeof(double)));
    glEnableVertexAttribArray(2);
}

void read_file() {
    ifstream infile(filename_list[filename_current]);
    if (!infile.is_open()) {
        cerr << "Unable to open file";
        return;
    }

    string line;

    getline(infile, line);
    istringstream iss(line);
    iss >> num_records >> num_features;

    dataarr.resize(num_records);
    int recordIndex = 0;
    while (getline(infile, line)) {
        istringstream recordStream(line);
        int in;
        while (recordStream >> in) {
            dataarr[recordIndex].push_back(in);
            num_min = min(num_min, (double)in);
            num_max = max(num_max, (double)in);
            if (dataarr[recordIndex].size() == 3) ++recordIndex;
        }
    }

    // 關閉文件
    infile.close();

    return;
}

void init() {
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dis(num_min, num_max);

    lattice.resize(32, vector<vector<double>>(32, vector<double>(num_features, 0)));
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            /*lattice[i][j][0] = i*5;
            lattice[i][j][1] = j * 5;
            lattice[i][j][2] = 0;*/
            for (int k = 0; k < num_features; k++) {
                lattice[i][j][k] = dis(gen);
            }
        }
    }
}

pair<int, int> chose_winner(int it) {
    double k = 1, dmin = 0, diff;
    vector<double> data_P;
    pair<int, int> dmin_winner(0, 0);

    data_P = dataarr[0];
    for (int i = 0; i < num_features; i++) {
        dmin += (data_P[i] - lattice[0][0][i]) * (data_P[i] - lattice[0][0][i]);
    }
    
    data_P = dataarr[it];
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            if (i == 0 && j == 0) continue;
            diff = 0;
            for (int m = 0; m < num_features; m++) {
                diff += (data_P[m] - lattice[i][j][m]) * (data_P[m] - lattice[i][j][m]);
            }
            if (diff < dmin) {
                dmin = diff;
                k = 1;
                dmin_winner = { i, j };
            }
            else if (diff == dmin) {
                k++;
                srand(time(NULL));
                double r = (double)rand() / (RAND_MAX + 1.0);
                if (r >= 1 / k) {
                    dmin_winner = { i, j };
                }
            }
        }
    }
    return dmin_winner;
}

double scaling_coeff(pair<int, int> dmin_winner, pair<int, int> neighbore_node) {
    double dis = (dmin_winner.first - neighbore_node.first) * (dmin_winner.first - neighbore_node.first) + (dmin_winner.second - neighbore_node.second) * (dmin_winner.second - neighbore_node.second);
    dis = sqrt(dis);
    return exp(-dis);
}

void som() {
    int it;
    vector<double> data_P;
    pair<int, int> dmin_winner;
    double dis1, dis2, norm;
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> rand_dis(0, num_records - 1);
    while (1) {
        norm = 0;
        it = rand_dis(gen);
        //srand(time(NULL));
        //it = (double)rand() / RAND_MAX * num_records;
        //cout << it << " ";
        dmin_winner = chose_winner(it);
        
        data_P = dataarr[it];
        for (int i = 0; i < num_features; i++) {
            double hold = lattice[dmin_winner.first][dmin_winner.second][i] +
                learning_rate * (data_P[i] - lattice[dmin_winner.first][dmin_winner.second][i]);
            norm += abs(hold - lattice[dmin_winner.first][dmin_winner.second][i]);
            lattice[dmin_winner.first][dmin_winner.second][i] = hold;
        }

        for (int i = 0; i < 32; i++) {
            for (int j = 0; j < 32; j++) {
                if (mode == Options::Cylindrical) {
                    dis1 = (dmin_winner.first - i) * (dmin_winner.first - i) + (dmin_winner.second - j) * (dmin_winner.second - j);
                    dis2 = (32 - abs(dmin_winner.first - i)) * (32 - abs(dmin_winner.first - i)) + (dmin_winner.second - j) * (dmin_winner.second - j);
                    dis1 = min(dis1, dis2);
                    dis1 = sqrt(dis1);
                    if (dis1 <= dis_range) {
                        for (int k = 0; k < num_features; k++) {
                            double hold = lattice[i][j][k] +
                                learning_rate * (data_P[k] - lattice[i][j][k]);//scaling_coeff(dmin_winner, { i, j }) *
                            norm += abs(hold - lattice[i][j][k]);
                            lattice[i][j][k] = hold;
                        }
                    }
                }
                else {
                    dis1 = (dmin_winner.first - i) * (dmin_winner.first - i) + (dmin_winner.second - j) * (dmin_winner.second - j);
                    dis1 = sqrt(dis1);
                    if (dis1 <= dis_range) {
                        for (int k = 0; k < num_features; k++) {
                            double hold = lattice[i][j][k] +
                                learning_rate * (data_P[k] - lattice[i][j][k]);//scaling_coeff(dmin_winner, { i, j }) *
                            norm += abs(hold - lattice[i][j][k]);
                            lattice[i][j][k] = hold;
                        }
                    }
                }
            }
        }

        iteration_cnt++;
        if (iteration_cnt == iteration_limit || norm <= 0.00000001) {//迭帶太多次或權重變動小
            som_training = false;
            break;
        }
        if (iteration_cnt % iteration == 0) {
            init_vao_vbo();
            learning_rate = learning_rate_start * exp(-((double)iteration_cnt / (double)iteration_limit));
            dis_range *= (double)(iteration_limit - iteration_cnt) / iteration_limit * (double)(iteration_limit - iteration_cnt) / iteration_limit;
            dis_range = fmax(1, dis_range);
            //needs_render = true;
            break;
        }
    }
    return;
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

    ImGui::Checkbox("mesh", &mesh);

    if (ImGui::RadioButton("Cylindrical", mode == Options::Cylindrical)) {
        mode = Options::Cylindrical;
        re0 = true;
    }ImGui::SameLine();
    if (ImGui::RadioButton("Plane", mode == Options::Plane)) {
        mode = Options::Plane;
        re0 = true;
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
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);
    //glFrontFace(GL_CCW);/*面剔除*/
    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // build and compile our shader program
    Shader ourShader("vex.vs", "fram.fs"); // you can name your shader files however you like, "geometry.gs"

    read_file();
    bg = loadTexture("image\\geo_bg.jpg");
    init();
    //som();
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

        // Check if we need to render updated lattice
        if (som_training) {
            som(); // Continue the SOM iteration
        }

        if (re0) {
            cameraPos = glm::vec3(0.0f, 0.0f, 300.0f);
            cameraU = glm::mat3(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

            eyeDx = 0, eyeDy = 0, eyeDz = 0;
            pre_eyeDx = 0, pre_eyeDy, pre_eyeDz = 0;
            eyeAngx = 0, eyeAngy = 0, eyeAngz = 0;
            pre_eyeAngx = 0, pre_eyeAngy = 0, pre_eyeAngz = 0;

            glDeleteVertexArrays(1, &VAO);
            glDeleteBuffers(1, &VBO);
            num_min = INT_MAX, num_max = INT_MIN;
            learning_rate = 0.01, learning_rate_start = 0.01, dis_range = 32;
            iteration = 500, iteration_limit = 200000, iteration_cnt = 0;
            som_training = true;
            mesh = true;
            dataarr.clear();
            lattice.clear();

            read_file();
            init();
            init_vao_vbo();
            re0 = false;
        }

        // render
        // ------
        gui();
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // bind bg map
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, bg);

        // be sure to activate shader when setting uniforms/drawing objects
        ourShader.use();

        // view/projection transformations
        glm::mat4 projection = glm::ortho(-300.0f, 300.0f, -300.0f * (float)SCR_HEIGHT / (float)SCR_WIDTH, 300.0f * (float)SCR_HEIGHT / (float)SCR_WIDTH, 0.1f, 1000.0f);
        //glm::mat4 projection = glm::ortho(0.0f, (float)SCR_WIDTH, (float)SCR_HEIGHT, 0.0f, -1.0f, 1.0f);
        glm::mat4 view;
        view = glm::lookAt(cameraPos, cameraPos - cameraU[2], cameraU[1]);
        glm::mat4 model = glm::mat4(1.0f);
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);
        ourShader.setMat4("model", model);
        ourShader.setVec3("viewPos", cameraPos);
        ourShader.setInt("mesh", (mesh == true) ? 1 : 0);

        // light properties
        ourShader.setVec3("light.direction", -500.0f, -500.0f, -500.0f);
        ourShader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
        ourShader.setVec3("light.diffuse", 0.8f, 0.8f, 0.8f);
        ourShader.setVec3("light.specular", 0.1f, 0.1f, 0.1f);

        // material properties
        ourShader.setInt("material.diffuse", 0);
        ourShader.setVec3("material.specular", 0.5f, 0.5f, 0.5f);
        ourShader.setFloat("material.shininess", 89.599998f);

        ourShader.use();
        glBindVertexArray(VAO);
        if (mesh) glDrawArrays(GL_LINES, 0, VaoData.size() / 8);
        else glDrawArrays(GL_TRIANGLES, 0, VaoData.size() / 8);

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