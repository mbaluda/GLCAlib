///\file GLCAlib.h
///\brief Hardware accelerated Cellular Automata library
///
///It contains the public interface of the GLCAlib library.

#ifndef GLCAlib_H
#define GLCAlib_H

// prototypes
namespace GLCAlib{
///\brief Initialize OpenGL and executes the given shader
///@param[in] arc: number of parameters on the commend line\n
///@param[in] argv: holds parameters passed on the commend line\n
///@param[in] image: buffer containing the input data\n
///@param[in] x: width of the input
///@param[in] y: height of the input
///@param[in] shader: the program executed on the GPU
///@param[in] gui: if TRUE, visualizes the computation evolution
///@param[in] iterations: length of the computation in generations
void init(int argc, char** argv, float* image, int x, int y, char* shader, bool gui=true, int iterations=0);

///\brief Loads an RGBA image from the file imname to the given buffer
///@param[in] buffer: a buffer for storing the image\n
///@param[in] imname: the name of the image to load\n
///@param[in] length: size of the image (4*x*y being RGBA)
void loadImage(float* buffer, char* imname, int length);

///\brief Saves an RGBA image to the file imname
///@param[in] buffer: the image content\n
///@param[in] imname: the name of the file where the image will be saved\n
///@param[in] length: size of the image (4*x*y being RGBA)
void saveImage(float* buffer, char* imname, int length);
}

#endif

/**\mainpage GLCAlib: Cellular Automata on GPU

\author Mauro Baluda (matr.038208)\n mauro@bglug.it
 
\section intro_sec Introduction
 
This project is meant as an exercise in GPU programming.\n
My goal was to develop a simple framework that permits to easily build Cellular Automata in which both the evolution and the visualization are accelerated by the GPU.\n

\section GPGPU GPGPU programming

Perform non-graphical computations taking advantage of the parallelism and programmability of modern GPU's is commonly called GPGPU [1].\n
Some specific programming languages [2] are being developed to permit to use the GPU power without a deep knowledge of the graphic pipeline, but they are rather new and little portable, so I decided to develop my project using OpenGL and his shading language GLSL.\n
My code is heavily based on two well known tutorials [3,4] available on the internet.

\section implementation_sec Cellular Automata on GPU library

\subsection dev Platforms and Dependencies.
The library was developed on Linux using standard C but there should not be any problem compiling the code under different operating systems such as MS windows.\n
To simplify OpenGL management I used freeGLUT [5] and an extension loader named GLEW [6]. I preferred freeGLUT over the most famous GLUT because it gives better control over the application lifecycle introducing the function glutLeaveMainLoop().\n
Both this library are free and multiplatform.

related files: GLCAlib.h GLCAlib.cpp

\subsection using Using the library.
Using the library to develop custom accelerated CA is very simple, the function init takes care of everything\n\n
void init(int argc, char** argv, float* image, int x, int y, char* shader, bool gui=true, int iterations=0);\n\n
it takes as input a number of parameters to control automaton creation:\n
-argc: number of parameters on the commend line\n
-argv: holds parameters passed on the commend line\n
-image: buffer containing the input data in RGBA format normalized between 0 and 1\n
-x: width of the input\n
-y: height of the input\n
-shader: the fragment shader program to be executed on the GPU, takes care of CA evolution step\n
-gui: if TRUE, visualizes the computation evolution\n
-iterations: length of the computation (in generations)\n

Image can be loaded and saved to RGBA files using two trivial functions: loadImage and saveImage.\n
You may want to use other image formats, this can easily be done using some external library like MagickCore [7] or CImg [8].

related files: GLCAlib.h

\subsection graphic Graphical display of data.
Because of the presence of the data into the graphic adapter memory, displaying it on screen requires little effort and can be done efficiently, this reason candidates GPGPU for physical realtime simulations both in videogames and in scientific computation.\n
Manage visualization could require the execution of a second fragment shader program, In my implementation I limited this feature to raw visualization of data (usefull also for debugging).

\section sample_sec Sample program: Wireworld Computer
Wireworld [9] is a cellular automaton invented by Brian Silverman in about 1984.\n
It can simulate electronic circuits and is actually Turing-complete [10].\n

related files: GLwworld.cpp

\subsection wwrules Wireworld Description
The cells of the automaton can be in one of four different states, forming a pattern on the grid. The four states are:\n
-blank, shown in the pictures here in black;\n
-'copper', shown here in yellow;\n
-'electron head', or just 'head' for short, shown here as white; and\n
-'electron tail', or 'tail', shown in cyan.

Time proceeds in discrete steps called generations. At each generation the state of each cell may change; whether it changes, and what it changes to, depend on its current state and the state of its eight nearest neighbour cells according to a simple set of rules:\n
1. a blank square always stays blank\n
2. an electron head always becomes an electron tail\n
3. an electron tail always becomes copper\n
4. copper stays as copper unless it has just one or two neighbours that are electron heads, in which case it becomes an electron head\n

The initial state of the automaton determines it's evolution and the function it calculates.\n
The included program, due to Michael Fryers, calculates prime numbers.

\image html wworld-small.png
\image latex wworld.png width=\textwidth

\subsection wwrun Running Wireworld
The program GLwworld realize the above automaton and can be executed from the command line requiring some parameters:\n
Param 1: Filename of the input RGBA image\n
Param 2: Filename of the input RGBA image\n
Param 3: problem size x\n
Param 4: problem size y\n
Param 5: 0 = no comparison of results, 1 = compare GPU vs CPU\n
Param 6: number of iterations\n
Param 7: 0 = no GUI, 1 = GUI

The included shell script GLwworld.sh runs the program with some default parameters and compares GPU and CPU performances, running the program without a GUI gives better speed results.

related files: GLwworld.sh
 
\section conway Sample program: Conway's Game of Life
The Game of Life [11] is a cellular automaton devised by the British mathematician John Horton Conway in 1970. It is the best-known example of a cellular automaton and is Turing-complete [10].

related files: GLconway.cpp

\subsection liferules Game Description
The universe of the Game of Life is an infinite two-dimensional orthogonal grid of square cells, each of which is in one of two possible states, live or dead. Every cell interacts with its eight neighbours, which are the cells that are directly horizontally, vertically, or diagonally adjacent. At each step in time, the following transitions occur:\n
1. Any live cell with fewer than two live neighbours dies, as if by loneliness.\n
2. Any live cell with more than three live neighbours dies, as if by overcrowding.\n
3. Any live cell with two or three live neighbours lives, unchanged, to the next generation.\n
4. Any dead cell with exactly three live neighbours comes to life.

In the past years a number of interesting initial configurations have been studied, the include one simulates a memory unit.

\image html life.png
\image latex life.png width=\textwidth

\subsection liferun Running Conway's Game of Life
The program GLconway realize the above automaton and can be executed from the command line requiring some parameters:\n
Param 1: Filename of the input RGBA image\n
Param 2: Filename of the input RGBA image\n
Param 3: problem size x\n
Param 4: problem size y\n
Param 5: 0 = no comparison of results, 1 = compare GPU vs CPU\n
Param 6: number of iterations\n
Param 7: 0 = no GUI, 1 = GUI

The included shell script GLconway.sh runs the program with some default parameters and compares GPU and CPU performances, running the program without a GUI gives better speed results.

related files: GLconway.sh

\section blur Sample program: Image Processing
Although intended for Cellular Automata this library can easily be used for other computations on square grids of floating point numbers, in this example we developed an image blur program that applies a 3x3 convolution matrix filter to a color image as shown below:\n

\image html blur-small.png
\image latex blur.png width=\textwidth

related files: GLblur.cpp

\subsection blurun Blurring an image
The program GLblur realize the above convolution filtering and can be executed from the command line requiring some parameters:\n
Param 1: Filename of the input RGBA image\n
Param 2: Filename of the input RGBA image\n
Param 3: problem size x\n
Param 4: problem size y\n

The included shell script GLblur.sh runs the program with some default parameters.\n
A filter like this can be probably used in real-time over a video sequence.

related files: GLblur.sh

\section performance_sec Performances
To estimate the performance of our software running on GPU and CPU, we launched GLwworld (the most complex automata) on a number of different platform obtaining the results shown below:

\image html perf.png
\image latex perf.pdf width=\textwidth

\section future_sec Conclusions
I consider the results satisfactory, more powerful adapters or also clusters of them can be used for more demanding applications.\n
In the future I would like to improve the display part of the program using a GUI library like QT [12] and allowing the use of a second fragment shader.\n
More complex Cellular Automata can be used for simulations of physical and social phenomena, I think the presented technique could be a budget solution also for this kind of applications.

\section documentation_sec Documentation
The project and the code documentation was written using doxygen [13] a multi-language documentation system.

\section links Resources
[1] General-Purpose computation on GPUs\n
http://www.gpgpu.org/\n
http://en.wikipedia.org/wiki/GPGPU

[2] GPGPU languages.\n
http://graphics.stanford.edu/projects/brookgpu/\n
http://developer.nvidia.com/object/cuda.html\n
http://ati.de/companyinfo/researcher/documents.html\n
http://www.rapidmind.net/

[3] GPGPU::Basic Math Tutorial.\n
http://www.mathematik.uni-dortmund.de/~goeddeke/gpgpu/tutorial.html

[4] OpenGL Frame Buffer Object Tutorial.\n
http://www.gamedev.net/reference/articles/article2333.asp

[5] freeGLUT.\n
http://freeglut.sourceforge.net/ 

[6] The OpenGL Extension Wrangler Library.\n
http://glew.sourceforge.net/

[7] The MagickCore API.\n
http://www.imagemagick.org/script/magick-core.php

[8] C++ Template Image Processing Library.\n
http://cimg.sourceforge.net/

[9] The Wireworld computer.\n
http://www.quinapalus.com/wi-index.html

[10] Turing completeness\n
http://en.wikipedia.org/wiki/Turing_completeness

[11] Conway's Game of Life.\n
http://en.wikipedia.org/wiki/Conway's_Game_of_Life

[12] QT library.\n
http://trolltech.com/

[13] Doxygen multilanguage documentation system.\n
http://www.stack.nl/~dimitri/doxygen/
*/
