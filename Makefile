.PHONY: $(PROG) all clean run debug tsan $(OBJS)

PROG	:=msl-gol

# compiler
CC_C	:= gcc
CFLAGS	:= -Iinclude -std=c2x -g
CFLAGS	+=$(shell sdl2-config --cflags) 
CFLAGS	+=$(shell sdl2-config --cflags) 

# source
SRC 	:= $(wildcard src/*.c)

#object files glob
OBJS	:= $(SRC:.c=.o)

# linker
LDFLAGS	:=
LDLIBS	:= $(shell sdl2-config --libs)	


# TERM_COLS:=$(shell tput cols) 
FMT_RESET	:=tput sgr0
FMT_REDBANNER	:=tput rev; tput bold; tput setaf 1
FMT_GREENBANNER	:=tput rev; tput bold; tput setaf 2
FMT_YELLOWBANNER:=tput rev; tput bold; tput setaf 3
FMT_REV		:=tput rev; tput bold; 


all: $(PROG)


#compile .c into .o (compilation proper)
%.o : %.c
	@$(FMT_GREENBANNER)
	@echo " COMPILE SRC -> OBJ  > " 
	@$(FMT_RESET)
	$(CC_C) $(CFLAGS) -c $< -o $@ 
	@printf "\n"

# build executable (linking)
$(PROG): $(OBJS) 
	@$(FMT_YELLOWBANNER); echo " LINKING OBJ -> BIN  > "; $(FMT_RESET)
	$(CC_C) $(LDFLAGS) $(OBJS) -o $(PROG) $(LDLIBS) 
	@printf "\n"

# RUN PROGRAM - DEFINE A:= ANY PROGRAM_ARGS
run: clean $(PROG)
	@$(FMT_REDBANNER)
	@echo " EXECUTING BINARY: " 
	@$(FMT_RESET)
	$(ASAN_ENV) ./$(PROG) $(A)
	@printf "\n"

# DEBUG PROGRAM
debug: $(PROG)
	lldb -o run -- $(PROG) $(TERM_COLS) $(A)

# ADSAN

	
# Target specific variables: this allows us to change variables only for certain targets
tsan: CFLAGS  += -fsanitize=thread -fno-omit-frame-pointer 
tsan: LDFLAGS += -fsanitize=thread
tsan: LDLIBS  += -fsanitize=thread
tsan: clean run 
	
# Address san: lower overhead than thread-san, cleaner stack traces,
asan: CFLAGS  += -fsanitize=address -fno-omit-frame-pointer
asan: LDFLAGS += -fsanitize=address
asan: LDLIBS  += -fsanitize=address
asan: ASAN_ENV:= ASAN_OPTIONS=abort_on_error=1
asan: clean run 

usan: CFLAGS  += -fsanitize=undefined -fno-omit-frame-pointer 
usan: LDFLAGS += -fsanitize=undefined 
usan: LDLIBS  += -fsanitize=undefined 
usan: clean run 


ausan: CFLAGS  += -fsanitize=address,undefined -fno-omit-frame-pointer
ausan: LDFLAGS += -fsanitize=address,undefined
ausan: LDLIBS  += -fsanitize=address,undefined
ausan: CFLAGS+= -fsanitize-address-use-after-scope
ausan: ASAN_ENV:= ASAN_OPTIONS=abort_on_error=1

ausan: CFLAGS  += -fsanitize=undefined -fno-omit-frame-pointer 
ausan: LDFLAGS += -fsanitize=undefined 
ausan: LDLIBS  += -fsanitize=undefined 
ausan: clean run




clean: 
	@$(FMT_REV)
	@echo " REMOVING EXECUTABLES AND OBJECT FILES... "
	@$(FMT_RESET)
	rm -f $(PROG) $(OBJS)
