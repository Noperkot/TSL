#!/usr/bin/env make
# System: MinGW on Windows under armLinux

PREFIX = /usr/bin/i686-w64-mingw32
EXECUTABLE ?= tsl.exe

CXX = $(PREFIX)-g++
WRS = $(PREFIX)-windres
LNK = $(CXX)

CXXFLAGS ?= -m32 -municode -Wall -Os -fno-exceptions 
LDFLAGS  ?= -m32 -municode -mwindows -s -static -Wl,--no-insert-timestamp -Wl,--gc-sections

SRC_EXT = cpp c
BLD_DIR = build
OBJ_DIR = $(BLD_DIR)/obj
SRC_DIR = src
RES_DIR = src/res
SOURCES = $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(OBJ_DIR)/resource.o $(OBJ_DIR)/version.o $(addprefix $(OBJ_DIR)/,$(addsuffix .o, $(basename $(notdir $(SOURCES)))))
HEADERS = $(wildcard $(SRC_DIR)/*.h) $(wildcard $(RES_DIR)/*.h)

$(EXECUTABLE): $(OBJ_DIR) $(BLD_DIR) $(OBJECTS)
	@echo
	@echo -n Linking $(EXECUTABLE)
	@$(LNK) $(OBJECTS) -o $(BLD_DIR)/$(EXECUTABLE) $(LDFLAGS)
	@ #upx -9 $(BLD_DIR)/$(EXECUTABLE)
	@ #echo $(EXECUTABLE) `du -sh $(BLD_DIR)/$(EXECUTABLE) | cut -f1`
	@echo " -" `ls -lh $(BLD_DIR)/$(EXECUTABLE) | awk '{print $$5}'`
	@echo

define compile_rule
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.$1 $(HEADERS) Makefile
	@echo Compile $$<
	@$(CXX) $$< $ -c -o $$@ $(CXXFLAGS)
endef
$(foreach EXT,$(SRC_EXT),$(eval $(call compile_rule,$(EXT))))

$(OBJ_DIR)/resource.o: $(RES_DIR)/resource.rc $(RES_DIR)/resource.h $(RES_DIR)/xpmanifest.xml Makefile
	@echo Compile $<
	@iconv -f UTF-16LE -t UTF-8 $(RES_DIR)/resource.rc -o $(RES_DIR)/resource_utf8.rc
	@$(WRS) --codepage=65001 -i $(RES_DIR)/resource_utf8.rc -o $(OBJ_DIR)/resource.o
	@rm $(RES_DIR)/resource_utf8.rc

$(OBJ_DIR)/version.o: $(RES_DIR)/version.rc $(RES_DIR)/version.h Makefile
	@echo Compile $<
	@$(WRS) --codepage=65001 -i $< -o $@

$(OBJ_DIR):
	@echo Create dir \'$(OBJ_DIR)\'
	@mkdir -p $(OBJ_DIR)

$(BLD_DIR):
	@echo Create dir \'$(BLD_DIR)\'
	@mkdir -p $(BLD_DIR)

clean:
	@rm -f $(OBJ_DIR)/*
	@rm -f $(BLD_DIR)/$(EXECUTABLE)