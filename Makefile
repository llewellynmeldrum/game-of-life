.PHONY: $(PROG) all clean run debug tsan $(OBJS)
# check for sdl2 install, should be the only dependency other than xcode i guess
ifeq (, $(shell which sdl2-config 2>/dev/null))
$(error "SDL2 not found! Please install it with 'brew install sdl2' or your package manager.")
endif

PROG	:= hello_metal

# compiler
CXX	:= clang++
STD	:= -std=c++17
CFLAGS	:= -Iinclude -Ithirdparty/metal-cpp
CFLAGS 	+= $(shell sdl2-config --cflags)



# env variables
MTL_HUD_ENABLED=1
OBJC_DEBUG_MISSING_POOLS=YES

MTL_HUD_INSIGHTS_ENABLED=1

# source
SRC 	:= $(wildcard src/*.cpp) 

#object files glob
OBJS	:= $(SRC:.cpp=.o)

# linker
LDFLAGS	:= -framework Metal -framework Foundation -framework QuartzCore
LDLIBS := $(shell sdl2-config --libs)


# METAL STUFF

METAL_SOURCES:= $(wildcard shaders/*.metal) 
AIR_OBJECTS:= $(SHADERS:.metal=.air)
METALLIB:= default.metallib


all: $(PROG)


%.air: %.metal
	xcrun -sdk macosx metal -c $< -o $@

$(METALLIB): $(AIR_OBJECTS)
	xcrun -sdk macosx metallib $^ -o $@


#compile .c into .o (compilation proper)
%.o: %.cpp $(METALLIB)
	@$(FMT_GREENBANNER)
	@echo " COMPILE SRC -> OBJ  > " 
	@$(FMT_RESET)
	$(CXX) $(STD) $(CFLAGS) -c $< -o $@ 
	@printf "\n"

# build executable (linking)
$(PROG): $(OBJS)
	@$(FMT_YELLOWBANNER); echo " LINKING OBJ -> BIN  > "; $(FMT_RESET)
	$(CXX) $(STD) $(LDFLAGS) $(OBJS) -o $(PROG) $(LDLIBS) 
	@printf "\n"

# RUN PROGRAM - DEFINE A:= ANY PROGRAM_ARGS
run: clean $(PROG)
	@$(FMT_REDBANNER)
	@echo " EXECUTING BINARY: " 
	@$(FMT_RESET)
	$(ASAN_ENV) ./$(PROG) $(A)
	@printf "\n"

# DEBUG PROGRAM
debug: CFLAGS+= -g
debug: $(PROG)
	lldb -o run -- $(PROG) $(TERM_COLS) $(A)

clean: 
	@$(FMT_REV)
	@echo " REMOVING EXECUTABLES AND OBJECT FILES... "
	@$(FMT_RESET)
	rm -f $(PROG) $(OBJS) $(AIR_OBJECTS)




# ADSAN
	
tsan: CFLAGS  += -g -fsanitize=thread -fno-omit-frame-pointer 
tsan: LDFLAGS += -g -fsanitize=thread
tsan: LDLIBS  += -fsanitize=thread
tsan: clean run 
	
# Address san: lower overhead than thread-san, cleaner stack traces,
asan: CFLAGS  += -g -fsanitize=address -fno-omit-frame-pointer
asan: LDFLAGS += -g -fsanitize=address
asan: LDLIBS  += -fsanitize=address
asan: ASAN_ENV:= ASAN_OPTIONS=abort_on_error=1
asan: clean run 

usan: CFLAGS  += -fsanitize=undefined -fno-omit-frame-pointer 
usan: LDFLAGS += -fsanitize=undefined 
usan: LDLIBS  += -fsanitize=undefined 
usan: clean run 


ausan: CFLAGS  += -g -fsanitize=address,undefined -fno-omit-frame-pointer
ausan: LDFLAGS += -g -fsanitize=address,undefined
ausan: LDLIBS  += -fsanitize=address,undefined
ausan: CFLAGS+= -fsanitize-address-use-after-scope
ausan: ASAN_ENV:= ASAN_OPTIONS=abort_on_error=1

ausan: CFLAGS  += -fsanitize=undefined -fno-omit-frame-pointer 
ausan: LDFLAGS += -fsanitize=undefined 
ausan: LDLIBS  += -fsanitize=undefined 
ausan: clean run





FMT_RESET	:=tput sgr0
FMT_REDBANNER	:=tput rev; tput bold; tput setaf 1
FMT_GREENBANNER	:=tput rev; tput bold; tput setaf 2
FMT_YELLOWBANNER:=tput rev; tput bold; tput setaf 3
FMT_REV		:=tput rev; tput bold; 
