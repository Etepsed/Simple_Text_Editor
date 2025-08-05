SRC = src/main.c\
      src/editor.c\
      src/utils.c
      

ExcName = texteditor

all:
	mkdir -p build
	gcc $(SRC) -o ./build/$(ExcName)

clean:
	rm -rf build

dev:
	clear
	rm -rf build
	mkdir -p build
	@if gcc $(SRC) -o ./build/$(ExcName); then \
		clear; \
		./build/$(ExcName); \
	else \
		echo "Error: compilation error. Nothing was executed!"; \
	fi

