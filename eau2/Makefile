# Install the google test framework under the test directory
install-gtest:
	cd ./tests && cmake .

# Run all the tests
gtest:
	-cd ./tests && make --no-print-directory && ./runTests

# Remove all files to get a clean branch.
gclean:
	find ./tests/* ! -name "CMakeLists.txt" ! -name "CMakeLists.txt.in" ! -name "test*" -exec rm -rf "{}" +;
