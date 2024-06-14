#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include <algorithm>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <objLoader/VertexObj.h>

using namespace std;

static vector<VertexObj> loadObjFile(const char *filePath)
{
    // vertex attribute vectors
    vector<glm::vec3> vertexPositions;
    vector<glm::vec2> texCoords;
    vector<glm::vec3> normalCoords;

    // face vectors
    vector<GLint> vertexPositionIndices;
    vector<GLint> texCoordIndices;
    vector<GLint> normalIndices;

    // vertex array
    vector<VertexObj> vertices;

    stringstream strStream;

    ifstream file(filePath);
    string line = "";
    string prefix = "";
    glm::vec3 tempV3;
    glm::vec2 tempV2;
    GLint tempGlint;

    if (!file.is_open())
    {
        throw "ERROR:: COULD NOT OPEN OBJ FILE!";
    }

    // read 1 line at time
    while (getline(file, line))
    {
        // get prefix
        strStream.clear();
        strStream.str(line);
        strStream >> prefix;

        if (prefix == "#" || prefix == "o" || prefix == "s" || prefix == "use_mtl")
        {
        }

        // position
        else if (prefix == "v")
        {
            strStream >> tempV3.x >> tempV3.y >> tempV3.z;
            vertexPositions.push_back(tempV3);
        }

        // texCoord
        else if (prefix == "vt")
        {
            strStream >> tempV2.x >> tempV2.y;
            texCoords.push_back(tempV2);
        }

        // normalCoord
        else if (prefix == "vn")
        {
            strStream >> tempV3.x >> tempV3.y >> tempV3.z;
            normalCoords.push_back(tempV3);
        }
        else if (prefix == "f")
        {
            int counter = 0;
            while (strStream >> tempGlint)
            {
                if (counter == 0)
                {
                    vertexPositionIndices.push_back(tempGlint);
                }

                else if (counter == 1)
                {
                    texCoordIndices.push_back(tempGlint);
                }

                else if (counter == 2)
                {
                    normalIndices.push_back(tempGlint);
                }

                if (strStream.peek() == '/')
                {
                    counter++;
                    strStream.ignore(1, '/');
                }
                else if (strStream.peek() == ' ')
                {
                    counter++;
                    strStream.ignore(1, ' ');
                }

                if (counter > 2)
                {
                    counter = 0;
                }
            }
        }
        else
        {
        }
        // Build final vertex array!
        vertices.resize(vertexPositionIndices.size(), VertexObj());

        for (size_t i = 0; i < vertices.size(); i++)
        {
            vertices[i].position = vertexPositions[vertexPositionIndices[i] - 1];
            if (texCoords.size() > 0)
            {
                vertices[i].texCoord = texCoords[texCoordIndices[i] - 1];
            }
            if (normalCoords.size() > 0)
            {
                vertices[i].normalCoord = normalCoords[normalIndices[i] - 1];
            }
        }

        // DEBUG
    }
    // successfully loaded!
    cout << "Loaded OBJ File!!" << endl;
    return vertices;
}