all:
	g++ --std=c++14 -I. *.cpp glfwmain/*.cpp helpers/*.cpp -m64 -lGL -lpthread `pkg-config --cflags --libs glm glew glfw3` -o vectorio
debug:
	g++ -g --std=c++14 -I. *.cpp glfwmain/*.cpp helpers/*.cpp -lGL -lpthread `pkg-config --cflags --libs glm glew glfw3` -o vectoriodbg
xr:
	g++ -g -shared -fPIC --std=c++14 -I. *.cpp helpers/shader.cpp helpers/texture.cpp -m64 -lGL -lpthread `pkg-config --cflags --libs glm glew glfw3` -o libvectorioxr.so


