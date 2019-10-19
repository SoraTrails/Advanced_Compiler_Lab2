BOOST_LIB=${BOOST_LIB_PATH}
BOOST_INCLUDE=${BOOST_INCLUDE_PATH}
src = $(wildcard *.cpp)
target = $(patsubst %.cpp, %.out, ${src})
header = $(patsubst %.cpp, %.h, ${src})

${target}: ${src} ${header}
	g++ -L${BOOST_LIB} -I${BOOST_INCLUDE} -g --std=c++11 -o $@ $<