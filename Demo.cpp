#include "Demo.h"



Demo::Demo() {

}


Demo::~Demo() {
}



void Demo::Init() {
	// build and compile our shader program
	// ------------------------------------
	shaderProgram = BuildShader("vertexShader.vert", "fragmentShader.frag", nullptr);

	BuildBlade();
	BuildGuard();
	BuildHandle();

	InitCamera();
}

void Demo::DeInit() {
	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glDeleteVertexArrays(1, &VAO2);
	glDeleteBuffers(1, &VBO2);
	glDeleteBuffers(1, &EBO2);
	glDeleteVertexArrays(1, &VAO3);
	glDeleteBuffers(1, &VBO3);
	glDeleteBuffers(1, &EBO3);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void Demo::ProcessInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}

	// zoom camera
	// -----------
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
		if (fovy < 90) {
			fovy += 0.0001f;
		}
	}

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		if (fovy > 0) {
			fovy -= 0.0001f;
		}
	}

	// update camera movement 
	// -------------
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		MoveCamera(CAMERA_SPEED);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		MoveCamera(-CAMERA_SPEED);
	}

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		StrafeCamera(-CAMERA_SPEED);
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		StrafeCamera(CAMERA_SPEED);
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		rotate = !rotate;
	}

	// update camera rotation
	// ----------------------
	double mouseX, mouseY;
	double midX = screenWidth / 2;
	double midY = screenHeight / 2;
	float angleY = 0.0f;
	float angleZ = 0.0f;

	// Get mouse position
	glfwGetCursorPos(window, &mouseX, &mouseY);
	if ((mouseX == midX) && (mouseY == midY)) {
		return;
	}

	// Set mouse position
	glfwSetCursorPos(window, midX, midY);

	// Get the direction from the mouse cursor, set a resonable maneuvering speed
	angleY = (float)((midX - mouseX)) / 1000;
	angleZ = (float)((midY - mouseY)) / 1000;

	// The higher the value is the faster the camera looks around.
	viewCamY += angleZ * 2;

	// limit the rotation around the x-axis
	if ((viewCamY - posCamY) > 8) {
		viewCamY = posCamY + 8;
	}
	if ((viewCamY - posCamY) < -8) {
		viewCamY = posCamY - 8;
	}
	RotateCamera(-angleY);


}

void Demo::Update(double deltaTime) {
	if (rotate) {
		angle += (float)((deltaTime * 0.5f) / 1000);
	}
}

void Demo::InitCamera()
{
	posCamX = 0.0f;
	posCamY = 0.8f;
	posCamZ = 4.0f;
	viewCamX = 2.5f;
	viewCamY = 0.0f;
	viewCamZ = 0.0f;
	upCamX = 0.0f;
	upCamY = 1.0f;
	upCamZ = 0.0f;
	CAMERA_SPEED = 0.0001f;
	fovy = 45.0f;
	glfwSetInputMode(this->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}


void Demo::MoveCamera(float speed)
{
	float x = viewCamX - posCamX;
	float z = viewCamZ - posCamZ;
	// forward positive cameraspeed and backward negative -cameraspeed.
	posCamX = posCamX + x * speed;
	posCamZ = posCamZ + z * speed;
	viewCamX = viewCamX + x * speed;
	viewCamZ = viewCamZ + z * speed;
}

void Demo::StrafeCamera(float speed)
{
	float x = viewCamX - posCamX;
	float z = viewCamZ - posCamZ;
	float orthoX = -z;
	float orthoZ = x;

	// left positive cameraspeed and right negative -cameraspeed.
	posCamX = posCamX + orthoX * speed;
	posCamZ = posCamZ + orthoZ * speed;
	viewCamX = viewCamX + orthoX * speed;
	viewCamZ = viewCamZ + orthoZ * speed;
}

void Demo::RotateCamera(float speed)
{
	float x = viewCamX - posCamX;
	float z = viewCamZ - posCamZ;
	viewCamZ = (float)(posCamZ + glm::sin(speed) * x + glm::cos(speed) * z);
	viewCamX = (float)(posCamX + glm::cos(speed) * x - glm::sin(speed) * z);
}

void Demo::Render() {

	glViewport(0, 0, this->screenWidth, this->screenHeight);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	//glEnable(GL_CULL_FACE);

	glEnable(GL_DEPTH_TEST);

	// Pass perspective projection matrix
	glm::mat4 projection = glm::perspective(fovy, (GLfloat)this->screenWidth / (GLfloat)this->screenHeight, 0.1f, 100.0f);
	GLint projLoc = glGetUniformLocation(this->shaderProgram, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	// LookAt camera (position, target/direction, up)
	glm::vec3 cameraPos = glm::vec3(posCamX, posCamY, posCamZ);
	glm::vec3 cameraFront = glm::vec3(viewCamX, viewCamY, viewCamZ);
	glm::mat4 view = glm::lookAt(cameraPos, cameraFront, glm::vec3(upCamX, upCamY, upCamZ));
	GLint viewLoc = glGetUniformLocation(this->shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	glm::mat4 model;
	model = glm::scale(model, glm::vec3(3.0f,3.0f,3.0f));
	model = glm::rotate(model, angle, glm::vec3(0, 1, 0));
	GLint modelLoc = glGetUniformLocation(this->shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	// set lighting attributes
	GLint viewPosLoc = glGetUniformLocation(this->shaderProgram, "viewPos");
	glUniform3f(viewPosLoc, cameraPos.x, cameraPos.y, cameraPos.z);
	glUniform3f(glGetUniformLocation(this->shaderProgram, "dirLight.direction"), 0.0f, -1.0f, -1.0f);
	glUniform3f(glGetUniformLocation(this->shaderProgram, "dirLight.ambient"), 0.25, 0.25, 0.25);
	glUniform3f(glGetUniformLocation(this->shaderProgram, "dirLight.diffuse"), 0.25, 0.25, 0.25);
	glUniform3f(glGetUniformLocation(this->shaderProgram, "dirLight.specular"), 0.25, 0.25, 0.25);

	// point light 1
	glUniform3f(glGetUniformLocation(this->shaderProgram, "pointLights[0].position"), 1.0f, 0.8f, -1.0f);
	glUniform3f(glGetUniformLocation(this->shaderProgram, "pointLights[0].ambient"), 1.0f, 0.5f, 0.5f);
	glUniform3f(glGetUniformLocation(this->shaderProgram, "pointLights[0].diffuse"), 1.0f, 0.5f, 0.5f);
	glUniform3f(glGetUniformLocation(this->shaderProgram, "pointLights[0].specular"), 1.0f, 0.5f, 0.5f);
	glUniform1f(glGetUniformLocation(this->shaderProgram, "pointLights[0].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(this->shaderProgram, "pointLights[0].linear"), 0.5f);
	glUniform1f(glGetUniformLocation(this->shaderProgram, "pointLights[0].quadratic"), 0.032f);

	// point light 2
	glUniform3f(glGetUniformLocation(this->shaderProgram, "pointLights[1].position"), -3.0f, -1.0f, 3.0f);
	glUniform3f(glGetUniformLocation(this->shaderProgram, "pointLights[1].ambient"), 0.5f, 0.5f, 1.0f);
	glUniform3f(glGetUniformLocation(this->shaderProgram, "pointLights[1].diffuse"), 0.5f, 0.5f, 1.0f);
	glUniform3f(glGetUniformLocation(this->shaderProgram, "pointLights[1].specular"), 0.5f, 0.5f, 1.0f);
	glUniform1f(glGetUniformLocation(this->shaderProgram, "pointLights[1].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(this->shaderProgram, "pointLights[1].linear"), 0.09f);
	glUniform1f(glGetUniformLocation(this->shaderProgram, "pointLights[1].quadratic"), 0.032f);

	// spotLight
	glUniform3fv(glGetUniformLocation(this->shaderProgram, "spotLight.position"), 1, &cameraPos[0]);
	glUniform3fv(glGetUniformLocation(this->shaderProgram, "spotLight.direction"), 1, &cameraFront[0]);
	glUniform3f(glGetUniformLocation(this->shaderProgram, "spotLight.ambient"), 0.5f, 1.0f, 0.5f);
	glUniform3f(glGetUniformLocation(this->shaderProgram, "spotLight.diffuse"), 0.5f, 1.0f, 0.5f);
	glUniform3f(glGetUniformLocation(this->shaderProgram, "spotLight.specular"), 0.5f, 1.0f, 0.5f);
	glUniform1f(glGetUniformLocation(this->shaderProgram, "spotLight.constant"), 1.0f);
	glUniform1f(glGetUniformLocation(this->shaderProgram, "spotLight.linear"), 0.09f);
	glUniform1f(glGetUniformLocation(this->shaderProgram, "spotLight.quadratic"), 0.032f);
	glUniform1f(glGetUniformLocation(this->shaderProgram, "spotLight.cutOff"), glm::cos(glm::radians(8.5f)));
	glUniform1f(glGetUniformLocation(this->shaderProgram, "spotLight.outerCutOff"), glm::cos(glm::radians(15.0f)));



	DrawBlade();
	DrawGuard();
	DrawHandle();

	glDisable(GL_DEPTH_TEST);
}

void Demo::BuildBlade() {
	// load image into texture memory
	// ------------------------------
	// Load and create a texture 
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height;
	unsigned char* image = SOIL_load_image("metal2.png", &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenTextures(1, &stexture);
	glBindTexture(GL_TEXTURE_2D, stexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	image = SOIL_load_image("metal_specular.png", &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);

	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	float vertices[] = {
		// format position, tex coords
		//left
		0.03f,		0.9f,	0.0f,		0.5, 4,		0.0f,0.0f,1.0f, //0
		0.03f,		0.6f,	0.015f,		0.5, 3,		0.0f,0.0f,1.0f, //1
		-0.04f,		0.7f,	0.0f,		0, 3.5,		0.0f,0.0f,1.0f, //2
		-0.025f,	0.0f,	0.0f,		0, 0,		0.0f,0.0f,1.0f, //3
		0.03f,		0.0f,	0.015f,		0.5, 0,		0.0f,0.0f,1.0f, //4

		//right
		0.03f,		0.9f,	0.0f,		0.5, 4,		0.0f,0.0f,-1.0f, //5
		0.03f,		0.6f,	-0.015f,	0.5, 3,		0.0f,0.0f,-1.0f, //6
		-0.04f,		0.7f,	0.0f,		0, 3.5,		0.0f,0.0f,-1.0f, //7
		-0.025f,	0.0f,	0.0f,		0, 0,		0.0f,0.0f,-1.0f, //8
		0.03f,		0.0f,	-0.015f,	0.5, 0,		0.0f,0.0f,-1.0f, //9

		//back
		0.03f,		0.9f,	0.0f,		0.25, 4,	1.0f,0.0f,0.0f, //10
		0.03f,		0.6f,	0.015f,		0, 3,		1.0f,0.0f,0.0f, //11
		0.03f,		0.6f,	-0.015f,	0.5, 3,		1.0f,0.0f,0.0f, //12
		0.03f,		0.0f,	0.015f,		0, 0,		1.0f,0.0f,0.0f, //13
		0.03f,		0.0f,	-0.015f,	0.5, 0,		1.0f,0.0f,0.0f, //14
	};

	unsigned int indices[] = {
		//left
		0,1,2,
		2,1,3,
		1,3,4,
		//right
		5,7,6,
		7,8,9,
		7,9,6,
		//back
		10,11,12,
		11,13,14,
		11,12,14,
	};

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// define position pointer layout 0
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(0 * sizeof(GLfloat)));
	glEnableVertexAttribArray(0);

	// define texcoord pointer layout 1
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// define normal pointer layout 2
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
	// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
	glBindVertexArray(0);

	// remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

}

void Demo::DrawBlade()
{
	UseShader(this->shaderProgram);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glUniform1i(glGetUniformLocation(this->shaderProgram, "material.diffuse"), 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, stexture);
	glUniform1i(glGetUniformLocation(this->shaderProgram, "material.specular"), 1);

	GLint shininessMatLoc = glGetUniformLocation(this->shaderProgram, "material.shininess");
	glUniform1f(shininessMatLoc, 0.1f);

	glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);

}

void Demo::BuildGuard() {
	// load image into texture memory
	// ------------------------------
	// Load and create a texture 
	glGenTextures(1, &texture2);
	glBindTexture(GL_TEXTURE_2D, texture2);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height;
	unsigned char* image = SOIL_load_image("wood2.png", &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenTextures(1, &stexture2);
	glBindTexture(GL_TEXTURE_2D, stexture2);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	image = SOIL_load_image("wood_specular.png", &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);

	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	float vertices[] = {
		// format position, tex coords
		//top
		-0.04f,		0.0f,		0.02f,		1, 0,	0.0f,1.0f,0.0f,	//0
		0.045f,		0.0f,		0.03f,		1, 1,	0.0f,1.0f,0.0f,	//1
		0.045f,		0.0f,		-0.03f,		0, 1,	0.0f,1.0f,0.0f,	//2
		-0.04f,		0.0f,		-0.02f,		0, 0,	0.0f,1.0f,0.0f,	//3
		//front
		-0.04f,		0.0f,		0.02f,		0, 1,	0.0f,0.0f,1.0f,	//4
		0.045f,		0.0f,		0.03f,		1, 1,	0.0f,0.0f,1.0f,	//5
		0.045f,		-0.02f,		0.03f,		1, 0,	0.0f,0.0f,1.0f,	//6
		-0.04f,		-0.02f,		0.02f,		0, 0,	0.0f,0.0f,1.0f,	//7
		//back
		-0.04f,		0.0f,		-0.02f,		0, 1,	0.0f,0.0f,-1.0f,//8
		0.045f,		0.0f,		-0.03f,		1, 1,	0.0f,0.0f,-1.0f,//9
		0.045f,		-0.02f,		-0.03f,		1, 0,	0.0f,0.0f,-1.0f,//10
		-0.04f,		-0.02f,		-0.02f,		0, 0,	0.0f,0.0f,-1.0f,//11
		//left
		-0.04f,		0.0f,		0.02f,		1, 1,	-1.0f,0.0f,0.0f,//12
		-0.04f,		0.0f,		-0.02f,		0, 1,	-1.0f,0.0f,0.0f,//13
		-0.04f,		-0.02f,		0.02f,		1, 0,	-1.0f,0.0f,0.0f,//14
		-0.04f,		-0.02f,		-0.02f,		0, 0,	-1.0f,0.0f,0.0f,//15
		//right
		0.045f,		0.0f,		0.03f,		1, 1,	1.0f,0.0f,0.0f,	//16
		0.045f,		0.0f,		-0.03f,		0, 1,	1.0f,0.0f,0.0f,	//17
		0.045f,		-0.02f,		0.03f,		1, 0,	1.0f,0.0f,0.0f,	//18
		0.045f,		-0.02f,		-0.03f,		0, 0,	1.0f,0.0f,0.0f,	//19
		//bottom
		-0.04f,		-0.02f,		0.02f,		0, 0,	0.0f,-1.0f,0.0f,//20
		0.045f,		-0.02f,		0.03f,		1, 0,	0.0f,-1.0f,0.0f,//21
		0.045f,		-0.02f,		-0.03f,		1, 1,	0.0f,-1.0f,0.0f,//22
		-0.04f,		-0.02f,		-0.02f,		0, 1,	0.0f,-1.0f,0.0f,//23

		//top
		-0.026f,	-0.18f,		0.018f,		0, 0,	0.0f,1.0f,0.0f, //24
		0.035f,		-0.18f,		0.018f,		1, 0,	0.0f,1.0f,0.0f, //25
		0.035f,		-0.18f,		-0.018f,	1, 1,	0.0f,1.0f,0.0f, //26
		-0.026f,	-0.18f,		-0.018f,	0, 1,	0.0f,1.0f,0.0f, //27
		//left
		-0.026f,	-0.18f,		0.018f,		1, 1,	-1.0f,0.0f,0.0f,//28
		-0.026f,	-0.19f,		0.018f,		0, 1,	-1.0f,0.0f,0.0f,//29
		-0.026f,	-0.19f,		-0.018f,	0, 0,	-1.0f,0.0f,0.0f,//30
		-0.026f,	-0.18f,		-0.018f,	1, 0,	-1.0f,0.0f,0.0f,//31
		//right
		0.035f,		-0.18f,		0.018f,		1, 0,	1.0f,0.0f,0.0f, //32
		0.035f,		-0.19f,		0.018f,		0, 1,	1.0f,0.0f,0.0f, //33
		0.035f,		-0.19f,		-0.018f,	0, 0,	1.0f,0.0f,0.0f, //34
		0.035f,		-0.18f,		-0.018f,	1, 0,	1.0f,0.0f,0.0f, //35
		//front
		-0.026f,	-0.18f,		0.018f,		0, 1,	0.0f,0.0f,1.0f, //36
		0.035f,		-0.18f,		0.018f,		1, 1,	0.0f,0.0f,1.0f, //37
		0.035f,		-0.19f,		0.018f,		1, 0,	0.0f,0.0f,1.0f, //38
		-0.026f,	-0.19f,		0.018f,		0, 0,	0.0f,0.0f,1.0f, //39
		//back
		-0.026f,	-0.18f,		-0.018f,	0, 1,	0.0f,0.0f,-1.0f,//40
		0.035f,		-0.18f,		-0.018f,	1, 1,	0.0f,0.0f,-1.0f,//41
		0.035f,		-0.19f,		-0.018f,	1, 0,	0.0f,0.0f,-1.0f,//42
		-0.026f,	-0.19f,		-0.018f,	0, 0,	0.0f,0.0f,-1.0f,//43
		//bottom
		-0.026f,	-0.19f,		0.018f,		1, 0,	0.0f,-1.0f,0.0f,//44
		0.035f,		-0.19f,		0.018f,		1, 1,	0.0f,-1.0f,0.0f,//45
		0.035f,		-0.19f,		-0.018f,	0, 1,	0.0f,-1.0f,0.0f,//46
		-0.026f,	-0.19f,		-0.018f,	0, 0,	0.0f,-1.0f,0.0f,//47


		//-0.026f,	-0.18f,		0.018f,		0, 0,		//8
		//0.035f,	-0.18f,		0.018f,		2, 0,		//9
		//0.035f,	-0.18f,		-0.018f,	2, 2,		//10
		//-0.026f,	-0.18f,		-0.018f,	0, 2,		//11
		//-0.026f,	-0.19f,		0.018f,		0, 0,		//12
		//0.035f,	-0.19f,		0.018f,		2, 0,		//13
		//0.035f,	-0.19f,		-0.018f,	2, 2,		//14
		//-0.026f,	-0.19f,		-0.018f,	0, 2,		//15

	};

	unsigned int indices[] = {
		//top
		0,1,2,
		0,2,3,
		//front
		4,5,6,
		4,6,7,
		//back
		8,9,10,
		8,10,11,
		//left
		12,13,14,
		13,14,15,
		//right
		16,17,18,
		17,18,19,
		//bottom
		20,21,22,
		21,22,23,

		//top
		24,25,26,
		24,26,27,
		//left
		28,29,30,
		28,30,31,
		//right
		32,33,34,
		32,34,35,
		//front
		36,37,38,
		36,38,39,
		//back
		40,41,42,
		40,42,43,
		//bottom
		44,45,46,
		44,46,47,
		
		
	};

	glGenVertexArrays(1, &VAO2);
	glGenBuffers(1, &VBO2);
	glGenBuffers(1, &EBO2);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(VAO2);

	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO2);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// define position pointer layout 0
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(0 * sizeof(GLfloat)));
	glEnableVertexAttribArray(0);

	// define texcoord pointer layout 1
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// define normal pointer layout 2
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
	// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
	glBindVertexArray(0);

	// remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

}

void Demo::DrawGuard()
{
	UseShader(this->shaderProgram);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, texture2);
	glUniform1i(glGetUniformLocation(this->shaderProgram, "material.diffuse"), 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, stexture2);
	glUniform1i(glGetUniformLocation(this->shaderProgram, "material.specular"), 3);

	GLint shininessMatLoc = glGetUniformLocation(this->shaderProgram, "material.shininess");
	glUniform1f(shininessMatLoc, 0.025f);

	glBindVertexArray(VAO2); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized

	glDrawElements(GL_TRIANGLES, 128, GL_UNSIGNED_INT, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);

}

void Demo::BuildHandle() {
	// load image into texture memory
	// ------------------------------
	// Load and create a texture 
	glGenTextures(1, &texture3);
	glBindTexture(GL_TEXTURE_2D, texture3);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height;
	unsigned char* image = SOIL_load_image("leather.png", &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenTextures(1, &stexture3);
	glBindTexture(GL_TEXTURE_2D, stexture3);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	image = SOIL_load_image("leather_specular.png", &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);

	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	float vertices[] = {
		// format position, tex coords
		//front
		-0.02f,		-0.18f,		0.012f,		0, 0,	0.0f,0.0f,1.0f, //0
		0.028f,		-0.18f,		0.012f,		2, 0,	0.0f,0.0f,1.0f, //1
		0.028f,		-0.02f,		0.012f,		2, 2,	0.0f,0.0f,1.0f, //2
		-0.025f,	-0.02f,		0.012f,		0, 2,	0.0f,0.0f,1.0f, //3
		//back
		-0.02f,		-0.18f,		-0.012f,	0, 0,	0.0f,0.0f,-1.0f, //4
		0.028f,		-0.18f,		-0.012f,	2, 0,	0.0f,0.0f,-1.0f, //5
		0.028f,		-0.02f,		-0.012f,	2, 2,	0.0f,0.0f,-1.0f, //6
		-0.025f,	-0.02f,		-0.012f,	0, 2,	0.0f,0.0f,-1.0f, //7
		//left
		-0.02f,		-0.18f,		0.012f,		2, 0,	-1.0f,0.0f,0.0f, //8
		-0.02f,		-0.18f,		-0.012f,	0, 0,	-1.0f,0.0f,0.0f, //9
		-0.025f,	-0.02f,		0.012f,		2, 2,	-1.0f,0.0f,0.0f, //10
		-0.025f,	-0.02f,		-0.012f,	0, 2,	-1.0f,0.0f,0.0f, //11
		//right
		0.028f,		-0.18f,		0.012f,		0, 0,	1.0f,0.0f,0.0f, //12
		0.028f,		-0.18f,		-0.012f,	2, 0,	1.0f,0.0f,0.0f, //13
		0.028f,		-0.02f,		0.012f,		0, 2,	1.0f,0.0f,0.0f, //14
		0.028f,		-0.02f,		-0.012f,	2, 2,	1.0f,0.0f,0.0f, //15

	};

	unsigned int indices[] = {
		//front
		0,1,2,
		3,0,2,
		//back
		4,5,6,
		7,4,6,
		//left
		8,10,9,
		9,10,11,
		//right
		12,13,14,
		13,14,15,
	};

	glGenVertexArrays(1, &VAO3);
	glGenBuffers(1, &VBO3);
	glGenBuffers(1, &EBO3);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(VAO3);

	glBindBuffer(GL_ARRAY_BUFFER, VBO3);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO3);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// define position pointer layout 0
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(0 * sizeof(GLfloat)));
	glEnableVertexAttribArray(0);

	// define texcoord pointer layout 1
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// define normal pointer layout 2
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
	// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
	glBindVertexArray(0);

	// remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

}

void Demo::DrawHandle ()
{
	UseShader(this->shaderProgram);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, texture3);
	glUniform1i(glGetUniformLocation(this->shaderProgram, "material.diffuse"), 4);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, stexture3);
	glUniform1i(glGetUniformLocation(this->shaderProgram, "material.specular"), 5);

	GLint shininessMatLoc = glGetUniformLocation(this->shaderProgram, "material.shininess");
	glUniform1f(shininessMatLoc, 0.08f);

	glBindVertexArray(VAO3); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);

}

int main(int argc, char** argv) {
	RenderEngine &app = Demo();
	app.Start("Sword 3D Model with Primitives", 1280, 720, false, false);
}