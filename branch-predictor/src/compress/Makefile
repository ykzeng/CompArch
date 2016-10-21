CXX		=	g++
CXXFLAGS	=	-g

all:	ct

clean:
	rm -f ct *.o

ct:	ct.cc trace.cc branch.h trace.h
	$(CXX) $(CXXFLAGS) -o ct ct.cc trace.cc
