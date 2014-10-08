all:
	g++ gpxfixaltitude.cpp --std=c++0x -o  gpsfixaltitude

debug:
	g++ gpxfixaltitude.cpp --std=c++0x -g -o  gpsfixaltitude

clang:
	clang++ gpxfixaltitude.cpp --std=c++0x -g -o  gpsfixaltitude
