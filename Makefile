COMPILER_FLAGS=-Wall -Wextra -g -O2 -fPIC

CXXFLAGS=$(COMPILER_FLAGS) -I$(DNANEXUS_HOME)/share/dnanexus/src/cpp
CFLAGS=$(COMPILER_FLAGS) -I$(HTSLIB_ROOT)

LDFLAGS=-L$(DNANEXUS_HOME)/share/dnanexus/src/cpp/dxcpp/build \
		-L$(DNANEXUS_HOME)/share/dnanexus/src/cpp/dxjson/build \
		-L$(DNANEXUS_HOME)/share/dnanexus/src/cpp/SimpleHttpLib/build
LDLIBS=-ldxcpp -ldxjson -ldxhttp -lcurl -lboost_system -lboost_regex -lboost_thread

C_SRCS=$(wildcard src/*.c)
CXX_SRCS=$(wildcard src/*.cc)
C_OBJS=$(patsubst %.c,%.o,$(C_SRCS))
CXX_OBJS=$(patsubst %.cc,%.o,$(CXX_SRCS))

all: hfile_dnanexus.so
.PHONY: all

hfile_dnanexus.so: $(C_OBJS) $(CXX_OBJS)
	$(CXX) -shared $(LDFLAGS) $^ -o $@ $(LDLIBS)
