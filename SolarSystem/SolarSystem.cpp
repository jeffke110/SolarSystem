#include <glad/glad.h>
#include <GLFW/glfw3.h>


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Camera.h"
#include "Sphere.h"
#include "Texture.h"

#include <iostream>

#define M_PI 3.14159265358979323846

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

struct Planet {
    float orbitRadius;      // Distance from Sun
    float orbitSpeed;       // Revolution speed (relative to Earth's)
    float scale;            // Scaling factor (relative to Sun)
    float rotationSpeed;    // Self-rotation speed
    GLuint textureID;       // Texture ID for rendering
};

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    glEnable(GL_DEPTH_TEST);
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
    
    // -------------------------------------------------------------------------------------------

    planetShader.use();
    planetShader.setInt("texture1", 0);

    std::vector<Planet> planets = {
    {5.0f, 4.15f, 0.00351f, 1.0f / 58.6f, mercuryTexture.textureID}, // Mercury
    {7.0f, 1.62f, 0.0087f, 1.0f / 243.0f, venusTexture.textureID},   // Venus
    {10.0f, 1.0f, 0.00916f, 1.0f, earthTexture.textureID},           // Earth
    {15.0f, 0.53f, 0.00487f, 1.0f, marsTexture.textureID},           // Mars
    {30.0f, 0.08f, 0.1005f, 2.5f, jupiterTexture.textureID},         // Jupiter
    {40.0f, 0.03f, 0.0837f, 2.3f, saturnTexture.textureID},          // Saturn
    {50.0f, 0.0119f, 0.0365f, 1.4f, uranusTexture.textureID},        // Uranus
    {60.0f, 0.00606f, 0.0354f, 1.3f, neptuneTexture.textureID}       // Neptune
    };

    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);
        glClearColor(0.01f, 0.01f, 0.01f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);



        // SUN
        glBindTexture(GL_TEXTURE_2D, sunTexture.textureID);
        glm::mat4 model = glm::mat4(1.0f);
        planetShader.setMat4("model", model);
        sphere.renderSphere();

        // Render each planet
        for (const auto& planet : planets) {
            float timeScale = 1.0f;
            float time = glfwGetTime() * timeScale; 

            // Calculate planet position (XZ plane orbit)
            glm::vec3 position = glm::vec3(
                sin(time * planet.orbitSpeed) * planet.orbitRadius,
                0.0f,
                cos(time * planet.orbitSpeed) * planet.orbitRadius
            );

            // Create transformation matrix
            model = glm::mat4(1.0f);
            model = glm::translate(model, position); // Move to orbit position
            model = glm::rotate(model, time * planet.rotationSpeed, glm::vec3(0, 1, 0)); // Self-rotate
            //model = glm::scale(model, glm::vec3(planet.scale)); // Scale planet size

            // Bind texture and render
            glBindTexture(GL_TEXTURE_2D, planet.textureID);
            planetShader.setMat4("model", model);
            sphere.renderSphere();

        }
        
        planetShader.use();

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        planetShader.setMat4("projection", projection);

        glm::mat4 view = camera.GetViewMatrix();
        planetShader.setMat4("view", view);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    sphere.DeleteBuffers();
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
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
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}