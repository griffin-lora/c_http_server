TARGET := app

SOURCES := $(wildcard src/*.c)
RESOURCES := $(wildcard resource/*.html)
LIBS := -lpthread
OBJECTS := $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCES)))
DEPENDS := $(patsubst %.c,%.d,$(patsubst %.cpp,%.d,$(SOURCES)))
RESOURCE_OBJECTS := $(patsubst %.html,%.o,$(RESOURCES))

CFLAGS = -O2 -std=c2x -Wall -Wextra -Wpedantic -Wconversion -Wno-override-init -Wno-pointer-arith -Werror -Isrc -Ilib -g
CXXFLAGS = -O2 -Isrc -Ilib

.PHONY: build run clean

build: $(OBJECTS) $(RESOURCE_OBJECTS)
	$(CXX) $(CFLAGS) -o $(TARGET).elf $(OBJECTS) $(RESOURCE_OBJECTS) $(LIBS)

run: build
	@./$(TARGET).elf

clean:
	$(RM) $(OBJECTS) $(DEPENDS) $(RESOURCE_OBJECTS)

-include $(DEPENDS)

%.o: %.c Makefile
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

%.o: %.cpp Makefile
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

%.o: %.html Makefile
	$(LD) -r -b binary $< -o $@