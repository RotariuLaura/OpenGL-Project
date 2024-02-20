#if defined (__APPLE__)
    #define GLFW_INCLUDE_GLCOREARB
    #define GL_SILENCE_DEPRECATION
#else
    #define GLEW_STATIC
    #include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include <glm/glm.hpp> //core glm functionality
#include <glm/gtc/matrix_transform.hpp> //glm extension for generating common transformation matrices
#include <glm/gtc/matrix_inverse.hpp> //glm extension for computing inverse matrices
#include <glm/gtc/type_ptr.hpp> //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"
#include "SkyBox.hpp"

#include <iostream>

// window
gps::Window myWindow;

// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;

// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;

glm::vec3 lightPos2;
glm::vec3 lightColor2;

// shader uniform locations
GLint modelLoc;
GLint viewLoc;
GLint projectionLoc;
GLint normalMatrixLoc;
GLint lightDirLoc;
GLint lightColorLoc;
GLint lightPos2Loc;
GLint lightColorLoc2;

// camera
gps::Camera myCamera(
    glm::vec3(0.0f, 0.0f, 3.0f),
    glm::vec3(0.0f, 0.0f, -10.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.05f;

GLboolean pressedKeys[1024];

// models
gps::Model3D scena;
gps::Model3D obiect, obiect2, obiect3, obiecte, obiect_de_miscat, obiect_de_miscat2;
GLfloat angle, angle2, angle3;

// shaders
gps::Shader myBasicShader;

gps::SkyBox mySkyBox;
gps::Shader skyboxShader;

GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR) {
		std::string error;
		switch (errorCode) {
            case GL_INVALID_ENUM:
                error = "INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                error = "INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                error = "INVALID_OPERATION";
                break;
            case GL_OUT_OF_MEMORY:
                error = "OUT_OF_MEMORY";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error = "INVALID_FRAMEBUFFER_OPERATION";
                break;
        }
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
	//TODO
    glViewport(0, 0, width, height);
    projection = glm::perspective(glm::radians(45.0f),
        (float)width / (float)height,
        0.1f, 20.0f);
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

}

bool nightFlag, fogFlag;
bool rotateScene = false;
bool animationFlag = false, animationFlag2 = false, animation2Flag = false;
bool wireframeMode = false;
bool polygonalMode = false;
bool smoothMode = false;

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

	if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        } else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }

    if (key == GLFW_KEY_F && action == GLFW_PRESS) {
        fogFlag = !fogFlag;
        myBasicShader.useShaderProgram();
        glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "fogCond"), fogFlag ? 1 : 0);
    }

    if (key == GLFW_KEY_N && action == GLFW_PRESS) {
        nightFlag = !nightFlag;
        myBasicShader.useShaderProgram();
        glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "nightCond"), nightFlag ? 1 : 0);
        skyboxShader.useShaderProgram();
        glUniform1i(glGetUniformLocation(skyboxShader.shaderProgram, "nightCond"), nightFlag ? 1 : 0);
    }
    if (key == GLFW_KEY_G && action == GLFW_PRESS) {
        rotateScene = !rotateScene;
    }
    if (key == GLFW_KEY_L && action == GLFW_PRESS) {
        animationFlag = !animationFlag;
    }
    if (key == GLFW_KEY_K && action == GLFW_PRESS) {
        animationFlag2 = !animationFlag2;
    }
    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        animation2Flag = !animation2Flag;
    }
    if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
        wireframeMode = !wireframeMode;
        if (wireframeMode) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); 
        }
        else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); 
        }
    }

    if (key == GLFW_KEY_X && action == GLFW_PRESS) {
        polygonalMode = !polygonalMode;
        if (polygonalMode) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT); 
        }
        else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); 
        }
    }

    /*if (key == GLFW_KEY_C && action == GLFW_PRESS) {
        smoothMode = !smoothMode;
        if (smoothMode) {
            glShadeModel(GL_SMOOTH); 
        }
        else {
            glShadeModel(GL_FLAT); 
        }
    }*/
}

float lastX, lastY;
bool firstMouse = true;
float sensitivity = 0.1f;
float yaw, pitch;

void updateShaderUniforms() {
    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view)) * lightDir));
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    //TODO
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        if (!firstMouse) {
            float xDiff = (float)xpos - lastX;
            float yDiff = (float)ypos - lastY;
            lastX = (float)xpos;
            lastY = (float)ypos;
            xDiff *= sensitivity;
            yDiff *= sensitivity;
            yaw += xDiff;
            pitch += yDiff;
            if (pitch > 89.0f)
                pitch = 89.0f;
            if (pitch < -89.0f)
                pitch = -89.0f;
            myCamera.rotate(pitch, yaw);
            view = myCamera.getViewMatrix();
            updateShaderUniforms();
        }
        else {
            lastX = float(xpos);
            lastY = float(ypos);
            firstMouse = false;
        }
    }
}

glm::mat4 modelObiectDeMiscat, modelObiectDeMiscat2;

void processMovement() {
	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
		//update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for scena
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

    if (pressedKeys[GLFW_KEY_Q]) {
        angle -= 1.0f;
        // update model matrix for teapot
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        modelObiectDeMiscat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        // update normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
    }

    if (pressedKeys[GLFW_KEY_E]) {
        angle += 1.0f;
        // update model matrix for teapot
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        modelObiectDeMiscat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        // update normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
    }
    if (pressedKeys[GLFW_KEY_U]) {
        myCamera.move(gps::MOVE_UP, cameraSpeed);
        view = myCamera.getViewMatrix();
        updateShaderUniforms();
    }

    if (pressedKeys[GLFW_KEY_Y]) {
        myCamera.move(gps::MOVE_DOWN, cameraSpeed);
        view = myCamera.getViewMatrix();
        updateShaderUniforms();
    }
}

void initOpenGLWindow() {
    myWindow.Create(1024, 768, "OpenGL Project Core");
}

void setWindowCallbacks() {
	glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
}

void initOpenGLState() {
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
	glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	//glEnable(GL_CULL_FACE); // cull face
	//glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initModels() {
    scena.LoadModel("models/scena/scena2.obj");
    obiect.LoadModel("models/obiect/obiect.obj");
    obiect.LoadModel("models/obiect/obiect2.obj");
    obiect.LoadModel("models/obiect/obiect3.obj");
    obiect.LoadModel("models/obiect/obiecte.obj");
    obiect_de_miscat.LoadModel("models/obiect_de_miscat/obiect_de_miscat.obj");
    obiect_de_miscat2.LoadModel("models/obiect_de_miscat/obiect_de_miscat2.obj");
}

void initShaders() {
	myBasicShader.loadShader(
        "shaders/basic.vert",
        "shaders/basic.frag");
    skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
    skyboxShader.useShaderProgram();
}


void initUniforms() {
	myBasicShader.useShaderProgram();

    // create model matrix for teapot
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
	modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");

	// get view matrix for current camera
	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
	// send view matrix to shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // compute normal matrix for teapot
    normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

	// create projection matrix
	projection = glm::perspective(glm::radians(45.0f),
                               (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
                               0.1f, 20.0f);
	projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
	// send projection matrix to shader
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));	

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
	lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
	// send light dir to shader
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

    lightPos2 = glm::vec3(0.0f, 1.0f, 1.0f);
    lightPos2Loc = glGetUniformLocation(myBasicShader.shaderProgram, "lightPos2");
    // send light dir to shader
    glUniform3fv(lightPos2Loc, 1, glm::value_ptr(lightPos2));

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
	// send light color to shader
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
  
    //set light color
    lightColor2 = glm::vec3(1.0f, 1.0f, 1.0f); //white light
    lightColorLoc2 = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor2");
    // send light color to shader
    glUniform3fv(lightColorLoc2, 1, glm::value_ptr(lightColor2));

    fogFlag = false;
    glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "fogCond"), 0);

    nightFlag = false;
    glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "nightCond"), 0);
}

void renderScena(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();

    //send teapot model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    //send teapot normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw teapot
    scena.Draw(shader);
}


void renderObiect(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();
    //send model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    //send normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw 
    obiect.Draw(shader);
}

void renderObiect2(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();
    //send model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    //send normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw 
    obiect2.Draw(shader);
}

void renderObiect3(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();
    //send model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    //send normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw 
    obiect3.Draw(shader);
}

void renderObiecte(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();
    //send model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    //send normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw 
    obiecte.Draw(shader);
}

glm::vec3 objectPosition(0.0f, 0.0f, 0.0f);

void renderObiectDeMiscat(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();
    // send model matrix data to shader for obiect_de_miscat
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelObiectDeMiscat));

    // send normal matrix data to shader for obiect_de_miscat
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::inverseTranspose(view * modelObiectDeMiscat))));

    // draw obiect_de_miscat
    obiect_de_miscat.Draw(shader);
}

void renderObiectDeMiscat2(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();
    // send model matrix data to shader for obiect_de_miscat
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelObiectDeMiscat2));

    // send normal matrix data to shader for obiect_de_miscat
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::inverseTranspose(view * modelObiectDeMiscat2))));

    // draw obiect_de_miscat
    obiect_de_miscat2.Draw(shader);
}

void updateObiectDeMiscat() {
    if (animationFlag) 
    {
        angle += 0.05f;
        glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), objectPosition);
        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
        modelObiectDeMiscat = translationMatrix * rotationMatrix;
    }
}

void updateObiectDeMiscat2() {
    if (animationFlag2)
    {
        angle -= 0.05f; 
        glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), objectPosition);
        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
        modelObiectDeMiscat = translationMatrix * rotationMatrix;
    }
}

glm::vec3 objectPosition2(0.0f, 0.0f, 0.0f);

void update2ObiectDeMiscat() {
    if (animation2Flag)
    {
        angle2 += 0.5f;
        glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), objectPosition2);
        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(angle2), glm::vec3(0.0f, 1.0f, 0.0f));
        modelObiectDeMiscat2 = translationMatrix * rotationMatrix;
    }
}

float sceneRotationAngle = 0.0f; 
void rotate() {
    if (rotateScene) {
        sceneRotationAngle += 0.5f;  
        model = glm::rotate(glm::mat4(1.0f), glm::radians(sceneRotationAngle), glm::vec3(0, 1, 0));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }
}

void initSkyBox() {
    std::vector<const GLchar*> faces;
    faces.push_back("skybox/right.tga");
    faces.push_back("skybox/left.tga");
    faces.push_back("skybox/top.tga");
    faces.push_back("skybox/bottom.tga");
    faces.push_back("skybox/back.tga");
    faces.push_back("skybox/front.tga");
    mySkyBox.Load(faces);
}

void renderScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderObiectDeMiscat(myBasicShader);
    renderObiectDeMiscat2(myBasicShader);
	//render the scene
	renderScena(myBasicShader);
    mySkyBox.Draw(skyboxShader, view, projection);
    renderObiect(myBasicShader);
    renderObiect2(myBasicShader);
    renderObiect3(myBasicShader);
    renderObiecte(myBasicShader);
}

void cleanup() {
    myWindow.Delete();
    //cleanup code for your own data
}


int main(int argc, const char * argv[]) {

    try {
        initOpenGLWindow();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    initOpenGLState();
	initModels();
	initShaders();
	initUniforms();
    setWindowCallbacks();
    initSkyBox();

	glCheckError();
	// application loop
	while (!glfwWindowShouldClose(myWindow.getWindow())) {
        processMovement();
        rotate();
        updateObiectDeMiscat();
        updateObiectDeMiscat2();
        update2ObiectDeMiscat();
        renderScene();

		glfwPollEvents();
		glfwSwapBuffers(myWindow.getWindow());

		glCheckError();
	}

	cleanup();

    return EXIT_SUCCESS;
}
