MAIN =\
main.o \
../src/EPDLite/commands.o

CPPFLAGS = -DTEST
CXXFLAGS = -Wall -Wextra -Werror -std=c++11 -g
LDFLAGS = 
OBJECTS = $(MAIN)

all: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o main.out $(LDFLAGS)

%.o : %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

clean:
	@$(RM) $(MAIN)
