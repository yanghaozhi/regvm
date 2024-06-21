.PHONY:all

all:
	make -C core
	make -C ext
	make -C vasm

clean:
	make clean -C core
	make clean -C ext
	make clean -C vasm

cov:
	make cov -C core
	make cov -C ext
	make cov -C vasm
