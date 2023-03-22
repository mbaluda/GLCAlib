RM=rm -Rf
CXXFLAGS=-O3
LDFLAGS=-lGLEW -lGL -lGLU -lglut

LIB=GLCAlib
DOC=doxygen
DOC_FILES=html mystl.tag

all: GLconway GLwworld GLblur 
lib: ${LIB}

${LIB}: ${LIB}.cpp
	$(CXX) -c -o ${LIB} $(CXXFLAGS) $<

GLconway: GLconway.cpp ${LIB}
	$(CXX) $(CXXFLAGS) -o GLconway ${LIB} $< $(LDFLAGS)

GLwworld: GLwworld.cpp ${LIB}
	$(CXX) $(CXXFLAGS) -o GLwworld ${LIB} $< $(LDFLAGS)

GLblur: GLblur.cpp ${LIB}
	$(CXX) $(CXXFLAGS) -o GLblur ${LIB} $< $(LDFLAGS)

doc:
	$(DOC)

clean:
	$(RM) GLconway GLwworld GLblur ${LIB} $(DOC_FILES)
