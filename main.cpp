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
#include "ObjLoader.h"


MeshData probe; 

bool earthLightOn = true;

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


GLuint loadTexture(const char* path) {
    GLuint textureID;
    glGenTextures(1, &textureID);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = (nrChannels == 3) ? GL_RGB : GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    } else {
        std::cerr << "Failed to load texture at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

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
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) 
        earthLightOn = true;
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
        earthLightOn = false;
        
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
    glDisable(GL_CULL_FACE);
    glFrontFace(GL_CW); // Use clockwise as front-facing instead of default CCW

    // ====== SHADOW MAP INIT ======
    GLuint depthMapFBO = 0, depthMap = 0;
    const unsigned int SHADOW_SIZE = 2048;

    glGenFramebuffers(1, &depthMapFBO);

    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
             SHADOW_SIZE, SHADOW_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // ====== END SHADOW MAP INIT ======

    Shader shader("vertex.glsl", "fragment.glsl");

    Shader depthShader("shadow_depth.vert", "shadow_depth.frag");

    
    probe = loadOBJ("Asteroid/Asteroid.obj");

    GLuint asteroidTexture = loadTexture("Asteroid/Asteroid.jpg");


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

        
        shader.setVec3("viewPos", camera.Position);

        // === SUN directional light (bright + warm) ===
        shader.setVec3("sun.ambient",   glm::vec3(0.6f, 0.5f, 0.4f));
        shader.setVec3("sun.diffuse",   glm::vec3(5.0f, 4.0f, 3.0f));
        shader.setVec3("sun.specular",  glm::vec3(2.5f, 2.3f, 2.0f));

        
        
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

        shader.setVec3("earthLight.position",  earthPosition);
        shader.setVec3("earthLight.ambient",   glm::vec3(0.2f, 0.2f, 0.4f));
        shader.setVec3("earthLight.diffuse", earthLightOn ? glm::vec3(2.0f, 2.6f, 3.6f) : glm::vec3(0.0f));
        shader.setVec3("earthLight.specular", earthLightOn ? glm::vec3(1.2f, 1.4f, 2.2f) : glm::vec3(0.0f));
        shader.setFloat("earthLight.constant",  1.0f);
        shader.setFloat("earthLight.linear",    0.0f);
        shader.setFloat("earthLight.quadratic", 0.0f);

       // === Light-space matrix for Sun ===
        glm::vec3 center = glm::vec3(0.0f); // center of solar system
        float tSun = glfwGetTime() * 0.2f;
        glm::vec3 sunDir = glm::normalize(glm::vec3(cos(tSun), 0.1f, sin(tSun)));
        glm::vec3 lightPos = center - sunDir * 50.0f;

        float orthoRange = 30.0f;
        glm::mat4 lightProj = glm::ortho(-orthoRange, orthoRange, -orthoRange, orthoRange, 1.0f, 120.0f);
        glm::mat4 lightView = glm::lookAt(lightPos, center, glm::vec3(0, 1, 0));
        glm::mat4 lightSpaceMatrix = lightProj * lightView;

        shader.setVec3("sun.direction", sunDir);

        // ====== DEPTH PASS ======
        glViewport(0, 0, SHADOW_SIZE, SHADOW_SIZE);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);

        depthShader.use();
        depthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        // Draw your scene from light's perspective
        // (reuse your draw functions but with depthShader)
        // Skip drawing the Sun so it doesn't cast shadows
        // renderScene(depthShader, true); // <-- you'll define this to draw all shadow-casters
        // depth pass: draw all shadow casters (NO SUN)
        renderPlanet(earth,   depthShader, earthPosition,   earthScale,   earthSpin, 23.5f);
        renderPlanet(moon,    depthShader, moonPosition,    moonScale);
        renderPlanet(mercury, depthShader, mercuryPosition, mercuryScale, mercurySpin, 0.0f);
        renderPlanet(venus,   depthShader, venusPosition,   venusScale,   venusSpin, 177.0f);
        renderPlanet(mars,    depthShader, marsPosition,    marsScale,    marsSpin, 25.0f);
        renderPlanet(phobos,  depthShader, phobosPosition,  phobosScale);
        renderPlanet(deimos,  depthShader, deimosPosition,  deimosScale);
        renderPlanet(jupiter, depthShader, jupiterPosition, jupiterScale, jupiterSpin, 3.0f);
        renderPlanet(uranus,  depthShader, uranusPosition,  uranusScale,  uranusSpin, 97.8f);
        renderPlanet(saturn,  depthShader, saturnPosition,  saturnScale,  saturnSpin, 26.7f);
        // (Optionally skip rings in depth if they’re alpha; leave them out for now)
        renderPlanet(neptune, depthShader, neptunePosition, neptuneScale, neptuneSpin, 28.3f);

        // probe in depth pass
        glm::mat4 probeModelMatrixDepth = glm::translate(glm::mat4(1.0f), glm::vec3(25.0f, 0.0f, -5.0f));
        probeModelMatrixDepth = glm::scale(probeModelMatrixDepth, glm::vec3(0.001f));
        depthShader.setMat4("model", probeModelMatrixDepth);
        glBindVertexArray(probe.VAO);
        glDrawElements(GL_TRIANGLES, probe.indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);


        glBindFramebuffer(GL_FRAMEBUFFER, 0);

       // ====== MAIN PASS ======
        int fbw, fbh;
        glfwGetFramebufferSize(window, &fbw, &fbh);
        glViewport(0, 0, fbw, fbh);   // <-- FIX: full window size (HiDPI safe)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        shader.setMat4("projection", projection);  
        shader.setMat4("view", view);    
        shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

       // shadow map on unit 1
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        shader.setInt("shadowMap", 1);

        // make sure planet/albedo textures use unit 0 inside renderPlanet(...)
        glActiveTexture(GL_TEXTURE0);

        // rendering
        shader.setBool("isSun", true);
        renderPlanet(sun, shader, glm::vec3(0.0f), earthScale * 10.0f);
        shader.setBool("isSun", false);
        shader.setBool("isEarth", true);
        renderPlanet(earth, shader, earthPosition, earthScale, earthSpin, 23.5f);
        shader.setBool("isEarth", false);
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
        

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, asteroidTexture);
        shader.setInt("texture1", 0); // or whatever your sampler name is
        

        glm::mat4 probeModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(25.0f, 0.0f, -5.0f));
        probeModelMatrix = glm::scale(probeModelMatrix, glm::vec3(0.001f));
        shader.setMat4("model", probeModelMatrix);

        glBindVertexArray(probe.VAO);
        glDrawElements(GL_TRIANGLES, probe.indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);


        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
