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

fastbuild: ./src/main.cpp
	g++ -std=c++11 ./src/main.cpp -O3 -pthread

# Run the mainfile
run: build
	./a.out -pseudo -num_nodes 3 -app demo

# Valgrind the mainfile
valgr: build
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -v ./a.out -pseudo -num_nodes 3 -app linus

# Cleans only the compiled binaries and datafiles
clean:
	rm -rf a.out*

# Examples of running our application
pseudo: build
	./a.out -pseudo -num_nodes 3 -app demo
	
triv: build
	./a.out -pseudo -num_nodes 1 -app trivial

triv-net: build
	./a.out -num_nodes 1 -index 0 -ip 127.0.0.1 -port 8080 -app trivial 

# Here's the Milestone 3 example code.
demo: build
	./a.out -pseudo -num_nodes 3 -app demo

# Here's the Milestone 4 example code
wc: build
	./a.out -pseudo -num_nodes 3 -app wc -file "./data/100k.txt"

# Milestone 5
# We found 7289210 users in 20 minutes on our third attempt.
linus: build
	./a.out -pseudo -num_nodes 3 -app linus

# Milestone 3 example on a network.
demo-net1: build
	./a.out -num_nodes 3 -index 0 -ip 127.0.0.1 -port 8080 -app demo

demo-net2:
	./a.out -num_nodes 3 -index 1 -ip 127.0.0.2 -port 8080 -server_ip 127.0.0.1 -server_port 8080 -app demo

demo-net3:
	./a.out -num_nodes 3 -index 2 -ip 127.0.0.3 -port 8080 -server_ip 127.0.0.1 -server_port 8080 -app demo

# Word count application on a network.
wc-net1: build
	./a.out -num_nodes 3 -index 0 -ip 127.0.0.1 -port 8080 -app wc -file "./data/100k.txt"

wc-net2: 
	./a.out -num_nodes 3 -index 1 -ip 127.0.0.2 -port 8080 -server_ip 127.0.0.1 -server_port 8080 -app wc -file "./data/100k.txt"

wc-net3:
	./a.out -num_nodes 3 -index 2 -ip 127.0.0.3 -port 8080 -server_ip 127.0.0.1 -server_port 8080 -app wc -file "./data/100k.txt"

# Linus application on a network
linus-net1: fastbuild
	./a.out -num_nodes 3 -index 0 -ip 127.0.0.1 -port 8080 -app linus

linus-net2:
	./a.out -num_nodes 3 -index 1 -ip 127.0.0.2 -port 8080 -server_ip 127.0.0.1 -server_port 8080 -app linus

linus-net3:
	./a.out -num_nodes 3 -index 2 -ip 127.0.0.3 -port 8080 -server_ip 127.0.0.1 -server_port 8080 -app linus