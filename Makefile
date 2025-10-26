.PHONY: all clean run debug help h ?
all: $(EXE)

# ===== MAKE VARIABLES ===== 
EXE_NAME :=compute_mult
CXX	:=clang++
STD	:= -std=c++17
CCFLAGS	:=-Iinclude -Ithirdparty/metal-cpp $(shell sdl2-config --cflags) $(STD)
LDFLAGS	:=-framework Metal -framework Foundation -framework QuartzCore $(STD)
LDLIBS	:=$(shell sdl2-config --libs)
EXE_DIR	:=bin
SRC_DIR	:=src
OBJ_DIR :=build
SRC_EXT	:=.cpp
OBJ_EXT :=.o
SRC 	:=$(wildcard $(SRC_DIR)/*$(SRC_EXT)) 
OBJS 	:=$(patsubst $(SRC_DIR)/%$(SRC_EXT),$(OBJ_DIR)/%$(OBJ_EXT),$(SRC))
EXE 	:=$(EXE_DIR)/$(EXE_NAME)
# p.s: read all their values at compile time with 'make [h|help|?]'

# ===== ENV VARIABLES ===== 
OBJC_DEBUG_MISSING_POOLS:=YES
MTL_HUD_ENABLED		:=1
MTL_HUD_INSIGHTS_ENABLED:=1


# pattern rule to build src files -> obj files: 	" COMPILE SRC to OBJ " 
$(OBJ_DIR)/%$(OBJ_EXT) : $(SRC_DIR)/%$(SRC_EXT) $(OBJ_DIR)
	@$(ECHO_COMP_BANNER)
	$(CXX) $(CCFLAGS) -c $< -o $@ 

# normal rule to build obj files -> target exe:  	" LINKING OBJ to BIN " 
$(EXE): $(OBJ_DIR) $(OBJS) $(EXE_DIR)
	@$(ECHO_LINK_BANNER)
	$(CXX) $(LDFLAGS) $(OBJS) -o $(EXE) $(LDLIBS)

# phony rule to run the binary with optional args:  	" EXECUTING BINARY " 
run: $(EXE)
	@$(ECHO_EXE_BANNER)
	./$(EXE) $(ARGS)

	
#NOTE: AVOID PUTTING MAKE VARIABLES IN CLEAN COMMAND! BE VERY SPECIFIC SO YOU DONT NUKE YOUR SOURCE FILES ON ACCIDENT LIKE LAST TIME
clean: 
	$(ECHO_CLEAN_BANNER)
	rm -f build/*.o src/*.o bin/*

# ------------ DEBUGGING ------------ #
debug: $(EXE)
	lldb -o run -- $(EXE) $(TERM_COLS) $(ARGS)

	
# Address san: for detecting memory errors (over/underflows, UAF, UAS, double free)
asan: CCFLAGS+= -fsanitize=address -fno-omit-frame-pointer -g
asan: LDFLAGS+= -fsanitize=address -fno-omit-frame-pointer
asan: ASAN_OPTIONS:=abort_on_error=1
asan: clean run 

# undefined san: specifically checking for undefined behaviour
usan:  CCFLAGS +=-fsanitize=undefined -fno-omit-frame-pointer -g
usan:  LDFLAGS+=-fsanitize=undefined -fno-omit-frame-pointer 
usan: ASAN_OPTIONS:=abort_on_error=1
usan: clean run 

# ausan:	 the best of both worlds? 
ausan: CCFLAGS+=-fsanitize=address,undefined -fno-omit-frame-pointer -g
ausan: LDFLAGS+=-fsanitize=address,undefined -fno-omit-frame-pointer
ausan: ASAN_OPTIONS:=abort_on_error=1
ausan: clean run

# leaksan:	memory leaks specifically

lsan: CCFLAGS +=-fsanitize=address,undefined -fno-omit-frame-pointer -g
lsan: LDFLAGS+=-fsanitize=address,undefined -fno-omit-frame-pointer
lsan: ASAN_OPTIONS:=abort_on_error=1,detect_leaks=1
lsan: clean run

# allsan:	do it all brev
allsan: CCFLAGS+=-fsanitize=address,undefined -fno-omit-frame-pointer -g
allsan: LDFLAGS+==-fsanitize=address,undefined -fno-omit-frame-pointer
allsan: ASAN_OPTIONS:=abort_on_error=1,detect_leaks=1
allsan: clean run

# thread san:	multithreading related errors (races, dealocks). Higher overhead than others
tsan: CCFLAGS+= -fsanitize=thread -fno-omit-frame-pointer -g
tsan: LDFLAGS+= -fsanitize=thread -fno-omit-frame-pointer 
tsan: ASAN_OPTIONS:=abort_on_error=1
tsan: clean run 

ltsan: CCFLAGS+= -fsanitize=thread -fno-omit-frame-pointer -g
ltsan: LDFLAGS+= -fsanitize=thread -fno-omit-frame-pointer 
ltsan: ASAN_OPTIONS:=abort_on_error=1,detect_leaks=1
ltsan: clean run 

# ------------ AUTOMATIC TARGETS ------------ #
$(OBJ_DIR):
	@mkdir -p $@

$(EXE_DIR):
	@mkdir -p $@


# ------------ HELP ------------ #
help h ?: HELP
.PHONY: HELP
HELP:
	
	@$(FMT_REV)
	@printf "===== MAKE VARIABLES =====\n"; $(FMT_RESET)
	@$(FMT_ALT1) 	printf "SRC      = $(SRC)\n" 
	@$(FMT_ALT2) 	printf "EXE_DIR  = $(EXE_DIR )\n"
	@$(FMT_ALT1)	printf "EXE_NAME = $(EXE_NAME)\n"
	@$(FMT_ALT2)	printf "EXE      = $(EXE)\n"
	@$(FMT_ALT1)	printf "SRC_DIR  = $(SRC_DIR)\n"
	@$(FMT_ALT2)	printf "OBJ_DIR  = $(OBJ_DIR)\n"
	@$(FMT_ALT1)	printf "SRC_EXT  = $(SRC_EXT)\n"
	@$(FMT_ALT2)	printf "OBJ_EXT  = $(OBJ_EXT)\n"
	@$(FMT_ALT1)	printf "CXX     = $(CXX)\n"
	@$(FMT_ALT2)	printf "SRC      = $(SRC)\n"
	@$(FMT_ALT1)	printf "OBJS     = $(OBJS)\n"
	@$(FMT_ALT2)	printf "CCFLAGS  = $(CCFLAGS)\n"
	@$(FMT_ALT1)	printf "LDFLAGS  = $(LDFLAGS)\n"
	@$(FMT_ALT2)	printf "LDLIBS   = $(LDLIBS)\n"
	@$(FMT_ALT1)	printf "ALLFLAGS = $(ALLFLAGS)\n"




# ------------ PRETTY PRINTING ------------ #
FMT_RESET	:=tput sgr0;
FMT_REDBANNER	:=tput rev; tput bold; tput setaf 1;
FMT_GREENBANNER	:=tput rev; tput bold; tput setaf 2;
FMT_YELLOWBANNER:=tput rev; tput bold; tput setaf 3;
FMT_REV		:=tput rev; tput bold;


FMT_ALT1	:= tput setaf 7;
FMT_ALT2	:= tput sgr0;


ECHO_LINK_BANNER := @$(FMT_YELLOWBANNER) 	printf " LINKING OBJ to BIN -> "; $(FMT_RESET) 	printf "\t"
ECHO_COMP_BANNER := @$(FMT_GREENBANNER)		printf " COMPILE SRC to OBJ -> "; $(FMT_RESET)	printf "\t"
ECHO_EXE_BANNER := @$(FMT_REDBANNER) 		printf " EXECUTING BINARY -> "; $(FMT_RESET)	printf "\t"
ECHO_CLEAN_BANNER:= @$(FMT_REV) 		printf " REMOVING EXEUTABLES AND OBJECT FILES... "; $(FMT_RESET)printf "\t"


# EXTRA MAKEFILE STUFF:
# :_______1-1_______: To exclude certain files from compilation:
# EXCLUDE_PAT	:= unit_tests.c
# SRC 		:= $(filter-out src/$(EXCLUDE_SRC), $(wildcard src/*.c))
