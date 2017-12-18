all:aio_test.cpp
#	g++ main.cpp -g -o raidmeter -lrt
	g++ aio_test.cpp -g -o aio_test -lrt -Wnonnull
