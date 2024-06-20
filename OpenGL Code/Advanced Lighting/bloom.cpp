#define GLM_ENABLE_EXPERIMENTAL

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <math.h>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Shaders/Shader.h>
#include <Camera/camera.h>

#include <vector>
#include <map>

#include <objLoader/objLoader.h>
#include <Model_assimp/Model.h>

using namespace std;

void frameBufferSizeCallback(GLFWwindow *window, int width, int height);
void inputCallback(GLFWwindow *window);
void mouseCallback(GLFWwindow *window, double xPos, double yPos);
void scrollWheelCallback(GLFWwindow *window, double xOffset, double yOffset);
unsigned int loadTexture2D(const char *filePath);
unsigned int loadCubeMap(vector<string> cubeMapFilePaths);
void renderScene(const Shader &shader, unsigned int texture1, unsigned int texture2);
void renderQuad();
void renderCube();

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;
float heightScale = 0.1f;
float exposure = 1.0f;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastMouseX = (float)SCR_WIDTH / 2.0;
float lastMouseY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;
bool isBloom = true;
bool isBloomActive = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
    //----------------------Initialize GLFW and GLAD--------------------
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(800, 800, "Hello Window!", NULL, NULL);

    if (window == NULL)
    {
        cout << "Failed to create window!" << endl;
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        cout << "Failed to start GLAD" << endl;
        return -1;
    }

    glViewport(0, 0, 800, 800);
    glfwSetFramebufferSizeCallback(window, frameBufferSizeCallback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouseCallback);

    glfwSetScrollCallback(window, scrollWheelCallback);

    //----------------Create and Bind VAOs, and VBOs----------------------
    //----------------Custom frame buffer for HDR and bloom------------------------

    // since we want to render colors past 1.0, we need our own frame buffer to do that
    unsigned int hdrFBO;
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

    unsigned int fColorBuffers[2]; // 1 for normal scene, 1 for especially bright stuff (look in fragment.fs)
    glGenTextures(2, fColorBuffers);
    unsigned int attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};

    for (int i = 0; i < 2; i++) // get texture stuff set up
    {

        glBindTexture(GL_TEXTURE_2D, fColorBuffers[i]);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glBindTexture(GL_TEXTURE_2D, 0);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, fColorBuffers[i], 0);
    }

    glDrawBuffers(2, attachments); // tell openGL we wish to write to 2 color attachments

    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        cout << "Error: frame buffer not complete :(" << endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    unsigned int blurFBOs[2]; // we need a blur FBO for horizontal gaussian, another for vertical
    glGenFramebuffers(2, blurFBOs);

    unsigned int blurColorBuffers[2]; // 2 color buffers for gaussian components
    glGenTextures(2, blurColorBuffers);

    for (int i = 0; i < 2; i++) // no depth attachment since we care about only colors
    {
        glBindFramebuffer(GL_FRAMEBUFFER, blurFBOs[i]);
        glBindTexture(GL_TEXTURE_2D, blurColorBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurColorBuffers[i], 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;
    }

    //-----------Create and compile shaders-------------------------------
    Shader shader("../resources/shaders/bloom/vertex.vs", "../resources/shaders/bloom/fragment.fs");
    Shader bloomShader("../resources/shaders/bloom/bloom.vs", "../resources/shaders/bloom/bloom.fs");
    Shader lightShader("../resources/shaders/bloom/lightVertex.vs", "../resources/shaders/bloom/lightFragment.fs");
    Shader blurShader("../resources/shaders/bloom/blur.vs", "../resources/shaders/bloom/blur.fs");

    //---------------------Texture Loading--------------------------------

    unsigned int woodDiffuse = loadTexture2D("../resources/textures/toybox/wood.png");
    unsigned int containerDiffuse = loadTexture2D("../resources/textures/crate.png");

    shader.use();
    glUniform1i(glGetUniformLocation(shader.ID, "diffuseTex"), 0);

    bloomShader.use();
    glUniform1i(glGetUniformLocation(bloomShader.ID, "hdrBuffer"), 0);
    glUniform1i(glGetUniformLocation(bloomShader.ID, "bloomBuffer"), 1);

    blurShader.use();
    glUniform1i(glGetUniformLocation(blurShader.ID, "image"), 0); // here so that we can get data from blur color buffers

    // -------------lighting info-------------
    std::vector<glm::vec3> lightPositions;
    lightPositions.push_back(glm::vec3(0.0f, 0.5f, 1.5f));
    lightPositions.push_back(glm::vec3(-4.0f, 0.5f, -3.0f));
    lightPositions.push_back(glm::vec3(3.0f, 0.5f, 1.0f));
    lightPositions.push_back(glm::vec3(-.8f, 2.4f, -1.0f));
    // colors
    std::vector<glm::vec3> lightColors;
    lightColors.push_back(glm::vec3(5.0f, 5.0f, 5.0f));
    lightColors.push_back(glm::vec3(10.0f, 0.0f, 0.0f));
    lightColors.push_back(glm::vec3(0.0f, 0.0f, 15.0f));
    lightColors.push_back(glm::vec3(0.0f, 5.0f, 0.0f));

    // ----------------------------configure global opengl state-----------------------------
    glEnable(GL_DEPTH_TEST);
    //----------------Render Loop-----------------------------------------
    while (!glfwWindowShouldClose(window))
    {
        float curFrame = glfwGetTime();
        deltaTime = curFrame - lastFrame;
        lastFrame = curFrame;

        glm::mat4 modelMat = glm::mat4(1.0f);
        glm::mat4 viewMat = glm::mat4(1.0f);
        glm::mat4 projectionMat = glm::mat4(1.0f);

        // 1: render everything to our hdrFBO
        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
        glClearColor(0.1, 0.1, 0.1, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        viewMat = camera.GetViewMatrix();
        projectionMat = glm::perspective(glm::radians(camera.Zoom), 800.0f / 800.0f, 0.1f, 1000.0f);

        // render scene objects
        shader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, containerDiffuse);

        for (int i = 0; i < lightPositions.size(); i++)
        {
            glUniform3f(glGetUniformLocation(shader.ID, ("lights[" + to_string(i) + "].position").c_str()), lightPositions[i].x, lightPositions[i].y, lightPositions[i].z);
            glUniform3f(glGetUniformLocation(shader.ID, ("lights[" + to_string(i) + "].color").c_str()), lightColors[i].x, lightColors[i].y, lightColors[i].z);
        }

        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "viewMat"), 1, GL_FALSE, glm::value_ptr(viewMat));
        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projectionMat"), 1, GL_FALSE, glm::value_ptr(projectionMat));

        renderScene(shader, woodDiffuse, containerDiffuse);

        // render lighting objects
        lightShader.use();
        glUniformMatrix4fv(glGetUniformLocation(lightShader.ID, "viewMat"), 1, GL_FALSE, glm::value_ptr(viewMat));
        glUniformMatrix4fv(glGetUniformLocation(lightShader.ID, "projectionMat"), 1, GL_FALSE, glm::value_ptr(projectionMat));
        for (int i = 0; i < lightPositions.size(); i++)
        {
            modelMat = glm::mat4(1.0f);
            modelMat = glm::translate(modelMat, lightPositions[i]);
            modelMat = glm::scale(modelMat, glm::vec3(0.25f));
            glUniformMatrix4fv(glGetUniformLocation(lightShader.ID, "modelMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
            glUniform3f(glGetUniformLocation(lightShader.ID, "lightColor"), lightColors[i].x, lightColors[i].y, lightColors[i].z);
            renderCube();
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 2: Apply gaussian blur, we essentially blur back and forth 10 times 5 in horizontal, 5 in vertical. we dont do this in hdr tex
        blurShader.use();
        bool horizontal = true, firstIter = true;
        int blurAmt = 10;
        for (int i = 0; i < blurAmt; i++)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, blurFBOs[horizontal]);
            glUniform1f(glGetUniformLocation(blurShader.ID, "isHorizontal"), horizontal);
            glBindTexture(GL_TEXTURE_2D, firstIter ? fColorBuffers[1] : blurColorBuffers[!horizontal]); // blur only the especially bright stuff!

            renderQuad();
            horizontal = !horizontal;

            if (firstIter)
            {
                firstIter = !firstIter;
            }
        }

        // 3: now we render to default fbo, we have 2 textures, our hdr one, and our bloom one, we then render
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        bloomShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fColorBuffers[0]); // represents hdr stuff
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, blurColorBuffers[!horizontal]); // our blured bright stuff

        glUniform1f(glGetUniformLocation(bloomShader.ID, "exposure"), exposure);
        glUniform1f(glGetUniformLocation(bloomShader.ID, "isBloom"), isBloom);
        renderQuad();

        inputCallback(window);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void renderScene(const Shader &shader, unsigned int texture1, unsigned int texture2)
{
    glUniform1i(glGetUniformLocation(shader.ID, "invertNormals"), false);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1);

    glm::mat4 modelMat = glm::mat4(1.0f);
    modelMat = glm::translate(modelMat, glm::vec3(0.0f, -1.0f, 0.0));
    modelMat = glm::scale(modelMat, glm::vec3(12.5f, 0.5f, 12.5f));
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "modelMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
    renderCube();

    glBindTexture(GL_TEXTURE_2D, texture2);
    modelMat = glm::mat4(1.0f);
    modelMat = glm::translate(modelMat, glm::vec3(0.0f, 1.5f, 0.0));
    modelMat = glm::scale(modelMat, glm::vec3(0.5f));
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "modelMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
    renderCube();

    modelMat = glm::mat4(1.0f);
    modelMat = glm::translate(modelMat, glm::vec3(2.0f, 0.0f, 1.0));
    modelMat = glm::scale(modelMat, glm::vec3(0.5f));
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "modelMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
    renderCube();

    modelMat = glm::mat4(1.0f);
    modelMat = glm::translate(modelMat, glm::vec3(-1.0f, -1.0f, 2.0));
    modelMat = glm::rotate(modelMat, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "modelMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
    renderCube();

    modelMat = glm::mat4(1.0f);
    modelMat = glm::translate(modelMat, glm::vec3(0.0f, 2.7f, 4.0));
    modelMat = glm::rotate(modelMat, glm::radians(23.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
    modelMat = glm::scale(modelMat, glm::vec3(1.25));
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "modelMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
    renderCube();

    modelMat = glm::mat4(1.0f);
    modelMat = glm::translate(modelMat, glm::vec3(-2.0f, 1.0f, -3.0));
    modelMat = glm::rotate(modelMat, glm::radians(124.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "modelMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
    renderCube();

    modelMat = glm::mat4(1.0f);
    modelMat = glm::translate(modelMat, glm::vec3(-3.0f, 0.0f, 0.0));
    modelMat = glm::scale(modelMat, glm::vec3(0.5f));
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "modelMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
    renderCube();
}

// renders a 1x1 quad in NDC with manually calculated tangent vectors
// ------------------------------------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,
            1.0f,
            0.0f,
            0.0f,
            1.0f,
            -1.0f,
            -1.0f,
            0.0f,
            0.0f,
            0.0f,
            1.0f,
            1.0f,
            0.0f,
            1.0f,
            1.0f,
            1.0f,
            -1.0f,
            0.0f,
            1.0f,
            0.0f,
        };

        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);

        glBindVertexArray(quadVAO);

        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);

        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

unsigned int cubeVAO = 0, cubeVBO = 0;

void renderCube()
{
    if (cubeVAO == 0)
    {
        float vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,   // top-right
            1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,  // bottom-right
            1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,   // top-right
            -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,  // top-left
            // front face
            -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
            1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,  // bottom-right
            1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,   // top-right
            1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,   // top-right
            -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,  // top-left
            -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,   // top-right
            -1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // top-left
            -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // bottom-right
            -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,   // top-right
                                                                // right face
            1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,     // top-left
            1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,   // bottom-right
            1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,    // top-right
            1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,   // bottom-right
            1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,     // top-left
            1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,    // bottom-left
            // bottom face
            -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
            1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,  // top-left
            1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,   // bottom-left
            1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,   // bottom-left
            -1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,  // bottom-right
            -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
            // top face
            -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
            1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,   // bottom-right
            1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,  // top-right
            1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,   // bottom-right
            -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
            -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f   // bottom-left
        };

        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);

        glBindVertexArray(cubeVAO);

        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);

        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

void frameBufferSizeCallback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void inputCallback(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    const float cameraSpeed = 2.5f * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        camera.ProcessKeyboard(FORWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        camera.ProcessKeyboard(LEFT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        camera.ProcessKeyboard(RIGHT, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        if (exposure > 0.0f)
            exposure -= 0.0005f;
        else
            exposure = 0.0f;
    }
    else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        if (exposure < 1.0f)
            exposure += 0.0005f;
        else
            exposure = 1.0f;
    }

    else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !isBloomActive)
    {
        isBloom = !isBloom;
        isBloomActive = true;
    }

    else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
    {
        isBloomActive = false;
    }
}

void mouseCallback(GLFWwindow *window, double xPos, double yPos)
{

    if (firstMouse)
    {
        firstMouse = false;
        lastMouseX = xPos;
        lastMouseY = yPos;
    }

    float xOffset = xPos - lastMouseX;
    float yOffset = yPos - lastMouseY;

    lastMouseX = xPos;
    lastMouseY = yPos;

    camera.ProcessMouseMovement(xOffset, yOffset);
}

void scrollWheelCallback(GLFWwindow *window, double xOffset, double yOffset)
{
    camera.ProcessMouseScroll(yOffset);
}

unsigned int loadTexture2D(char const *filePath)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, numOfChannels;
    unsigned char *imageData = stbi_load(filePath, &width, &height, &numOfChannels, 0);
    if (imageData)
    {
        GLenum format;
        if (numOfChannels == 1)
            format = GL_RED;
        else if (numOfChannels == 3)
            format = GL_RGB;
        else if (numOfChannels == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, imageData);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(imageData);
    }
    else
    {
        cout << "Texture failed to load at file path: " << filePath << endl;
        stbi_image_free(imageData);
    }

    return textureID;
}

unsigned int loadCubeMap(vector<string> cubeMapFilePaths)
{
    int width, height, channelAmt;
    unsigned int cubeMapId;

    unsigned char *imageData;

    glGenTextures(1, &cubeMapId);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapId);

    for (int i = 0; i < cubeMapFilePaths.size(); i++)
    {
        imageData = stbi_load(cubeMapFilePaths[i].c_str(), &width, &height, &channelAmt, 0);

        if (imageData)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData);
            stbi_image_free(imageData);
        }

        else
        {
            cout << "Failed to load image at file path: " << cubeMapFilePaths[i] << endl;
            stbi_image_free(imageData);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return cubeMapId;
}