CC      := gcc
INCLUDE := -Iinclude
LIBS    := 
CARGS   := $(INCLUDE) -ggdb -Wall -Wextra -Werror -pedantic
OUT     := run

objects += main.o

build: $(addprefix obj/, $(objects))
	$(CC) $(CARGS) -o ./$(OUT) $^ $(LIBS)

obj/%.o: src/%.c
	@mkdir -p ./obj
	$(CC) -c $(CARGS) -o $@ $^

run: build
	./$(OUT)

gdb: build
	gdb ./$(OUT)

clean:
	rm $(OUT)
	rm -r ./obj
