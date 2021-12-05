#include "Demo.h"



Demo::Demo() {

}


Demo::~Demo() {
}



void Demo::Init() {
	// build and compile our shader program
	// ------------------------------------
	shadowmapShader = BuildShader("shadowMapping.vert", "shadowMapping.frag", nullptr);
	depthmapShader = BuildShader("depthMap.vert", "depthMap.frag", nullptr);
	BuildDepthMap();

	BuildBlade();
	BuildGuard();
	BuildHandle();
	BuildPlane();

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
	glDeleteVertexArrays(1, &VAOp);
	glDeleteBuffers(1, &VBOp);
	glDeleteBuffers(1, &EBOp);
	glDeleteBuffers(1, &depthMapFBO);
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
	posCamY = 0.5f;
	posCamZ = -2.0f;
	viewCamX = 0.0f;
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

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	// Step 1 Render depth of scene to texture
	// ----------------------------------------
	glm::mat4 lightProjection, lightView;
	glm::mat4 lightSpaceMatrix;
	float near_plane = 1.0f, far_plane = 7.5f;
	lightProjection = glm::ortho(-1.0f, 1.0f, -1.0f, 5.0f, near_plane, far_plane);
	lightView = glm::lookAt(glm::vec3(-2.0f, 4.0f, -1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0, 1.0, 0.0));
	lightSpaceMatrix = lightProjection * lightView;
	// render scene from light's point of view
	UseShader(this->depthmapShader);
	glUniformMatrix4fv(glGetUniformLocation(this->depthmapShader, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
	glViewport(0, 0, this->SHADOW_WIDTH, this->SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	DrawBlade(this->depthmapShader);
	DrawGuard(this->depthmapShader);
	DrawHandle(this->depthmapShader);
	//DrawCube(this->depthmapShader);
	//DrawPlane(this->depthmapShader);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);




	// Step 2 Render scene normally using generated depth map
	// ------------------------------------------------------
	glViewport(0, 0, this->screenWidth, this->screenHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Pass perspective projection matrix
	UseShader(this->shadowmapShader);
	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)this->screenWidth / (GLfloat)this->screenHeight, 0.1f, 100.0f);
	glUniformMatrix4fv(glGetUniformLocation(this->shadowmapShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	// LookAt camera (position, target/direction, up)
	glm::vec3 cameraPos = glm::vec3(posCamX, posCamY, posCamZ);
	glm::vec3 cameraFront = glm::vec3(viewCamX, viewCamY, viewCamZ);
	glm::mat4 view = glm::lookAt(cameraPos, cameraFront, glm::vec3(upCamX, upCamY, upCamZ));
	glUniformMatrix4fv(glGetUniformLocation(this->shadowmapShader, "view"), 1, GL_FALSE, glm::value_ptr(view));

	// Setting Light Attributes
	glUniformMatrix4fv(glGetUniformLocation(this->shadowmapShader, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
	glUniform3f(glGetUniformLocation(this->shadowmapShader, "viewPos"), cameraPos.x, cameraPos.y, cameraPos.z);
	glUniform3f(glGetUniformLocation(this->shadowmapShader, "lightPos"), -2.0f, 4.0f, -1.0f);

	// Configure Shaders
	glUniform1i(glGetUniformLocation(this->shadowmapShader, "diffuseTexture"), 0);
	glUniform1i(glGetUniformLocation(this->shadowmapShader, "shadowMap"), 1);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ZERO);

	DrawPlane(this->shadowmapShader);
	if (!rotate) {
		glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);
	}
	DrawBlade(this->shadowmapShader);
	DrawGuard(this->shadowmapShader);
	DrawHandle(this->shadowmapShader);

	glDisable(GL_BLEND);
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

void Demo::DrawBlade(GLuint shader)
{
	UseShader(shader);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMap);

	glm::mat4 model;
	model = glm::rotate(model, angle, glm::vec3(0, 1, 0));
	GLint modelLoc = glGetUniformLocation(shader, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

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

void Demo::DrawGuard(GLuint shader)
{
	UseShader(shader);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture2);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMap);

	glm::mat4 model;
	model = glm::rotate(model, angle, glm::vec3(0, 1, 0));
	GLint modelLoc = glGetUniformLocation(shader, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

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

void Demo::DrawHandle (GLuint shader)
{
	UseShader(shader);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture3);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMap);

	glm::mat4 model;
	model = glm::rotate(model, angle, glm::vec3(0, 1, 0));
	GLint modelLoc = glGetUniformLocation(shader, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glBindVertexArray(VAO3); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
}

void Demo::BuildPlane()
{
	// Load and create a texture 
	glGenTextures(1, &texturep);
	glBindTexture(GL_TEXTURE_2D, texturep);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height;
	unsigned char* image = SOIL_load_image("wood.png", &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Build geometry
	GLfloat vertices[] = {
		// format position, tex coords
		// bottom
		-25.0f,	-0.5f, -25.0f,  0,  0, 0.0f,  1.0f,  0.0f,
		25.0f,	-0.5f, -25.0f, 25,  0, 0.0f,  1.0f,  0.0f,
		25.0f,	-0.5f,  25.0f, 25, 25, 0.0f,  1.0f,  0.0f,
		-25.0f,	-0.5f,  25.0f,  0, 25, 0.0f,  1.0f,  0.0f,
	};

	GLuint indices[] = { 0,  2,  1,  0,  3,  2 };

	glGenVertexArrays(1, &VAOp);
	glGenBuffers(1, &VBOp);
	glGenBuffers(1, &EBOp);

	glBindVertexArray(VAOp);

	glBindBuffer(GL_ARRAY_BUFFER, VBOp);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOp);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(0);
	// TexCoord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	// Normal attribute
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0); // Unbind VAO
}

void Demo::DrawPlane(GLuint shader)
{
	UseShader(shader);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texturep);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMap);

	glm::mat4 model;
	model = glm::translate(model, glm::vec3(0, 0.2f, 0));
	GLint modelLoc = glGetUniformLocation(shader, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glBindVertexArray(VAOp); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
}

void Demo::BuildDepthMap() {
	// configure depth map FBO
	// -----------------------
	glGenFramebuffers(1, &depthMapFBO);
	// create depth texture
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, this->SHADOW_WIDTH, this->SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int main(int argc, char** argv) {
	RenderEngine &app = Demo();
	app.Start("Sword 3D Model with Primitives", 1280, 720, false, false);
}