#define GLFW_DLL 1

#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>


GLFWwindow* window;


typedef struct {
  float position[3];
  float color[4];
  float texcoord[2];
} Vertex;


const Vertex Vertices[] = {
  {{1, -1, 0}, {1, 0, 0, 1}, {1,0}}, //Bottom Right
  {{1, 1, 0}, {0, 1, 0, 1}, {1,1}},  //Top Right
  {{-1, 1, 0}, {0, 0, 1, 1}, {0,1}}, //Top Left
  {{-1, -1, 0}, {0, 0, 0, 1}, {0,0}} //Bottom Left
};


const GLubyte Indices[] = {
  0, 1, 2,
  2, 3, 0
};
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
        //fread(image->buffer, (sizeof(&image->buffer)), height*width, fh);
        //printf("%d\n", fgetc(fh));
        j=0;
        tracker=0;
        while((c = fgetc(fh)) != EOF){
            //If character I grab is a space, iterate until I no longer have a space
            if(isspace(c)){
                //Get rid of extra spaces in case there is more than one space between each pixel data
                while(isspace(c = fgetc(fh))){
                }
                ungetc(c, fh);
            }
            else{
                if((int)c > MAX_COLORS){
                    fprintf(stderr, "Some pixel data is larger than Max color size allowed.");
                    exit(1);
                    }
                if(tracker == 0){
                    image->buffer[j].r = c;
                    tracker++;
                }
                else if(tracker == 1){
                    image->buffer[j].g = c;
                    tracker++;
                }
                else{
                    image->buffer[j].b = c;
                    tracker = 0;
                    j++;
                    }
            }
            //printf("C is %d\n", (int)c);
        }
    ungetc(c, fh);
    }
    fclose(fh);
    return 0;
}


char* vertex_shader_src =
  "attribute vec4 Position;\n"
  "attribute vec4 SourceColor;\n"
  "\n"
  "varying vec4 DestinationColor;\n"
  "\n"
  "void main(void) {\n"
  "    DestinationColor = SourceColor;\n"
  "    gl_Position = Position;\n"
  "}\n";


char* fragment_shader_src =
  "varying lowp vec4 DestinationColor;\n"
  "\n"
  "void main(void) {\n"
  "    gl_FragColor = DestinationColor;\n"
  "}\n";


GLint simple_shader(GLint shader_type, char* shader_src) {

  GLint compile_success = 0;

  int shader_id = glCreateShader(shader_type);

  glShaderSource(shader_id, 1, &shader_src, 0);

  glCompileShader(shader_id);

  glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compile_success);

  if (compile_success == GL_FALSE) {
    GLchar message[256];
    glGetShaderInfoLog(shader_id, sizeof(message), 0, &message[0]);
    printf("glCompileShader Error: %s\n", message);
    exit(1);
  }

  return shader_id;
}


int simple_program() {

  GLint link_success = 0;

  GLint program_id = glCreateProgram();
  GLint vertex_shader = simple_shader(GL_VERTEX_SHADER, vertex_shader_src);
  GLint fragment_shader = simple_shader(GL_FRAGMENT_SHADER, fragment_shader_src);

  glAttachShader(program_id, vertex_shader);
  glAttachShader(program_id, fragment_shader);

  glLinkProgram(program_id);

  glGetProgramiv(program_id, GL_LINK_STATUS, &link_success);

  if (link_success == GL_FALSE) {
    GLchar message[256];
    glGetProgramInfoLog(program_id, sizeof(message), 0, &message[0]);
    printf("glLinkProgram Error: %s\n", message);
    exit(1);
  }

  return program_id;
}


static void error_callback(int error, const char* description) {
  fputs(description, stderr);
}


int main(int argc, char *argv[]) {
  if(argc != 2){
      fprintf(stderr, "Error: Not enough arguments need image file name. \n");
  }
  char *input_file = argv[1];
  ppm_read(input_file);

  GLint program_id, position_slot, color_slot;
  GLuint vertex_buffer;
  GLuint index_buffer;

  glfwSetErrorCallback(error_callback);

  // Initialize GLFW library
  if (!glfwInit())
    return -1;

  glfwDefaultWindowHints();
  glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

  // Create and open a window
  window = glfwCreateWindow(image->width,
                            image->height,
                            "Hello World",
                            NULL,
                            NULL);

  if (!window) {
    glfwTerminate();
    printf("glfwCreateWindow Error\n");
    exit(1);
  }

  glfwMakeContextCurrent(window);

  program_id = simple_program();
  glUseProgram(program_id);

//Loading my Image
  GLuint myTexture;
  glGenTextures(1, &myTexture);
  glBindTexture(GL_TEXTURE_2D, myTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, //No LOD
  				GL_RGBA, //FORMAT... GL_RGBA
  				image->width,
  				image->height,
  				0, //No Border
  				GL_RGBA,
  				MAX_COLORS, //whatever your numeric representation is...
  				image->buffer);
          printf("Image loaded successfully?");
//End Loading my Image


  position_slot = glGetAttribLocation(program_id, "Position");
  color_slot = glGetAttribLocation(program_id, "SourceColor");
  glEnableVertexAttribArray(position_slot);
  glEnableVertexAttribArray(color_slot);

  // Create Buffer
  glGenBuffers(1, &vertex_buffer);

  // Map GL_ARRAY_BUFFER to this buffer
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

  // Send the data
  glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

  glGenBuffers(1, &index_buffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);

  // Repeat
  while (!glfwWindowShouldClose(window)) {

    glClearColor(0, 104.0/255.0, 55.0/255.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glViewport(0, 0, image->width, image->height);

    glVertexAttribPointer(position_slot,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(Vertex),
                          0);

    glVertexAttribPointer(color_slot,
                          4,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(Vertex),
                          (GLvoid*) (sizeof(float) * 3));

    glDrawElements(GL_TRIANGLES,
                   sizeof(Indices) / sizeof(GLubyte),
                   GL_UNSIGNED_BYTE, 0);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}
