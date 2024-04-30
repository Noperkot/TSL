#!/usr/bin/env make
# System: MinGW on Windows under armLinux

PREFIX = i686-w64-mingw32
EXECUTABLE ?= tsl.exe

CXX = $(PREFIX)-g++
WRS = $(PREFIX)-windres
LD  = $(PREFIX)-g++
OTIMFLAG ?= -O2
CXXFLAGS = -m32 -municode -std=c++11 -Wall -g0 $(OTIMFLAG) -fno-exceptions
LDFLAGS  = -m32 -municode -mwindows -s -static -Wl,--no-insert-timestamp -Wl,--gc-sections

SRC_EXT = cpp c
BLD_DIR = build
OBJ_DIR = $(BLD_DIR)/obj
SRC_DIR = src
RES_DIR = src/res
SOURCES = $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(OBJ_DIR)/resource.o $(OBJ_DIR)/version.o $(addprefix $(OBJ_DIR)/,$(addsuffix .o, $(basename $(notdir $(SOURCES)))))
HEADERS = $(wildcard $(SRC_DIR)/*.h) $(wildcard $(RES_DIR)/*.h)
ICONS   = logo.ico inactive.ico cir_grn.ico cir_red.ico
ICODIR  = $(RES_DIR)/icons
VPATH   = $(ICODIR)

.PHONY: icons clean incbuild

$(EXECUTABLE): $(OBJ_DIR) $(BLD_DIR) $(OBJECTS)
	@echo
	@echo -n Linking $(EXECUTABLE)
	@$(LD) $(OBJECTS) -o $(BLD_DIR)/$(EXECUTABLE) $(LDFLAGS)
#	@$(PREFIX)-strip -R .comment $(BLD_DIR)/$(EXECUTABLE)
#	@upx -9 --overlay=strip $(BLD_DIR)/$(EXECUTABLE)
#	@echo $(EXECUTABLE) `du -sh $(BLD_DIR)/$(EXECUTABLE) | cut -f1`
	@echo " -" `ls -lh $(BLD_DIR)/$(EXECUTABLE) | awk '{print $$5}'`
	@echo

define compile_rule
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.$1 $(HEADERS) Makefile
	@echo Compile $$<
	@$(CXX) $$< $ -c -o $$@ $(CXXFLAGS)
endef
$(foreach EXT,$(SRC_EXT),$(eval $(call compile_rule,$(EXT))))
	
$(OBJ_DIR)/resource.o: $(RES_DIR)/resource.rc $(RES_DIR)/resource.h $(ICONS) $(RES_DIR)/xpmanifest.xml  Makefile
	@echo Compile $<
	@$(WRS) --codepage=65001 -i $< -o $@
#	?????????????????????????????????????????????????????????????????????????????????????
#	@iconv -f UTF-16LE $(RES_DIR)/resource.rc -t UTF-8 -o $(RES_DIR)/resource_utf8.rc
#	@$(WRS) --codepage=65001 -i $(RES_DIR)/resource_utf8.rc -o $(OBJ_DIR)/resource.o
#	@rm $(RES_DIR)/resource_utf8.rc

$(OBJ_DIR)/version.o: $(RES_DIR)/version.rc incbuild $(RES_DIR)/version.h $(RES_DIR)/build.h Makefile
	@echo '#define _COMPILER_ "$(shell $(CXX) --version | grep ^$(CXX)), $(OTIMFLAG)"' > $(RES_DIR)/compiler.h
	@echo Compile $<
	@$(WRS) --codepage=65001 -i $< -o $@

$(OBJ_DIR):
	@echo Create dir \'$(OBJ_DIR)\'
	@mkdir -p $(OBJ_DIR)

$(BLD_DIR):
	@echo Create dir \'$(BLD_DIR)\'
	@mkdir -p $(BLD_DIR)

icons:  $(ICONS) ../tsIcons/Makefile
	$(MAKE) -C ../tsIcons $(ICONS) 
	mv -f -t $(ICODIR) $(addprefix ../tsIcons/,$(ICONS))

clean:
	@rm -f $(OBJ_DIR)/*
	@rm -f $(BLD_DIR)/$(EXECUTABLE)

incbuild:
	@cd $(RES_DIR) && python build.py
