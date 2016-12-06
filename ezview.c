#define GLFW_DLL 1

#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLFW/glfw3.h>

#include "linmath.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#define PI 3.14159265
float rotate_value = 0;
float scale_value = 1;
float translate_x =0;
float translate_y =0;
float translate_z =0;
float shear_value = 0;
float x_ratio = 0;



typedef struct PPMpixel{
    //Test is '\n' at beginning of each line of rgb
    unsigned char r,g,b;
    } PPMpixel;

typedef struct PPMimage {
    int width, height;
    int input_filetype;
    PPMpixel *buffer;
    } PPMimage;

PPMimage *image;
//Definition for max colors, and also for buffer size usage
#define MAX_COLORS 255
//Function to convert ASCII to Binary Bits and Binary Bits to ASCII
int ppm_convert(int output_type, int input_filetype);
//Function to reset my temp buffer that I use to put ascii characters into image buffer
void *memset(void *str, int c, size_t n);
//Function I use to help grab all ascii characters but not spaces
int isspace(int c);

//Read input_file and put file type, image dimensions, and pixel data into image struct.
int ppm_read(char *input_file){
    FILE *fh;
    int max_color, width, height;
    int tracker = 0;
    int c, i,j,k;
    char temp_buffer[4];

    //Try to open as a P3 File, if unable try to open as a P6 file
    fh = fopen(input_file, "r");
    if(fh == 0){
       fh = fopen(input_file, "rb");
    }
    //If fh == 0 then file did not open, error out
    if(fh == 0){
        fclose(fh);
        fprintf(stderr, "Error: Unable to open file '%s' \n", input_file);
        exit(1);
    }
    //Check the first character of the opened file to make sure it is a PPM file, otherwise error out
    c = fgetc(fh);
    if (c!= 'P'){
        fprintf(stderr, "Error: This is not a PPM file. \n");
        exit(1);
    }
    ungetc(c, fh);
    //Allocate memory for image
    image = (PPMimage*)malloc(sizeof(PPMimage));
    if(image == 0){
        fprintf(stderr, "Unable to allocate memory \n");
        exit(1);
    }
    c = fgetc(fh);
    c = fgetc(fh);
    //Put original Input File type # into struct for convert reference
    image->input_filetype = c;
    //Skip over magic Number
    while((c = fgetc(fh)) != '\n'){
    }
    //Skip over the Comments
    c = fgetc(fh);
    while (c=='#'){
        while (fgetc(fh) != '\n');
            c=fgetc(fh);
    }
    ungetc(c, fh);
    //Get image dimensions
    if(fscanf(fh, "%d %d", &image->width, &image->height) !=2){
        fprintf(stderr, "A width or Height image dimensions is missing in the file.");
        exit(1);
    }
    width = image->width;
    height = image->height;
    //Scan next element, which is image max color, compare with 255, max image color for 8-bit pictures
    fscanf(fh, "%d", &max_color);
    if(max_color != MAX_COLORS){
        fprintf(stderr, "'%s' is not formatted into 8-bit color, ie max colors of %d", input_file, MAX_COLORS);
        exit(1);
    }
    //Code allocate room for pixel data
    image->buffer = (PPMpixel*)malloc((MAX_COLORS+1)*width*height);
    if(image->buffer == 0){
        fprintf(stderr, "Error: Memory could not be allocated.");
        exit(1);
    }
    //Remove unnecessary character still in file
    fgetc(fh);
    if(image->input_filetype == '3'){
        j=0;
        i=0;
        //HELP, this is bugging out for outputing P3 image
        while ((c = fgetc(fh)) != EOF){
            //If character I grab is a space, then I have a full rgb component, add to buffer
            if(isspace(c)){

                //Get rid of extra spaces in case there is more than one space between each pixel data
                while(isspace(c = fgetc(fh))){
                }
                ungetc(c, fh);
                //Convert Pixel data to an int
                k = atoi(temp_buffer);
                //Check if pixel data is too large
                if(k > MAX_COLORS){
                    fprintf(stderr, "Some pixel data is larger than Max color size allowed.");
                    exit(1);
                }
                //Put pixel data into corresponding RGB
                tracker++;
                if(tracker == 1){
                    image->buffer[j].r = k;
                }
                if(tracker == 2){
                    image->buffer[j].g = k;
                }
                if(tracker == 3){
                    image->buffer[j].b = k;
                    tracker = 0;
                }
                j++;
                i = 0;
                //Empty out temp buffer since some numbers may only be 1 digit or 2 digit instead of max 3
                memset(temp_buffer, '\0', sizeof(temp_buffer));
            }
            else{
                //Put part of full number into temp_buffer until I grab entire number
                temp_buffer[i++] = c;
            }
        }
    ungetc(c, fh);
    }
    //If a P6 file, read the entire thing into buffer, need to fix minor bug if possible
    if(image->input_filetype == '6'){
        fread(image->buffer, (sizeof(&image->buffer)), height*width, fh);
    }
    fclose(fh);
    return 0;
}


typedef struct {
  float Position[2];
  float TexCoord[2];
} Vertex;

// (-1, 1)  (1, 1)
// (-1, -1) (1, -1)

Vertex vertexes[] = {
  {{-1, 1}, {0, 0}}, //Bottom Left
  {{1, 1},  {1, 0}},  //Bottom Right
  {{-1, -1},  {0, 1}},  //Top Left

  {{1, 1}, {1, 0}}, //Bottom Right
  {{1, -1},  {1, 1}}, //Top Right
  {{-1, -1},  {0, 1}} //Top Left

};

const GLubyte Indices[] = {
  0, 1, 2,
  2, 3, 0
};

static const char* vertex_shader_text =
"uniform mat4 MVP;\n"
"attribute vec2 TexCoordIn;\n"
"attribute vec2 vPos;\n"
"varying vec2 TexCoordOut;\n"
"void main()\n"
"{\n"
"    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
"    TexCoordOut = TexCoordIn;\n"
"}\n";

static const char* fragment_shader_text =
"varying lowp vec2 TexCoordOut;\n"
"uniform sampler2D Texture;\n"
"void main()\n"
"{\n"
"    gl_FragColor = texture2D(Texture, TexCoordOut);\n"
"}\n";

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
        glfwSetWindowShouldClose(window, GLFW_TRUE);}
    if (key == GLFW_KEY_A && action == GLFW_PRESS){
      //printf("Rotate Left\n");
      rotate_value += PI/2;
      }
    if (key == GLFW_KEY_D && action == GLFW_PRESS){
      //printf("Rotate Right\n");
      rotate_value -= PI/2;
      }
    if (key == GLFW_KEY_W && action == GLFW_PRESS){
      //printf("Scale Up\n");
      scale_value += 0.3;
      }
    if (key == GLFW_KEY_S && action == GLFW_PRESS){
      //printf("Scale Down\n");
      scale_value -= 0.3;
      }
    if (key == GLFW_KEY_Z && action == GLFW_PRESS){
      //printf("Shear Left\n");
      shear_value -= 0.3;
      }
    if (key == GLFW_KEY_X && action == GLFW_PRESS){
      //printf("Shear Right\n");
      shear_value +=0.3;
      }
    if (key == GLFW_KEY_Q && action == GLFW_PRESS){
      //printf("Translate Left\n");
      translate_x -= 0.3;
      //translate_y += 1;
      //translate_z += 0.3;
      }
    if (key == GLFW_KEY_E && action == GLFW_PRESS){
      //printf("Translate Right\n");
      translate_x += 0.3;
      }
}

void glCompileShaderOrDie(GLuint shader) {
  GLint compiled;
  glCompileShader(shader);
  glGetShaderiv(shader,
		GL_COMPILE_STATUS,
		&compiled);
  if (!compiled) {
    GLint infoLen = 0;
    glGetShaderiv(shader,
		  GL_INFO_LOG_LENGTH,
		  &infoLen);
    char* info = malloc(infoLen+1);
    GLint done;
    glGetShaderInfoLog(shader, infoLen, &done, info);
    printf("Unable to compile shader: %s\n", info);
    exit(1);
  }
}

int main(int argc, char *argv[]){
  if(argc != 2){
    fprintf(stderr, "Error: Not enough arguments need image file name. \n");
    exit(1);
  }
  char *input_file = argv[1];
  ppm_read(input_file);
  //printf("image width: %d, image height: %d, input type: %d", image->width, image->height, image->input_filetype);
  //Set Aspect Ratio of Triangles for rectangular images
  x_ratio = (float)image->width/(float)image->height;
  vertexes[0].Position[0]*= x_ratio;
  vertexes[1].Position[0]*= x_ratio;
  vertexes[2].Position[0]*= x_ratio;
  vertexes[3].Position[0]*= x_ratio;
  vertexes[4].Position[0]*= x_ratio;
  vertexes[5].Position[0]*= x_ratio;
  //Aspect Ration Done


  GLFWwindow* window;
  GLuint vertex_buffer, vertex_shader, fragment_shader, program, index_buffer;
  GLint mvp_location, vpos_location, vcol_location;
  glfwSetErrorCallback(error_callback);

  if (!glfwInit())
      exit(EXIT_FAILURE);

      glfwDefaultWindowHints();
      glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
      glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
      glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
      glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

  window = glfwCreateWindow(850, 480, "Simple example", NULL, NULL);
  if (!window)
  {
      glfwTerminate();
      exit(EXIT_FAILURE);
  }

  glfwSetKeyCallback(window, key_callback);

  glfwMakeContextCurrent(window);
  // gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
  glfwSwapInterval(1);

  // NOTE: OpenGL error checks have been omitted for brevity

  glGenBuffers(1, &vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertexes), vertexes, GL_STATIC_DRAW);

  glGenBuffers(1, &index_buffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);


  vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
  glCompileShaderOrDie(vertex_shader);

  fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
  glCompileShaderOrDie(fragment_shader);

  program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);
  // more error checking! glLinkProgramOrDie!

  mvp_location = glGetUniformLocation(program, "MVP");
  assert(mvp_location != -1);

  vpos_location = glGetAttribLocation(program, "vPos");
  assert(vpos_location != -1);

  GLint texcoord_location = glGetAttribLocation(program, "TexCoordIn");
  assert(texcoord_location != -1);

  GLint tex_location = glGetUniformLocation(program, "Texture");
  assert(tex_location != -1);

  glEnableVertexAttribArray(vpos_location);
  glVertexAttribPointer(vpos_location,
  	  2,
  	  GL_FLOAT,
  	  GL_FALSE,
                        sizeof(Vertex),
  	  (void*) 0);

  glEnableVertexAttribArray(texcoord_location);
  glVertexAttribPointer(texcoord_location,
  	  2,
  	  GL_FLOAT,
  	  GL_FALSE,
                        sizeof(Vertex),
  	  (void*) (sizeof(float) * 2));

  GLuint texID;
  glGenTextures(1, &texID);
  glBindTexture(GL_TEXTURE_2D, texID);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image->width, image->height, 0, GL_RGB,
   GL_UNSIGNED_BYTE, image->buffer);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texID);
  glUniform1i(tex_location, 0);

//HELP How to adjust ratio of triangle for rectangular images?



  while (!glfwWindowShouldClose(window))
  {
      float ratio;
      x_ratio = (float)image->width/(float)image->height;
      int width, height;
      mat4x4 m, p, mvp;
      mat4x4 rm, tm, sm,shm, arm;

      glfwGetFramebufferSize(window, &width, &height);
      ratio = width / (float) height;

      glViewport(0, 0, width, height);
      glClear(GL_COLOR_BUFFER_BIT);

      mat4x4_identity(m); //affine matrix
      mat4x4_identity(rm);  //Rotation Matrix
      mat4x4_identity(tm);  //Translate Matrix
      mat4x4_identity(sm);  //Scale Matrix
      mat4x4_identity(shm); //Shear Matrix
      mat4x4_identity(arm);

      //Calculating Values for matrix manipulation
      mat4x4_rotate_Z(rm, rm, rotate_value);
      mat4x4_translate(tm,translate_x,translate_y,translate_z);
      mat4x4_scale_aniso(sm, sm, scale_value, scale_value, scale_value);
      mat4x4_shear(shm, shm, shear_value, shear_value);

      //Creating a Single Affine Matrix
      mat4x4_add(m,tm,m);
      mat4x4_add(m,arm,m);
      mat4x4_add(m,sm,m);
      mat4x4_add(m,shm,m);
      mat4x4_mul(m,rm,m);

      mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
      mat4x4_mul(mvp, p, m);

      glUseProgram(program);
      glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) mvp);

      //glDrawElements(GL_TRIANGLES,sizeof(Indices) / sizeof(GLubyte),GL_UNSIGNED_BYTE, (void*) Indices);
      glDrawArrays(GL_TRIANGLES, 0, 6);

      glfwSwapBuffers(window);
      glfwPollEvents();
  }

  glfwDestroyWindow(window);

  glfwTerminate();
  exit(EXIT_SUCCESS);
}

//! [code]
