///\file GLblur.cpp
///\brief GPGPU-based blur convolution filter.
///
///Realizes a convolution using the GLCAlib library.\n 

// includes
#include <iostream>

#include "GLCAlib.h"
///\brief Implements in GLSL the convolution for blurring images
///
///Convolution Matrix 3x3\n
///0.1 0.1 0.1\n
///0.1 0.2 0.1\n
///0.1 0.1 0.1\n
char* shader="uniform sampler2DRect texture_A;" \
             "void main(void) {" \
             "    gl_FragColor = texture2DRect(texture_A, gl_TexCoord[0].st)*0.2+" \

             "    texture2DRect(texture_A, gl_TexCoord[0].st + vec2(-1.0, -1.0))*0.1+" \
             "    texture2DRect(texture_A, gl_TexCoord[0].st + vec2(0.0, -1.0))*0.1+" \
             "    texture2DRect(texture_A, gl_TexCoord[0].st + vec2(1.0, -1.0))*0.1+" \

             "    texture2DRect(texture_A, gl_TexCoord[0].st + vec2(-1.0, 0.0))*0.1+" \
             "    texture2DRect(texture_A, gl_TexCoord[0].st + vec2(1.0, 0.0))*0.1+" \

             "    texture2DRect(texture_A, gl_TexCoord[0].st + vec2(-1.0, 1.0))*0.1+" \
             "    texture2DRect(texture_A, gl_TexCoord[0].st + vec2(0.0, 1.0))*0.1+" \
             "    texture2DRect(texture_A, gl_TexCoord[0].st + vec2(1.0, 1.0))*0.1;" \
             "}";
///The input image filename
char* infilename;
///The output image filename
char* outfilename;
///Width of the image
int x;
///Height of the image
int y;
///Size of the image (4*x*y being RGBA)
int N;

///\brief Just reads input and calls GLCAlib functions
///
///Images are read in RGBA format to avoid extra depandencies,
///Cimg or Imagemagick could be used to load images in other formats
///@param[in] argc: nuber of parameters on th ecommand line:\n
///@param[in] argv: holds parameters passed on the commend line:\n
///Param 1: Filename of the input RGBA image\n
///Param 2: Filename of the output RGBA image\n
///Param 3: problem size x\n
///Param 4: problem size y\n
int main(int argc, char** argv) {
    //cerr<<"main"<<endl;

    // parse command line
    if (argc < 5) {
        std::cout<<"Command line parameters:\n";
        std::cout<<"Param 1: Filename of the input RGBA image\n";
        std::cout<<"Param 2: Filename of the input RGBA image\n";
        std::cout<<"Param 3: problem size x\n";
        std::cout<<"Param 4: problem size y\n";
        exit(0);
    } else {
        infilename = argv[1];
        outfilename = argv[2];

        x =	atoi(argv[3]);
        y =	atoi(argv[4]);
    }

    //cerr<<"calc texture dimensions"<<endl;
    //textureParameters.texFormat == GL_RGBA
    N=4*x*y;
    float* image = new float[N];
    GLCAlib::loadImage(image, infilename, N);
    GLCAlib::init(argc, argv, image, x, y, shader, false, 1);
    //std::cout<<"save"<<std::endl;
    GLCAlib::saveImage(image, outfilename, N);

    return 0;
}
