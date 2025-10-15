.PHONY: $(PROG) all clean run debug

PROG	:=msl-gol

# compiler
CC_C	:= gcc
CFLAGS	:= -Iinclude $(shell sdl2-config --cflags) -std=c2x
SRC 	:= $(wildcard src/*.c)
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
run: $(PROG)
	@$(FMT_REDBANNER)
	@echo " EXECUTING BINARY: " 
	@$(FMT_RESET)
	./$(PROG) $(A)
	@printf "\n"

# DEBUG PROGRAM
debug: $(PROG)
	lldb -o run -- $(PROG) $(TERM_COLS) $(A)
clean: 
	@$(FMT_REV)
	@echo " REMOVING EXECUTABLES AND OBJECT FILES... "
	@$(FMT_RESET)
	rm -f $(PROG) $(OBJS)
