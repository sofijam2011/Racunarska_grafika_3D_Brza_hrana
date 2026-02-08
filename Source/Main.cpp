#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>
#include <thread>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../Header/glm/glm.hpp"
#include "../Header/glm/gtc/type_ptr.hpp"
#include "../Header/glm/gtc/matrix_transform.hpp"

#include "../Header/Util.h"

// konstante
const int TARGET_FPS = 75;
const double FRAME_TIME = 1.0 / TARGET_FPS;
const float MOVE_SPEED = 2.0f;
const float MOUSE_SENSITIVITY = 0.1f;
const float CAMERA_SPEED = 3.0f;

// stanja
enum GameState {
    STATE_MENU,
    STATE_COOKING,
    STATE_ASSEMBLY,
    STATE_FINISHED
};

// sastojci
enum Ingredient {
    ING_DONJA_ZEMICKA,   // 0
    ING_PLJESKAVICA,     // 1
    ING_KECAP,           // 2
    ING_SENF,            // 3
    ING_KRASTAVCICI,     // 4
	ING_KRASTAVCICI2,    // 5 
    ING_LUK,             // 6
    ING_SALATA,          // 7
    ING_SIR,             // 8
    ING_PARADAJZ,        // 9
    ING_GORNJA_ZEMICKA,  // 10
    ING_TOTAL            // 11
};

// boje sastojaka (rgb)
glm::vec3 ingredientColors[] = {
    glm::vec3(0.9f, 0.7f, 0.4f),   // donja zemicka - svetlobraon
    glm::vec3(0.4f, 0.22f, 0.12f), // pljeskavica - tamnobraon
    glm::vec3(0.8f, 0.1f, 0.1f),   // kecap - crvena
    glm::vec3(0.9f, 0.8f, 0.1f),   // senf - zuta
    glm::vec3(0.5f, 0.7f, 0.3f),   // krastavcici - zelena
    glm::vec3(0.5f, 0.7f, 0.3f),   // krastavcici - zelena
    glm::vec3(0.95f, 0.9f, 0.95f), // luk - bela
    glm::vec3(0.3f, 0.8f, 0.3f),   // salata - svetlozelena
    glm::vec3(1.0f, 0.75f, 0.15f), // sir - narandzasta
    glm::vec3(0.95f, 0.3f, 0.2f),  // paradajz - crvena
    glm::vec3(0.85f, 0.65f, 0.35f) // gornja zemicka - svetlobraon
};

// visine sastojaka (za slaganje)
float ingredientHeights[] = {
    0.04f,  // donja zemicka 
    0.03f,  // pljeskavica 
    0.001f,  // kecap 
    0.001f,  // senf 
    0.03f,  // krastavcici
    0.03f,  // krastavcici
    0.01f,  // luk
    0.01f,  // salata
    0.015f,  // sir
    0.025f,  // paradajz
    0.1f    // gornja zemicka
};

// globalne promenljive
GameState currentState = STATE_MENU;

// dimenzije prozora
int wWidth = 800;
int wHeight = 600;

// kamera
glm::vec3 cameraPos = glm::vec3(0.0f, 2.0f, 5.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, -0.3f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float yaw = -90.0f;
float pitch = -15.0f;
float lastX = 400.0f;
float lastY = 300.0f;
bool firstMouse = true;

// svetlo
bool lightOn = true;
glm::vec3 lightPos = glm::vec3(2.0f, 3.0f, 2.0f);

// renderovanje opcije
bool depthTestEnabled = true;
bool cullFaceEnabled = false;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// pljeskavica (za cooking fazu)
glm::vec3 pattyPos = glm::vec3(0.0f, 0.7f, 0.0f);
float cookProgress = 0.0f;

// assembly faza
int currentIngredient = 0;
glm::vec3 ingredientPos = glm::vec3(0.0f, 1.5f, 0.0f);
int ingredientsPlaced = 0;
float stackHeight = 0.0f;  // Visina vec slozenih sastojaka

// sto i tanjir pozicije
glm::vec3 tablePos = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 platePos = glm::vec3(0.0f, 0.51f, 0.0f);  // Na vrhu stola
float plateRadius = 0.4f;

// barice (promaseni kecap/senf)
struct Puddle {
    glm::vec3 position;
    float y;
    glm::vec3 color;
};
std::vector<Puddle> puddles;

// mis
bool mouseClicked = false;
double mouseX = 0.0, mouseY = 0.0;

// space 
bool spaceJustPressed = false;

std::vector<float> placedYPositions; // ovde cuvam finalne y pozicije
std::vector<int> placedIngredients; //ovde cuvam indekse sastojaka

// 3D modeli
Model breadModel;
Model pattyModel;  
Model krastavcicModel;
Model lukModel;
Model salataModel;
Model sirModel;
Model paradajzModel;
Model gornjaModel;

bool spaceReady = true; 


// geometrija

// kocka sa normalama
void generateCube(std::vector<float>& vertices) {
    float cubeVerts[] = {
        // pozicije          // normale
        // zadnja strana
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        // prednja strana
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        // leva strana
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        // desna strana
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         // donja strana
         -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
          0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
          0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
          0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
         -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
         -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         // gornja strana
         -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
          0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
          0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
          0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
         -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
         -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
    };
    vertices.assign(cubeVerts, cubeVerts + sizeof(cubeVerts) / sizeof(float));
}

// cilindar - tanjir itd.
void generateCylinder(std::vector<float>& vertices, int segments, float radius, float height) {
    vertices.clear();
    float halfH = height / 2.0f;

    for (int i = 0; i < segments; i++) {
        float angle1 = 2.0f * 3.14159f * i / segments;
        float angle2 = 2.0f * 3.14159f * (i + 1) / segments;

        float x1 = radius * cos(angle1);
        float z1 = radius * sin(angle1);
        float x2 = radius * cos(angle2);
        float z2 = radius * sin(angle2);

        // gornja povrsina
        vertices.insert(vertices.end(), { 0.0f, halfH, 0.0f, 0.0f, 1.0f, 0.0f });
        vertices.insert(vertices.end(), { x1, halfH, z1, 0.0f, 1.0f, 0.0f });
        vertices.insert(vertices.end(), { x2, halfH, z2, 0.0f, 1.0f, 0.0f });

        // donja povrsina
        vertices.insert(vertices.end(), { 0.0f, -halfH, 0.0f, 0.0f, -1.0f, 0.0f });
        vertices.insert(vertices.end(), { x2, -halfH, z2, 0.0f, -1.0f, 0.0f });
        vertices.insert(vertices.end(), { x1, -halfH, z1, 0.0f, -1.0f, 0.0f });

        // bocna povrsina
        float nx1 = cos(angle1), nz1 = sin(angle1);
        float nx2 = cos(angle2), nz2 = sin(angle2);

        vertices.insert(vertices.end(), { x1, -halfH, z1, nx1, 0.0f, nz1 });
        vertices.insert(vertices.end(), { x2, -halfH, z2, nx2, 0.0f, nz2 });
        vertices.insert(vertices.end(), { x2, halfH, z2, nx2, 0.0f, nz2 });

        vertices.insert(vertices.end(), { x2, halfH, z2, nx2, 0.0f, nz2 });
        vertices.insert(vertices.end(), { x1, halfH, z1, nx1, 0.0f, nz1 });
        vertices.insert(vertices.end(), { x1, -halfH, z1, nx1, 0.0f, nz1 });
    }
}

// kupa (za vrh flasice kecapa/senfa)
void generateCone(std::vector<float>& vertices, int segments, float radius, float height) {
    vertices.clear();

    for (int i = 0; i < segments; i++) {
        float angle1 = 2.0f * 3.14159f * i / segments;
        float angle2 = 2.0f * 3.14159f * (i + 1) / segments;

        float x1 = radius * cos(angle1);
        float z1 = radius * sin(angle1);
        float x2 = radius * cos(angle2);
        float z2 = radius * sin(angle2);

        // bocna povrsina kupe
        glm::vec3 v1(x1, 0, z1);
        glm::vec3 v2(x2, 0, z2);
        glm::vec3 apex(0, -height, 0);  

        glm::vec3 edge1 = v1 - apex;
        glm::vec3 edge2 = v2 - apex;
        glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

        vertices.insert(vertices.end(), { 0.0f, -height, 0.0f, normal.x, normal.y, normal.z });
        vertices.insert(vertices.end(), { x1, 0.0f, z1, normal.x, normal.y, normal.z });
        vertices.insert(vertices.end(), { x2, 0.0f, z2, normal.x, normal.y, normal.z });

        // baza kupe
        vertices.insert(vertices.end(), { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f });
        vertices.insert(vertices.end(), { x2, 0.0f, z2, 0.0f, 1.0f, 0.0f });
        vertices.insert(vertices.end(), { x1, 0.0f, z1, 0.0f, 1.0f, 0.0f });
    }
}

// quad za 2D elemente (HUD)
void generateQuad(std::vector<float>& vertices) {
    float quadVerts[] = {
        // pozicije      // texCoords
        -1.0f,  1.0f,    0.0f, 1.0f,
        -1.0f, -1.0f,    0.0f, 0.0f,
         1.0f, -1.0f,    1.0f, 0.0f,

        -1.0f,  1.0f,    0.0f, 1.0f,
         1.0f, -1.0f,    1.0f, 0.0f,
         1.0f,  1.0f,    1.0f, 1.0f
    };
    vertices.assign(quadVerts, quadVerts + sizeof(quadVerts) / sizeof(float));
}

// callback funkcije
void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    wWidth = width;
    wHeight = height;
    lastX = width / 2.0f;
    lastY = height / 2.0f;
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    mouseX = xpos;
    mouseY = ypos;

    // rotacija kamere samo u cooking i assembly stanjima
    if (currentState != STATE_COOKING && currentState != STATE_ASSEMBLY) return;

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = (xpos - lastX) * MOUSE_SENSITIVITY;
    float yoffset = (lastY - ypos) * MOUSE_SENSITIVITY;
    lastX = xpos;
    lastY = ypos;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        mouseClicked = true;
    }
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // ESC - izlaz
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    // L - pali/gasi svetlo
    if (key == GLFW_KEY_L && action == GLFW_PRESS) {
        lightOn = !lightOn;
        std::cout << "Svetlo: " << (lightOn ? "UKLJUCENO" : "ISKLJUCENO") << std::endl;
    }

    // 1 - iskljuci depth test
    if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
        depthTestEnabled = true;
        glEnable(GL_DEPTH_TEST);
        std::cout << "Depth Test: UKLJUCEN" << std::endl;
    }

    // 2 - ukljuci depth test
    if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
        depthTestEnabled = false;
        glDisable(GL_DEPTH_TEST);
        std::cout << "Depth Test: ISKLJUCEN" << std::endl;
    }

    // 3 - ukljuci face culling
    if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
        cullFaceEnabled = true;
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        std::cout << "Face Culling: UKLJUCEN" << std::endl;
    }

    // 4 - iskljuci face culling
    if (key == GLFW_KEY_4 && action == GLFW_PRESS) {
        cullFaceEnabled = false;
        glDisable(GL_CULL_FACE);
        std::cout << "Face Culling: ISKLJUCEN" << std::endl;
    }

    // ENTER - prelazak iz menu u cooking
    if (key == GLFW_KEY_ENTER && action == GLFW_PRESS && currentState == STATE_MENU) {
        currentState = STATE_COOKING;
        std::cout << "Prelazim u COOKING stanje!" << std::endl;
    }

    // SPACE - za kecap/senf
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        spaceJustPressed = true;
    }
}

// main
int main() {
    // inicijalizacija GLFW
    if (!glfwInit()) {
        std::cout << "GLFW GRESKA!" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // fullscreen
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    wWidth = mode->width;
    wHeight = mode->height;
    lastX = wWidth / 2.0f;
    lastY = wHeight / 2.0f;

    GLFWwindow* window = glfwCreateWindow(wWidth, wHeight, "3D Brza Hrana", monitor, NULL);
    if (!window) {
        std::cout << "PROZOR GRESKA!" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetKeyCallback(window, keyCallback);

    // inicijalizacija GLEW
    if (glewInit() != GLEW_OK) {
        std::cout << "GLEW GRESKA!" << std::endl;
        return -1;
    }

    std::cout << "OpenGL verzija: " << glGetString(GL_VERSION) << std::endl;

    // shaderi
    unsigned int phongShader = createShader("phong.vert", "phong.frag");
    unsigned int hudShader = createShader("hud.vert", "hud.frag");

    // teksture
    unsigned int texStudentInfo = loadImageToTexture("Textures/student_info.png");
    if (texStudentInfo != 0) {
        glBindTexture(GL_TEXTURE_2D, texStudentInfo);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
        std::cout << "Student info tekstura ucitana uspesno!" << std::endl;
    }
    else {
        std::cout << "GRESKA: Student infotekstura nije ucitana!" << std::endl;
    }

    // prijatno tekstura
    unsigned int texPrijatno = loadImageToTexture("Textures/prijatno.png");
    if (texPrijatno != 0) {
        glBindTexture(GL_TEXTURE_2D, texPrijatno);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
        std::cout << "Prijatno tekstura ucitana uspesno!" << std::endl;
    }
    else {
        std::cout << "GRESKA: Prijatno tekstura nije ucitana!" << std::endl;
    }

    // geometrija
    // kocka (rerna, sto, noge)
    std::vector<float> cubeVerts;
    generateCube(cubeVerts);

    unsigned int cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, cubeVerts.size() * sizeof(float), cubeVerts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // cilindar (tanjir, flasice)
    std::vector<float> cylinderVerts;
    generateCylinder(cylinderVerts, 32, 0.3f, 0.08f);

    unsigned int cylinderVAO, cylinderVBO;
    glGenVertexArrays(1, &cylinderVAO);
    glGenBuffers(1, &cylinderVBO);
    glBindVertexArray(cylinderVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cylinderVBO);
    glBufferData(GL_ARRAY_BUFFER, cylinderVerts.size() * sizeof(float), cylinderVerts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // tanjir (veci cilindar)
    std::vector<float> plateVerts;
    generateCylinder(plateVerts, 32, 0.4f, 0.03f);

    unsigned int plateVAO, plateVBO;
    glGenVertexArrays(1, &plateVAO);
    glGenBuffers(1, &plateVBO);
    glBindVertexArray(plateVAO);
    glBindBuffer(GL_ARRAY_BUFFER, plateVBO);
    glBufferData(GL_ARRAY_BUFFER, plateVerts.size() * sizeof(float), plateVerts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // kupa (vrh flasice)
    std::vector<float> coneVerts;
    generateCone(coneVerts, 32, 0.04f, 0.08f);

    unsigned int coneVAO, coneVBO;
    glGenVertexArrays(1, &coneVAO);
    glGenBuffers(1, &coneVBO);
    glBindVertexArray(coneVAO);
    glBindBuffer(GL_ARRAY_BUFFER, coneVBO);
    glBufferData(GL_ARRAY_BUFFER, coneVerts.size() * sizeof(float), coneVerts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // HUD quad
    std::vector<float> quadVerts;
    generateQuad(quadVerts);

    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, quadVerts.size() * sizeof(float), quadVerts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // 3d model donje zemicke
    breadModel = loadOBJModel("Resources/donja.obj");
    if (breadModel.VAO == 0) {
        std::cout << "GRESKA: Ne mogu ucitati model donje zemicke!" << std::endl;
    } else {
        std::cout << "Model donje zemicke uspesno ucitan! Verteksi: " << breadModel.vertexCount << std::endl;
    }

    // 3d model pljeskavice
    pattyModel = loadOBJModel("Resources/pljeskavica.obj");
    if (pattyModel.VAO == 0) {
        std::cout << "GRESKA: Ne mogu ucitati model pljeskavice!" << std::endl;
    } else {
        std::cout << "Model pljeskavice uspesno ucitan! Verteksi: " << pattyModel.vertexCount << std::endl;
    }

    krastavcicModel = loadOBJModel("Resources/krastavcic.obj");
    if (krastavcicModel.VAO == 0) {
        std::cout << "GRESKA: Ne mogu ucitati model krastavcica!" << std::endl;
    }
    else {
        std::cout << "Model krastavcica uspesno ucitan! Verteksi: " << krastavcicModel.vertexCount << std::endl;
    }

    lukModel = loadOBJModel("Resources/luk.obj");
    if (lukModel.VAO == 0) { 
        std::cout << "GRESKA: Ne mogu ucitati model luka!" << std::endl;
    }
    else {
        std::cout << "Model luka uspesno ucitan! Verteksi: " << lukModel.vertexCount << std::endl;
    }

    salataModel = loadOBJModel("Resources/salata.obj");
    if (salataModel.VAO == 0) { 
        std::cout << "GRESKA: Ne mogu ucitati model salate!" << std::endl;
    }
    else {
        std::cout << "Model salate uspesno ucitan! Verteksi: " << salataModel.vertexCount << std::endl;
    }
    
    sirModel = loadOBJModel("Resources/sir.obj");
    if (sirModel.VAO == 0) { 
        std::cout << "GRESKA: Ne mogu ucitati model sira!" << std::endl;
    }
    else {
        std::cout << "Model sira uspesno ucitan! Verteksi: " << sirModel.vertexCount << std::endl;
    }

    paradajzModel = loadOBJModel("Resources/krastavcic.obj");
    if (paradajzModel.VAO == 0) { 
        std::cout << "GRESKA: Ne mogu ucitati model paradajza!" << std::endl;
    }
    else {
        std::cout << "Model paradajza uspesno ucitan! Verteksi: " << paradajzModel.vertexCount << std::endl;
    }

    gornjaModel = loadOBJModel("Resources/gornja.obj");
    if (gornjaModel.VAO == 0) { 
        std::cout << "GRESKA: Ne mogu ucitati model gornje zemicke!" << std::endl;
    }
    else {
        std::cout << "Model gornje zemicke uspesno ucitan! Verteksi: " << gornjaModel.vertexCount << std::endl;
    }

    // uniforme
    glUseProgram(phongShader);

    unsigned int modelLoc = glGetUniformLocation(phongShader, "uModel");
    unsigned int viewLoc = glGetUniformLocation(phongShader, "uView");
    unsigned int projLoc = glGetUniformLocation(phongShader, "uProjection");
    unsigned int viewPosLoc = glGetUniformLocation(phongShader, "uViewPos");
    unsigned int lightPosLoc = glGetUniformLocation(phongShader, "uLightPos");
    unsigned int lightColorLoc = glGetUniformLocation(phongShader, "uLightColor");
    unsigned int objectColorLoc = glGetUniformLocation(phongShader, "uObjectColor");
    unsigned int lightOnLoc = glGetUniformLocation(phongShader, "uLightOn");

    // projekcija (perspektiva)
    glm::mat4 projection = glm::perspective(glm::radians(45.0f),
        (float)wWidth / (float)wHeight,
        0.1f, 100.0f);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // postavke
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glCullFace(GL_BACK);

    std::cout << "\n=== KONTROLE ===" << std::endl;
    std::cout << "ENTER - pocni igru" << std::endl;
    std::cout << "WASD - pomeri objekat (X/Z)" << std::endl;
    std::cout << "SPACE - pomeri gore / cedi sos" << std::endl;
    std::cout << "C - pomeri dole" << std::endl;
    std::cout << "Strelice - pomeri kameru" << std::endl;
    std::cout << "Mis - rotiraj kameru" << std::endl;
    std::cout << "L - pali/gasi svetlo" << std::endl;
    std::cout << "ESC - izlaz" << std::endl;
    std::cout << "================\n" << std::endl;

    // timing za frame limiter
    auto lastTime = std::chrono::high_resolution_clock::now();

    // glavna petlja
    while (!glfwWindowShouldClose(window)) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> deltaTimeChrono = currentTime - lastTime;
        lastTime = currentTime;
        deltaTime = (float)deltaTimeChrono.count();

        glfwPollEvents();

        // input
        // kamera - strelice (samo u cooking i assembly)
        if (currentState == STATE_COOKING || currentState == STATE_ASSEMBLY) {
            if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
                cameraPos += CAMERA_SPEED * deltaTime * cameraFront;
            if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
                cameraPos -= CAMERA_SPEED * deltaTime * cameraFront;
            if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
                cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * CAMERA_SPEED * deltaTime;
            if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
                cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * CAMERA_SPEED * deltaTime;
        }

        // cooking faza
        if (currentState == STATE_COOKING) {
            // pljeskavica - WASD + SPACE/C
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                pattyPos.z -= MOVE_SPEED * deltaTime;
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                pattyPos.z += MOVE_SPEED * deltaTime;
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                pattyPos.x -= MOVE_SPEED * deltaTime;
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                pattyPos.x += MOVE_SPEED * deltaTime;
            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
                pattyPos.y += MOVE_SPEED * deltaTime;
            if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
                pattyPos.y -= MOVE_SPEED * deltaTime;

            // dimenzije rerne i ringle
            float ovenMinX = -0.6f, ovenMaxX = 0.6f;
            float ovenMinZ = -0.6f, ovenMaxZ = 0.6f;
            float ovenTopY = 0.5f;

            float ringlaMinX = -0.4f, ringlaMaxX = 0.4f;
            float ringlaMinZ = -0.4f, ringlaMaxZ = 0.4f;
            float ringlaTopY = 0.52f;

            float pattyHalfHeight = -0.01f;

            float pattyOnRinglaY = ringlaTopY + pattyHalfHeight;  
            float pattyOnOvenY = ovenTopY + pattyHalfHeight;      

            bool aboveRingla = (pattyPos.x >= ringlaMinX && pattyPos.x <= ringlaMaxX &&
                pattyPos.z >= ringlaMinZ && pattyPos.z <= ringlaMaxZ);

            bool aboveOvenOnly = (pattyPos.x >= ovenMinX && pattyPos.x <= ovenMaxX &&
                pattyPos.z >= ovenMinZ && pattyPos.z <= ovenMaxZ) && !aboveRingla;

            // kolizija
            if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
                if (aboveRingla) {
                    if (pattyPos.y <= pattyOnRinglaY) {
                        pattyPos.y = pattyOnRinglaY;
                    }
                }
                else if (aboveOvenOnly) {
                    if (pattyPos.y <= pattyOnOvenY) {
                        pattyPos.y = pattyOnOvenY;
                    }
                }
            }

            // opste granice kretanja
            if (pattyPos.x < -3.0f) pattyPos.x = -3.0f;
            if (pattyPos.x > 3.0f) pattyPos.x = 3.0f;
            if (pattyPos.z < -3.0f) pattyPos.z = -3.0f;
            if (pattyPos.z > 3.0f) pattyPos.z = 3.0f;
            if (pattyPos.y < 0.0f) pattyPos.y = 0.0f;
            if (pattyPos.y > 3.0f) pattyPos.y = 3.0f;

            // provera da li je pljeskavica na ringli
            bool onRingla = (aboveRingla && fabs(pattyPos.y - pattyOnRinglaY) < 0.05f);

            // debug output - ispisuje y poziciju kontinuirano
            static int frameCounter = 0;
            if (++frameCounter % 60 == 0) {  // svake sekunde pri 60 FPS
                std::cout << "Patty Y=" << pattyPos.y << " | Target=" << pattyOnRinglaY 
                          << " | OnRingla=" << (onRingla ? "DA" : "NE") << std::endl;
            }

            // ako je na ringli, krece przenje
            if (onRingla && cookProgress < 1.0f) {
                cookProgress += 0.2f * deltaTime;
                if (cookProgress >= 1.0f) {
                    cookProgress = 1.0f;
                    std::cout << "Pljeskavica ispecena! Prelazim u ASSEMBLY..." << std::endl;
                    currentState = STATE_ASSEMBLY;
                    currentIngredient = ING_DONJA_ZEMICKA;
                    ingredientPos = glm::vec3(0.0f, 1.5f, 0.0f);
                    stackHeight = 0.0f;
                }
            }
        }

        // assembly faza
        if (currentState == STATE_ASSEMBLY) {
            // pomeranje trenutnog sastojka - WASD + SPACE/SHIFT
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                ingredientPos.z -= MOVE_SPEED * deltaTime;
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                ingredientPos.z += MOVE_SPEED * deltaTime;
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                ingredientPos.x -= MOVE_SPEED * deltaTime;
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                ingredientPos.x += MOVE_SPEED * deltaTime;

            // za kecap i senf, SPACE cedi; za ostale, SPACE pomera gore
            if (currentIngredient != ING_KECAP && currentIngredient != ING_SENF) {
                if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
                    ingredientPos.y += MOVE_SPEED * deltaTime;
            }
            if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
                ingredientPos.y -= MOVE_SPEED * deltaTime;

            // granice kretanja
            if (ingredientPos.x < -2.0f) ingredientPos.x = -2.0f;
            if (ingredientPos.x > 2.0f) ingredientPos.x = 2.0f;
            if (ingredientPos.z < -2.0f) ingredientPos.z = -2.0f;
            if (ingredientPos.z > 2.0f) ingredientPos.z = 2.0f;

            // kolizija u tri nivoa
            float minHeight = 0.05f; // pod

            // 1. provera da li je iznad tanjira
            bool isOverPlate = (std::abs(ingredientPos.x) < 0.8f && std::abs(ingredientPos.z) < 0.8f);

            // 2. provera da li je iznad stola
            bool isOverTable = (std::abs(ingredientPos.x) < 1.25f && std::abs(ingredientPos.z) < 1.25f);

            if (currentIngredient == ING_KECAP || currentIngredient == ING_SENF) {
                if (isOverPlate) {
                    // nivo 1: iznad burgera (zaustavlja se na sastojcima)
                    minHeight = 0.55f + stackHeight + 0.12f;
                }
                else if (isOverTable) {
                    // nivo 2: iznad stola, ali van tanjira (zaustavlja se na stolu)
                    minHeight = 1.05f;
                }
                else {
                    // nivo 3: van stola 
                    minHeight = 0.5f;
                }
            }
            else {
                // ostali sastojci (meso, sir...) uvek idu na tanjir
                minHeight = 0.55f;
            }

            // konacna primena granice
            if (ingredientPos.y < minHeight) {
                ingredientPos.y = minHeight;
            }
            if (ingredientPos.y > 3.0f) ingredientPos.y = 3.0f;

            // pozicija tanjira
            float plateY = 0.55f;
            // stackHeight je DNO sledeceg sastojka
            float plateTargetY = plateY + stackHeight;

            // da li je iznad tanjira?
            float distFromPlate = sqrt(ingredientPos.x * ingredientPos.x + ingredientPos.z * ingredientPos.z);
            bool abovePlate = (distFromPlate < plateRadius);

            // debug output za assembly
            static int assemblyFrameCounter = 0;
            if (++assemblyFrameCounter % 60 == 0) {
                std::cout << "Assembly: Ingredient=" << currentIngredient 
                          << " | Y=" << ingredientPos.y 
                          << " | Target=" << plateTargetY 
                          << " | StackH=" << stackHeight
                          << " | AbovePlate=" << (abovePlate ? "DA" : "NE") << std::endl;
            }

            // kecap i senf logika
            if (currentIngredient == ING_KECAP || currentIngredient == ING_SENF) {
                if (spaceJustPressed) {
                    spaceJustPressed = false;

                    if (abovePlate) {
                        Puddle p;
                        p.position = glm::vec3(ingredientPos.x, 0.0f, ingredientPos.z);

                        p.y = 0.55f + stackHeight + 0.05f;

                        p.color = (currentIngredient == ING_KECAP) ? glm::vec3(0.8f, 0.1f, 0.1f) : glm::vec3(0.9f, 0.8f, 0.1f);
                        puddles.push_back(p);

                        stackHeight += ingredientHeights[currentIngredient];
                        ingredientsPlaced++;
                        currentIngredient++;
                        
                        if (currentIngredient >= ING_TOTAL) {
                            std::cout << "GOTOVO! Prijatno!" << std::endl;
                            currentState = STATE_FINISHED;
                        }
                    }
                    else {
                        Puddle p;
                        p.position = glm::vec3(ingredientPos.x, 0.0f, ingredientPos.z);
                        p.color = (currentIngredient == ING_KECAP) ? glm::vec3(0.8f, 0.1f, 0.1f) : glm::vec3(0.9f, 0.8f, 0.1f);

                        float limitX = 1.2f;
                        float limitZ = 1.2f;

                        if (std::abs(ingredientPos.x) <= limitX && std::abs(ingredientPos.z) <= limitZ) {
                            // na stolu
                            p.y = 0.51f;
                            std::cout << "Promaseno! Barica na stolu." << std::endl;
                        }
                        else {
                            // na podu 
                            p.y = 0.01f;
                            std::cout << "Promaseno! Barica na podu." << std::endl;
                        }
                        puddles.push_back(p);
                    }
                }
            }
            else {
                
                if (abovePlate && ingredientPos.y <= plateTargetY + 0.05f) {

                    float finalY = plateTargetY;

                    // ako je krastavac, luk ili salata, smanjujem visinu sletanja
                    // jer su ti modeli sami po sebi "visoki" unutar svog fajla
                    if (currentIngredient >= 4 && currentIngredient <= 10) {
                        finalY = stackHeight + 0.55f; 
                    }

                    placedYPositions.push_back(finalY);
                    placedIngredients.push_back(currentIngredient);
                    std::cout << "Postavljeno: sastojak " << currentIngredient << " na Y=" << finalY << std::endl;

                    // 2. azuriram visinu za sledeci sastojak
                    stackHeight += ingredientHeights[currentIngredient];
                    ingredientsPlaced++;
                    currentIngredient++;

                    // 3. resetujem poziciju za novi sastojak
                    ingredientPos = glm::vec3(0.0f, 1.5f, 0.0f);

                    if (currentIngredient >= ING_TOTAL) {
                        currentState = STATE_FINISHED;
                    }
                }
            }

            spaceJustPressed = false;  
        }

        // renderovanje
        // zastita od nevalidnih dimenzija prozora
        if (wWidth <= 0 || wHeight <= 0) {
            glfwSwapBuffers(window);
            continue;
        }

        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // view matrica
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        // azuriram projekciju sa zastitom od deljenja sa nulom
        float aspectRatio = (float)wWidth / (float)wHeight;
        projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);

        // 3d scena
        glUseProgram(phongShader);
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3fv(viewPosLoc, 1, glm::value_ptr(cameraPos));
        glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));
        glUniform3f(lightColorLoc, 1.0f, 0.95f, 0.8f);
        glUniform1i(lightOnLoc, lightOn ? 1 : 0);

        glm::mat4 model;


        // pod
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -0.5f, 0.0f));
        model = glm::scale(model, glm::vec3(10.0f, 0.1f, 10.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3f(objectColorLoc, 0.3f, 0.25f, 0.2f);
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // cooking scena
        if (currentState == STATE_COOKING) {
            // rerna
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
            model = glm::scale(model, glm::vec3(1.2f, 1.0f, 1.2f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glUniform3f(objectColorLoc, 0.15f, 0.15f, 0.15f);
            glBindVertexArray(cubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);

            // ringla
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0f, 0.51f, 0.0f));
            model = glm::scale(model, glm::vec3(0.8f, 0.02f, 0.8f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glUniform3f(objectColorLoc, 0.9f, 0.2f, 0.1f);
            glBindVertexArray(cubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);

            // pljeska
            float pR = 0.85f + (0.4f - 0.85f) * cookProgress;
            float pG = 0.5f + (0.22f - 0.5f) * cookProgress;
            float pB = 0.5f + (0.12f - 0.5f) * cookProgress;

            model = glm::mat4(1.0f);
            model = glm::translate(model, pattyPos);
            model = glm::scale(model, glm::vec3(0.3f, 0.08f, 0.3f)); 
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glUniform3f(objectColorLoc, pR, pG, pB);
            
            if (pattyModel.VAO != 0) {
                glBindVertexArray(pattyModel.VAO);
                glDrawArrays(GL_TRIANGLES, 0, pattyModel.vertexCount);
            } else {
                // fallback na cilindar ako model nije ucitan
                glBindVertexArray(cylinderVAO);
                glDrawArrays(GL_TRIANGLES, 0, cylinderVerts.size() / 6);
            }
        }

        // assembly scena
        if (currentState == STATE_ASSEMBLY || currentState == STATE_FINISHED) {
            // sto 
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0f, 0.45f, 0.0f));
            model = glm::scale(model, glm::vec3(2.0f, 0.1f, 1.5f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glUniform3f(objectColorLoc, 0.45f, 0.3f, 0.15f);
            glBindVertexArray(cubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);

            // noge stola (4 komada)
            float legPositions[4][2] = {
                {-0.8f, -0.55f}, {0.8f, -0.55f}, {-0.8f, 0.55f}, {0.8f, 0.55f}
            };
            for (int i = 0; i < 4; i++) {
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(legPositions[i][0], -0.05f, legPositions[i][1]));
                model = glm::scale(model, glm::vec3(0.1f, 0.9f, 0.1f));
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                glUniform3f(objectColorLoc, 0.35f, 0.2f, 0.1f);
                glBindVertexArray(cubeVAO);
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }

            // tanjir
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0f, 0.52f, 0.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glUniform3f(objectColorLoc, 0.9f, 0.9f, 0.85f);
            glBindVertexArray(plateVAO);
            glDrawArrays(GL_TRIANGLES, 0, plateVerts.size() / 6);

            // barice na stolu
            for (const auto& p : puddles) {
                glm::mat4 modelPuddle = glm::mat4(1.0f);
                modelPuddle = glm::translate(modelPuddle, glm::vec3(p.position.x, p.y, p.position.z));

               modelPuddle = glm::scale(modelPuddle, glm::vec3(0.75f, 0.02f, 0.75f));

                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelPuddle));
                glUniform3fv(objectColorLoc, 1, glm::value_ptr(p.color));

                glBindVertexArray(cylinderVAO);
                glDrawArrays(GL_TRIANGLES, 0, 32 * 12);
            }

            // vec postavljeni sastojci
            float currentY = 0.55f;  // pocinje od vrha tanjira
            for (int i = 0; i < (int)placedYPositions.size(); i++) {
                int ingIndex = placedIngredients[i]; // uzmem pravi indeks sastojka
                glm::mat4 modelPlaced = glm::mat4(1.0f);

                // prvo translacija na visinu gde treba biti na tanjuru
                modelPlaced = glm::translate(modelPlaced, glm::vec3(0.0f, placedYPositions[i], 0.0f));

                glUniform3fv(objectColorLoc, 1, glm::value_ptr(ingredientColors[ingIndex])); 

                if (ingIndex == 0) { // donja zemicka
                    modelPlaced = glm::scale(modelPlaced, glm::vec3(0.35f));
                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelPlaced));
                    glBindVertexArray(breadModel.VAO);
                    glDrawArrays(GL_TRIANGLES, 0, breadModel.vertexCount);
                }
                else if (ingIndex == 1) { // pljeska
                    float scaleFactor = 0.30f;

                    float yOffset = -0.96f * scaleFactor; 

                    float zOffset = 0.10f * scaleFactor;

                    float xOffset = 0.05f * scaleFactor;

                    modelPlaced = glm::translate(modelPlaced, glm::vec3(xOffset, yOffset, zOffset));
                    modelPlaced = glm::scale(modelPlaced, glm::vec3(scaleFactor));

                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelPlaced));
                    glBindVertexArray(pattyModel.VAO);
                    glDrawArrays(GL_TRIANGLES, 0, pattyModel.vertexCount);
                }
                else if (ingIndex == 4) { // krastavcic 
                    float scaleFactor = 0.20f;
                    float yOffset = -1.5f * scaleFactor;

                    modelPlaced = glm::translate(modelPlaced, glm::vec3(0.0f, yOffset, 0.0f));
                    modelPlaced = glm::scale(modelPlaced, glm::vec3(scaleFactor));

                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelPlaced));
                    glUniform3f(objectColorLoc, 0.0f, 0.35f, 0.0f); // Zelena boja

                    glBindVertexArray(krastavcicModel.VAO);
                    glDrawArrays(GL_TRIANGLES, 0, krastavcicModel.vertexCount);
                }
                else if (ingIndex == 5) { // krastavcic
                    float scaleFactor = 0.20f;
                    float yOffset = -1.5f * scaleFactor;
                    float xOffset = -0.2f;

                    modelPlaced = glm::translate(modelPlaced, glm::vec3(xOffset, yOffset, 0.0f));
                    modelPlaced = glm::scale(modelPlaced, glm::vec3(scaleFactor));

                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelPlaced));
                    glUniform3f(objectColorLoc, 0.0f, 0.35f, 0.0f); 

                    glBindVertexArray(krastavcicModel.VAO);
                    glDrawArrays(GL_TRIANGLES, 0, krastavcicModel.vertexCount);
                }

                else if (ingIndex == 6) { // luk
                    float scaleFactor = 0.25f;
                    float yOffset = -0.50f * scaleFactor; 
                    modelPlaced = glm::translate(modelPlaced, glm::vec3(0.0f, yOffset, 0.0f));
                    modelPlaced = glm::scale(modelPlaced, glm::vec3(scaleFactor));
                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelPlaced));
                    glUniform3f(objectColorLoc, 0.9f, 0.9f, 0.8f);
                    glBindVertexArray(lukModel.VAO);
                    glDrawArrays(GL_TRIANGLES, 0, lukModel.vertexCount);
                }
                else if (ingIndex == 7) { // salata na burgeru
                    float scaleFactor = 0.35f;
                    float yOffset = -2.3f * scaleFactor;

                    modelPlaced = glm::translate(modelPlaced, glm::vec3(0.0f, yOffset, 0.0f));
                    modelPlaced = glm::scale(modelPlaced, glm::vec3(scaleFactor));

                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelPlaced));
                    glUniform3f(objectColorLoc, 0.2f, 0.6f, 0.2f); 

                    glBindVertexArray(salataModel.VAO);
                    glDrawArrays(GL_TRIANGLES, 0, salataModel.vertexCount);
                }
                else if (ingIndex == 8) { // sir na burgeru
                    float scaleFactor = 0.35f; 
                    float yOffset = -1.3f * scaleFactor;

                    modelPlaced = glm::translate(modelPlaced, glm::vec3(0.0f, yOffset, 0.0f));
                    modelPlaced = glm::scale(modelPlaced, glm::vec3(scaleFactor));

                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelPlaced));
                    glUniform3f(objectColorLoc, 1.0f, 0.8f, 0.0f);

                    glBindVertexArray(sirModel.VAO);
                    glDrawArrays(GL_TRIANGLES, 0, sirModel.vertexCount);
                }
                else if (ingIndex == 9) { // paradajz na burgeru
                    float scaleFactor = 0.50f;  
                    float yOffset = -1.7f * scaleFactor;  
                    float xOffset = -0.2f;
					
                    modelPlaced = glm::translate(modelPlaced, glm::vec3(xOffset, yOffset, 0.0f));
                    modelPlaced = glm::scale(modelPlaced, glm::vec3(scaleFactor));

                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelPlaced));
                    glUniform3f(objectColorLoc, 0.9f, 0.2f, 0.1f); 

                    glBindVertexArray(paradajzModel.VAO);
                    glDrawArrays(GL_TRIANGLES, 0, paradajzModel.vertexCount);
                }
                else if (ingIndex == 10) { // gornja zemicka na burgeru
                    float scaleFactor = 0.35f;
                    float yOffset = -2.7f; 

                    modelPlaced = glm::translate(modelPlaced, glm::vec3(0.0f, yOffset * scaleFactor, 0.0f));

                    modelPlaced = glm::scale(modelPlaced, glm::vec3(scaleFactor));

                    glUniform3f(objectColorLoc, 0.9f, 0.7f, 0.4f);
                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelPlaced)); // Ne zaboravi shaderu poslati novu matricu!

                    glBindVertexArray(gornjaModel.VAO);
                    glDrawArrays(GL_TRIANGLES, 0, gornjaModel.vertexCount);
                    }
                else { 

                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelPlaced));
                    glBindVertexArray(cylinderVAO);
                    glDrawArrays(GL_TRIANGLES, 0, 32 * 12);
                }
            }

            // trenutni sastojak koji se pomera (samo u assembly)
            if (currentState == STATE_ASSEMBLY && currentIngredient < ING_TOTAL) {
                if (currentIngredient == ING_KECAP || currentIngredient == ING_SENF) {
                    glm::vec3 bottleColor = (currentIngredient == ING_KECAP) ?
                        glm::vec3(0.8f, 0.1f, 0.1f) : glm::vec3(0.9f, 0.8f, 0.1f);

                    // 1. telo flasice - valjak
                    model = glm::mat4(1.0f);
                    model = glm::translate(model, ingredientPos);
                    model = glm::scale(model, glm::vec3(0.3f, 1.5f, 0.3f));
                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                    glUniform3f(objectColorLoc, bottleColor.r, bottleColor.g, bottleColor.b);
                    glBindVertexArray(cylinderVAO);
                    glDrawArrays(GL_TRIANGLES, 0, cylinderVerts.size() / 6);

                    // 2. vrh flasice - kupa
                   float coneYOffset = 0.07f;

                    model = glm::mat4(1.0f);
                    model = glm::translate(model, glm::vec3(ingredientPos.x, ingredientPos.y - coneYOffset, ingredientPos.z));
                    
                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                    glUniform3f(objectColorLoc, bottleColor.r * 0.8f, bottleColor.g * 0.8f, bottleColor.b * 0.8f);
                    glBindVertexArray(coneVAO);
                    glDrawArrays(GL_TRIANGLES, 0, coneVerts.size() / 6);
                
                
                
                
            }
                else {
                    // donja zemicka
                    if (currentIngredient == ING_DONJA_ZEMICKA && breadModel.VAO != 0) {
                        model = glm::mat4(1.0f);
                        model = glm::translate(model, glm::vec3(ingredientPos.x, ingredientPos.y + 0.02f, ingredientPos.z));
                        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                        model = glm::scale(model, glm::vec3(0.35f, 0.35f, 0.35f));
                        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                        glm::vec3 breadColor = ingredientColors[currentIngredient] * 1.3f;
                        glUniform3f(objectColorLoc, breadColor.r, breadColor.g, breadColor.b);
                        glBindVertexArray(breadModel.VAO);
                        glDrawArrays(GL_TRIANGLES, 0, breadModel.vertexCount);
                    } 
                    // pljeska
                    else if (currentIngredient == ING_PLJESKAVICA && pattyModel.VAO != 0) {
                        model = glm::mat4(1.0f);
                        model = glm::translate(model, glm::vec3(ingredientPos.x, ingredientPos.y - 0.03f, ingredientPos.z));
                        model = glm::scale(model, glm::vec3(0.35f, 0.35f, 0.35f));
                        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                        glm::vec3 pattyColor = ingredientColors[currentIngredient] * 1.3f;
                        glUniform3f(objectColorLoc, pattyColor.r, pattyColor.g, pattyColor.b);
                        glBindVertexArray(pattyModel.VAO);
                        glDrawArrays(GL_TRIANGLES, 0, pattyModel.vertexCount);
                    }
                    else if (currentIngredient == ING_KRASTAVCICI) { // krastavcic
                        float scaleFactor = 0.20f;
                        float yOffset = -0.50f * scaleFactor;
                        float xOffset = 0.0f;
                        float zOffset = 0.0f;

                        model = glm::mat4(1.0f);
                        model = glm::translate(model, ingredientPos);
                        model = glm::translate(model, glm::vec3(xOffset, yOffset, zOffset));
                        model = glm::scale(model, glm::vec3(scaleFactor));

                        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

                        glUniform3f(objectColorLoc, ingredientColors[currentIngredient].r,
                            ingredientColors[currentIngredient].g,
                            ingredientColors[currentIngredient].b);

                        glBindVertexArray(krastavcicModel.VAO);
                        glDrawArrays(GL_TRIANGLES, 0, krastavcicModel.vertexCount);
                    }
                    else if (currentIngredient == ING_KRASTAVCICI2) { // krastavcic2
                        float scaleFactor = 0.20f;
                        float yOffset = -0.50f * scaleFactor;
                        float xOffset = -0.2f;
                        float zOffset = 0.0f;

                        model = glm::mat4(1.0f);
                        model = glm::translate(model, ingredientPos);
                        model = glm::translate(model, glm::vec3(xOffset, yOffset, zOffset));
                        model = glm::scale(model, glm::vec3(scaleFactor));

                        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

                        glUniform3f(objectColorLoc, ingredientColors[currentIngredient].r,
                            ingredientColors[currentIngredient].g,
                            ingredientColors[currentIngredient].b);

                        glBindVertexArray(krastavcicModel.VAO);
                        glDrawArrays(GL_TRIANGLES, 0, krastavcicModel.vertexCount);
                    }
                    else if (currentIngredient == ING_LUK) { // luk
                        float scaleFactor = 0.25f;
                        float yOffset = -0.50f * scaleFactor;
                        
                        model = glm::mat4(1.0f); 
                        model = glm::translate(model, ingredientPos); 
                        model = glm::translate(model, glm::vec3(0.0f, yOffset, 0.0f));
                        model = glm::scale(model, glm::vec3(scaleFactor));
                        
                        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                        glUniform3f(objectColorLoc, 0.9f, 0.9f, 0.8f);
                        glBindVertexArray(lukModel.VAO);
                        glDrawArrays(GL_TRIANGLES, 0, lukModel.vertexCount);
                    }
                    else if (currentIngredient == ING_SALATA) { // salata dok pada
                        float scaleFactor = 0.35f;
                        float yOffset = -1.8f * scaleFactor;

                        model = glm::mat4(1.0f);
                        model = glm::translate(model, ingredientPos);
                        model = glm::translate(model, glm::vec3(0.0f, yOffset, 0.0f));
                        model = glm::scale(model, glm::vec3(scaleFactor));

                        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                        glUniform3f(objectColorLoc, 0.2f, 0.6f, 0.2f);

                        glBindVertexArray(salataModel.VAO);
                        glDrawArrays(GL_TRIANGLES, 0, salataModel.vertexCount);
                    }
                    else if (currentIngredient == ING_SIR) { // sir dok pada
                        float scaleFactor = 0.35f;
                        float yOffset = -0.9f * scaleFactor;

                        model = glm::mat4(1.0f);
                        model = glm::translate(model, ingredientPos);
                        model = glm::translate(model, glm::vec3(0.0f, yOffset, 0.0f));
                        model = glm::scale(model, glm::vec3(scaleFactor));

                        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                        glUniform3f(objectColorLoc, 1.0f, 0.8f, 0.0f);

                        glBindVertexArray(sirModel.VAO);
                        glDrawArrays(GL_TRIANGLES, 0, sirModel.vertexCount);
                    }
                    else if (currentIngredient == ING_PARADAJZ) { // paradajz dok pada
                        float scaleFactor = 0.50f;
                        float yOffset = -1.7f * scaleFactor;
                        float xOffset = -0.2f;

                        model = glm::mat4(1.0f);
                        model = glm::translate(model, ingredientPos);
                        model = glm::translate(model, glm::vec3(xOffset, yOffset, 0.0f));
                        model = glm::scale(model, glm::vec3(scaleFactor));

                        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                        glUniform3f(objectColorLoc, 0.9f, 0.2f, 0.1f); 

                        glBindVertexArray(paradajzModel.VAO);
                        glDrawArrays(GL_TRIANGLES, 0, paradajzModel.vertexCount);
                        }
                    else if (currentIngredient == ING_GORNJA_ZEMICKA && gornjaModel.VAO != 0) {
                        float scaleFactor = 0.35f;
                        float yOffset = -2.7f; 

                        model = glm::mat4(1.0f);
                        model = glm::translate(model, ingredientPos);
                        model = glm::translate(model, glm::vec3(0.0f, yOffset * scaleFactor, 0.0f));
                        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                        model = glm::scale(model, glm::vec3(scaleFactor));

                        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                        glm::vec3 breadColor = glm::vec3(0.9f, 0.7f, 0.4f) * 1.3f;
                        glUniform3f(objectColorLoc, breadColor.r, breadColor.g, breadColor.b);

                        glBindVertexArray(gornjaModel.VAO);
                        glDrawArrays(GL_TRIANGLES, 0, gornjaModel.vertexCount);
                        }

                        // fallback : kvadar
                    else {
                        std::cout << "usao sam ovde: " << currentIngredient << std::endl;
                        model = glm::mat4(1.0f);
                        model = glm::translate(model, ingredientPos);
                        model = glm::scale(model, glm::vec3(0.35f, ingredientHeights[currentIngredient] * 5.0f, 0.35f));
                        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                        glUniform3f(objectColorLoc, ingredientColors[currentIngredient].r,
                            ingredientColors[currentIngredient].g,
                            ingredientColors[currentIngredient].b);
                        glBindVertexArray(cubeVAO);
                        glDrawArrays(GL_TRIANGLES, 0, 36);
                    }
                }
            }
        }

        // svetlosni indikator
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(0.1f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3f(objectColorLoc, lightOn ? 1.0f : 0.2f, lightOn ? 0.95f : 0.2f, lightOn ? 0.0f : 0.2f);
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // 2d HUD
        glDisable(GL_DEPTH_TEST);
        glUseProgram(hudShader);

        unsigned int hudColorLoc = glGetUniformLocation(hudShader, "uColor");
        unsigned int hudPosLoc = glGetUniformLocation(hudShader, "uPos");
        unsigned int hudScaleLoc = glGetUniformLocation(hudShader, "uScale");
        unsigned int hudUseTextureLoc = glGetUniformLocation(hudShader, "uUseTexture");
        unsigned int hudTextureLoc = glGetUniformLocation(hudShader, "uTexture");

        if (currentState == STATE_MENU) {
            glUniform1i(hudUseTextureLoc, 0);
            // sugme
            glUniform4f(hudColorLoc, 0.8f, 0.2f, 0.1f, 1.0f);
            glUniform2f(hudPosLoc, 0.0f, 0.0f);
            glUniform2f(hudScaleLoc, 0.3f, 0.1f);
            glBindVertexArray(quadVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            // tekst
            glUniform4f(hudColorLoc, 1.0f, 0.9f, 0.8f, 1.0f);
            glUniform2f(hudPosLoc, 0.0f, 0.0f);
            glUniform2f(hudScaleLoc, 0.2f, 0.02f);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        if (currentState == STATE_COOKING) {
            glUniform1i(hudUseTextureLoc, 0);
            // loading bar pozadina
            glUniform4f(hudColorLoc, 0.3f, 0.3f, 0.3f, 0.8f);
            glUniform2f(hudPosLoc, 0.0f, 0.85f);
            glUniform2f(hudScaleLoc, 0.4f, 0.03f);
            glBindVertexArray(quadVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            // loading bar progress
            glUniform4f(hudColorLoc, 0.2f, 0.8f, 0.2f, 1.0f);
            glUniform2f(hudPosLoc, -0.4f * (1.0f - cookProgress), 0.85f);
            glUniform2f(hudScaleLoc, 0.4f * cookProgress, 0.03f);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        if (currentState == STATE_FINISHED) {
            if (texPrijatno != 0) {
                glUniform1i(hudUseTextureLoc, 1);
                glUniform1i(hudTextureLoc, 0);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, texPrijatno);

                // dimenzije teksture 
                float imgW = 800.0f;
                float imgH = 200.0f;
                float desiredWidthNDC = 0.8f;

                float scaleX = desiredWidthNDC;
                float scaleY = desiredWidthNDC * (imgH / imgW) * ((float)wWidth / (float)wHeight);

                float posX = 0.0f;
                float posY = 0.75f;

                glUniform4f(hudColorLoc, 1.0f, 1.0f, 1.0f, 1.0f);
                glUniform2f(hudPosLoc, posX, posY);
                glUniform2f(hudScaleLoc, scaleX / 2.0f, scaleY / 2.0f);
                glBindVertexArray(quadVAO);
                glDrawArrays(GL_TRIANGLES, 0, 6);

                glBindTexture(GL_TEXTURE_2D, 0);
                glUniform1i(hudUseTextureLoc, 0);
            }
            else {
                // fallback ako nema teksture
                glUniform1i(hudUseTextureLoc, 0);
                glUniform4f(hudColorLoc, 0.2f, 0.8f, 0.2f, 1.0f);
                glUniform2f(hudPosLoc, 0.0f, 0.3f);
                glUniform2f(hudScaleLoc, 0.5f, 0.15f);
                glBindVertexArray(quadVAO);
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }
        }

        // studentski podaci
        if (texStudentInfo != 0) {
            glUniform1i(hudUseTextureLoc, 1);
            glUniform1i(hudTextureLoc, 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texStudentInfo);

            float imgW = 600.0f;
            float imgH = 300.0f;
            float desiredWidthNDC = 0.48f;

            float scaleX = desiredWidthNDC;
            float scaleY = desiredWidthNDC * (imgH / imgW) * ((float)wWidth / (float)wHeight);

            float posX = 1.0f - scaleX - 0.02f;
            float posY = -1.0f + 0.02f;

            glUniform4f(hudColorLoc, 1.0f, 1.0f, 1.0f, 0.7f);
            glUniform2f(hudPosLoc, posX + scaleX / 2.0f, posY + scaleY / 2.0f);
            glUniform2f(hudScaleLoc, scaleX / 2.0f, scaleY / 2.0f);
            glBindVertexArray(quadVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            glBindTexture(GL_TEXTURE_2D, 0);
            glUniform1i(hudUseTextureLoc, 0);
        }

        if (depthTestEnabled) {
            glEnable(GL_DEPTH_TEST);
        }

        mouseClicked = false;
        glfwSwapBuffers(window);

        // frame limiter
        auto frameEnd = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = frameEnd - currentTime;
        if (elapsed.count() < FRAME_TIME) {
            std::this_thread::sleep_for(std::chrono::duration<double>(FRAME_TIME - elapsed.count()));
        }
    }

    // cleanup
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteBuffers(1, &cubeVBO);
    glDeleteVertexArrays(1, &cylinderVAO);
    glDeleteBuffers(1, &cylinderVBO);
    glDeleteVertexArrays(1, &plateVAO);
    glDeleteBuffers(1, &plateVBO);
    glDeleteVertexArrays(1, &coneVAO);
    glDeleteBuffers(1, &coneVBO);
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
    
    // cleanup 3D modela
    if (breadModel.VAO != 0) {
        glDeleteVertexArrays(1, &breadModel.VAO);
        glDeleteBuffers(1, &breadModel.VBO);
    }
    if (pattyModel.VAO != 0) {
        glDeleteVertexArrays(1, &pattyModel.VAO);
        glDeleteBuffers(1, &pattyModel.VBO);
    }
    if (krastavcicModel.VAO != 0) {
        glDeleteVertexArrays(1, &krastavcicModel.VAO);
        glDeleteBuffers(1, &krastavcicModel.VBO);
    }
    if (lukModel.VAO != 0) {
        glDeleteVertexArrays(1, &lukModel.VAO);
        glDeleteBuffers(1, &lukModel.VBO);
    }

    glDeleteProgram(phongShader);
    glDeleteProgram(hudShader);
    if (texStudentInfo != 0) glDeleteTextures(1, &texStudentInfo);
    if (texPrijatno != 0) glDeleteTextures(1, &texPrijatno);

    glfwTerminate();
    return 0;
}