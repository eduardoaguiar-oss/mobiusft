.PHONY: all cmake-clean cmake-install

all:
	mkdir -p build
	cd build && cmake .. -DCMAKE_INSTALL_PREFIX=$(prefix)
	cd build && make

cmake-clean:
	rm -rf build

cmake-install:
	cd build && make install

# Hook into Automake's standard targets
clean-local: cmake-clean

install-exec-local: cmake-install

# Include CMakeLists.txt and source files in the distribution
EXTRA_DIST =				\
	CMakeLists.txt			\
	main.cpp			\
	evidence_loader_impl.cpp	\
	*.hpp
