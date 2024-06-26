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
#include <random>
#include <cmath>

using namespace std;

void frameBufferSizeCallback(GLFWwindow *window, int width, int height);
void inputCallback(GLFWwindow *window);
void mouseCallback(GLFWwindow *window, double xPos, double yPos);
void scrollWheelCallback(GLFWwindow *window, double xOffset, double yOffset);
unsigned int loadTexture2D(const char *filePath);
float lerp(float a, float b, float f);
void renderScene(const Shader &shader, unsigned int texture1, unsigned int texture2);
void renderQuad();
void renderCube();
void renderDragon();

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;

// camera
Camera camera(glm::vec3(0.0f, 2.0f, 5.0f));
float lastMouseX = (float)SCR_WIDTH / 2.0;
float lastMouseY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

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
    //----------------Custom frame buffer --------------------------------

    /*instead of sending stuff from vertex to fragment per object per pixel, we can instead render everything without lighting and retrieve info
    such as the position, normals, and textures and store them in colorBuffers (textures/ g-buffers). Then we will have 3 textures
    that we then light the scene based on. That way we are only lighting per pixel instead of per object per pixel.*/

    unsigned int gFBO; // make the frame buffer for our g buffers
    glGenFramebuffers(1, &gFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, gFBO);

    // some data for each g-buffer
    unsigned int textureTypes[] = {GL_RGBA16F, GL_RGBA16F, GL_RGBA};
    unsigned int textureDataTypes[] = {GL_FLOAT, GL_FLOAT, GL_UNSIGNED_BYTE};
    unsigned int colorBuffers[3]; // {position, normal, albedo/specular}
    unsigned int attachments[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    glGenTextures(3, colorBuffers); // generate 3 textures and use array elements as ids

    for (int i = 0; i < 3; i++)
    {
        glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, textureTypes[i], SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, textureDataTypes[i], NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        if (i == 0)
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }

        glBindTexture(GL_TEXTURE_2D, 0);

        glFramebufferTexture2D(GL_FRAMEBUFFER, attachments[i], GL_TEXTURE_2D, colorBuffers[i], 0);
    }
    glDrawBuffers(3, attachments); // tell openGL we want to draw to these attachments

    unsigned int RBO; // we need depth in here too!
    glGenRenderbuffers(1, &RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, RBO);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        cout << "Error: frame buffer not complete :(" << endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    unsigned int ssaoFBO;
    glGenFramebuffers(1, &ssaoFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);

    unsigned int ssaoColorBuffer;
    glGenTextures(1, &ssaoColorBuffer);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    unsigned int ssaoBlurFBO;
    glGenFramebuffers(1, &ssaoBlurFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);

    unsigned int ssaoBlurColorBuffer;
    glGenTextures(1, &ssaoBlurColorBuffer);
    glBindTexture(GL_TEXTURE_2D, ssaoBlurColorBuffer);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SCR_WIDTH, SCR_HEIGHT, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoBlurColorBuffer, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //-----------Create and compile shaders-------------------------------
    Shader geometryPassShader("../resources/shaders/SSAO/geometrypass.vs", "../resources/shaders/SSAO/geometrypass.fs");
    Shader lightingPassShader("../resources/shaders/SSAO/lightingPass.vs", "../resources/shaders/SSAO/lightingPass.fs");
    Shader quadShader("../resources/shaders/SSAO/quadVertex.vs", "../resources/shaders/SSAO/quadFragment.fs"); // for seeing individual textures of g-buffer
    Shader lightCubeShader("../resources/shaders/SSAO/lightCube.vs", "../resources/shaders/SSAO/lightCube.fs");
    Shader ssaoShader("../resources/shaders/ssao/ssao.vs", "../resources/shaders/ssao/ssao.fs");
    Shader ssaoBlurShader("../resources/shaders/ssao/ssaoBlur.vs", "../resources/shaders/ssao/ssaoBlur.fs");
    //---------------------Texture Loading--------------------------------

    unsigned int floorDiffuse = loadTexture2D("../resources/textures/floor/floor_Diffuse.jpg");
    unsigned int floorSpecular = loadTexture2D("../resources/textures/floor/floor_Specular.png");

    geometryPassShader.use();
    glUniform1i(glGetUniformLocation(geometryPassShader.ID, "diffuseTex"), 0);
    glUniform1i(glGetUniformLocation(geometryPassShader.ID, "specularTex"), 1);

    lightingPassShader.use();
    glUniform1i(glGetUniformLocation(lightingPassShader.ID, "gPosition"), 0);
    glUniform1i(glGetUniformLocation(lightingPassShader.ID, "gNormal"), 1);
    glUniform1i(glGetUniformLocation(lightingPassShader.ID, "gAlbedoSpec"), 2);
    glUniform1i(glGetUniformLocation(lightingPassShader.ID, "ssao"), 3);

    ssaoShader.use();
    glUniform1i(glGetUniformLocation(ssaoShader.ID, "gPos"), 0);
    glUniform1i(glGetUniformLocation(ssaoShader.ID, "gNormal"), 1);
    glUniform1i(glGetUniformLocation(ssaoShader.ID, "gNoise"), 2);

    ssaoBlurShader.use();
    glUniform1i(glGetUniformLocation(ssaoBlurShader.ID, "ssaoTex"), 0);

    quadShader.use();
    glUniform1i(glGetUniformLocation(geometryPassShader.ID, "image"), 0);

    //----lighting positions------
    const int LIGHT_AMT = 32;
    vector<glm::vec3> lightPositions;
    vector<glm::vec3> lightColors;

    glm::vec3 lightPos = glm::vec3(2.0, 4.0, -2.0);
    glm::vec3 lightColor = glm::vec3(0.2, 0.2, 0.7);

    //-------Hemispheres for SSAO-----
    uniform_real_distribution<float> randomFloats(0.0, 1.0);
    default_random_engine generator;
    vector<glm::vec3> ssaoKernal;

    for (int i = 0; i < 64; i++)
    {
        glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator)); // to save costs, we sample from tangent space, hence z is not in NDC

        float scale = (float)i / 64.0;
        scale = lerp(0.1f, 1.0f, scale * scale);

        sample = glm::normalize(sample);   // normalize becuase normals are direction vectors
        sample *= randomFloats(generator); // apply some random noise
        sample *= scale;                   // place more weight onto fragments that are closer to current fragment
        ssaoKernal.push_back(sample);      // push back
    }

    vector<glm::vec3> ssaoNoise;

    for (int i = 0; i < 16; i++)
    {
        glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f);
        ssaoNoise.push_back(noise);
    }

    unsigned int ssaoNoiseTex;

    glGenTextures(1, &ssaoNoiseTex);
    glBindTexture(GL_TEXTURE_2D, ssaoNoiseTex);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, 0);
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

        // 1. Render the scene and store the position, normals, albedo and specular into 3 textures
        glBindFramebuffer(GL_FRAMEBUFFER, gFBO);
        glClearColor(0.0, 0.0, 0.0, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        viewMat = camera.GetViewMatrix();
        projectionMat = glm::perspective(glm::radians(camera.Zoom), 800.0f / 800.0f, 0.1f, 1000.0f);

        geometryPassShader.use();
        glUniformMatrix4fv(glGetUniformLocation(geometryPassShader.ID, "viewMat"), 1, GL_FALSE, glm::value_ptr(viewMat));
        glUniformMatrix4fv(glGetUniformLocation(geometryPassShader.ID, "projectionMat"), 1, GL_FALSE, glm::value_ptr(projectionMat));

        // render scene objects
        renderScene(geometryPassShader, floorDiffuse, floorSpecular);

        // with scene data in gFBO buffers, we wanna start doing ssao
        glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
        glClear(GL_COLOR_BUFFER_BIT);
        ssaoShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, colorBuffers[1]);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, ssaoNoiseTex); // our funny noise

        glUniformMatrix4fv(glGetUniformLocation(ssaoShader.ID, "projectionMat"), 1, GL_FALSE, glm::value_ptr(projectionMat));

        for (int i = 0; i < 64; i++)
        {
            glUniform3f(glGetUniformLocation(ssaoShader.ID, ("samples[" + to_string(i) + "]").c_str()), ssaoKernal[i].x, ssaoKernal[i].y, ssaoKernal[i].z);
        }
        renderQuad();

        // now we blur
        glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
        glClear(GL_COLOR_BUFFER_BIT);

        ssaoBlurShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
        renderQuad();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // We then use these 4 textures and light our scene with it!
        lightingPassShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, colorBuffers[1]);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, colorBuffers[2]);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, ssaoBlurColorBuffer);

        // glUniform3f(glGetUniformLocation(lightingPassShader.ID, "viewPos"), camera.Position.x, camera.Position.y, camera.Position.z);
        float constant = 1.0f;
        float linear = 0.7f;
        float quadratic = 1.8f;

        // to help midigate on costs, we can get the largest radius until a fragment is considered unlit. If the distance between light and fragment is less than radius, we light
        const float maxBrightness = std::fmaxf(std::fmaxf(lightColor.r, lightColor.g), lightColor.b);
        float radius = (-linear + std::sqrt(linear * linear - 4 * quadratic * (constant - (256.0f / 5.0f) * maxBrightness))) / (2.0f * quadratic);

        glUniform1f(glGetUniformLocation(lightingPassShader.ID, "light.radius"), radius);
        glUniform3f(glGetUniformLocation(lightingPassShader.ID, "light.position"), lightPos.x, lightPos.y, lightPos.z);
        glUniform3f(glGetUniformLocation(lightingPassShader.ID, "light.color"), lightColor.x, lightColor.y, lightColor.z);

        renderQuad();

        // for seeing individual color buffers
        // glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // quadShader.use();
        // glActiveTexture(GL_TEXTURE0);
        // glBindTexture(GL_TEXTURE_2D, ssaoBlurColorBuffer);
        // renderQuad();

        inputCallback(window);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

float lerp(float a, float b, float f)
{
    return a * (1.0 - f) + (b * f);
}

void renderScene(const Shader &shader, unsigned int texture1, unsigned int texture2)
{

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture2);

    glm::mat4 modelMat = glm::mat4(1.0f);

    modelMat = glm::translate(modelMat, glm::vec3(0.0, 7.0f, 0.0f));
    modelMat = glm::scale(modelMat, glm::vec3(7.5f, 7.5f, 7.5f));
    glUniform1f(glGetUniformLocation(shader.ID, "invertNormals"), 1);
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "modelMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
    renderCube();

    modelMat = glm::mat4(1.0f);
    modelMat = glm::translate(modelMat, glm::vec3(0.0f, -0.5f, 0.0));
    modelMat = glm::scale(modelMat, glm::vec3(40.0f));
    glUniform1f(glGetUniformLocation(shader.ID, "invertNormals"), 0);
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "modelMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
    renderDragon();
}

// renders a 1x1 quad in NDC
// -------------------------
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

unsigned int dragonVAO = 0, dragonVBO = 0;
vector<VertexObj> dragon = loadObjFile("../resources/models/misc/dragon.obj");

void renderDragon()
{
    if (dragonVAO == 0)
    {
        glGenVertexArrays(1, &dragonVAO);
        glGenBuffers(1, &dragonVBO);

        glBindVertexArray(dragonVAO);

        glBindBuffer(GL_ARRAY_BUFFER, dragonVBO);

        glBufferData(GL_ARRAY_BUFFER, dragon.size() * sizeof(VertexObj), dragon.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexObj), (void *)offsetof(VertexObj, position));
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexObj), (void *)offsetof(VertexObj, normalCoord));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexObj), (void *)offsetof(VertexObj, texCoord));
        glEnableVertexAttribArray(2);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    glBindVertexArray(dragonVAO);
    glDrawArrays(GL_TRIANGLES, 0, dragon.size());
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