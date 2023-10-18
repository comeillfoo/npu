CXX=g++
LD=g++

SC_PKGCONFIG=../.venv/lib/pkgconfig
INCLUDE_DIRS=-Iinclude
BUILD_DIR=build
NODES=$(addprefix nodes/,mem.cpp bus.cpp io.cpp cpu.cpp dma.cpp)
BUILD_NODES=$(addprefix $(BUILD_DIR)/, $(NODES:.cpp=.o))


CFLAGS=-g -Wall -pedantic -Wno-long-long \
	-DSC_INCLUDE_DYNAMIC_PROCESSES -fpermissive $(INCLUDE_DIRS)

CFLAGS+=$(shell PKG_CONFIG_PATH="${PKG_CONFIG_PATH}:$(SC_PKGCONFIG)" \
	pkg-config --cflags systemc)


LDFLAGS=$(shell PKG_CONFIG_PATH="${PKG_CONFIG_PATH}:$(SC_PKGCONFIG)" \
	pkg-config --libs systemc)
LDFLAGS+=-lm

EXEC=model


all: $(EXEC)


$(EXEC): $(BUILD_NODES) $(BUILD_DIR)/main.o
	$(LD) -o $@ $^ $(LDFLAGS)


$(BUILD_DIR)/%.o: %.cpp
	mkdir -p $(BUILD_DIR)/nodes
	$(CXX) -o $@ -c $(CFLAGS) $<


clean:
	rm -rf $(BUILD_DIR)
	rm -f $(EXEC)

.PHONY: clean
