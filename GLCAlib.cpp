///\file GLCAlib.cpp
///\brief GPGPU-based Cellular Automata Library.
///
///Implements in GPU a framwork for Cellular Automata computing

//includes
#include <iostream>
#include <fstream>
#include <GL/glew.h>
#include <GL/freeglut.h>

using namespace std;
namespace GLCAlib {
void initGLEW(void);
void initFBO(void);
void initGLSL(void);

bool checkFramebufferStatus(void);
void checkGLErrors(const char *label);
void printInfoLog(GLhandleARB obj);

void setupTexture (const GLuint texID);
void createTextures(void);
void transferToTexture(float* image, GLuint texID);
void transferFromTexture(float* data);

void run(void);
void swap(void);

void display();
void reshape(int width, int height);

///The data matrix (Texture)
float* data;
///Width of the matrix
int texSize_x;
///Height of the matrix
int texSize_y;
///Size of the image (4*x*y being RGBA)
int N;
///If TRUE displays the Automata evolution
bool withgui;
///number of iterations required
int numIterations=0;
long countIterations=0;

///timing vars
time_t start, end;
/////needed for real-time performance extimation
//long lastc, lasti;

///number of iterations between 2 successive visualizations\n
///lower is smoother and slower.
int refrashrate=1;

///texture identifiers
GLuint TexID_A[2];

///\brief ping-pong management vars
///In the shader, textures are  alternatively read-only and write-only
int writeTex = 0;
int readTex = 1;
GLenum attachmentpoints[] = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT };

///GLSL vars
GLhandleARB programObject;
GLhandleARB shaderObject;
GLint Param_A;

///FBO identifier
GLuint fb;

///handle the (eventually offscreen) window
GLuint glutWindowHandle;

///struct for variable parts of GL calls (texture format, float format etc)
struct struct_textureParameters {
    char* name;
    GLenum texTarget;
    GLenum texInternalFormat;
    GLenum texFormat;
    char* shader_source;
}
textureParameters;

///\brief Initialize OpenGL and executes the given shader
///@param[in] arc: number of parameters on the commend line\n
///@param[in] argv: holds parameters passed on the commend line\n
///@param[in] image: buffer containing the input data\n
///@param[in] x: width of the input
///@param[in] y: height of the input
///@param[in] shader: the program executed on the GPU
///@param[in] gui: if TRUE, visualizes the computation evolution
///@param[in] iterations: length of the computation in generations
void init(int argc, char** argv, float* image, int x, int y, char* shader, bool gui=true, int iterations=0) {
    //cerr<<"main"<<endl;
    textureParameters.name				= "TEXRECT - float_ARB - RGBA - 32";
    textureParameters.texTarget			= GL_TEXTURE_RECTANGLE_ARB;
    textureParameters.texInternalFormat	= GL_RGBA32F_ARB;
    textureParameters.texFormat			= GL_RGBA;
    textureParameters.shader_source		= shader;

	//cerr<<"assign parameters to global variables"<<endl;
    data=image;
    texSize_x=x;
    texSize_y=y;
    N=4*texSize_x*texSize_y;
    numIterations=iterations;
    withgui=gui;

    //cerr<<"calc texture dimensions"<<endl;
    cout<<textureParameters.name<<", x="<<texSize_x<<", y="<<texSize_y<<", numIter="<<numIterations<<endl;

    //cerr<<"init glut and glew"<<endl;
    glutInit (&argc, argv);
    if (withgui) {
        //cerr<<"loading GUI"<<endl;
        glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);
        glutDisplayFunc(display);
        glutReshapeFunc(reshape);
        glutIdleFunc(run);
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glutInitWindowSize(texSize_x, texSize_y);
    }
    glutWindowHandle = glutCreateWindow(argv[0]);

    initGLEW();

    //cerr<<"init offscreen framebuffer"<<endl;
    initFBO();

    //cerr<<"create textures for vectors"<<endl;
    createTextures();

    //cerr<<"init shader runtime"<<endl;
    initGLSL();

    //cerr<<"init textures"<<endl;
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, attachmentpoints[writeTex], textureParameters.texTarget, TexID_A[writeTex], 0);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, attachmentpoints[readTex], textureParameters.texTarget, TexID_A[readTex], 0);
    if (!checkFramebufferStatus()) {
        cout<<"glFramebufferTexture2DEXT():\t [FAIL]"<<endl;
        exit (1);
    } else {
        //cerr<<"glFramebufferTexture2DEXT():\t //[PASS]"<<endl;
    }

    // enable GLSL program
    glUseProgramObjectARB(programObject);

    //START MAIN COMPUTATION
    start = time(NULL);
    if (withgui){
        glutMainLoop();
	} else
        while (countIterations++!=numIterations) run();
    end = time (NULL);
    time_t total = end-start;

    //transfer the data back
    transferFromTexture(data);

    //cerr<<"calc and print Iterations/sec"<<endl;
    if (total>0) cout<<"GPU Iterations/sec: "<<countIterations/total<<endl;

    //cerr<<"clean up"<<endl;
    glFinish();
	//cerr<<"DeleteFramebuffer"<<endl;
    glDeleteFramebuffersEXT(1, &fb);
	//cerr<<"DeleteTextures"<<endl;
    glDeleteTextures(2, TexID_A);
}

///Sets up a floating point texture with NEAREST filtering.
///(mipmaps etc. are unsupported for floating point textures)
void setupTexture (const GLuint texID) {
    //cerr<<"Inside setupTexture"<<endl;
    //cerr<<"make active and bind"<<endl;
    glBindTexture(textureParameters.texTarget,texID);
    //cerr<<"turn off filtering and wrap modes"<<endl;
    glTexParameteri(textureParameters.texTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(textureParameters.texTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(textureParameters.texTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(textureParameters.texTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    //cerr<<"define texture with floating point format"<<endl;
    glTexImage2D(textureParameters.texTarget,0,textureParameters.texInternalFormat,texSize_x,texSize_y,0,textureParameters.texFormat,GL_FLOAT,0);
    //cerr<<"check if that worked"<<endl;
    if (glGetError() != GL_NO_ERROR) {
        cout<<"glTexImage2D():\t\t\t [FAIL]"<<endl;
        exit (1);
    } else {
        //cerr<<"glTexImage2D():\t\t\t [PASS]"<<endl;
    }
    //cerr<<"Created a "<<texSize_x<<"by "<<texSize_y<<" floating point texture."<<endl;
}


///creates textures, sets proper viewport etc.
void createTextures (void) {
    //cerr<<"Inside createTexture"<<endl;
    //cerr<<"two textures, alternatingly read-only and write-only,"<<endl;
    glGenTextures (2, TexID_A);
    //cerr<<"setup textures"<<endl;
    setupTexture (TexID_A[readTex]);
    transferToTexture(data,TexID_A[readTex]);
    setupTexture (TexID_A[writeTex]);
    transferToTexture(data,TexID_A[writeTex]);
    //cerr<<"set texenv mode from modulate (the default) to replace)"<<endl;
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    //cerr<<"check if something went completely wrong"<<endl;
    checkGLErrors ("createFBOandTextures()");
}

///Transfers data to texture.
///Check web page for detailed explanation on the difference between ATI and NVIDIA.
void transferToTexture (float* data, GLuint texID) {
    //cerr<<"Inside transferToTexture"<<endl;
    // version (a): HW-accelerated on NVIDIA
//	glBindTexture(textureParameters.texTarget, texID);
//	glTexSubImage2D(textureParameters.texTarget,0,0,0,texSize_x,texSize_y,textureParameters.texFormat,GL_FLOAT,data);
    // version (b): HW-accelerated on ATI
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, textureParameters.texTarget, texID, 0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
    glRasterPos2i(0,0);
    glDrawPixels(texSize_x,texSize_y,textureParameters.texFormat,GL_FLOAT,data);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, textureParameters.texTarget, 0, 0);
}

///Transfers data from current texture, and stores it in given array.
void transferFromTexture(float* data) {
    //cerr<<"Inside transferFromTexture"<<endl;
    glReadBuffer(attachmentpoints[readTex]);
    glReadPixels(0, 0, texSize_x, texSize_y, textureParameters.texFormat, GL_FLOAT, data);
}

///Sets up GLEW to initialise OpenGL extensions
void initGLEW (void) {
    //cerr<<"Inside initGLEW"<<endl;
    int err = glewInit();
    //cerr<<"sanity check"<<endl;
    if (GLEW_OK != err) {
        cout<<(char*)glewGetErrorString(err)<<endl;
        exit(1);
    }
}

///Creates framebuffer object, binds it to reroute rendering operations
///from the traditional framebuffer to the offscreen buffer
void initFBO() {
    //cerr<<"Inside initFBO"<<endl;
    //cerr<<"create FBO (off-screen framebuffer)"<<endl;
    glGenFramebuffersEXT(1, &fb);
    //cerr<<"bind offscreen framebuffer (that is, skip the window-specific render target)"<<endl;
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
    //cerr<<"viewport for 1:1 pixel=texture mapping"<<endl;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, texSize_x, 0.0, texSize_y);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glViewport(0, 0, texSize_x, texSize_y);
}

///Sets up the GLSL runtime and creates shader.
void initGLSL(void) {
    //cerr<<"Inside initGLSL"<<endl;
    //cerr<<"create program object"<<endl;
    programObject = glCreateProgramObjectARB();
    //programObjectDisp = glCreateProgramObjectARB();
    //cerr<<"create shader object (fragment shader) and attach to program"<<endl;
    shaderObject = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
    glAttachObjectARB (programObject, shaderObject);
    //cerr<<"set source to shader object"<<endl;
    const GLcharARB* source = textureParameters.shader_source;
    glShaderSourceARB(shaderObject, 1, &source, NULL);
    //cerr<<"compile and print compilation errors (no need to do additional error"<<endl;
    //cerr<<"checking, glLinkProgramARB() fails if compilation was wrong)"<<endl;
    glCompileShaderARB(shaderObject);
    printInfoLog(shaderObject);
    //cerr<<"link program object together and check for errors"<<endl;
    GLint success;
    glLinkProgramARB(programObject);
    glGetObjectParameterivARB(programObject, GL_OBJECT_LINK_STATUS_ARB, &success);
    if (!success) {
        //cerr<<"Shader could not be linked!"<<endl;
        exit (1);
    }

    // Get location of the texture samplers for future use
    Param_A = glGetUniformLocationARB(programObject, "texture_A");
}

///Performs the actual calculation.
void run(void) {
    //cerr<<"Inside run"<<endl;
    // set render destination
    glDrawBuffer (attachmentpoints[writeTex]);
    // enable texture (read-only)
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(textureParameters.texTarget,TexID_A[readTex]);
    glUniform1iARB(Param_A,0); // texunit 0

    // render the quad with unnormalized texcoords
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0);
    glVertex2f(0.0, 0.0);
    glTexCoord2f(texSize_x, 0.0);
    glVertex2f(texSize_x, 0.0);
    glTexCoord2f(texSize_x, texSize_y);
    glVertex2f(texSize_x, texSize_y);
    glTexCoord2f(0.0, texSize_y);
    glVertex2f(0.0, texSize_y);
    glEnd();

    // swap role of the two textures (read-only source becomes
    // write-only target and the other way round):
    swap();

    if ((countIterations%refrashrate)==0) display();
    //cerr<<refrashrate<<endl;

    //if (clock()-lastc>CLOCKS_PER_SEC) {
        //float speed=(countIterations-lasti)*CLOCKS_PER_SEC/(clock()-lastc);
        //cerr<<"\nsimulation speed: "<<speed<<"hz\nrefrash:"<<speed/refrashrate<<"fps (1/"<<refrashrate<<")\n";
        //lasti=countIterations;
        //lastc=clock();
    //}
    if (withgui)
        if (countIterations++==numIterations) glutLeaveMainLoop();
}

///Checks for OpenGL errors.
///Extremely useful debugging function: When developing, make sure to call this after almost every GL call.
void checkGLErrors (const char *label) {
    //cerr<<"Inside checkGLErrors"<<endl;
    GLenum errCode;
    const GLubyte *errStr;

    if ((errCode = glGetError()) != GL_NO_ERROR) {
        errStr = gluErrorString(errCode);
        cout<<"OpenGL ERROR: "<<(char*)errStr<<"(Label: "<<label<<")"<<endl;
    }
}

///Checks framebuffer status.
///Copied directly out of the spec, modified to deliver a return value.
bool checkFramebufferStatus() {
    //cerr<<"Inside checkFramebufferStatus"<<endl;
    GLenum status;
    status = (GLenum) glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    switch(status) {
    case GL_FRAMEBUFFER_COMPLETE_EXT:
        return true;
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
        cout<<"Framebuffer incomplete, incomplete attachment"<<endl;
        return false;
    case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
        cout<<"Unsupported framebuffer format"<<endl;
        return false;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
        cout<<"Framebuffer incomplete, missing attachment"<<endl;
        return false;
    case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
        cout<<"Framebuffer incomplete, attached images must have same dimensions"<<endl;
        return false;
    case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
        cout<<"Framebuffer incomplete, attached images must have same format"<<endl;
        return false;
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
        cout<<"Framebuffer incomplete, missing draw buffer"<<endl;
        return false;
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
        cout<<"Framebuffer incomplete, missing read buffer"<<endl;
        return false;
    }
    return false;
}

///copied from http://www.lighthouse3d.com/opengl/glsl/index.php?oglinfo
void printInfoLog(GLhandleARB obj) {
    //cerr<<"Inside printInfoLog"<<endl;
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;
    glGetObjectParameterivARB(obj, GL_OBJECT_INFO_LOG_LENGTH_ARB, &infologLength);
    if (infologLength > 1) {
        infoLog=(char *)malloc(infologLength);
        glGetInfoLogARB(obj, infologLength, &charsWritten, infoLog);
        cout<<infoLog<<endl;
        free(infoLog);
    }
}

///\brief Saves an RGBA image to the file imname
///@param[in] buffer: the image content\n
///@param[in] imname: the name of the file where the image will be saved\n
///@param[in] length: size of the image (4*x*y being RGBA)e
void saveImage(float *p, char* imname, int length) {
    //cerr<<"Inside saveImage "<<imname<<endl;
    //remove(imname);
    ofstream file;
    file.open(imname,ios::out|ios::binary);//open a file
    for (int i=0; i<length; ++i) {
		if (p[i]>1) p[i]=1;//clipping values
		else if (p[i]<0) p[i]=0;//clipping values
        file<<(unsigned char)(255*p[i]);//write to it 
    }
    file.close();//close it
}

///\brief Loads an RGBA image from the file imname to the given buffer
///@param[in] buffer: a buffer for storing the image\n
///@param[in] imname: the name of the image to load\n
///@param[in] length: size of the image (4*x*y being RGBA)
void loadImage(float *p, char* imname, int length) {
    //cerr<<"Inside loadImage "<<imname<<endl;
    ifstream file;
    file.open(imname,ios::in|ios::binary);
    for (int i=0; i<length; ++i) {
        unsigned char pixel;
        file>>pixel;
        p[i]=(float)pixel/255.0;
    }
    file.close();//close it
}

///swaps the role of the two textures (read-only and write-only)
void swap(void) {
    //cerr<<"Inside swap"<<endl;
    if (writeTex == 0) {
        writeTex = 1;
        readTex = 0;
    } else {
        writeTex = 0;
        readTex = 1;
    }
}

///Renders the state of the data matrix
void display() {
	//binds drawing target to display
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    // render a full-screen quad textured with the results of our
    // computation.  Note that this is not part of the computation: this
    // is only the visualization of the results.
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0);
    glVertex2f(0.0, texSize_y);
    glTexCoord2f(texSize_x, 0.0);
    glVertex2f(texSize_x, texSize_y);
    glTexCoord2f(texSize_x, texSize_y);
    glVertex2f(texSize_x, 0.0);
    glTexCoord2f(0.0, texSize_y);
    glVertex2f(0.0, 0.0);
    glEnd();

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
}

void reshape(int width, int height) {}
}//END NAMESPACE
