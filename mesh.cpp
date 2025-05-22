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
#include <glm/gtx/quaternion.hpp>  // para glm::toMat4
 

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
std::string modelPath; // Caminho do modelo

// Parâmetros de medidas do model
glm::vec3 center;
glm::vec3 size;
float tamanho_default = 1.0f; // Tamanho padrão do modelo


// Parâmetros de visualização
float fov = 60.0f;
bool visualizationWireframe = false;
float deslocamentoDefault = 0.1f; // Deslocamento padrão do modelo
glm::vec3 position(0.0f, 0.0f, 0.0f);
float escalaModel = 1.0f; // Escala do modelo
float autoEscala;


// Parâmetros TrackBall
float lastX = 0.0f;
float lastY = 0.0f;
float angle = 0.0f;
glm::vec3 axis(0.0f, 0.0f, 1.0f); // Eixo de rotação inicial

// Inicializa a matriz de rotação ROld como identidade
glm::mat4 ROld = glm::mat4(1.0f); // Matriz de rotação inicial


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
 
    // -------------- Define model matrix --------------

	// Translada para o centro de coordenadas (-center) do modelo e -position
	glm::mat4 Tc = glm::translate(glm::mat4(1.0f), glm::vec3(-center.x , -center.y, -center.z)); // Translada para o centro 

	// Faz escala e rotações 

	// Escala o modelo para o tamanho de escala do scroll e escala auto

    glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(escalaModel * autoEscala, escalaModel * autoEscala, escalaModel * autoEscala));


	 // Rotação feita pelo mouse
	 glm::quat quaternionLast = glm::angleAxis(glm::radians(angle), glm::normalize(axis));
	 glm::mat4 RLast = glm::toMat4(quaternionLast);




	// Translada para a posição desejada (position)
	glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, position.z));

	 
    glm::mat4 model = T*RLast*ROld*S*Tc;  
 
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
    projection = glm::perspective(glm::radians(fov), (win_width/(float)win_height), 0.1f, 10.0f);

    loc = glGetUniformLocation(program, "projection");
    // Send matrix to shader.
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(projection));

	// Muda o modo de visualização
	if (visualizationWireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
 
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());
 
    glutSwapBuffers();
 }

 void resetTransform()
 {
	 position = glm::vec3(0.0f, 0.0f, 0.0f);
	 escalaModel = 1.0f;
	 angle = 0.0f;
	 axis = glm::vec3(0.0f, 0.0f, 1.0f);
	 ROld = glm::mat4(1.0f); // Reseta a matriz de rotação acumulada
	 lastX = 0.0f;
	 lastY = 0.0f;
	 visualizationWireframe = false;
 }
 
 void reshape(int width, int height)
 {
     win_width = width;
     win_height = height;
     glViewport(0, 0, width, height);
     glutPostRedisplay();
 }
 void specialKeys (int key, int x, int y)
 {
	 switch (key)
	 {
		 case GLUT_KEY_UP:
			 position.y += deslocamentoDefault;
			 break;
		 case GLUT_KEY_DOWN:
			 position.y -= deslocamentoDefault;
			 break;
		 case GLUT_KEY_LEFT:
			 position.x -= deslocamentoDefault;
			 break;
		 case GLUT_KEY_RIGHT:
			 position.x += deslocamentoDefault;
			 break;
	 }
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
				case 'w':
				case 'W':
					position.z += deslocamentoDefault;
					break;
				case 's':
				case 'S':
					position.z -= deslocamentoDefault;
					break;
				case 'v':
				case 'V':
					visualizationWireframe = !visualizationWireframe;
					break;
				case 'r':
				case 'R':
					resetTransform();
					break;
         }
     
     glutPostRedisplay();
 }
void mouse(int button, int state, int x, int y)
{

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		lastX = x;
		lastY = y;
	}
	if ( button == GLUT_LEFT_BUTTON && state == GLUT_UP)
	{
		// Atualiza a matriz de rotação ROld com a rotação atual
		glm::quat quaternionLast = glm::angleAxis(glm::radians(angle), glm::normalize(axis));
		glm::mat4 RLast = glm::toMat4(quaternionLast);
		ROld = RLast * ROld; // Atualiza a matriz de rotação acumulada

		
		angle = 0.0f; // Reseta o ângulo para evitar acumulação
		lastX = 0.0f;
		lastY = 0.0f;
		glutPostRedisplay();
	}
}
void motion(int x, int y)
{
	if (x != lastX || y != lastY)
	{
		float dx = (x - lastX) / (float)win_width;
		float dy = (y - lastY) / (float)win_height;

		axis = glm::normalize(glm::cross(glm::vec3(-dx, dy, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
		angle = sqrt(dx * dx + dy * dy) * 180.0f; // Ajuste o fator de escala conforme necessário
		
		glutPostRedisplay();
	}
}
void scroll(int button, int dir, int x, int y)
{
	if (dir > 0)
	{
		escalaModel += 0.1f;
	}
	else
	{
		if (escalaModel > 0.1f)
			escalaModel -= 0.1f;
	}
	glutPostRedisplay();
}
float calculateAutoScale(glm::vec3 size, float fovDegrees, float aspectRatio, float fillFactor = 0.8f) {
    // Calcula o raio da bounding box (meia-diagonal)
    float radius = glm::length(size) * 0.5f;

    // Converte FOV para radianos
    float fov = glm::radians(fovDegrees);

    // Calcula a "altura visível" no plano z = -camera.z
    float visibleHeight = 2.0f * radius / fillFactor;

    // A distância ideal da câmera para ver o objeto com o FOV dado
    float distance = visibleHeight / (2.0f * tan(fov / 2.0f));

    // Agora calculamos o fator de escala para colocar o modelo dentro da viewport
    float scale = (fillFactor * distance) / radius;

    return scale;
}

void calculateShapeBounds(const std::vector<Vertex>& vertices)
{
	// Calcular os limites do modelo no espaço 3D
	glm::vec3 minBounds(std::numeric_limits<float>::max());
	glm::vec3 maxBounds(std::numeric_limits<float>::lowest());
	for (const auto& vertex : vertices)
	{
		minBounds = glm::min(minBounds, vertex.position);
		maxBounds = glm::max(maxBounds, vertex.position);
	}
	center = (minBounds + maxBounds) / 2.0f;
	size = maxBounds - minBounds;

    float aspectRatio = win_width / (float)win_height;
    autoEscala = calculateAutoScale(size, fov, aspectRatio);
	
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
    calculateShapeBounds(vertices);
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
 
bool isValidPath(const std::string& path)
{
	// Verifica se o caminho é válido
	FILE* file = fopen(path.c_str(), "r");
	if (file)
	{
		fclose(file);
		return true;
	}
	return false;	
}
void showHelp()
{
	std::cout << "Uso: ./mesh <caminho_do_modelo>" << std::endl;
	std::cout << "Exemplo: ./mesh ../models/teapot.obj" << std::endl;
	std::cout << "Teclas:" << std::endl;
	
}

 int main(int argc, char** argv)
 {
	if (argc != 2)
	{
		std::cerr << "Número inválido de argumentos. Use -h ou --help para mais informações." << std::endl;
		return 1;
	}	
	// Argumento -h ou --help
	if ((strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0))
	{
		showHelp();
		return 0;
	}
	//Verifica se o número de argumentos é válido, sendo que o unico numero válido é 2
	
	// Verifica se o caminho do modelo foi passado como argumento
	 if (!isValidPath(argv[1]))
	 {
		 std::cerr << "Caminho inválido para o modelo: " << argv[1] << std::endl;
		 return 1;
	 }

	modelPath = argv[1]; // Caminho do modelo
	

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
	glutSpecialFunc(specialKeys);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutMouseWheelFunc(scroll);
 
     glutMainLoop();
 }
 