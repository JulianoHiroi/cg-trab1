#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>
#include <iostream>
#include <string>
#include <limits>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <glm/gtx/string_cast.hpp>
#include "../lib/utils.h"
 

// Variáveis globais 
 int win_width  = 600;
 int win_height = 600;
 int program;
 unsigned int VAO;
 unsigned int VBO;

 struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
};
std::vector<Vertex> vertices; // Substituir o vetor de float por um vetor de Vertex
std::string modelPath = "models/cube.obj"; // Caminho do modelo

float angle = 0.0f;
float fov = 60.0f;
bool usePerspective = true;


const char* vertex_code =
"#version 330 core\n"
"layout (location = 0) in vec3 position;\n"
"layout (location = 1) in vec3 normal;\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"out vec3 Normal;\n"
"void main()\n"
"{\n"
"    gl_Position = projection * view * model * vec4(position, 1.0);\n"
"    Normal = mat3(transpose(inverse(model))) * normal;\n"
"}\n";
 
 /** Fragment shader. */
 const char* fragment_code =
 "#version 330 core\n"
 "in vec3 Normal;\n"
 "out vec4 FragColor;\n"
 "void main()\n"
 "{\n"
 "    // Usa o componente Y da normal para determinar a cor\n"
 "    float intensity = (Normal.y + 1.0) * 0.5; // Mapeia de [-1,1] para [0,1]\n"
 "    vec3 color = mix(vec3(0.2, 0.4, 0.8), vec3(0.8, 0.4, 0.2), intensity);\n"
 "    FragColor = vec4(color, 1.0);\n"
 "}\n";
 
 
 /* Functions. */
 void display(void);
 void reshape(int, int);
 void keyboard(unsigned char, int, int);
 void initData(void);
 void initShaders(void);


 void display()
 {
	glClearColor(0.2, 0.3, 0.3, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(program);
	glBindVertexArray(VAO);
 
    // Define model matrix.
     glm::mat4 Rx = glm::rotate(glm::mat4(1.0f), glm::radians(30.0f), glm::vec3(1.0f,1.0f,0.0f));
     glm::mat4 T  = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f,-0.5f,0.0f));
     glm::mat4 model = T*Rx;
 
     unsigned int loc = glGetUniformLocation(program, "model");
	// Send matrix to shader.
     glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(model));
     
 
     glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
     glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
     glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
     glm::mat4 view = glm::lookAt(cameraPos, target, up);
 
     loc = glGetUniformLocation(program, "view");
        // Send matrix to shader.
     glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(view));



    // Define projection matrix.
     glm::mat4 projection;
     if(usePerspective)
         projection = glm::perspective(glm::radians(fov), (win_width/(float)win_height), 0.1f, 10.0f);
     else
         projection = glm::ortho(-1.5f, 1.5f, -1.5f, 1.5f, 0.1f, 100.0f);
         

	
    loc = glGetUniformLocation(program, "projection");
    // Send matrix to shader.
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(projection));
 
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());
 
    glutSwapBuffers();
 }
 
 void reshape(int width, int height)
 {
     win_width = width;
     win_height = height;
     glViewport(0, 0, width, height);
     glutPostRedisplay();
 }
 
 
 void keyboard(unsigned char key, int x, int y)
 {
         switch (key)
         {
                 case 27:
                         glutLeaveMainLoop();
                 case 'q':
                 case 'Q':
                         glutLeaveMainLoop();
                 case 'p':
                 case 'P':
                     usePerspective = !usePerspective;
                     break;
                 case 'w':
                     if (usePerspective) fov = glm::min(fov + 5.0f, 120.0f);
                     break;
                 case 's':
                     if (usePerspective) fov = glm::max(fov - 5.0f, 10.0f);
                     break;
         }
     
     glutPostRedisplay();
 }
 
 void loadModelMesh(const char* path)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, 
        aiProcess_Triangulate | 
        aiProcess_GenNormals | 
        aiProcess_JoinIdenticalVertices |
        aiProcess_PreTransformVertices);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "Erro ao carregar modelo: " << importer.GetErrorString() << std::endl;
        return;
    }

    vertices.clear();
    
    // Processa todos os meshes da cena
    for (unsigned int m = 0; m < scene->mNumMeshes; m++) {
        aiMesh* mesh = scene->mMeshes[m];
        
        std::cout << "Processando mesh " << m << " com " << mesh->mNumFaces << " faces" << std::endl;
        
        // Processa todas as faces do mesh atual
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
            {
                unsigned int index = face.mIndices[j];
                Vertex vertex;
                vertex.position = glm::vec3(
                    mesh->mVertices[index].x,
                    mesh->mVertices[index].y,
                    mesh->mVertices[index].z
                );
                vertex.normal = glm::vec3(
                    mesh->mNormals[index].x,
                    mesh->mNormals[index].y,
                    mesh->mNormals[index].z
                );
                vertices.push_back(vertex);
            }
        }
    }
    std::cout << "Total de vértices carregados: " << vertices.size() << std::endl;
}
 
 void initData()
 {
    loadModelMesh(modelPath.c_str());
     
     // Vertex array.
     glGenVertexArrays(1, &VAO);
     glBindVertexArray(VAO);
 
    // Vertex buffer
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
     
    // Set attributes.
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

     // Unbind Vertex Array Object.
     glBindVertexArray(0);
     
     glEnable(GL_DEPTH_TEST);
 }
 

 void initShaders()
 {
     program = createShaderProgram(vertex_code, fragment_code);
 }
 
 int main(int argc, char** argv)
 {
     glutInit(&argc, argv);
     glutInitContextVersion(3, 3);
     glutInitContextProfile(GLUT_CORE_PROFILE);
     glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
     glutInitWindowSize(win_width,win_height);
     glutCreateWindow(argv[0]);
     glewInit();
 
         // Init vertex data for the triangle.
         initData();
     
         // Create shaders.
         initShaders();
     
         glutReshapeFunc(reshape);
         glutDisplayFunc(display);
         glutKeyboardFunc(keyboard);
 
     glutMainLoop();
 }
 