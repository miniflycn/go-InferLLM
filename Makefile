INCLUDE_PATH := $(abspath ./)
LIBRARY_PATH := $(abspath ./)

ifndef UNAME_S
UNAME_S := $(shell uname -s)
endif

ifndef UNAME_P
UNAME_P := $(shell uname -p)
endif

ifndef UNAME_M
UNAME_M := $(shell uname -m)
endif

CCV := $(shell $(CC) --version | head -n 1)
CXXV := $(shell $(CXX) --version | head -n 1)

# Mac OS + Arm can report x86_64
# ref: https://github.com/ggerganov/whisper.cpp/issues/66#issuecomment-1282546789
ifeq ($(UNAME_S),Darwin)
	ifneq ($(UNAME_P),arm)
		SYSCTL_M := $(shell sysctl -n hw.optional.arm64 2>/dev/null)
		ifeq ($(SYSCTL_M),1)
			# UNAME_P := arm
			# UNAME_M := arm64
			warn := $(warning Your arch is announced as x86_64, but it seems to actually be ARM64. Not fixing that can lead to bad performance. For more info see: https://github.com/ggerganov/whisper.cpp/issues/66\#issuecomment-1282546789)
		endif
	endif
endif

#
# Compile flags
#

BUILD_TYPE?=
# keep standard at C11 and C++11
CFLAGS   = -I./InferLLM -I. -O3 -DNDEBUG -std=c11 -fPIC
CXXFLAGS = -I./InferLLM -I./InferLLM/application -O3 -DNDEBUG -std=c++11 -fPIC
LDFLAGS  =

# warnings
CFLAGS   += -Wall -Wextra -Wpedantic -Wcast-qual -Wdouble-promotion -Wshadow -Wstrict-prototypes -Wpointer-arith -Wno-unused-function
CXXFLAGS += -Wall -Wextra -Wpedantic -Wcast-qual -Wno-unused-function
LDFLAGS  =

# OS specific
# TODO: support Windows
ifeq ($(UNAME_S),Linux)
	CFLAGS   += -pthread
	CXXFLAGS += -pthread
endif
ifeq ($(UNAME_S),Darwin)
	CFLAGS   += -pthread
	CXXFLAGS += -pthread
endif
ifeq ($(UNAME_S),FreeBSD)
	CFLAGS   += -pthread
	CXXFLAGS += -pthread
endif
ifeq ($(UNAME_S),NetBSD)
	CFLAGS   += -pthread
	CXXFLAGS += -pthread
endif
ifeq ($(UNAME_S),OpenBSD)
	CFLAGS   += -pthread
	CXXFLAGS += -pthread
endif
ifeq ($(UNAME_S),Haiku)
	CFLAGS   += -pthread
	CXXFLAGS += -pthread
endif

ifndef LLAMA_NO_ACCELERATE
	# Mac M1 - include Accelerate framework.
	# `-framework Accelerate` works on Mac Intel as well, with negliable performance boost (as of the predict time).
	ifeq ($(UNAME_S),Darwin)
		CFLAGS  += -DGGML_USE_ACCELERATE
		LDFLAGS += -framework Accelerate
	endif
endif
ifneq ($(filter aarch64%,$(UNAME_M)),)
	CFLAGS += -mcpu=native
	CXXFLAGS += -mcpu=native
endif
ifneq ($(filter armv6%,$(UNAME_M)),)
	# Raspberry Pi 1, 2, 3
	CFLAGS += -mfpu=neon-fp-armv8 -mfp16-format=ieee -mno-unaligned-access
endif
ifneq ($(filter armv7%,$(UNAME_M)),)
	# Raspberry Pi 4
	CFLAGS += -mfpu=neon-fp-armv8 -mfp16-format=ieee -mno-unaligned-access -funsafe-math-optimizations
endif
ifneq ($(filter armv8%,$(UNAME_M)),)
	# Raspberry Pi 4
	CFLAGS += -mfp16-format=ieee -mno-unaligned-access
endif

# libbinding.a: prepare
# 	cp ./build/libInferLLM.a ./libbinding.a

binding.o: prepare
	$(CXX) $(CXXFLAGS) -I./build binding.cpp -o binding.o -c $(LDFLAGS)

prepare:
	mkdir -p build
	cd build && cmake ../InferLLM && cmake --build .

libbinding.a: prepare binding.o
	ar src libbinding.a build/CMakeFiles/InferLLM.dir/src/core/device.cpp.o build/CMakeFiles/InferLLM.dir/src/core/graph.cpp.o build/CMakeFiles/InferLLM.dir/src/core/kvstotage.cpp.o build/CMakeFiles/InferLLM.dir/src/core/model.cpp.o build/CMakeFiles/InferLLM.dir/src/core/model_imp.cpp.o build/CMakeFiles/InferLLM.dir/src/core/op.cpp.o build/CMakeFiles/InferLLM.dir/src/core/tensor.cpp.o build/CMakeFiles/InferLLM.dir/src/core/thread_pool.cpp.o build/CMakeFiles/InferLLM.dir/src/file.cpp.o build/CMakeFiles/InferLLM.dir/src/graph/baichuan.cpp.o build/CMakeFiles/InferLLM.dir/src/graph/chatGLM.cpp.o build/CMakeFiles/InferLLM.dir/src/graph/graph_imp.cpp.o build/CMakeFiles/InferLLM.dir/src/graph/llama.cpp.o build/CMakeFiles/InferLLM.dir/src/kern/naive/naive.cpp.o build/CMakeFiles/InferLLM.dir/src/kern/optimized/kernel_opt.cpp.o build/CMakeFiles/InferLLM.dir/src/utils.cpp.o binding.o

clean:
	rm -rf *.o
	rm -rf *.a
	rm -rf build