TARGET := app

GLSLC := glslc

SOURCES := $(wildcard src/*.c)
SHADER_SOURCES := $(wildcard shader/*.vert) $(wildcard shader/*.frag)
LIBS := -lpthread
OBJECTS := $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCES)))
DEPENDS := $(patsubst %.c,%.d,$(patsubst %.cpp,%.d,$(SOURCES)))
SHADER_OBJECTS := $(patsubst %.vert,%.spv,$(patsubst %.frag,%.spv,$(SHADER_SOURCES)))

CFLAGS = -O2 -std=c2x -Wall -Wextra -Wpedantic -Wconversion -Wno-override-init -Wno-pointer-arith -Werror -Isrc -Ilib
CXXFLAGS = -O2 -Isrc -Ilib

.PHONY: build run clean

build: $(OBJECTS) $(SHADER_OBJECTS)
	$(CXX) $(CFLAGS) -o $(TARGET).elf $(OBJECTS) $(LIBS)

run: build
	@./$(TARGET).elf

clean:
	$(RM) $(OBJECTS) $(DEPENDS) $(SHADER_OBJECTS)

-include $(DEPENDS)

%.o: %.c Makefile
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

%.o: %.cpp Makefile
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

%.spv: %.vert Makefile
	$(GLSLC) $< -o $@

%.spv: %.frag Makefile
	$(GLSLC) $< -o $@
