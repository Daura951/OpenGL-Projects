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

float deltaTime = 0.0f, lastFrame = 0.0f;
float lastMouseX = 400, lastMouseY = 400;

Camera camera(glm::vec3(0, 0, 55.0f));

bool firstMouse = true;

float skyboxVertices[] = {
    // positions
    -1.0f, 1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, 1.0f, -1.0f,
    -1.0f, 1.0f, -1.0f,

    -1.0f, -1.0f, 1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f, 1.0f, -1.0f,
    -1.0f, 1.0f, -1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f, -1.0f, 1.0f,

    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, -1.0f, 1.0f,
    -1.0f, -1.0f, 1.0f,

    -1.0f, 1.0f, -1.0f,
    1.0f, 1.0f, -1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f, 1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f, 1.0f,
    1.0f, -1.0f, 1.0f};

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
    vector<VertexObj> planet = loadObjFile("../resources/models/space/planet.obj");

    unsigned int planetVAO, planetVBO;

    glGenVertexArrays(1, &planetVAO);
    glGenBuffers(1, &planetVBO);

    glBindVertexArray(planetVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planetVBO);

    glBufferData(GL_ARRAY_BUFFER, planet.size() * sizeof(VertexObj), planet.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexObj), (void *)offsetof(VertexObj, position));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexObj), (void *)offsetof(VertexObj, texCoord));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VertexObj), (void *)offsetof(VertexObj, normalCoord));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    vector<VertexObj> asteroid = loadObjFile("../resources/models/space/rock.obj");

    int amtOfAsteroids = 3000;
    glm::mat4 *modelMatricies;
    modelMatricies = new glm::mat4[amtOfAsteroids];
    srand(glfwGetTime());
    float radius = 100.0f;
    float offset = 2.5f;

    for (int i = 0; i < amtOfAsteroids; i++)
    {
        if (i % 1000 == 0)
        {
            radius /= 1.5f;
        }

        glm::mat4 modelMat = glm::mat4(1.0f);
        float angle = (float)i / (float)amtOfAsteroids * 360.0f;
        float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float x = sin(angle) * radius + displacement;
        displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float y = displacement * .4f;
        displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float z = cos(angle) * radius + displacement;
        modelMat = glm::translate(modelMat, glm::vec3(x, y, z));

        float scale = (rand() % 20) / 100.0f + 0.05f;
        modelMat = glm::scale(modelMat, glm::vec3(scale));

        float rotAngle = (rand() % 360);
        modelMat = glm::rotate(modelMat, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

        modelMatricies[i] = modelMat;
    }

    unsigned int rockVAO, rockVBO, modelMatVBO;

    glGenVertexArrays(1, &rockVAO);
    glGenBuffers(1, &rockVBO);
    glGenBuffers(1, &modelMatVBO);

    glBindVertexArray(rockVAO);
    glBindBuffer(GL_ARRAY_BUFFER, rockVBO);

    glBufferData(GL_ARRAY_BUFFER, asteroid.size() * sizeof(VertexObj), asteroid.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexObj), (void *)offsetof(VertexObj, position));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexObj), (void *)offsetof(VertexObj, texCoord));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VertexObj), (void *)offsetof(VertexObj, normalCoord));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, modelMatVBO);
    glBufferData(GL_ARRAY_BUFFER, amtOfAsteroids * sizeof(glm::mat4), &modelMatricies[0], GL_STATIC_DRAW);

    // set up the model matricies for the isntanced asteroids. However, glsl only allows things of size vec4 so we have to
    // do this in order to fix a matrix into the vertex attrib array. Since a mat4 is 4 vec4s, we just create
    // 3 additional vertex attributes that sequentially follow in memory
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)0);
    glEnableVertexAttribArray(3);

    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(sizeof(glm::vec4)));
    glEnableVertexAttribArray(4);

    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(2 * sizeof(glm::vec4)));
    glEnableVertexAttribArray(5);

    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(3 * sizeof(glm::vec4)));
    glEnableVertexAttribArray(6);

    // Make it so these attribs are instanced!
    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);
    glVertexAttribDivisor(5, 1);
    glVertexAttribDivisor(6, 1);

    unsigned int skyboxVAO, skyboxVBO;

    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);

    glBindVertexArray(skyboxVAO);

    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);

    // we only need position since the coords of a cubemap function as texCoords
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    //-----------Create and compile shaders-------------------------------

    Shader planetShader("../resources/shaders/depth_test/vertex.vs", "../resources/shaders/depth_test/fragment.fs");
    Shader asteroidShader("../resources/shaders/instancing_scene/vertex.vs", "../resources/shaders/instancing_scene/fragment.fs");
    Shader skyboxShader("../resources/shaders/cubemap/SB_vertex.vs", "../resources/shaders/cubemap/SB_fragment.fs");

    //---------------------Texture Loading--------------
    unsigned int planetDiffuse = loadTexture2D("../resources/textures/space/mars.png");
    unsigned int asteroidDiffuse = loadTexture2D("../resources/textures/space/rock.png");

    planetShader.use();
    glUniform1i(glGetUniformLocation(planetShader.ID, "texture1"), 0);

    asteroidShader.use();
    glUniform1i(glGetUniformLocation(asteroidShader.ID, "texture1"), 0);

    vector<string> cubeMapTexturePaths{
        "../resources/textures/space_skybox/bkg1_right.png",
        "../resources/textures/space_skybox/bkg1_left.png",
        "../resources/textures/space_skybox/bkg1_top.png",
        "../resources/textures/space_skybox/bkg1_bot.png",
        "../resources/textures/space_skybox/bkg1_front.png",
        "../resources/textures/space_skybox/bkg1_back.png"};

    unsigned int cubeMapTex = loadCubeMap(cubeMapTexturePaths);

    skyboxShader.use();
    glUniform1i(glGetUniformLocation(skyboxShader.ID, "skybox"), 0);

    glEnable(GL_DEPTH_TEST);
    //----------------Render Loop-----------------------

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

        // render planet
        planetShader.use();
        glBindVertexArray(planetVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, planetDiffuse);

        viewMat = camera.GetViewMatrix();
        projectionMat = glm::perspective(glm::radians(camera.Zoom), 800.0f / 800.0f, 0.1f, 1000.0f);

        glUniformMatrix4fv(glGetUniformLocation(planetShader.ID, "viewMat"), 1, GL_FALSE, glm::value_ptr(viewMat));
        glUniformMatrix4fv(glGetUniformLocation(planetShader.ID, "projectionMat"), 1, GL_FALSE, glm::value_ptr(projectionMat));

        modelMat = glm::translate(modelMat, glm::vec3(0.0f, -3.0f, 0.0f));
        modelMat = glm::scale(modelMat, glm::vec3(4.0f, 4.0f, 4.0f));
        glUniformMatrix4fv(glGetUniformLocation(planetShader.ID, "modelMat"), 1, GL_FALSE, glm::value_ptr(modelMat));

        glDrawArrays(GL_TRIANGLES, 0, planet.size());

        // render asteroid instaced so that we dont cosntantly make amtOfAsteroid render calls
        asteroidShader.use();
        glUniformMatrix4fv(glGetUniformLocation(asteroidShader.ID, "viewMat"), 1, GL_FALSE, glm::value_ptr(viewMat));
        glUniformMatrix4fv(glGetUniformLocation(asteroidShader.ID, "projectionMat"), 1, GL_FALSE, glm::value_ptr(projectionMat));
        glBindVertexArray(rockVAO);
        glBindTexture(GL_TEXTURE_2D, asteroidDiffuse);
        glDrawArraysInstanced(GL_TRIANGLES, 0, asteroid.size(), amtOfAsteroids);

        // Render skybox, use LEQUAL so that skybox can pass depth test (look in shader to see what we do to Z axis)
        glDepthFunc(GL_LEQUAL);
        skyboxShader.use();
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTex);

        viewMat = glm::mat4(glm::mat3(camera.GetViewMatrix())); // Eliminate homogeneous coord so that translation wont occur
        glUniformMatrix4fv(glGetUniformLocation(skyboxShader.ID, "viewMat"), 1, GL_FALSE, glm::value_ptr(viewMat));
        glUniformMatrix4fv(glGetUniformLocation(skyboxShader.ID, "projectionMat"), 1, GL_FALSE, glm::value_ptr(projectionMat));

        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDepthFunc(GL_LESS);

        // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        inputCallback(window);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    //--------Clear everything out-----------------
    glDeleteProgram(planetShader.ID);

    glfwTerminate();
    return 0;
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