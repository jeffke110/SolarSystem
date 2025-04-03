#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Camera.h"
#include "Sphere.h"
#include "Texture.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <iostream>

#define M_PI 3.14159265358979323846

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool mouseVisibility = false;

struct Planet {
    float orbitRadius;
    float orbitSpeed;
    float scale;
    float rotationSpeed;
    GLuint textureID;
};

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Solar System Simulation", NULL, NULL);
    if (!window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    glEnable(GL_DEPTH_TEST);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    Shader planetShader("shader.vs", "shader.fs");
    Shader lightingShader("lighting_shader.vs", "lighting_shader.fs");
    Sphere sphere;

    Texture sunTexture("sun.jpg");
    Texture mercuryTexture("mercury.jpg");
    Texture venusTexture("venus.jpg");
    Texture earthTexture("earth.jpg");
    Texture marsTexture("mars.jpg");
    Texture jupiterTexture("jupiter.jpg");
    Texture saturnTexture("saturn.jpg");
    Texture uranusTexture("uranus.jpg");
    Texture neptuneTexture("neptune.jpg");

    planetShader.use();
    planetShader.setInt("texture1", 0);

    std::vector<Planet> planets = {
        {5.0f, 1.0f, 0.00916f, 1.0f, earthTexture.textureID},           // Earth
        {7.0f, 1.62f, 0.0087f, 1.0f / 243.0f, venusTexture.textureID},  // Venus
        {15.0f, 0.53f, 0.00487f, 1.03f, marsTexture.textureID},         // Mars
        {30.0f, 0.08f, 0.1005f, 2.5f, jupiterTexture.textureID},        // Jupiter
        {40.0f, 0.03f, 0.0837f, 2.3f, saturnTexture.textureID},         // Saturn
        {50.0f, 0.0119f, 0.0365f, 1.4f, uranusTexture.textureID},       // Uranus
        {60.0f, 0.00606f, 0.0354f, 1.3f, neptuneTexture.textureID},     // Neptune
        {3.0f, 4.15f, 0.00351f, 1.0f / 58.6f, mercuryTexture.textureID} // Mercury
    };

    float planetScale = 1.0f;
    static const char* timeModes[] = { "1 sec = 1 year", "1 sec = 1 month", "1 sec = 1 week", "1 sec = 1 day" };
    static int currentMode = 0; // default: 1 sec = 1 day
    float timeScaleDaysPerSecond = 1.0f; // ensure it's synced with currentMode
    float timeScaleRotation = 1.0f;


    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);
        glClearColor(0.01f, 0.01f, 0.01f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);

        float time = glfwGetTime();
        float simTimeInDays = time / timeScaleDaysPerSecond;
        int earthDayCounter = static_cast<int>(simTimeInDays);

        glBindTexture(GL_TEXTURE_2D, sunTexture.textureID);
        glm::mat4 model = glm::mat4(1.0f);
        planetShader.setMat4("model", model);
        sphere.renderSphere();

        for (const auto& planet : planets) {


            float orbitAngle = simTimeInDays * planet.orbitSpeed * 2.0f * M_PI;
            float rotationAngle = ( planet.rotationSpeed * 2.0f * M_PI )  * timeScaleRotation * time;

            glm::vec3 position = glm::vec3(
                sin(orbitAngle) * planet.orbitRadius,
                0.0f,
                cos(orbitAngle) * planet.orbitRadius
            );
            model = glm::mat4(1.0f);
            model = glm::translate(model, position);
            model = glm::rotate(model, rotationAngle, glm::vec3(0, 1, 0));
            model = glm::scale(model, glm::vec3(planet.scale) * planetScale);

            glBindTexture(GL_TEXTURE_2D, planet.textureID);
            planetShader.setMat4("model", model);
            sphere.renderSphere();
        }

        planetShader.use();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        planetShader.setMat4("projection", projection);
        glm::mat4 view = camera.GetViewMatrix();
        planetShader.setMat4("view", view);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Controls (tab: show mouse)");
        ImGui::Text("Solar System Simulation");
        ImGui::Text("Camera Position: %.1f, %.1f, %.1f", camera.Position.x, camera.Position.y, camera.Position.z);
        ImGui::SliderFloat("Planet size", &planetScale, 1.0f, 100.0f);
        if (ImGui::Combo("Time Scale", &currentMode, timeModes, IM_ARRAYSIZE(timeModes))) {
            switch (currentMode) {
            case 0: timeScaleDaysPerSecond = 1.0f; timeScaleRotation = 365.0f * 30.0f * 7.0f;  break;
            case 1: timeScaleDaysPerSecond = 7.0f; timeScaleRotation = 30.437 * 7.0f; break;
            case 2: timeScaleDaysPerSecond = 30.437f; timeScaleRotation = 7.0f;  break;
            case 3: timeScaleDaysPerSecond = 365.0f; timeScaleRotation = 1.0f; break;
            }
        }
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    sphere.DeleteBuffers();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    static bool tabPressedLastFrame = false;
    bool tabPressed = glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS;
    if (tabPressed && !tabPressedLastFrame) {
        mouseVisibility = !mouseVisibility;
        glfwSetInputMode(window, GLFW_CURSOR, mouseVisibility ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
        lastX = SCR_WIDTH / 2.0f;
        lastY = SCR_HEIGHT / 2.0f;
    }
    tabPressedLastFrame = tabPressed;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    if (mouseVisibility) return;
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;
    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
