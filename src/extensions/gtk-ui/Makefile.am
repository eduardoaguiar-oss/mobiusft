.PHONY: all cmake-clean cmake-install

all:
	mkdir -p build
	cd build && cmake .. -DCMAKE_INSTALL_PREFIX=$(prefix)
	+cd build && make

cmake-clean:
	rm -rf build

cmake-install: all
	+cd build && make install

clean-local: cmake-clean
install-exec-local: cmake-install

EXTRA_DIST = 		\
    CMakeLists.txt	\
    *.py		\
    icon.png		\
    resource/*.png
