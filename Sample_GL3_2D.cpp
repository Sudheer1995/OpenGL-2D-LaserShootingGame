#include <iostream>
#include <cmath>
#include <fstream>
#include <string.h>
#include <vector>
#include <stdlib.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GL/glut.h>
#include <vector>
#include <algorithm>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define HEIGHT 1000
#define WIDTH 1000
using namespace std;

//Copyright (c) 2016 Copyright Holder All Rights Reserved.

struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;

typedef struct GraphicalObjectColor {
  float x_ordinate;
  float y_ordinate;
  float z_ordinate;
  VAO *object;
  char color[20];
}GraphicalObjectColor;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

GLuint programID;
float zoom = 1.0f, pan = 0.0f;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path)
{
	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
}

void reshapeWindow (GLFWwindow* window, int width, int height)
{
  int fbwidth=width, fbheight=height;
  /* With Retina display on Mac OS X, GLFW's FramebufferSize
  is different from WindowSize */
  glfwGetFramebufferSize(window, &fbwidth, &fbheight);

  GLfloat fov = 90.0f;

  // sets the viewport of openGL renderer
  glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

  // set the projection matrix as perspective
  /* glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
  // Store the projection matrix in a variable for future use
  // Perspective projection for 3D views
  // Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

  // Ortho projection for 2D views
  Matrices.projection = glm::ortho((-4.0f+pan)/zoom, (4.0f+pan)/zoom, (-4.0f)/zoom, (4.0f)/zoom, 0.1f, 500.0f);
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}


/**************************
 * Customizable functions *
 **************************/
int object_selected = 0;
float shoot = 0;
int black_brick_flag = 0;
float speed = 0.0;
float bullet_x_coordinate = 0.0, bullet_y_coordinate = 0.0;
/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
float camera_rotation_angle = 90;
double present_time, previous_time;
double xpos, ypos;
double mousexpos, mouseypos;
double slope, cos_theta, sin_theta;
double mirror_length = 0.8;

class GraphicalObject
{
  public:
    float x_ordinate;
    float y_ordinate;
    float z_ordinate;
    float angle;
    char color;
    glm::mat4 translate_matrix;
    glm::mat4 rotate_matrix;
  public:
    GraphicalObject(float X=0, float Y=0, float Z=0, float rotation=0, char colour='D')
    {
      x_ordinate = X;
      y_ordinate = Y;
      z_ordinate = Z;
      angle = rotation;
      color = colour;
    }
    void rotator(float rotation=0, glm::vec3 rotating_vector=glm::vec3(0,0,1))
    {
      rotate_matrix = glm::rotate((float)(rotation*M_PI/180.0f), rotating_vector);
    }
    void render(VAO *object)
    {
      glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
      glm::vec3 target (0, 0, 0);
      glm::vec3 up (0, 1, 0);
      Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane
      glm::mat4 VP = Matrices.projection * Matrices.view;
      glm::mat4 MVP;
      Matrices.model = glm::mat4(1.0f);
      translate_matrix = glm::translate(glm::vec3(x_ordinate, y_ordinate, z_ordinate));
      Matrices.model *= translate_matrix*rotate_matrix;
      MVP = VP * Matrices.model;
      glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
      draw3DObject(object);
    }
};

GraphicalObject Canon_body(-3.6, 0, 0, 0, 'M'),
                Canon_piston(-3.6, 0, 0, 0, 'M'),
                Left_bin_top(-2, -2.5, 0, 0, 'B'),
                Left_bin_bot(-2, -3.91421, 0, 0, 'R'),
                Left_bin_body(-2, -3.2071, 0, 0, 'R'),
                Right_bin_top(2, -2.5, 0, 0, 'B'),
                Right_bin_bot(2, -3.91421, 0, 0, 'G'),
                Right_bin_body(2, -3.2071, 0, 0, 'G'),
                Bullet(-3.6, 0, 0, 0, 'R');

vector <GraphicalObject> Stars;
vector <GraphicalObject> Mirrors;

void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
     // Function is called first on GLFW_PRESS.

    if (action == GLFW_RELEASE) {
        switch (key) {
            case GLFW_KEY_SPACE:
              break;
            case GLFW_KEY_C:

                break;
            case GLFW_KEY_P:

                break;
            case GLFW_KEY_X:
                // do something ..
                break;
            default:
                break;
        }
    }
    else if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_SPACE:
                if(shoot == 0){
                  shoot = 1;
                  Bullet.angle = Canon_piston.angle;
                  bullet_x_coordinate = Canon_body.x_ordinate;
                  bullet_y_coordinate = Canon_body.y_ordinate;
                  slope = tan(Canon_piston.angle*M_PI/180.0f);
                  Canon_piston.angle = atan(slope)*180.0f/M_PI;
                  cos_theta = 1/sqrt(1+slope*slope);
                  sin_theta = sqrt(1-cos_theta*cos_theta);
                }
                  break;
            case GLFW_KEY_UP:
              if(zoom < 2)
              {
                zoom += 0.1;
                reshapeWindow(window,WIDTH,HEIGHT);
              }
              break;
            case GLFW_KEY_LEFT:
              pan -= 0.1;
              reshapeWindow(window,WIDTH,HEIGHT);
              break;
            case GLFW_KEY_RIGHT:
              pan += 0.1;
              reshapeWindow(window,WIDTH,HEIGHT);
              break;
            case GLFW_KEY_DOWN:
              if(zoom > 1)
              {
                zoom -= 0.1;
                reshapeWindow(window,WIDTH,HEIGHT);
              }
              break;
            case GLFW_KEY_N:
                speed += 0.01;
                break;
            case GLFW_KEY_M:
                if(speed > 0)
                  speed -= 0.01;
                break;
            case GLFW_KEY_ESCAPE:
                quit(window);
                break;
            case GLFW_KEY_A: //canon rotates CCW
              if(Canon_piston.angle < 72)
                Canon_piston.angle += 18;
              break;
            case GLFW_KEY_D: // canon rotates CW
              if(Canon_piston.angle > -72)
                Canon_piston.angle -= 18;
              break;
            case GLFW_KEY_S: // canon moves up
              if(Canon_body.y_ordinate < 3){
                  Canon_body.y_ordinate++;
                  Canon_piston.y_ordinate++;
              }
              break;
            case GLFW_KEY_F:  // canon moves down
              if(Canon_body.y_ordinate > -1.5){
                  Canon_body.y_ordinate--;
                  Canon_piston.y_ordinate--;
              }
              break;
            case GLFW_KEY_RIGHT_CONTROL:   // left bucket moves right
                if(Left_bin_top.x_ordinate < 2.5){
                  Left_bin_top.x_ordinate++;
                  Left_bin_bot.x_ordinate++;
                  Left_bin_body.x_ordinate++;
                }
                break;
            case GLFW_KEY_LEFT_CONTROL:   // left bucket moves left
              if(Left_bin_top.x_ordinate > -2){
                Left_bin_body.x_ordinate--;
                Left_bin_bot.x_ordinate--;
                Left_bin_top.x_ordinate--;
              }
                break;
            case GLFW_KEY_LEFT_ALT:   // right bucket moves left
                if(Right_bin_top.x_ordinate > -2){
                    Right_bin_top.x_ordinate--;
                    Right_bin_bot.x_ordinate--;
                    Right_bin_body.x_ordinate--;
                }
                break;
            case GLFW_KEY_RIGHT_ALT:  // right bucket moves right
                if(Right_bin_bot.x_ordinate < 2.5){
                    Right_bin_top.x_ordinate++;
                    Right_bin_bot.x_ordinate++;
                    Right_bin_body.x_ordinate++;
                }
                break;
            default:
                break;
        }
    }
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
            quit(window);
            break;
		default:
			break;
	}
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            if (action == GLFW_PRESS){
              glfwGetCursorPos(window, &xpos, &ypos);
              xpos = (8*(xpos/HEIGHT))-4;
              ypos = (-8*(ypos/WIDTH))+4;
              if(sqrt(pow(xpos-Left_bin_body.x_ordinate, 2)+
                      pow(ypos-Left_bin_body.y_ordinate, 2)) <= 0.75)
                        object_selected = 1;

              else if(sqrt(pow(xpos-Right_bin_body.x_ordinate, 2)+
                           pow(ypos-Right_bin_body.y_ordinate, 2)) <= 0.75)
                        object_selected = 2;

              else if(sqrt(pow(xpos-Canon_body.x_ordinate, 2)+
                           pow(ypos-Canon_body.y_ordinate, 2)) <= 0.3)
                        object_selected = 3;

              else if(shoot == 0){
                shoot = 1;
                slope = (ypos-Canon_body.y_ordinate)/(xpos+3.6);
                Canon_piston.angle = atan(slope)*180.0f/M_PI;
                Bullet.angle = atan(slope)*180.0f/M_PI;
                bullet_x_coordinate = Canon_body.x_ordinate;
                bullet_y_coordinate = Canon_body.y_ordinate;
                cos_theta = 1/sqrt(1+slope*slope);
                sin_theta = sqrt(1-cos_theta*cos_theta);
              }
              else
                object_selected = 0;
            }
            if(action == GLFW_RELEASE){
              object_selected = 0;
            }
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            if (action == GLFW_PRESS) {

            }
            break;
        default:
            break;
    }
}

void mouseScroll(GLFWwindow* window, double xoffset, double yoffset)
{
  if(yoffset == 1){
      zoom+=0.1;
    reshapeWindow(window, WIDTH, HEIGHT);
  }
  if(yoffset == -1){
      zoom-=0.1;
    reshapeWindow(window, WIDTH, HEIGHT);
  }
}
/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
int curr_bricks = 0;

GLfloat digitopbar[] = {
                  0.1,0.35,1,
                  -0.1,0.35,1,
                  -0.1,0.30,1,

                  -0.1,0.30,1,
                  0.1,0.30,1,
                  0.1,0.35,1
                };
GLfloat digitmidbar [] = {
                  0.1,0.05,1,
                  -0.1,0.05,1,
                  -0.1,-0.05,1,

                  -0.1,-0.05,1,
                  0.1,-0.05,1,
                  0.1,0.05,1
                };
GLfloat digitbotbar [] = {
                0.1,-0.30,1,
                -0.1,-0.30,1,
                -0.1,-0.35,1,

                -0.1,-0.35,1,
                0.1,-0.35,1,
                0.1,-0.30,1
                };
GLfloat digitlefttopbar [] = {
                              -0.05,0.30,1,
                              -0.1,0.30,1,
                              -0.1,0.05,1,

                              -0.1,0.05,1,
                              -0.05,0.05,1,
                              -0.05,0.30,1
                            };
GLfloat digitleftbotbar [] = {
                              -0.05,-0.05,1,
                              -0.1,-0.05,1,
                              -0.1,-0.30,1,

                              -0.1,-0.30,1,
                              -0.05,-0.30,1,
                              -0.05,-0.05,1
                              };
GLfloat digitrighttopbar [] = {
                              0.1,0.30,1,
                              0.05,0.30,1,
                              0.05,0.05,1,

                              0.05,0.05,1,
                              0.1,0.05,1,
                              0.1,0.30,1
                              };
GLfloat digitrightbotbar [] = {
                              0.1,-0.05,1,
                              0.05,-0.05,1,
                              0.05,-0.30,1,

                              0.05,-0.30,1,
                              0.1,-0.30,1,
                              0.1,-0.05,1
                              };
GLfloat darkyellow[] = {
                        1,1,0,
                        1,1,0,
                        1,1,0,
                        1,1,0,
                        1,1,0,
                        1,1,0
                      };
GLfloat red[] = {
                  1, 0.6, 0.6,
                  1, 0.30196, 0.30196,
                  1, 0.30196, 0.30196
                };

GLfloat yellow[] = {
                    1, 0.933, 0.50196,
                    1, 0.81961, 0.10196,
                    1, 0.81961, 0.10196,
                  };

GLfloat green[] = {
                    0.30196, 1, 0.30196,
                    0, 0.70196, 0,
                    0, 0.70196, 0
                  };
GLfloat ocean[] = {
                    0.6, 1, 1,
                    0, 0.70196, 0.70196,
                    0, 0.70196, 0.70196,
                  };

GLfloat maroon[] = {
                      0.12,0.12,0.12,
                      0, 0.12549,0.50196,//0.2, 0, 0.4,
                      0, 0.12549,0.50196
                    };
GLfloat white[] = {
                    0.74902, 0.74902, 0.74902,
                    0.90196, 0.90196, 0.90196,
                    0.90196, 0.90196, 0.90196
                  };
GLfloat piston[] = {
                    0.8, 0, 1,
                    0.87843, 0.4, 1,
                    0.87843, 0.4, 1
                    };
GLfloat grey[] = {
                  0.21961, 0.21961, 0.21961,
                  0.21961, 0.21961, 0.21961,
                  0.25098, 0.25098, 0.25098
                  };
GLfloat black[] = {
                  0.2,0.2,0.2,
                  0.4,0.4,0.4,
                  0.4,0.4,0.4
                 };
VAO *background,
    *left_bin_bot,
    *left_bin_top,
    *left_bin_body,
    *right_bin_bot,
    *right_bin_top,
    *right_bin_body,
    *canon_body,
    *canon_piston,
    *bullet,
    *blackmoon,
    *whitemoon,
    *topbar,
    *lefttopbar,
    *righttopbar,
    *midbar,
    *leftbotbar,
    *rightbotbar,
    *botbar;

vector<VAO*> mirrors;
vector<GraphicalObjectColor> bricks;
vector<GraphicalObjectColor> Bricks;
vector<VAO*> stars;

struct VAO *createPolygon(float radius, int sides, GLfloat color[])
{
  GLfloat vertex_buffer_data[9*sides];
  GLfloat color_buffer_data[9*sides];
  float angle = (float)360.0/sides;
  float curr_x = radius, curr_y = 0.0;
  int vertex = 0;
  for(int j = 0; j < sides; j++){

    color_buffer_data[vertex] = color[0];
    vertex_buffer_data[vertex++] = 0;
    color_buffer_data[vertex] = color[1];
    vertex_buffer_data[vertex++] = 0;
    color_buffer_data[vertex] = color[2];
    vertex_buffer_data[vertex++] = 0;

    color_buffer_data[vertex] = color[3];
    vertex_buffer_data[vertex++] = curr_x;
    color_buffer_data[vertex] = color[4];
    vertex_buffer_data[vertex++] = curr_y;
    color_buffer_data[vertex] = color[5];
    vertex_buffer_data[vertex++] = 0;

    float temp_x = (float)(curr_x*cos((float)angle*M_PI/180.0f) - curr_y*sin((float)angle*M_PI/180.0f));
    float temp_y = (float)(curr_x*sin((float)angle*M_PI/180.0f) + curr_y*cos((float)angle*M_PI/180.0f));
    curr_x = temp_x;
    curr_y = temp_y;
    color_buffer_data[vertex] = color[6];
    vertex_buffer_data[vertex++] = curr_x;
    color_buffer_data[vertex] = color[7];
    vertex_buffer_data[vertex++] = curr_y;
    color_buffer_data[vertex] = color[8];
    vertex_buffer_data[vertex++] = 0;
  }
  return create3DObject(GL_TRIANGLES, 3*sides, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createBackground()
{
  static const GLfloat vertex_buffer_data[] = {
    -5,-5,-2,
    5,-5,-2,
    5,5,-2,

    5,5,-2,
    -5,5,-2,
    -5,-5,-2
  };
  static GLfloat color_buffer_data[] = {
    0,0,0,
    0,0,0,
    0.2, 0, 0.4,

    0.2, 0, 0.4,
    0, 0.12549,0.50196,
    0,0,0,
  };
  background = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

VAO *createTriangle(float leftx, float lefty, float base, float height, GLfloat color[])
{
  GLfloat vertex_buffer_data[] = {
    leftx, lefty, -1,
    leftx+base, lefty, -1,
    leftx+base/2, lefty+height, -1
  };
  return create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color, GL_FILL);
}

VAO *createMirror(float length, float breadth)
{
    GLfloat vertex_buffer_data[] = {
      0, breadth/2, 0,
      length/2, 0, 0,
      -length/2, 0, 0,
    };

    GLfloat color_buffer_data[] = {
      1, 0.54902, 0,
      1, 0.54902, 0,
      1, 0.54902, 0
    };
    return create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createCanonShooter()
{
  static const GLfloat vertex_buffer_data [] = {
    0.5, 0.20, 0, // vertex 0
    0.5, -0.20, 0, // vertex 1
    0.7, 0, 0, // vertex 2
  };
  static GLfloat color_buffer_data[] = {
    0.54902, 0.10196, 1,
    0.54902, 0.10196, 1,
    0.65098, 0.30196, 1
  };
  canon_piston = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_FILL);
}

double dis = 0;

void get_selected_object(GLFWwindow *window,double mousexpos,double mouseypos)
{
  glfwGetCursorPos(window, &mousexpos, &mouseypos);
  mousexpos = (8*(mousexpos/HEIGHT))-4;
  mouseypos = (-8*(mouseypos/WIDTH))+4;
  if(object_selected == 1){
    if(mousexpos <= 2.5 && mousexpos >= -2){
      Left_bin_body.x_ordinate = mousexpos;
      Left_bin_top.x_ordinate = mousexpos;
      Left_bin_bot.x_ordinate = mousexpos;
    }
  }
  else if(object_selected == 2){
    if(mousexpos <= 2.5 && mousexpos >= -2){
      Right_bin_bot.x_ordinate = mousexpos;
      Right_bin_top.x_ordinate = mousexpos;
      Right_bin_body.x_ordinate = mousexpos;
    }
  }
  else if(object_selected == 3){
    if(mouseypos <= 3 && mouseypos >= -1.5){
      Canon_body.y_ordinate = mouseypos;
      Canon_piston.y_ordinate = mouseypos;
    }
  }
  else
    object_selected = 0;
}

int score = 0;

int checkCollison(GraphicalObject A, vector<GraphicalObjectColor> B)
{
  for(int items=0;items < curr_bricks;items++){
    if(sqrt(pow(A.x_ordinate-B[items].x_ordinate, 2)+
            pow(A.y_ordinate-B[items].y_ordinate, 2))<=0.1){
        if(strcmp(B[items].color,"red") || strcmp(B[items].color,"green"))
          score += 5;
        else
          score += 10;
        return items;
    }
  }
  return -1;
}

int checkMirrorCollison(float bullet_x, float bullet_y)
{
  for(int items=0;items < 4;items++)
  {
    double minX = min(Mirrors[items].x_ordinate+(mirror_length/2)*cos((float)(Mirrors[items].angle*M_PI/180.0f)),
                      Mirrors[items].x_ordinate-(mirror_length/2)*cos((float)(Mirrors[items].angle*M_PI/180.0f)));
    double minY = min(Mirrors[items].y_ordinate+(mirror_length/2)*sin((float)(Mirrors[items].angle*M_PI/180.0f)),
                      Mirrors[items].y_ordinate-(mirror_length/2)*sin((float)(Mirrors[items].angle*M_PI/180.0f)));
    double maxX = max(Mirrors[items].x_ordinate+(mirror_length/2)*cos((float)(Mirrors[items].angle*M_PI/180.0f)),
                      Mirrors[items].x_ordinate-(mirror_length/2)*cos((float)(Mirrors[items].angle*M_PI/180.0f)));
    double maxY = max(Mirrors[items].y_ordinate+(mirror_length/2)*sin((float)(Mirrors[items].angle*M_PI/180.0f)),
                      Mirrors[items].y_ordinate-(mirror_length/2)*sin((float)(Mirrors[items].angle*M_PI/180.0f)));

    if(bullet_x >= minX && bullet_x <= maxX && bullet_y >= minY && bullet_y <= maxY){
        if(abs((bullet_y-Mirrors[items].y_ordinate)-
            tan((float)(Mirrors[items].angle*M_PI/180.0f))*
            (bullet_x-Mirrors[items].x_ordinate)) <= 0.3){
                return items;
              }
            }
    }
  return -1;
}


void renderscore(double x, double y, int score)
{
  double x_ordinate = x, y_ordinate = y;
  glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
  glm::vec3 target (0, 0, 0);
  glm::vec3 up (0, 1, 0);
  Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane
  glm::mat4 VP = Matrices.projection * Matrices.view;
  glm::mat4 MVP;
  if(score == 0){
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 translator = glm::translate(glm::vec3(x_ordinate, y_ordinate, 1));
    Matrices.model *= translator;
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(create3DObject(GL_TRIANGLES, 6, digitopbar, darkyellow, GL_FILL));
    draw3DObject(create3DObject(GL_TRIANGLES, 6, digitlefttopbar, darkyellow, GL_FILL));
    draw3DObject(create3DObject(GL_TRIANGLES, 6, digitleftbotbar, darkyellow, GL_FILL));
    draw3DObject(create3DObject(GL_TRIANGLES, 6, digitbotbar, darkyellow, GL_FILL));
    draw3DObject(create3DObject(GL_TRIANGLES, 6, digitrightbotbar, darkyellow, GL_FILL));
    draw3DObject(create3DObject(GL_TRIANGLES, 6, digitrighttopbar, darkyellow, GL_FILL));
    return;
  }
  int tmp = score;
  while(tmp != 0){
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 translator = glm::translate(glm::vec3(x_ordinate, y_ordinate, 1));
    switch(tmp%10){
      case 0:
        Matrices.model *= translator;
        MVP = VP * Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(create3DObject(GL_TRIANGLES, 6, digitopbar, darkyellow, GL_FILL));
        draw3DObject(create3DObject(GL_TRIANGLES, 6, digitlefttopbar, darkyellow, GL_FILL));
        draw3DObject(create3DObject(GL_TRIANGLES, 6, digitleftbotbar, darkyellow, GL_FILL));
        draw3DObject(create3DObject(GL_TRIANGLES, 6, digitbotbar, darkyellow, GL_FILL));
        draw3DObject(create3DObject(GL_TRIANGLES, 6, digitrightbotbar, darkyellow, GL_FILL));
        draw3DObject(create3DObject(GL_TRIANGLES, 6, digitrighttopbar, darkyellow, GL_FILL));
        break;
      case 1:
        Matrices.model *= translator;
        MVP = VP * Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(create3DObject(GL_TRIANGLES, 6, digitrightbotbar, darkyellow, GL_FILL));
        draw3DObject(create3DObject(GL_TRIANGLES, 6, digitrighttopbar, darkyellow, GL_FILL));
          break;
      case 2:
        Matrices.model *= translator;
        MVP = VP * Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(create3DObject(GL_TRIANGLES, 6, digitopbar, darkyellow, GL_FILL));
        draw3DObject(create3DObject(GL_TRIANGLES, 6, digitrighttopbar, darkyellow, GL_FILL));
        draw3DObject(create3DObject(GL_TRIANGLES, 6, digitmidbar, darkyellow, GL_FILL));
        draw3DObject(create3DObject(GL_TRIANGLES, 6, digitleftbotbar, darkyellow, GL_FILL));
        draw3DObject(create3DObject(GL_TRIANGLES, 6, digitbotbar, darkyellow, GL_FILL));
          break;
      case 3:
        Matrices.model *= translator;
        MVP = VP * Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(create3DObject(GL_TRIANGLES, 6, digitopbar, darkyellow, GL_FILL));
        draw3DObject(create3DObject(GL_TRIANGLES, 6, digitbotbar, darkyellow, GL_FILL));
        draw3DObject(create3DObject(GL_TRIANGLES, 6, digitrightbotbar, darkyellow, GL_FILL));
        draw3DObject(create3DObject(GL_TRIANGLES, 6, digitrighttopbar, darkyellow, GL_FILL));
        draw3DObject(create3DObject(GL_TRIANGLES, 6, digitmidbar, darkyellow, GL_FILL));
        break;
      case 4:
        Matrices.model *= translator;
        MVP = VP * Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(create3DObject(GL_TRIANGLES, 6, digitlefttopbar, darkyellow, GL_FILL));
        draw3DObject(create3DObject(GL_TRIANGLES, 6, digitrightbotbar, darkyellow, GL_FILL));
        draw3DObject(create3DObject(GL_TRIANGLES, 6, digitrighttopbar, darkyellow, GL_FILL));
        draw3DObject(create3DObject(GL_TRIANGLES, 6, digitmidbar, darkyellow, GL_FILL));
        break;
      case 5:
        Matrices.model *= translator;
        MVP = VP * Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(create3DObject(GL_TRIANGLES, 6, digitopbar, darkyellow, GL_FILL));
        draw3DObject(create3DObject(GL_TRIANGLES, 6, digitlefttopbar, darkyellow, GL_FILL));
        draw3DObject(create3DObject(GL_TRIANGLES, 6, digitmidbar, darkyellow, GL_FILL));
        draw3DObject(create3DObject(GL_TRIANGLES, 6, digitbotbar, darkyellow, GL_FILL));
        draw3DObject(create3DObject(GL_TRIANGLES, 6, digitrightbotbar, darkyellow, GL_FILL));
        break;
      case 6:
        Matrices.model *= translator;
        MVP = VP * Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(create3DObject(GL_TRIANGLES, 6, digitopbar, darkyellow, GL_FILL));
        draw3DObject(create3DObject(GL_TRIANGLES, 6, digitlefttopbar, darkyellow, GL_FILL));
        draw3DObject(create3DObject(GL_TRIANGLES, 6, digitmidbar, darkyellow, GL_FILL));
        draw3DObject(create3DObject(GL_TRIANGLES, 6, digitleftbotbar, darkyellow, GL_FILL));
        draw3DObject(create3DObject(GL_TRIANGLES, 6, digitbotbar, darkyellow, GL_FILL));
        draw3DObject(create3DObject(GL_TRIANGLES, 6, digitrightbotbar, darkyellow, GL_FILL));
        break;
      case 7:
        Matrices.model *= translator;
        MVP = VP * Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(create3DObject(GL_TRIANGLES, 6, digitopbar, darkyellow, GL_FILL));
        draw3DObject(create3DObject(GL_TRIANGLES, 6, digitrightbotbar, darkyellow, GL_FILL));
        draw3DObject(create3DObject(GL_TRIANGLES, 6, digitrighttopbar, darkyellow, GL_FILL));
        break;
      case 8:
        Matrices.model *= translator;
        MVP = VP * Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(create3DObject(GL_TRIANGLES, 6, digitopbar, darkyellow, GL_FILL));
        draw3DObject(create3DObject(GL_TRIANGLES, 6, digitlefttopbar, darkyellow, GL_FILL));
        draw3DObject(create3DObject(GL_TRIANGLES, 6, digitleftbotbar, darkyellow, GL_FILL));
        draw3DObject(create3DObject(GL_TRIANGLES, 6, digitbotbar, darkyellow, GL_FILL));
        draw3DObject(create3DObject(GL_TRIANGLES, 6, digitrightbotbar, darkyellow, GL_FILL));
        draw3DObject(create3DObject(GL_TRIANGLES, 6, digitrighttopbar, darkyellow, GL_FILL));
        draw3DObject(create3DObject(GL_TRIANGLES, 6, digitmidbar, darkyellow, GL_FILL));
        break;
      case 9:
        Matrices.model *= translator;
        MVP = VP * Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(create3DObject(GL_TRIANGLES, 6, digitopbar, darkyellow, GL_FILL));
        draw3DObject(create3DObject(GL_TRIANGLES, 6, digitlefttopbar, darkyellow, GL_FILL));
        draw3DObject(create3DObject(GL_TRIANGLES, 6, digitbotbar, darkyellow, GL_FILL));
        draw3DObject(create3DObject(GL_TRIANGLES, 6, digitrightbotbar, darkyellow, GL_FILL));
        draw3DObject(create3DObject(GL_TRIANGLES, 6, digitrighttopbar, darkyellow, GL_FILL));
        draw3DObject(create3DObject(GL_TRIANGLES, 6, digitmidbar, darkyellow, GL_FILL));
        break;
    }
    tmp = tmp/10;
    x_ordinate -= 0.3;
  }
}

double past_time = 0.0;

void draw (GLFWwindow *window)
{
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glUseProgram (programID);
  get_selected_object(window, mousexpos, mouseypos);
  glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
  glm::vec3 target (0, 0, 0);
  glm::vec3 up (0, 1, 0);
  Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane
  glm::mat4 VP = Matrices.projection * Matrices.view;
  glm::mat4 MVP;

  //stars
  for(int items=0;items < 20;items++)
    Stars[items].render(stars[items]);

  // background
  Matrices.model = glm::mat4(1.0f);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(background);
  draw3DObject(createTriangle(-4, -4, 5, 4, grey));
  draw3DObject(createTriangle(-2, -4, 6, 5.5, grey));
  draw3DObject(createTriangle(2, -4, 3, 3, grey));
  draw3DObject(createTriangle(-6, -4, 5, 3.3, grey));

  //whitemoon
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translate_whitemoon = glm::translate(glm::vec3(-3.0, 3.3, 0));
  Matrices.model *= translate_whitemoon;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(whitemoon);

  //score
  renderscore(-3, 2, score);
  renderscore(3, 1, int(glfwGetTime()));

  // left_bin transformations
  //left bin bottom
  Left_bin_bot.rotator(85, glm::vec3(1, 0, 0));
  Left_bin_bot.render(left_bin_bot);
  // left bin body
  Left_bin_body.rotator(45);
  Left_bin_body.render(left_bin_body);
  //left bin top
  Left_bin_top.rotator(-80, glm::vec3(1, 0, 0));
  Left_bin_top.render(left_bin_top);

  // right_bin transformations
  //right bin bottom
  Right_bin_bot.rotator(85, glm::vec3(1, 0, 0));
  Right_bin_bot.render(right_bin_bot);
  //right bin body
  Right_bin_body.rotator(45);
  Right_bin_body.render(right_bin_body);
  //right bin top
  Right_bin_top.rotator(-80, glm::vec3(1, 0 ,0));
  Right_bin_top.render(right_bin_top);

  if(rand()%2){
    if(Mirrors[3].x_ordinate < 4)
      Mirrors[3].x_ordinate += 0.005;//(rand()%20)/10 - 1;
    if(Mirrors[3].y_ordinate < 4)
      Mirrors[3].y_ordinate += 0.003;
    }
  else{
    if(Mirrors[3].x_ordinate > 0)
      Mirrors[3].x_ordinate -= 0.004;
    if(Mirrors[3].y_ordinate > 0)
      Mirrors[3].y_ordinate -= 0.005;//(rand()%20)/10 - 1;
    }
  // Mirror Models
  for(int items=0;items < 4;items++){
    Mirrors[items].rotator(Mirrors[items].angle);
    Mirrors[items].render(mirrors[items]);
  }

  // canon body Tranformations
  // canon piston
  Canon_piston.rotator(Canon_piston.angle);
  Canon_piston.render(canon_piston);
  // canon body
  Canon_body.rotator(18);
  Canon_body.render(canon_body);
  //canon shoot
  if(shoot == 1) {
    if(fabs(Bullet.x_ordinate) >= 4 || fabs(Bullet.y_ordinate) >= 4 )
    {
      Bullet.x_ordinate = 3.6;
      Bullet.y_ordinate = Canon_body.y_ordinate;
      dis = 0;
      shoot = 0;
    }
    else {
      dis += 0.2;
      Bullet.x_ordinate = bullet_x_coordinate+dis*cos_theta;
      if(slope < 0)
        Bullet.y_ordinate = bullet_y_coordinate-dis*sin_theta;
      else
        Bullet.y_ordinate = bullet_y_coordinate+dis*sin_theta;
      Bullet.z_ordinate = 0;
      Bullet.render(bullet);
      if(checkCollison(Bullet, Bricks) != -1){
        shoot = 0;
        dis = 0;
        Bricks.erase(Bricks.begin()+checkCollison(Bullet, Bricks));
        curr_bricks--;
        Bullet.x_ordinate = 3.6;
        Bullet.y_ordinate = Canon_body.y_ordinate;
      }
      else if(checkMirrorCollison(Bullet.x_ordinate, Bullet.y_ordinate) != -1){
        int item = checkMirrorCollison(Bullet.x_ordinate, Bullet.y_ordinate);
        dis = 0;
        Bullet.angle = 2*Mirrors[item].angle - Bullet.angle;
        bullet_x_coordinate = Bullet.x_ordinate;
        bullet_y_coordinate = Bullet.y_ordinate;
        slope = tan((float)(Bullet.angle*M_PI/180.0f));
        cos_theta = cos((float)(Bullet.angle*M_PI/180.0f));
        sin_theta = sin((float)(Bullet.angle*M_PI/180.0f));
      }
      else{
        // DO NOTHING
      }
    }
  }

  //scoring
  for(int items = 0;items < curr_bricks;items++){
    if(Bricks[items].x_ordinate <= Left_bin_body.x_ordinate+0.7071&&
      Bricks[items].x_ordinate >= Left_bin_body.x_ordinate-0.7071&&
      Bricks[items].y_ordinate <= -2.5)
      {
      if(strcmp(Bricks[items].color,"red")){
        score += 5;
        Bricks.erase(Bricks.begin()+items);
        curr_bricks--;
        cout<<"Score: "<<score<<endl;
      }
      else if(strcmp(Bricks[items].color,"green")){
        score += 2;
        Bricks.erase(Bricks.begin()+items);
        curr_bricks--;
        cout<<"Score: "<<score<<endl;
      }
      else
        {
          Bricks.erase(Bricks.begin()+items);
          black_brick_flag = 1;
          curr_bricks--;
          return;
        }
    }
    else if(Bricks[items].x_ordinate <= Right_bin_body.x_ordinate+0.7071&&
            Bricks[items].x_ordinate >= Right_bin_body.x_ordinate-0.7071&&
            Bricks[items].y_ordinate <= -2.5){

      if(strcmp(Bricks[items].color,"red")){
        Bricks.erase(Bricks.begin()+items);
        curr_bricks--;
        score+=2;
        cout<<"Score: "<<score<<endl;
      }
      else if(strcmp(Bricks[items].color,"green")){
        score += 5;
        Bricks.erase(Bricks.begin()+items);
        curr_bricks--;
        cout<<"Score: "<<score<<endl;
      }
      else
        {
          Bricks.erase(Bricks.begin()+items);
          black_brick_flag = 1;
          curr_bricks--;
          return;
        }
    }
    else{
      continue;
    }
  }
  // falling block transformations
  for(int items = 0;items < curr_bricks;items++){
    Bricks[items].y_ordinate = Bricks[items].y_ordinate - fabs(0.01 + speed);
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 translate_bricks = glm::translate(glm::vec3(Bricks[items].x_ordinate, Bricks[items].y_ordinate, 0));
    Matrices.model *= translate_bricks;
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(Bricks[items].object);
  }

  if(glfwGetTime() - past_time > 0.4)
  {
    curr_bricks++;
    Bricks.push_back(bricks[curr_bricks]);
    past_time = glfwGetTime();
  }
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window; // window desciptor/handle
    const char *Game = "Laser Shooting";
    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        // exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

    if (!window) {
        glfwTerminate();
          // exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );

    /* --- register callbacks with GLFW --- */

    /* Register function to handle window resizes */
    /* With Retina display on Mac OS X GLFW's FramebufferSize
     is different from WindowSize */
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);
    glfwSetWindowTitle(window, Game);
    /* Register function to handle window close */
    glfwSetWindowCloseCallback(window, quit);

    /* Register function to handle keyboard input */
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

    /* Register function to handle mouse click */
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks
    glfwSetScrollCallback(window, mouseScroll);
    return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
  /* Objects should be created before any other gl function and shaders */
	// Create the models
  createBackground();

  // Bin Models
  left_bin_bot = createPolygon(0.7071,360,red);
  left_bin_top = createPolygon(0.7071,360,black);
  left_bin_body = createPolygon(1.0,4,red);
  right_bin_body = createPolygon(1.0,4,green);
  right_bin_top = createPolygon(0.7071,360,black);
  right_bin_bot = createPolygon(0.7071,360,green);
  whitemoon = createPolygon(0.6, 360, white);

  // Canon Models
  createCanonShooter();
  canon_body = createPolygon(0.4,10,piston);
  bullet = createPolygon(0.08, 6, red);
  for(int items = 0;items < 10000;items++){
    int tmp = rand()%3;
    if(tmp == 0){
      GraphicalObjectColor obj = {
                                  -2.5 + (rand()%50)/10.0,
                                  4.5,
                                  0,
                                  createPolygon(0.08, 6, red),
                                  "red"
                                };
      bricks.push_back(obj);
    }
    else if(tmp == 1){
      GraphicalObjectColor obj = {
                                -2.5 + (rand()%50)/10.0,
                                4.5,
                                0,
                                createPolygon(0.08, 6, green),
                                "green"
                              };
      bricks.push_back(obj);
    }
    else{
      GraphicalObjectColor obj = {
                              -2.5 + (rand()%50)/10.0,
                              4.5,
                              0,
                              createPolygon(0.08, 6, ocean),
                              "ocean"
                            };
      bricks.push_back(obj);
    }
  }

  // Mirror Models
  for(int items = 0;items < 4;items++)
    mirrors.push_back(createMirror(mirror_length, 0.3));
  GraphicalObject temp_mirror1(3, 3, 0, -45, 'R');
  Mirrors.push_back(temp_mirror1);
  GraphicalObject temp_mirror2(2.8, 0, 0, -80, 'R');
  Mirrors.push_back(temp_mirror2);
  GraphicalObject temp_mirror3(-2, 1.5, 0, 20, 'R');
  Mirrors.push_back(temp_mirror3);
  GraphicalObject temp_mirror4(2 ,2 , 0, -45, 'R');
  Mirrors.push_back(temp_mirror4);
  //Stars
  for(int items=0;items < 20; items++){
    stars.push_back(createPolygon(0.05, 5, yellow));
    GraphicalObject temp(2*((rand()%30)/10.0f)-3, 2*(rand()%40/10.0f)-4, -1, 0, 'Y');
    Stars.push_back(temp);
  }

  // Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );

	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");
	reshapeWindow (window, width, height);

  // Background color of the scene
	glClearColor (0.3f, 0.3f, 0.3f, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);
  cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
  cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
  cout << "VERSION: " << glGetString(GL_VERSION) << endl;
  cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
  GLFWwindow* window = initGLFW(WIDTH, HEIGHT);
	initGL (window, WIDTH, HEIGHT);
  glfwSetTime(0.0);
  while (!glfwWindowShouldClose(window) &&
          black_brick_flag == 0&&
        glfwGetTime() <= 60.0)
  {
      draw(window);
      // Swap Frame Buffer in double buffering
      glfwSwapBuffers(window);
      // Poll for Keyboard and mouse events
      glfwPollEvents();
  }
  glfwTerminate();
  cout<<"GAME OVER"<<endl;
  return 0;
}
