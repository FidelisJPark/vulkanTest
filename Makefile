default:
	g++ -O2 -s -std=c++14 ./*.cpp -I include -L lib -l:libvulkan.so.1 -o test
run:
	LD_LIBRARY_PATH=lib ./libvc_test
clean:
	rm -f test
