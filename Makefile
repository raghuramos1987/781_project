CC = g++
CFLAGS = -g 
LIBS = -lm -lX11 -lXi -lXmu -lglut -lGL -lGLU -lGLEW
DEPS = ply.h plyread.h normalsply.h opengl_hook.h main.h globals.h \
       transform.h 
OBJ = ply.o plyread.o normalsply.o main.o opengl_hook.o globals.o \
      transform.o 

%.o: %.cpp $(DEPS) 
	$(CC) -c -o $@ $< $(CFLAGS)


main: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS) 

clean:
	rm *.o main
