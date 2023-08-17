CXX = clang++
TARGET_FLAGS = -static
ARCH = native
NAME = WhiteCore
VERSION_MAJOR = 0
VERSION_MINOR = 2
CPP_FILES = src/main.cpp

C_GREEN = \033[1;92m
C_CYAN = \033[1;96m
C_WHITE = \033[1;97m
C_DEFAULT = \033[0m

ifneq ($(wildcard .git/*),)
	HASH := $(shell git rev-parse --short HEAD)
else
	HASH := unknown
endif

ifneq ($(CXX), g++)
	TARGET_FLAGS += -fuse-ld=lld
endif

ifeq ($(OS),Windows_NT)
    uname_S := Windows
else
    uname_S := $(shell uname -s)
endif

ifeq ($(uname_S), Windows)

ifeq ($(CXX), clang++)
	USE_INCBIN_TOOL = true
endif

	SUFFIX = .exe
	CPP_FILES += src/corenet.cpp
	CP = powershell cp
else
	CP = cp
	SUFFIX =
	SOURCES := $(shell find . -not -path './src/external/*' -name '*.cpp')
    HEADERS := $(shell find . -not -path './src/external/*' -name '*.h')
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
else
	DEFINE_FLAGS += -DNDEBUG
endif

ifeq ($(build), stats)
	DEFINE_FLAGS += -DTRACK_STATS
endif

EVALFILE = weights/master.bin
TMP_EVALFILE = tmp.bin
DEFINE_FLAGS += -DVERSION=\"v$(VERSION_MAJOR).$(VERSION_MINOR).$(HASH)\" -D_CRT_SECURE_NO_WARNINGS
CXXFLAGS = $(DEFINE_FLAGS) $(ARCH_FLAGS) -std=c++20 -O3 -flto=auto -pthread -Wall
EXE = $(NAME)
OUTPUT_BINARY = $(EXE)$(SUFFIX)
INCBIN_TOOL = incbin_tool$(SUFFIX)

build: $(OUTPUT_BINARY)
	@echo > /dev/null

clean:
	@rm $(OUTPUT_BINARY) $(INCBIN_TOOL) $(TMP_EVALFILE) src/corenet.cpp || true

bench: $(OUTPUT_BINARY)
	@echo Bench: $(shell ./$(OUTPUT_BINARY) bench | grep -Eo '^[0-9]+ nodes' | grep -o '[0-9]*')
	@python scripts/bench.py 1

train: $(OUTPUT_BINARY)
	@$(CP) $(OUTPUT_BINARY) train/WhiteCore
	@cd train && source py/bin/activate && python trainer.py && deactivate

format:
	@clang-format -i $(SOURCES) $(HEADERS)

$(TMP_EVALFILE):
	@$(CP) $(EVALFILE) $(TMP_EVALFILE)

$(INCBIN_TOOL): $(TMP_EVALFILE)
ifeq ($(USE_INCBIN_TOOL), true)
	@echo -e "$(C_CYAN)Compiling $(C_RED)$(INCBIN_TOOL)$(C_CYAN)...$(C_DEFAULT)"
	@clang -o $@ src/external/incbin/incbin.c
endif

$(OUTPUT_BINARY): $(HEADERS) $(SOURCES) $(INCBIN_TOOL)
ifeq ($(USE_INCBIN_TOOL), true)
	@./$(INCBIN_TOOL) src/network/nnue.h -o src/corenet.cpp
endif
	@echo -e "$(C_CYAN)Compiling $(C_WHITE)$(NAME)$(C_CYAN)...$(C_DEFAULT)"
	@$(CXX) $(TARGET_FLAGS) $(CXXFLAGS) -o $@ $(CPP_FILES)
	@echo -e "$(C_GREEN)Build has finished. $(C_DEFAULT)"
	@rm $(TMP_EVALFILE)

.PHONY: build clean bench train format
