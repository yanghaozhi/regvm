SUBDIRS=common core mem vasm c--

.PHONY:all

all: 
	@ for i in $(SUBDIRS) ; do make -j4 -C $$i ; done

release: 
	@ for i in $(SUBDIRS) ; do make release -j4 -C $$i ; done

cov: 
	@ for i in $(SUBDIRS) ; do make cov -j4 -C $$i ; done

clean:
	@ for i in $(SUBDIRS) ; do make clean -C $$i ; done

lsp: 
	rm -f ./compile_commands.json
	make clean
	bear -- make

