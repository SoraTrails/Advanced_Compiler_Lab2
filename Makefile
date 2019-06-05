src = $(wildcard *.cpp)
target = $(patsubst %.cpp, %.out, ${src})
header = $(patsubst %.cpp, %.h, ${src})

${target}: ${src} ${header}
	g++ -std=c++11 -o $@ $<