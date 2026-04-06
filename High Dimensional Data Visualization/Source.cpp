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

int num_records, num_features, total = 0;
int N = 984;
double e = 0.0000001, alpha = 0.3, lamda = 1.0, eps = 0.0000001;
double old_c, new_c = 0.0;
vector<vector<double>> dataarr, small_dataarr;
vector<vector<double>> old_dis;
vector<glm::dvec2> Q, Q_label[2], Q_pca[2];
vector<double> VaoData, ellipse_data;
glm::dvec2 Pu[2], eigenvector1[2], eigenvector2[2];
pair<double, double> eigenvalues[2];

unsigned int VBO[2], VAO[2];

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

void init_vao_vbo_1() {
    /*for (int Q_class = 0; Q_class < 2; Q_class++) {
        for (int i = 0; i < Q_pca[Q_class].size(); i++) {
            VaoData.push_back(Q_pca[Q_class][i][0]);
            VaoData.push_back(Q_pca[Q_class][i][1]);
            if (Q_class == 0) VaoData.push_back(0.0);
            else VaoData.push_back(1.0);
        }
    }*/
    for (int i = 0; i < Q.size(); i++) {
        VaoData.push_back(Q[i][0]);
        VaoData.push_back(Q[i][1]);
        VaoData.push_back(small_dataarr[i][15]);
    }

    glGenVertexArrays(2, VAO);
    glGenBuffers(2, VBO);

    glBindVertexArray(VAO[0]);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, VaoData.size() * sizeof(double), VaoData.data(), GL_STATIC_DRAW);//vertices要 * sizeof(float)

    // position attribute
    glVertexAttribPointer(0, 2, GL_DOUBLE, GL_FALSE, 3 * sizeof(double), (void*)0);
    glEnableVertexAttribArray(0);
    
    // 異常 attribute
    glVertexAttribPointer(1, 1, GL_DOUBLE, GL_FALSE, 3 * sizeof(double), (void*)(2 * sizeof(double)));
    glEnableVertexAttribArray(1);
}

void init_vao_vbo_2() {
    for (int Q_class = 0; Q_class < 2; Q_class++) {
        ellipse_data.push_back(Pu[Q_class].x);
        ellipse_data.push_back(Pu[Q_class].y);
        ellipse_data.push_back(eigenvector1[Q_class].x);
        ellipse_data.push_back(eigenvector1[Q_class].y);
        ellipse_data.push_back(eigenvector2[Q_class].x);
        ellipse_data.push_back(eigenvector2[Q_class].y);
        ellipse_data.push_back(eigenvalues[Q_class].first);
        ellipse_data.push_back(eigenvalues[Q_class].second);
        ellipse_data.push_back((double)Q_class);
    }

    glBindVertexArray(VAO[1]);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, ellipse_data.size() * sizeof(double), ellipse_data.data(), GL_STATIC_DRAW);

    // 中心點
    glVertexAttribPointer(0, 2, GL_DOUBLE, GL_FALSE, 9 * sizeof(double), (void*)0);
    glEnableVertexAttribArray(0);

    //第一個向量
    glVertexAttribPointer(1, 2, GL_DOUBLE, GL_FALSE, 9 * sizeof(double), (void*)(2 * sizeof(double)));
    glEnableVertexAttribArray(1);

    //第二個向量
    glVertexAttribPointer(2, 2, GL_DOUBLE, GL_FALSE, 9 * sizeof(double), (void*)(4 * sizeof(double)));
    glEnableVertexAttribArray(2);

    //特徵值1
    glVertexAttribPointer(3, 1, GL_DOUBLE, GL_FALSE, 9 * sizeof(double), (void*)(6 * sizeof(double)));
    glEnableVertexAttribArray(3);

    //特徵值2
    glVertexAttribPointer(4, 1, GL_DOUBLE, GL_FALSE, 9 * sizeof(double), (void*)(7 * sizeof(double)));
    glEnableVertexAttribArray(4);

    //哪個圓
    glVertexAttribPointer(5, 1, GL_DOUBLE, GL_FALSE, 9 * sizeof(double), (void*)(8 * sizeof(double)));
    glEnableVertexAttribArray(5);
}

void read_file() {
    ifstream infile("creditcard.dat");
    if (!infile.is_open()) {
        cerr << "Unable to open file";
        return;
    }
    
    string line;

    // 逐行讀取資料
    while (getline(infile, line)) {
        istringstream iss(line);
        vector<double> record;
        string value;

        if (total == 0) {
            int hh = 0;
            while (getline(iss, value, ',')) {
                if (hh == 0) num_records = stod(value);
                else  num_features = stod(value);
                hh++;
            }
        }

        else {
            // 逐個讀取每一個特徵值
            while (getline(iss, value, ',')) {
                record.push_back(stod(value));
            }
            // 添加到數據向量中
            dataarr.push_back(record);
        }
        
        total++;
    }

    // 關閉文件
    infile.close();

    // 從 dataarr 中隨機挑選40筆資料
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> dis(0, num_records - 1);

    set<int> chosen_indices;

    while (chosen_indices.size() < N) {
        chosen_indices.insert(dis(gen));
    }

    for (int index : chosen_indices) {
        small_dataarr.push_back(dataarr[index]);
    }

    return;
}

// Transpose an N*2 matrix to a 2*N matrix
vector<vector<double>> transposeNx2to2xN(const vector<glm::dvec2>& matrix) {
    int N = matrix.size();
    vector<vector<double>> transposed(2, vector<double>(N));

    for (int i = 0; i < N; ++i) {
        transposed[0][i] = matrix[i].x;
        transposed[1][i] = matrix[i].y;
    }

    return transposed;
}

//矩陣乘法，會乘出N*N
void matrixMultiply(const vector<vector<double>>& A, const vector<glm::dvec2>& B, vector<vector<double>>& result) {
    result.resize(2, vector<double>(2, 0.0));

    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {
            for (int k = 0; k < A[0].size(); ++k) {
                result[i][j] += A[i][k] * (j == 0 ? B[k].x : B[k].y);
            }
        }
    }
}

// Function to calculate the eigenvalues of a 2x2 symmetric matrix
pair<double, double> calculateEigenvalues(const vector<vector<double>>& matrix) {
    double a = matrix[0][0];
    double b = matrix[0][1];
    double d = matrix[1][1];

    double trace = a + d;
    double determinant = a * d - b * b;
    double term = sqrt((a - d) * (a - d) + 4 * b * b);

    double lambda1 = (trace + term) / 2.0;
    double lambda2 = (trace - term) / 2.0;

    return make_pair(lambda1, lambda2);
}

// Function to calculate the eigenvector for a given eigenvalue of a 2x2 symmetric matrix
glm::dvec2 calculateEigenvector(const vector<vector<double>>& matrix, double eigenvalue) {
    double a = matrix[0][0];
    double b = matrix[0][1];
    double d = matrix[1][1];

    glm::dvec2 eigenvector;
    if (b != 0) {
        eigenvector = glm::normalize(glm::dvec2(eigenvalue - d, b));
    }
    else {
        eigenvector = glm::normalize(glm::dvec2(1, 0));
    }

    return eigenvector;
}

void PCA() {
    for (int i = 0; i < N; i++) {
        if (small_dataarr[i][15] == 0) Q_label[0].push_back(Q[i]);
        else Q_label[1].push_back(Q[i]);
    }

    for (int Q_class = 0; Q_class < 2; Q_class++) {
        //mean
        Pu[Q_class] = glm::dvec2(0, 0);
        for (int i = 0; i < Q_label[Q_class].size(); i++) {
            Pu[Q_class] += Q_label[Q_class][i];
        }
        Pu[Q_class] /= Q_label[Q_class].size();

        //平移
        vector<glm::dvec2> XT;
        XT.resize(Q_label[Q_class].size());//40*2
        for (int i = 0; i < Q_label[Q_class].size(); i++) {
            XT[i] = Q_label[Q_class][i] - Pu[Q_class];
        }

        //form 2*2
        vector<vector<double>> X = transposeNx2to2xN(XT);//2*40
        vector<vector<double>> scatter_matrix;
        matrixMultiply(X, XT, scatter_matrix);

        cout << "A對稱矩陣\n";
        cout << scatter_matrix[0][0] << " " << scatter_matrix[0][1] << '\n';
        cout << scatter_matrix[1][0] << " " << scatter_matrix[1][1] << '\n';

        // Calculate eigenvalues
        eigenvalues[Q_class] = calculateEigenvalues(scatter_matrix);
        if (fabs(eigenvalues[Q_class].first) < fabs(eigenvalues[Q_class].second)) {
            swap(eigenvalues[Q_class].first, eigenvalues[Q_class].second);
        }
        cout << "Eigenvalues: " << eigenvalues[Q_class].first << ", " << eigenvalues[Q_class].second << '\n';

        // Calculate eigenvectors
        eigenvector1[Q_class] = calculateEigenvector(scatter_matrix, eigenvalues[Q_class].first);
        eigenvector2[Q_class] = calculateEigenvector(scatter_matrix, eigenvalues[Q_class].second);

        cout << "Eigenvector 1: (" << eigenvector1[Q_class].x << ", " << eigenvector1[Q_class].y << ")" << '\n';
        cout << "Eigenvector 2: (" << eigenvector2[Q_class].x << ", " << eigenvector2[Q_class].y << ")" << '\n';

        Q_pca[Q_class].resize(Q_label[Q_class].size());
        for (int i = 0; i < Q_label[Q_class].size(); i++) {
            Q_pca[Q_class][i].x = glm::dot(Q_label[Q_class][i], eigenvector1[Q_class]);
            Q_pca[Q_class][i].y = glm::dot(Q_label[Q_class][i], eigenvector2[Q_class]);
        }
    }
}

void compute_old_dis() {
    old_dis.resize(N, vector<double>(N, 0));
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            double acc = 0;
            for (int k = 0; k < num_features - 1; k++) {//最後一個是異常特徵，不參與計算
                acc += (small_dataarr[i][k] - small_dataarr[j][k]) * (small_dataarr[i][k] - small_dataarr[j][k]);
            }
            old_dis[i][j] = sqrt(acc);
            old_c += acc;
        }
    }
}

void compute_Q() {
    compute_old_dis();
    Q.resize(N);
    srand(time(NULL));
    for (int i = 0; i < N; i++) {//init
        double x = (double)rand() / (RAND_MAX + 1.0);
        Q[i][0] = x;
        x = (double)rand() / (RAND_MAX + 1.0);
        Q[i][1] = x;
    }

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            new_c += sqrt((Q[i][0] - Q[j][0]) * (Q[i][0] - Q[j][0]) + (Q[i][1] - Q[j][1]) * (Q[i][1] - Q[j][1]));
        }
    }

    while (abs(old_c - new_c) > eps) {//座標變動太小時跳出
        old_c = new_c;
        new_c = 0;
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                if (i == j) continue;
                double new_dis = sqrt((Q[i][0] - Q[j][0]) * (Q[i][0] - Q[j][0]) + (Q[i][1] - Q[j][1]) * (Q[i][1] - Q[j][1]));
                if (new_dis == 0.0) new_dis = e;

                glm::dvec2 deltaQi = lamda * (old_dis[i][j] - new_dis) / new_dis * (Q[i] - Q[j]);
                glm::dvec2 deltaQj = -deltaQi;
                Q[i] = Q[i] + deltaQi;
                Q[j] = Q[j] + deltaQj;
            }
        }

        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                new_c += sqrt((Q[i][0] - Q[j][0]) * (Q[i][0] - Q[j][0]) + (Q[i][1] - Q[j][1]) * (Q[i][1] - Q[j][1]));
            }
        }
        lamda = alpha * lamda;
    }
}

void gui() {
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("UI");

    if (ImGui::CollapsingHeader("Camera")) {
        ImGui::Separator();
        ImGui::Text("Transform:");
        ImGui::Separator();
        ImGui::Indent();

        ImGui::PushItemWidth(80);
        float cameraSpeed = static_cast<float>(10.0 * deltaTime);
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

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);
    //glFrontFace(GL_CCW);/*面剔除*/
    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // build and compile our shader program
    Shader ourShader("vex.vs", "fram.fs", "geometry.gs"); // you can name your shader files however you like
    Shader ellipse("ellipse_vex.vs", "ellipse_fram.fs", "ellipse_geometry.gs");

    read_file();
    compute_Q();
    PCA();
    init_vao_vbo_1();
    init_vao_vbo_2();

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
        glClearColor(1.0, 1.0, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // be sure to activate shader when setting uniforms/drawing objects
        ourShader.use();

        // view/projection transformations
        glm::mat4 projection = glm::ortho(-3.0f, 3.0f, -3.0f * (float)SCR_HEIGHT / (float)SCR_WIDTH, 3.0f * (float)SCR_HEIGHT / (float)SCR_WIDTH, 0.1f, 1000.0f);
        //glm::mat4 projection = glm::ortho(0.0f, (float)SCR_WIDTH, (float)SCR_HEIGHT, 0.0f, -1.0f, 1.0f);
        glm::mat4 view;
        view = glm::lookAt(cameraPos, cameraPos - cameraU[2], cameraU[1]);
        glm::mat4 model = glm::mat4(1.0f);
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);
        ourShader.setMat4("model", model);

        ellipse.use();
        ellipse.setMat4("projection", projection);
        ellipse.setMat4("view", view);
        ellipse.setMat4("model", model);

        ourShader.use();
        glBindVertexArray(VAO[0]);
        glDrawArrays(GL_POINTS, 0, VaoData.size() / 3);

        ellipse.use();
        glBindVertexArray(VAO[1]);
        glDrawArrays(GL_POINTS, 0, ellipse_data.size() / 9);

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
    glDeleteVertexArrays(2, VAO);
    glDeleteBuffers(2, VBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}