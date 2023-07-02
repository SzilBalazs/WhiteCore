CXX = clang++
TARGET_FLAGS = -static -fuse-ld=lld
ARCH = native
NAME = WhiteCore
VERSION_MAJOR = 0
VERSION_MINOR = 2
HASH := $(shell git rev-parse --short HEAD)
SOURCES := $(shell find $(SOURCEDIR) -name '*.cpp')
HEADERS := $(shell find $(SOURCEDIR) -name '*.h')

ifeq ($(OS),Windows_NT)
    uname_S := Windows
else
    uname_S := $(shell uname -s)
endif

ifeq ($(uname_S), Windows)
	SUFFIX = .exe
else
	SUFFIX =
endif

ifeq ($(ARCH), native)
	ARCH_FLAGS = -march=native
	DEFINE_FLAGS = -DNATIVE
endif

ifeq ($(ARCH), bmi2)
	ARCH_FLAGS = -march=x86-64 -mpopcnt -msse -msse2 -mssse3 -msse4.1 -mavx2 -mbmi -mbmi2
	DEFINE_FLAGS = -DAVX2 -DBMI2
endif

ifeq ($(ARCH), avx2)
	ARCH_FLAGS = -march=x86-64 -mpopcnt -msse -msse2 -mssse3 -msse4.1 -mavx2 -mbmi
    DEFINE_FLAGS = -DAVX2
endif

ifeq ($(ARCH), popcnt)
    ARCH_FLAGS = -march=x86-64 -mpopcnt
endif

# Native build with debug symbols
ifeq ($(build), debug)
	TARGET_FLAGS = -g3 -fno-omit-frame-pointer
	ARCH_FLAGS   = -march=native
	DEFINE_FLAGS = -DNATIVE
endif

DEFINE_FLAGS += -DVERSION=\"v$(VERSION_MAJOR).$(VERSION_MINOR).$(HASH)\" -DNDEBUG
CXXFLAGS = $(DEFINE_FLAGS) $(ARCH_FLAGS) -flto -std=c++20 -O3 -pthread -Wall
EXE = $(NAME)-v$(VERSION_MAJOR)-$(VERSION_MINOR)
OUTPUT_BINARY = $(EXE)$(SUFFIX)
INCBIN_TOOL = incbin_tool$(SUFFIX)

build: $(OUTPUT_BINARY)
	@echo > /dev/null

clean:
	@rm $(OUTPUT_BINARY) $(INCBIN_TOOL) src/corenet.cpp || true

bench: $(OUTPUT_BINARY)
	@echo Bench: $(shell ./$(OUTPUT_BINARY) bench | grep -Eo '^[0-9]+ nodes' | grep -o '[0-9]*')

$(INCBIN_TOOL):
ifeq ($(uname_S), Windows)
	@echo Compiling $(INCBIN_TOOL)
	@clang -o $@ src/external/incbin/incbin.c
endif

$(OUTPUT_BINARY): $(HEADERS) $(SOURCES) $(INCBIN_TOOL)
ifeq ($(uname_S), Windows)
	@./$(INCBIN_TOOL) src/main.cpp -o src/corenet.cpp
endif
	@echo Compiling $(NAME)
	@$(CXX) $(TARGET_FLAGS) $(CXXFLAGS) -o $@ src/*.cpp
	@echo Build has finished.

.PHONY: all
