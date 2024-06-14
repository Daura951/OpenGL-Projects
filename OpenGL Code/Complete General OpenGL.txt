#define GLM_ENABLE_EXPERIMENTAL

#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<iostream>
#include<math.h>
#include<stb_image.h>
#include<glm/glm.hpp>
#include<glm/gtx/transform.hpp>
#include<glm/gtc/type_ptr.hpp>

#include<Shaders/Shader.h>
#include<Camera/camera.h>

using namespace std;

void frameBufferSizeCallback(GLFWwindow* window, int width, int height);
void inputCallback(GLFWwindow* window);
void mouseCallback(GLFWwindow* window, double xPos, double yPos);
void scrollWheelCallback(GLFWwindow* window, double xOffset, double yOffset);
unsigned int loadTexture2D(const char* filePath);

float vertices[] = {
    // Front face
    -0.5f, -0.5f,  0.5f,   0.0f, 0.0f,   0.0f, 0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,   1.0f, 0.0f,   0.0f, 0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,   1.0f, 1.0f,   0.0f, 0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,   1.0f, 1.0f,   0.0f, 0.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,   0.0f, 1.0f,   0.0f, 0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,   0.0f, 0.0f,   0.0f, 0.0f, 1.0f,

    // Back face
    -0.5f, -0.5f, -0.5f,   0.0f, 0.0f,   0.0f, 0.0f, -1.0f,
     0.5f, -0.5f, -0.5f,   1.0f, 0.0f,   0.0f, 0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,   1.0f, 1.0f,   0.0f, 0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,   1.0f, 1.0f,   0.0f, 0.0f, -1.0f,
    -0.5f,  0.5f, -0.5f,   0.0f, 1.0f,   0.0f, 0.0f, -1.0f,
    -0.5f, -0.5f, -0.5f,   0.0f, 0.0f,   0.0f, 0.0f, -1.0f,

    // Left face
    -0.5f,  0.5f,  0.5f,   1.0f, 0.0f,   -1.0f, 0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,   1.0f, 1.0f,   -1.0f, 0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,   0.0f, 1.0f,   -1.0f, 0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,   0.0f, 1.0f,   -1.0f, 0.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,   0.0f, 0.0f,   -1.0f, 0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,   1.0f, 0.0f,   -1.0f, 0.0f, 0.0f,

    // Right face
     0.5f,  0.5f,  0.5f,   0.0f, 0.0f,   1.0f, 0.0f, 0.0f,
     0.5f,  0.5f, -0.5f,   1.0f, 0.0f,   1.0f, 0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,   1.0f, 1.0f,   1.0f, 0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,   1.0f, 1.0f,   1.0f, 0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,   0.0f, 1.0f,   1.0f, 0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,   0.0f, 0.0f,   1.0f, 0.0f, 0.0f,

    // Top face
    -0.5f,  0.5f, -0.5f,   0.0f, 1.0f,   0.0f, 1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,   1.0f, 1.0f,   0.0f, 1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,   1.0f, 0.0f,   0.0f, 1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,   1.0f, 0.0f,   0.0f, 1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,    0.0f, 0.0f,  0.0f, 1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,   0.0f, 1.0f,   0.0f, 1.0f, 0.0f,

    // Bottom face
    -0.5f, -0.5f, -0.5f,   0.0f, 0.0f,   0.0f, -1.0f, 0.0f,
     0.5f, -0.5f, -0.5f,   1.0f, 0.0f,   0.0f, -1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,   1.0f, 1.0f,   0.0f, -1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,   1.0f, 1.0f,   0.0f, -1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,   0.0f, 1.0f,   0.0f, -1.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,   0.0f, 0.0f,   0.0f, -1.0f, 0.0f
};
unsigned int indices[]  = {
    0,1,3,
    1,2,3
};

glm::vec3 cubePositions[] = {
    glm::vec3( 0.0f,  0.0f,  0.0f), 
    glm::vec3( 2.0f,  5.0f, -15.0f), 
    glm::vec3(-1.5f, -2.2f, -2.5f),  
    glm::vec3(-3.8f, -2.0f, -12.3f),  
    glm::vec3( 2.4f, -0.4f, -3.5f),  
    glm::vec3(-1.7f,  3.0f, -7.5f),  
    glm::vec3( 1.3f, -2.0f, -2.5f),  
    glm::vec3( 1.5f,  2.0f, -2.5f), 
    glm::vec3( 1.5f,  0.2f, -1.5f), 
    glm::vec3(-1.3f,  1.0f, -1.5f)  
};

glm::vec3 pointLightPositions[] = {
    glm::vec3( 0.7f,  0.2f,  2.0f),
    glm::vec3( 2.3f, -3.3f, -4.0f),
    glm::vec3(-4.0f,  2.0f, -12.0f),
    glm::vec3( 0.0f,  0.0f, -3.0f)
};

float deltaTime = 0.0f, lastFrame = 0.0f;
float lastMouseX = 400, lastMouseY = 400;

Camera camera(glm::vec3(0,0,-3));

bool firstMouse = true;

int main() 
{

    //----------------------Initialize GLFW and GLAD--------------------
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow*  window = glfwCreateWindow(800,800, "Hello Window!", NULL, NULL);

    if(window == NULL) {
        cout<<"Failed to create window!"<<endl;
        return -1;
    }

    glfwMakeContextCurrent(window);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout<<"Failed to start GLAD"<<endl;;
        return -1;
    }

    glViewport(0,0, 800, 800);
    glfwSetFramebufferSizeCallback(window, frameBufferSizeCallback);
    
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouseCallback);

    glfwSetScrollCallback(window, scrollWheelCallback);


    //----------------Create and Bind VAO, VBO, and EBO----------------------

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));
    //glEnableVertexAttribArray(1);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(5*sizeof(float)));
    glEnableVertexAttribArray(2);

    unsigned int lightVAO;
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    
    //-----------Create and compile shaders---------------------------
   
   
    Shader shaderProgram("../resources/shaders/vertex.vs", "../resources/shaders/fragment.fs");
    Shader lightShader("../resources/shaders/lightvert.vs", "../resources/shaders/lightfrag.fs");
    
    
    //---------------------Texture Loading--------------

  
        unsigned int diffuse = loadTexture2D("../resources/textures/boxdiff.png");
        unsigned int specular = loadTexture2D("../resources/textures/boxspec.png");



    glUniform1i(glGetUniformLocation(shaderProgram.ID, "material.diffuse"), 0);
    glUniform1i(glGetUniformLocation(shaderProgram.ID, "material.specular"), 1);

    glEnable(GL_DEPTH_TEST);


    //----------------Render Loop-----------------------

    while(!glfwWindowShouldClose(window)) {

        float curFrame = glfwGetTime();
        deltaTime = curFrame - lastFrame;
        lastFrame = curFrame;

        glm::mat4 modelMat = glm::mat4(1.0f);
        glm::mat4 viewMat = glm::mat4(1.0f);
        glm::mat4 projectionMat = glm::mat4(1.0f);

        glm::mat4 lightModelMat = glm::mat4(1.0f);




        glClearColor(0.1, 0.1, 0.1, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shaderProgram.use();

        glUniform3f(glGetUniformLocation(shaderProgram.ID, "camPos"), camera.Position.x, camera.Position.y, camera.Position.z);
        glUniform1f(glGetUniformLocation(shaderProgram.ID, "material.shininess"), 32);

        glUniform3f(glGetUniformLocation(shaderProgram.ID, "dirLight.direction"), -0.2f, -1.0f, -0.3f);
        glUniform3f(glGetUniformLocation(shaderProgram.ID, "dirLight.ambient"), 0.05f, 0.05f, 0.05f);
        glUniform3f(glGetUniformLocation(shaderProgram.ID, "dirLight.diffuse"), 0.4f, 0.4f, 0.4f);
        glUniform3f(glGetUniformLocation(shaderProgram.ID, "dirLight.specular"), 0.5f, 0.5f, 0.5f);

        for(int i = 0; i < (sizeof(pointLightPositions) / sizeof(glm::vec3)); i++)
        {
            string uniformHeader = "pointLights["+to_string(i)+"]";
            glUniform3f(glGetUniformLocation(shaderProgram.ID, (uniformHeader+".position").c_str()), pointLightPositions[i].x, pointLightPositions[i].y, pointLightPositions[i].z);
            glUniform3f(glGetUniformLocation(shaderProgram.ID, (uniformHeader+".ambient").c_str()), 0.05f, 0.05f, 0.05f);
            glUniform3f(glGetUniformLocation(shaderProgram.ID, (uniformHeader+".diffuse").c_str()), 0.8f, 0.8f, 0.8f);
            glUniform3f(glGetUniformLocation(shaderProgram.ID, (uniformHeader+".specular").c_str()), 1.0f, 1.0f, 1.0f);
            glUniform1f(glGetUniformLocation(shaderProgram.ID, (uniformHeader+".constant").c_str()), 1.0f);
            glUniform1f(glGetUniformLocation(shaderProgram.ID, (uniformHeader+".linear").c_str()), 0.09f);
            glUniform1f(glGetUniformLocation(shaderProgram.ID, (uniformHeader+".quadratic").c_str()), 0.032f);
        }

    
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuse);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specular);

        glBindVertexArray(VAO);
        for(int i = 0; i < (sizeof(cubePositions) / sizeof(glm::vec3)); i++) {
            modelMat = glm::mat4(1.0f);
            modelMat = glm::translate(modelMat, cubePositions[i]);
            modelMat = glm::rotate(modelMat, glm::radians(20.0f*i), glm::vec3(1.0f, .3f, .5f));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "modelMat"), 1,GL_FALSE, glm::value_ptr(modelMat));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }


        viewMat = camera.GetViewMatrix();
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "viewMat"), 1,GL_FALSE, glm::value_ptr(viewMat));

        projectionMat = glm::perspective(glm::radians(camera.Zoom), 800.0f/800.0f, 0.1f, 100.0f);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "projectionMat"), 1,GL_FALSE, glm::value_ptr(projectionMat));

        //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


        lightShader.use();


        glBindVertexArray(lightVAO);
        for(int i = 0; i < (sizeof(pointLightPositions) / sizeof(glm::vec3)); i++) {
            lightModelMat = glm::mat4(1.0f);
            lightModelMat = glm::translate(lightModelMat, pointLightPositions[i]);
            lightModelMat = glm::scale(lightModelMat, glm::vec3(0.2f));
            glUniformMatrix4fv(glGetUniformLocation(lightShader.ID, "modelMat"), 1,GL_FALSE, glm::value_ptr(lightModelMat));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glUniformMatrix4fv(glGetUniformLocation(lightShader.ID, "viewMat"), 1,GL_FALSE, glm::value_ptr(viewMat));
        glUniformMatrix4fv(glGetUniformLocation(lightShader.ID, "projectionMat"), 1,GL_FALSE, glm::value_ptr(projectionMat));
        
        
        
        
        inputCallback(window);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    //--------Clear everything out-----------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram.ID);

    glfwTerminate();
    return 0;
}

void frameBufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0,0, width, height);
}

void inputCallback(GLFWwindow* window) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    const float cameraSpeed= 2.5f * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera.ProcessKeyboard(FORWARD, deltaTime);
    }
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera.ProcessKeyboard(LEFT, deltaTime);
    }
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    }
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera.ProcessKeyboard(RIGHT, deltaTime);
    }

}

void mouseCallback(GLFWwindow* window, double xPos, double yPos) {
    
    if(firstMouse) {
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

void scrollWheelCallback(GLFWwindow* window, double xOffset, double yOffset) {
    camera.ProcessMouseScroll(yOffset);
}


unsigned int loadTexture2D(char const * filePath)
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