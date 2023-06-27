CXX = clang++
TARGET_FLAGS = -static
ARCH = native
NAME = WhiteCore
VERSION_MAJOR = 0
VERSION_MINOR = 1
TMP_DIR = .tmp
SOURCES := $(wildcard *.cpp)
OBJECTS := $(patsubst %.cpp, $(TMP_DIR)/%.o, $(SOURCES))
DEPENDS := $(patsubst %.cpp, $(TMP_DIR)/%.d, $(SOURCES))
HASH := $(shell git rev-parse --short HEAD)

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
	ARCH_FLAGS=-march=native
	DEFINE_FLAGS=-DNATIVE
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
CXXFLAGS = $(DEFINE_FLAGS) $(ARCH_FLAGS) -flto -std=c++20 -O3 -pthread -Wall -MMD
EXE = $(NAME)-v$(VERSION_MAJOR)-$(VERSION_MINOR)$(SUFFIX)

default: $(EXE)
	@echo > /dev/null

$(OBJECTS): | $(TMP_DIR)
$(DEPENDS): | $(TMP_DIR)

$(TMP_DIR):
	@mkdir $(TMP_DIR)

$(TMP_DIR)/%.o: %.cpp
	@echo Compiling $<
	@$(CXX) $(CXXFLAGS) -c -o $@ $<

$(EXE): $(OBJECTS)
	@echo Linking $(NAME)
	@$(CXX) $(TARGET_FLAGS) $(CXXFLAGS) -o $@ $^
	@echo Build has finished.

all: clean build

build: $(EXE)

clean:
	@rm -r .tmp $(EXE) *.gcda || true

.PHONY: all

-include ${DEPENDS}
