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
void renderQuad();

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;
float heightScale = 0.1f;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
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

    //-----------Create and compile shaders-------------------------------
    Shader shader("../resources/shaders/parallax_mapping/vertex.vs", "../resources/shaders/parallax_mapping/fragment.fs");

    //---------------------Texture Loading--------------------------------

    unsigned int diffuseMap = loadTexture2D("../resources/textures/toybox/wood.png");
    unsigned int normalMap = loadTexture2D("../resources/textures/toybox/toy_box_normal.png");
    unsigned int heightMap = loadTexture2D("../resources/textures/toybox/toy_box_disp.png");

    shader.use();
    glUniform1i(glGetUniformLocation(shader.ID, "diffuseTex"), 0);
    glUniform1i(glGetUniformLocation(shader.ID, "normalMap"), 1);
    glUniform1i(glGetUniformLocation(shader.ID, "displacementMap"), 2);

    // -------------lighting info-------------
    glm::vec3 lightPos(0.5f, 1.0f, 0.3f);

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

        glClearColor(0.1, 0.1, 0.1, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        viewMat = camera.GetViewMatrix();
        projectionMat = glm::perspective(glm::radians(camera.Zoom), 800.0f / 800.0f, 0.1f, 1000.0f);
        modelMat = glm::rotate(modelMat, glm::radians((float)glfwGetTime() * -10.0f), glm::normalize(glm::vec3(1.0f, 0.0f, 1.0f)));

        shader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, normalMap);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, heightMap);

        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "viewMat"), 1, GL_FALSE, glm::value_ptr(viewMat));
        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projectionMat"), 1, GL_FALSE, glm::value_ptr(projectionMat));
        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "modelMat"), 1, GL_FALSE, glm::value_ptr(modelMat));

        glUniform3f(glGetUniformLocation(shader.ID, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
        glUniform3f(glGetUniformLocation(shader.ID, "viewPos"), camera.Position.x, camera.Position.y, camera.Position.z);
        glUniform1f(glGetUniformLocation(shader.ID, "heightScale"), heightScale);

        renderQuad();

        // just so I can see where light is coming from
        modelMat = glm::mat4(1.0f);
        modelMat = glm::translate(modelMat, lightPos);
        modelMat = glm::scale(modelMat, glm::vec3(0.1f));
        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "modelMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
        renderQuad();

        inputCallback(window);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

// renders a 1x1 quad in NDC with manually calculated tangent vectors
// ------------------------------------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        // positions
        glm::vec3 pos1(-1.0, 1.0, 0.0);
        glm::vec3 pos2(-1.0, -1.0, 0.0);
        glm::vec3 pos3(1.0, -1.0, 0.0);
        glm::vec3 pos4(1.0, 1.0, 0.0);
        // texture coordinates
        glm::vec2 uv1(0.0, 1.0);
        glm::vec2 uv2(0.0, 0.0);
        glm::vec2 uv3(1.0, 0.0);
        glm::vec2 uv4(1.0, 1.0);
        // normal vector
        glm::vec3 nm(0.0, 0.0, 1.0);

        glm::vec3 tangent1, tangent2;
        glm::vec3 biTangent1, biTangent2;

        glm::vec3 edge1 = pos2 - pos1;
        glm::vec3 edge2 = pos3 - pos1;
        glm::vec2 deltaUV1 = uv2 - uv1;
        glm::vec2 deltaUV2 = uv3 - uv1;

        /* Stuff relating to normal mapping and parallax mapping is in what's known as tangent space,
            a coordinate system that is tangent to the surface of the model. In tangent space, the normal map
            can represent surface details such as bumps and dents, where all the normals and heights are pointing up
            (hence why a normal map is blue, because of the positive Z component).

            In our shaders, we need to transform normals and other vectors from model space into and out of tangent space.
            To achieve this, we need the tangent and bitangent vectors for each triangle. These vectors are calculated
            using the positions and texture coordinates of the vertices. With these vectors, we can create a matrix
            that transforms coordinates between model space and tangent space.
        */

        float factor = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent1.x = factor * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent1.y = factor * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent1.z = factor * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.z);

        biTangent1.x = factor * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        biTangent1.y = factor * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        biTangent1.z = factor * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

        edge1 = pos3 - pos1;
        edge2 = pos4 - pos1;
        deltaUV1 = uv3 - uv1;
        deltaUV2 = uv4 - uv1;

        factor = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent2.x = factor * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent2.y = factor * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent2.z = factor * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.z);

        biTangent2.x = factor * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        biTangent2.y = factor * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        biTangent2.z = factor * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

        float quadVertices[] = {
            // positions            // normal         // texcoords  // tangent                          // bitangent
            pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, biTangent1.x, biTangent1.y, biTangent1.z,
            pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, biTangent1.x, biTangent1.y, biTangent1.z,
            pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, biTangent1.x, biTangent1.y, biTangent1.z,

            pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, biTangent2.x, biTangent2.y, biTangent2.z,
            pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, biTangent2.x, biTangent2.y, biTangent2.z,
            pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, biTangent2.x, biTangent2.y, biTangent2.z};

        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);

        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);

        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void *)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void *)(8 * sizeof(float)));
        glEnableVertexAttribArray(3);

        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void *)(11 * sizeof(float)));
        glEnableVertexAttribArray(4);
    }

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
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
        if (heightScale > 0.0f)
            heightScale -= 0.0005f;
        else
            heightScale = 0.0f;
    }
    else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        if (heightScale < 1.0f)
            heightScale += 0.0005f;
        else
            heightScale = 1.0f;
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