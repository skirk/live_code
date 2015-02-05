PROJECT=program
CXX=g++
OPTS=-Wall -O3 -c -std=c++11

LDFLAGS= $(shell sdl2-config --libs) $(shell pkg-config --cflags --libs opencv) -lGL 
INCLUDEPATH= -Iinclude $(shell sdl2-config --cflags) 

all: $(PROJECT) shadercompiler obj/libdemo.so obj/shader.bin

$(PROJECT): src/core.cpp obj/gl_core_3_3.o obj/sdlutil.o
	$(CXX) -Wall -g $(INCLUDEPATH) $(LDFLAGS) $^ -o $@

obj/libdemo.so: obj/gl_core_3_3.o obj/demo.o obj/glslprogram.o
	$(CXX) -shared obj/gl_core_3_3.o obj/demo.o obj/glslprogram.o -o obj/libdemo.so

#suppressed warnings from deprecated string conversion
#should use cpp style headers
obj/gl_core_3_3.o: src/gl_core_3_3.c
	$(CXX) -fPIC  $(OPTS) -Wno-write-strings $(INCLUDEPATH) $^ -o $@

obj/demo.o: src/demo.cpp
	$(CXX) -fPIC $(OPTS) $(INCLUDEPATH) $^ -o $@

obj/glslprogram.o: src/glslprogram.cpp
	$(CXX) -fPIC $(OPTS) $(INCLUDEPATH) $^ -o $@

obj/shadercompiler.o: src/shadercompiler.cpp
	$(CXX) -std=c++11 $(OPTS) $(INCLUDEPATH) $^ -Iinclude -o  $@

obj/sdlutil.o: src/sdlutil.cpp
	$(CXX) -std=c++11 $(OPTS) $(INCLUDEPATH) $(LDFLAGS) $^ -Iinclude -o  $@

shadercompiler:  obj/gl_core_3_3.o obj/shadercompiler.o obj/glslprogram.o obj/sdlutil.o
	$(CXX) $^ $(LDFLAGS) -Iinclude  -o  $@ -lGL

obj/shader.bin:  shader/vertex.vert shader/fragment.frag shader/geometry.glsl
	./shadercompiler -v shader/vertex.vert -f shader/fragment.frag -g shader/geometry.glsl -o obj/shader.bin
	


clean:
	rm -Rf $(PROJECT) obj/* 

