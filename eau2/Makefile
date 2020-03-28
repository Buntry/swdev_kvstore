# Install the google test framework under the test directory
install-gtest:
	cd ./tests && cmake .

# Run all the tests
gtest:
	-cd ./tests && make --no-print-directory && ./runTests

# Remove all files to get a clean branch.
gclean:
	find ./tests/* ! -name "CMakeLists.txt" ! -name "CMakeLists.txt.in" ! -name "test*" -exec rm -rf "{}" +;

# Build the mainfile
build: ./src/main.cpp
	g++ -std=c++11 ./src/main.cpp -Wall -ggdb3 --pedantic -pthread

# Run the mainfile
run: build
	./a.out

# Valgrind the mainfile
valgr: build
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -v ./a.out

# Cleans only the compiled binaries and datafiles
clean:
	rm -rf a.out*

# Examples of running our application
pseudo: build
	./a.out -pseudo -num_nodes 3
