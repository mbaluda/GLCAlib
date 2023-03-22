///\file GLconway.cpp
///\brief GPGPU-based Conway's Game of Life.
///
///Realizes the cellular automata described in http://en.wikipedia.org/wiki/Conway's_Game_of_Life using the GLCAlib library.\n
///Provides a CPU version for performance comparison

// includes
#include <iostream>
#include <cstring>
#include "GLCAlib.h"

///\brief Implements in GLSL the rules of the Automata
///
///1. Any live cell with fewer than two live neighbours dies, as if by loneliness.\n
///2. Any live cell with more than three live neighbours dies, as if by overcrowding.\n
///3. Any live cell with two or three live neighbours lives, unchanged, to the next generationn
///4. Any dead cell with exactly three live neighbours comes to life.\n
char* shader="uniform sampler2DRect texture_A;" \

             "int sum;" \
             "vec4 dead = vec4(1.0,1.0,1.0,1.0);" \
             "vec4 alive = vec4(0.0,0.0,0.0,1.0);" \
             "void main(void) {" \
             "sum=0;" \
             "vec4 y = texture2DRect(texture_A, gl_TexCoord[0].st);" \

             "if (texture2DRect(texture_A, gl_TexCoord[0].st + vec2(-1.0, -1.0))==alive) ++sum;" \
             "if (texture2DRect(texture_A, gl_TexCoord[0].st + vec2(0.0, -1.0))==alive) ++sum;" \
             "if (texture2DRect(texture_A, gl_TexCoord[0].st + vec2(1.0, -1.0))==alive) ++sum;" \

             "if (texture2DRect(texture_A, gl_TexCoord[0].st + vec2(-1.0, 0.0))==alive) ++sum;" \
             "if (texture2DRect(texture_A, gl_TexCoord[0].st + vec2(1.0, 0.0))==alive) ++sum;" \

             "if (texture2DRect(texture_A, gl_TexCoord[0].st + vec2(-1.0, 1.0))==alive) ++sum;" \
             "if (texture2DRect(texture_A, gl_TexCoord[0].st + vec2(0.0, 1.0))==alive) ++sum;" \
             "if (texture2DRect(texture_A, gl_TexCoord[0].st + vec2(1.0, 1.0))==alive) ++sum;" \

             "if (sum<2) gl_FragColor = dead;" \
             "else if (sum>3) gl_FragColor = dead;" \
             "else if (sum==3) gl_FragColor = alive;" \
             "else gl_FragColor = y;" \
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
///If TRUE compares CPU and GPU performances
bool compareResults;
///If TRUE displays the Automata evolution
bool withgui;
///Length of the computation in generations
long numIterations;

///Performs and times the algorithm on the CPU
void CPUresults () {
    //cerr<<"Inside compareResults"<<endl;
    float* data_A = new float[N];
    float* data_B = new float[N];
    GLCAlib::loadImage(data_A, infilename, N);
    GLCAlib::loadImage(data_B, infilename, N);


    //cerr<<"calc on CPU"<<endl;
    long start=time(NULL);
    for (long n=0; n<numIterations; ++n) {
        for (int i=1; i<y-1; ++i) {
            for (int j=4; j<4*(x-1); j+=4) {

                //cerr<<"REPLACING THE SHADER SUBSTITUTE FOR THE SHADER"<<endl;
                int neighbours=0;
                if(data_A[4*x*(i-1)+j-4]==0.0) ++neighbours;
                if(data_A[4*x*(i-1)+j]==0.0) ++neighbours;
                if(data_A[4*x*(i-1)+j+4]==0.0) ++neighbours;

                if(data_A[4*x*i+j-4]==0.0) ++neighbours;
                if(data_A[4*x*i+j+4]==0.0) ++neighbours;

                if(data_A[4*x*(i+1)+j-4]==0.0) ++neighbours;
                if(data_A[4*x*(i+1)+j]==0.0) ++neighbours;
                if(data_A[4*x*(i+1)+j+4]==0.0) ++neighbours;

                //cerr<<"DIES"<<endl;
                if (neighbours<2 || neighbours>3) {
                    data_B[4*x*i+j]=1.0;
                    data_B[4*x*i+j+1]=1.0;
                    data_B[4*x*i+j+2]=1.0;
                    data_B[4*x*i+j+3]=1.0;
                } else if (neighbours==3) {
                    //cerr<<"LIVES"<<endl;
                    data_B[4*x*i+j]=0.0;
                    data_B[4*x*i+j+1]=0.0;
                    data_B[4*x*i+j+2]=0.0;
                    data_B[4*x*i+j+3]=1.0;
                } else {
                    //cerr<<"THE SAME"<<endl;
                    data_B[4*x*i+j]=data_A[4*x*i+j];
                    data_B[4*x*i+j+1]=data_A[4*x*i+j+1];
                    data_B[4*x*i+j+2]=data_A[4*x*i+j+2];
                    data_B[4*x*i+j+3]=data_A[4*x*i+j+3];
                }
            }
        }

        float* tmp=data_B;
        data_B=data_A;
        data_A=tmp;
    }
    long end = time(NULL);
    long total = end-start;
    if (total>0) std::cout<<"CPU Iterations/sec: "<<numIterations/total<<std::endl;

    GLCAlib::saveImage(data_A,strcat(outfilename, "CPU.rgba") , N);

    delete[] data_A;
    delete[] data_B;
}

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
///Param 5: 0=no comparison of results 1=compare GPU and CPU perfomances\n
///Param 6: number of iterations\n
///Param 7: 0=noGUI 1=GUI version\n
int main(int argc, char** argv) {
    //cerr<<"main"<<endl;

    // parse command line
    if (argc < 7) {
        std::cout<<"Command line parameters:\n";
        std::cout<<"Param 1: Filename of the input RGBA image\n";
        std::cout<<"Param 2: Filename of the input RGBA image\n";
        std::cout<<"Param 3: problem size x\n";
        std::cout<<"Param 4: problem size y\n";
        std::cout<<"Param 5: 0 = no comparison of results\n";
        std::cout<<"         1 = compare GPU vs CPU\n";
        std::cout<<"Param 6: number of iterations\n";
        std::cout<<"Param 7: 0 = no GUI\n";
        std::cout<<"         1 = GUI"<<std::endl;
        exit(0);
    } else {
        infilename = argv[1];
        outfilename = argv[2];

        x =	atoi(argv[3]);
        y =	atoi(argv[4]);

        switch (atoi(argv[5])) {
        case 0:
            compareResults = false;
            break;
        case 1:
            compareResults = true;
            break;
        default:
            std::cout<<"unknown parameter, exit"<<std::endl;
            exit(1);
        }

        numIterations = atoi (argv[6]);

        switch (atoi(argv[7])) {
        case 0:
            withgui = false;
            break;
        case 1:
            withgui = true;
            break;
        default:
            std::cout<<"unknown parameter, exit"<<std::endl;
            exit(1);
        }
    }

    //cerr<<"calc texture dimensions"<<endl;
    //textureParameters.texFormat == GL_RGBA
    N=4*x*y;
    float* image = new float[N];
    GLCAlib::loadImage(image, infilename, N);
    GLCAlib::init(argc, argv, image, x, y, shader, withgui, numIterations);
    //std::cout<<"save"<<std::endl;
    GLCAlib::saveImage(image, outfilename, N);
    //std::cout<<"compare"<<std::endl;
    if (compareResults) CPUresults ();

    return 0;
}
