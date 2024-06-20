#define GLM_ENABLE_EXPERIMENTAL

#include gladglad.h
#include GLFWglfw3.h
#include iostream
#include math.h
#include stb_image.h
#include glmglm.hpp
#include glmgtxtransform.hpp
#include glmgtctype_ptr.hpp

#include ShadersShader.h
#include Cameracamera.h

#include vector
#include map

using namespace std;

void frameBufferSizeCallback(GLFWwindow window, int width, int height);
void inputCallback(GLFWwindow window);
void mouseCallback(GLFWwindow window, double xPos, double yPos);
void scrollWheelCallback(GLFWwindow window, double xOffset, double yOffset);
unsigned int loadTexture2D(const char filePath);
unsigned int loadCubeMap(vectorstring cubeMapFilePaths);

float cubeVertices[] = {
     positions           texture Coords
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
    0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
    -0.5f, 0.5f, 0.5f, 0.0f, 1.0f,
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,

    -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
    -0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
    -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

    0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
    0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
    0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
    0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
    0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,

    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
    -0.5f, 0.5f, 0.5f, 0.0f, 0.0f,
    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f};

 float vertices[] = {
     -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
     0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
     0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
     0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
     -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
     -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,

     -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
     0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
     0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
     0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
     -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
     -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,

     -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
     -0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
     -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
     -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
     -0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
     -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,

     0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
     0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
     0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
     0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
     0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
     0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,

     -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,
     0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,
     0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
     0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
     -0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
     -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,

     -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
     0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
     0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
     0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
     -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
     -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f};

float skyboxVertices[] = {
     positions
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

float deltaTime = 0.0f, lastFrame = 0.0f;
float lastMouseX = 400, lastMouseY = 400;

Camera camera(glmvec3(0, 0, -3));

bool firstMouse = true;

int main()
{

    ----------------------Initialize GLFW and GLAD--------------------
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow window = glfwCreateWindow(800, 800, Hello Window!, NULL, NULL);

    if (window == NULL)
    {
        cout  Failed to create window!  endl;
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        cout  Failed to start GLAD  endl;
        return -1;
    }

    glViewport(0, 0, 800, 800);
    glfwSetFramebufferSizeCallback(window, frameBufferSizeCallback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouseCallback);

    glfwSetScrollCallback(window, scrollWheelCallback);

    ----------------Create and Bind VAOs, and VBOs----------------------

     Cube
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5  sizeof(float), (void )0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5  sizeof(float), (void )(3  sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

     skybox
    unsigned int skyboxVAO, skyboxVBO;

    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);

    glBindVertexArray(skyboxVAO);

    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3  sizeof(float), (void )0);
    glEnableVertexAttribArray(0);

    -----------Create and compile shaders---------------------------

    Shader redShader(..resourcesshadersadvanced_GLSLvertex.vs, ..resourcesshadersadvanced_GLSLRed_fragment.fs);
    Shader greenShader(..resourcesshadersadvanced_GLSLvertex.vs, ..resourcesshadersadvanced_GLSLGreen_fragment.fs);
    Shader blueShader(..resourcesshadersadvanced_GLSLvertex.vs, ..resourcesshadersadvanced_GLSLBlue_fragment.fs);
    Shader yellowShader(..resourcesshadersadvanced_GLSLvertex.vs, ..resourcesshadersadvanced_GLSLYellow_fragment.fs);
    Shader skyboxProgram(..resourcesshaderscubemapSB_vertex.vs, ..resourcesshaderscubemapSB_fragment.fs);

    -------------------------Uniform Buffer Objects-----------

    unsigned int redUBO = glGetUniformBlockIndex(redShader.ID, Matricies);  find the locations of the block buffer
    unsigned int greenUBO = glGetUniformBlockIndex(greenShader.ID, Matricies);
    unsigned int blueUBO = glGetUniformBlockIndex(blueShader.ID, Matricies);
    unsigned int yellowUBO = glGetUniformBlockIndex(yellowShader.ID, Matricies);

    glUniformBlockBinding(redShader.ID, redUBO, 0);  bind it to be on binding point 0
    glUniformBlockBinding(greenShader.ID, greenUBO, 0);
    glUniformBlockBinding(blueShader.ID, blueUBO, 0);
    glUniformBlockBinding(yellowShader.ID, yellowUBO, 0);

    unsigned int matrixUBO;

    glGenBuffers(1, &matrixUBO);  usual song and dance for generating a buffer
    glBindBuffer(GL_UNIFORM_BUFFER, matrixUBO);
    glBufferData(GL_UNIFORM_BUFFER, 2  sizeof(glmmat4), NULL, GL_STATIC_DRAW);  We allocate 2size of a mat4 for proj and view matrix
    glBindBuffer(GL_UNIFORM_BUFFER, 0);                                            unbind

    glBindBufferRange(GL_UNIFORM_BUFFER, 0, matrixUBO, 0, 2  sizeof(glmmat4));  We then need to bind the actual buffer to the binding point

    ---------------------Texture Loading--------------

     cubes
    unsigned int cubeDiffuse = loadTexture2D(..resourcestexturescrate.png);

    redShader.use();
    glUniform1i(glGetUniformLocation(redShader.ID, texture1), 0);  0 as in GL_TEXTURE0
    greenShader.use();
    glUniform1i(glGetUniformLocation(greenShader.ID, texture1), 0);  0 as in GL_TEXTURE0
    blueShader.use();
    glUniform1i(glGetUniformLocation(blueShader.ID, texture1), 0);  0 as in GL_TEXTURE0
    yellowShader.use();
    glUniform1i(glGetUniformLocation(yellowShader.ID, texture1), 0);  0 as in GL_TEXTURE0

     skybox, notice that we use a cubemap!
    vectorstring cubeMapTexturePaths{
        ..resourcestexturesskyboxright.jpg,
        ..resourcestexturesskyboxleft.jpg,
        ..resourcestexturesskyboxtop.jpg,
        ..resourcestexturesskyboxbottom.jpg,
        ..resourcestexturesskyboxfront.jpg,
        ..resourcestexturesskyboxback.jpg};

    unsigned int cubeMapTex = loadCubeMap(cubeMapTexturePaths);

    skyboxProgram.use();
    glUniform1i(glGetUniformLocation(skyboxProgram.ID, skybox), 0);

    glEnable(GL_DEPTH_TEST);

    ----------------------Pre render loop uniform bindings-------------------------
    glmmat4 projectionMat = glmperspective(glmradians(45.0f), 800.0f  800.0f, 0.1f, 100.0f);  we can be faster if we render projection once
    glBindBuffer(GL_UNIFORM_BUFFER, matrixUBO);                                                      bind to UBO
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glmmat4), glmvalue_ptr(projectionMat));         With 0 offset put projection matrix into buffer
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    ----------------Render Loop-----------------------

    while (!glfwWindowShouldClose(window))
    {
        float curFrame = glfwGetTime();
        deltaTime = curFrame - lastFrame;
        lastFrame = curFrame;

        glmmat4 modelMat = glmmat4(1.0f);
        glmmat4 viewMat = glmmat4(1.0f);
        glmmat4 projectionMatSB = glmmat4(1.0f);

        glClearColor(0.1, 0.1, 0.1, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT  GL_DEPTH_BUFFER_BIT);

        viewMat = camera.GetViewMatrix();
        glBindBuffer(GL_UNIFORM_BUFFER, matrixUBO);
        glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glmmat4), sizeof(glmmat4), glmvalue_ptr(viewMat));  with offset of size of mat4, put view matrix into buffer
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        projectionMatSB = glmperspective(glmradians(camera.Zoom), 800.0f  800.0f, 0.1f, 100.0f);

        skyboxProgram.use();
        viewMat = glmmat4(glmmat3(camera.GetViewMatrix()));  remove translation from view matrix by eliminating homogeneous coord
        glUniformMatrix4fv(glGetUniformLocation(skyboxProgram.ID, viewMat), 1, GL_FALSE, glmvalue_ptr(viewMat));
        glUniformMatrix4fv(glGetUniformLocation(skyboxProgram.ID, projectionMat), 1, GL_FALSE, glmvalue_ptr(projectionMatSB));

         CUBES
        redShader.use();
        glBindVertexArray(VAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, cubeDiffuse);

        modelMat = glmmat4(1.0f);
        modelMat = glmtranslate(modelMat, glmvec3(-0.75f, 0.75f, 0.0f));
        glUniformMatrix4fv(glGetUniformLocation(redShader.ID, modelMat), 1, GL_FALSE, glmvalue_ptr(modelMat));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        greenShader.use();
        modelMat = glmmat4(1.0f);
        modelMat = glmtranslate(modelMat, glmvec3(0.75f, 0.75f, 0.0f));  move top-right
        glUniformMatrix4fv(glGetUniformLocation(greenShader.ID, modelMat), 1, GL_FALSE, glmvalue_ptr(modelMat));
        glDrawArrays(GL_TRIANGLES, 0, 36);
         YELLOW
        yellowShader.use();
        modelMat = glmmat4(1.0f);
        modelMat = glmtranslate(modelMat, glmvec3(-0.75f, -0.75f, 0.0f));  move bottom-left
        glUniformMatrix4fv(glGetUniformLocation(yellowShader.ID, modelMat), 1, GL_FALSE, glmvalue_ptr(modelMat));
        glDrawArrays(GL_TRIANGLES, 0, 36);
         BLUE
        blueShader.use();
        modelMat = glmmat4(1.0f);
        modelMat = glmtranslate(modelMat, glmvec3(0.75f, -0.75f, 0.0f));  move bottom-right
        glUniformMatrix4fv(glGetUniformLocation(blueShader.ID, modelMat), 1, GL_FALSE, glmvalue_ptr(modelMat));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

         SKYBOX
        glDepthFunc(GL_LEQUAL);
        skyboxProgram.use();
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTex);

        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDepthFunc(GL_LESS);

         glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        inputCallback(window);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    --------Clear everything out-----------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);
    glDeleteProgram(redShader.ID);
    glDeleteProgram(skyboxProgram.ID);

    glfwTerminate();
    return 0;
}

void frameBufferSizeCallback(GLFWwindow window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void inputCallback(GLFWwindow window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    const float cameraSpeed = 2.5f  deltaTime;

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

void mouseCallback(GLFWwindow window, double xPos, double yPos)
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

void scrollWheelCallback(GLFWwindow window, double xOffset, double yOffset)
{
    camera.ProcessMouseScroll(yOffset);
}

unsigned int loadTexture2D(char const filePath)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, numOfChannels;
    unsigned char imageData = stbi_load(filePath, &width, &height, &numOfChannels, 0);
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
        cout  Texture failed to load at file path   filePath  endl;
        stbi_image_free(imageData);
    }

    return textureID;
}

unsigned int loadCubeMap(vectorstring cubeMapFilePaths)
{
    int width, height, amtOfChannels;
    unsigned char imageData;

    unsigned int cubeMapId;
    glGenTextures(1, &cubeMapId);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapId);

    for (int i = 0; i  cubeMapFilePaths.size(); i++)
    {
        imageData = stbi_load(cubeMapFilePaths[i].c_str(), &width, &height, &amtOfChannels, 0);

        if (imageData)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData);

            stbi_image_free(imageData);
        }
        else
        {
            cout  Failed to load texture at file path   cubeMapFilePaths[i];
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