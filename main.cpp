// core libraries for OpenGL
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

// custom classes
#include "Camera.h"
#include "Shader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "PlanetRenderer.h"




const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
// camera angles
Camera camera(glm::vec3(0.0f, 0.0f, 30.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float timeBoost = 0.0f; //time added by the user

void framebuffer_size_callback(GLFWwindow* window, int width, int height) { // resizing the window frame
    glViewport(0, 0, width, height);
}
void mouse_callback(GLFWwindow* window, double xpos, double ypos) { // mouse movement
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; 

    float sensitivity = 0.1f; // Lower values = less sensitive
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    lastX = xpos;
    lastY = ypos;
    camera.ProcessMouseMovement(xoffset, yoffset);
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(yoffset);
}
void processInput(GLFWwindow* window) { // key strokes for positioning of what angle the user wants to see
    float currentTime = glfwGetTime();
    deltaTime = currentTime - lastFrame;
    lastFrame = currentTime;
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
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
        timeBoost += 0.05f; // Skip forward in time while holding "3" key
}

int main() {
    glfwInit(); // initialize opengl
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Solar System", NULL, NULL); // creating the window
    if (window == NULL) { // error handling
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // callback functions
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    // error handling
    if (glewInit() != GLEW_OK) {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST); // depth buffer
    Shader shader("vertex.glsl", "fragment.glsl");
    
    // image textures of the planets
    Planet sun, earth, moon, mercury, venus, mars, phobos, deimos, saturn, saturnRings, jupiter, uranus, neptune;
    initPlanet(sun, "sun_texture.jpg");
    initPlanet(earth, "earth_texture.jpg");
    initPlanet(moon, "moon_texture.jpg");
    initPlanet(mercury, "mercury_texture.jpg");
    initPlanet(venus, "venus_texture.jpg");
    initPlanet(mars, "mars_texture.jpg");
    initPlanet(phobos, "phobos_texture.jpg");
    initPlanet(deimos, "deimos_texture.jpg");
    //TODO: get new textures for other planets
    initPlanet(saturn, "saturn_texture.jpg");
    initRings(saturnRings, "saturnRings_texture.png");
    initPlanet(jupiter, "jupiter_texture.jpg");
    initPlanet(uranus, "uranus_texture.jpg");
    initPlanet(neptune, "neptune_texture.jpg");


    // position of mercury
    glm::vec3 mercuryPosition = glm::vec3(2.0f, 0.0f, 0.0f);
    float mercuryScale = 0.25f;
    // position of earth
    glm::vec3 earthPosition = glm::vec3(5.0f, 0.0f, 0.0f);
    float earthScale = 0.5f;
    // position of moon
    glm::vec3 moonPosition = glm::vec3(6.0f, 0.0f, 0.0f);
    float moonScale = 0.15f;
    // position of venus
    glm::vec3 venusPosition = glm::vec3(3.5f, 0.0f, 0.0f);
    float venusScale = 0.4f;
    // position of mars
    glm::vec3 marsPosition = glm::vec3(8.0f, 0.0f, 0.0f);
    float marsScale = 0.5f;

    //TODO: double check numbers I just made up them
    //position of jupiter
    glm::vec3 jupiterPosition = glm::vec3(15.0f, 0.0f, 0.0f);
    float jupiterScale = 1.0f;
    //position of uranus
    glm::vec3 uranusPosition = glm::vec3(25.0f, 0.0f, 0.0f);
    float uranusScale = 0.8f;
    //position of saturn
    glm::vec3 saturnPosition = glm::vec3(35.0f, 0.0f, 0.0f);
    float saturnScale = 0.9f;
    //position of neptune
    glm::vec3 neptunePosition = glm::vec3(45.0f, 0.0f, 0.0f);
    float neptuneScale = 0.7f;




    while (!glfwWindowShouldClose(window)) {
        processInput(window); // input
        glClearColor(0.0f, 0.0f, 0.05f, 1.0f); // clears screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use(); // activating the shaders 
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), // camera matrices 
        (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        
        // Draw sun, planets, moons here
        float time = (glfwGetTime() * 0.2f) + timeBoost;
        float earthSpin = time * 50.0f;  // adjust speed as needed
        float mercurySpin = time * 5.0f;
        float venusSpin   = time * -1.0f;
        float marsSpin    = time * 48.0f;
        //TODO : Double check speeds, I just made up them
        float jupiterSpin = time * 12.0f;
        float uranusSpin = time * 13.0f;
        float saturnSpin = time * 16.0f; 
        float neptuneSpin = time * 25.0f; 


        // earth
        glm::vec3 earthPosition = glm::vec3(
            8.0f * cos(time),
            0.0f,
            8.0f * sin(time)
        );
        float earthScale = 0.5f;
        // moon
        glm::vec3 moonPosition = earthPosition + glm::vec3(
            1.0f * cos(time * 4.0f),
            0.0f,
            1.0f * sin(time * 4.0f)
        );
        float moonScale = earthScale * 0.27f;
        
        // mercury
        glm::vec3 mercuryPosition = glm::vec3(
            5.5f * cos(time * 2.0f),
            0.0f,
            5.5f * sin(time * 2.0f)
        );
        float mercuryScale = earthScale * 0.38f;

        // venus
        glm::vec3 venusPosition = glm::vec3(
            6.5f * cos(time * 1.3f),
            0.0f,
            6.5f * sin(time * 1.3f)
        );
        float venusScale = earthScale * 0.95f;

        // mars
        glm::vec3 marsPosition = glm::vec3(
            11.0f * cos(time * 0.9f),
            0.0f,
            11.0f * sin(time * 0.9f)
        );
        float marsScale = earthScale * 0.53f;
        
        glm::vec3 phobosPosition = marsPosition + glm::vec3(
            0.3f * cos(4.0f * time), // fast orbit
            0.0f,
            0.3f * sin(4.0f * time)
        );
        float phobosScale = marsScale * 0.05f;

        glm::vec3 deimosPosition = marsPosition + glm::vec3(
            0.6f * cos(1.0f * time), // slower orbit
            0.0f,
            0.6f * sin(1.0f * time)
        );
        float deimosScale = marsScale * 0.03f;

        //jupiter, uranus, saturn, neptune
        //TODO: double check numbers
        glm::vec3 jupiterPosition = glm::vec3(
            18.0f * cos(time * 0.5f),
            0.0f,
            18.0f * sin(time * 0.5f)
        );
        float jupiterScale = earthScale * 11.2f;

        glm::vec3 uranusPosition = glm::vec3(
            27.0f * cos(time * 0.3f),
            0.0f,
            27.0f * sin(time * 0.3f)
        );
        float uranusScale = earthScale * 4.0f;

        glm::vec3 saturnPosition = glm::vec3(
            38.0f * cos(time * 0.4f),
            0.0f,
            38.0f * sin(time * 0.4f)
        );
        float saturnScale = earthScale * 9.4f;

        glm::vec3 saturnRingsPosition = saturnPosition;
        float saturnRingsScale = saturnScale * 2.0f; // scale for rings

        glm::vec3 neptunePosition = glm::vec3(
            48.0f * cos(time * 0.2f),
            0.0f,
            48.0f * sin(time * 0.2f)
        );
        float neptuneScale = earthScale * 3.9f;


        // rendering
        renderPlanet(sun, shader, glm::vec3(0.0f), earthScale * 10.0f);
        renderPlanet(earth, shader, earthPosition, earthScale, earthSpin, 23.5f);
        renderPlanet(moon, shader, moonPosition, moonScale);
        renderPlanet(mercury, shader, mercuryPosition, mercuryScale, mercurySpin, 0.0f);
        renderPlanet(venus, shader, venusPosition, venusScale, venusSpin, 177.0f);
        renderPlanet(mars, shader, marsPosition, marsScale, marsSpin, 25.0f);
        renderPlanet(phobos, shader, phobosPosition, phobosScale);
        renderPlanet(deimos, shader, deimosPosition, deimosScale);
        //TODO: double check numbers
        renderPlanet(jupiter, shader, jupiterPosition, jupiterScale, jupiterSpin, 3.0f);
        renderPlanet(uranus, shader, uranusPosition, uranusScale, uranusSpin, 97.8f);
        renderPlanet(saturn, shader, saturnPosition, saturnScale, saturnSpin, 26.7f);
        renderRings(saturnRings, shader, saturnRingsPosition, saturnRingsScale);
        renderPlanet(neptune, shader, neptunePosition, neptuneScale, neptuneSpin, 28.3f);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
