EXEC        = simulator
OBJ         = mem_mgmt.o

CC          = gcc
CFLAGS      = -std=c17 -Wall -Wextra -Wpedantic -O3
CFLAGS_DBG  = -std=c17 -Wall -Wextra -Wpedantic -O0 -g
LDFLAGS     =

.PHONY: all
all: $(EXEC)
$(EXEC): %: $(OBJ)

.PHONY: debug
debug: CFLAGS = $(CFLAGS_DBG)
debug: all

.PHONY: clean
clean:
	rm -f $(EXEC) $(OBJ)
