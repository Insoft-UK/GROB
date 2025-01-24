grob:
	g++ -arch x86_64 -arch arm64 -std=c++20 src/*.cpp -o build/grob -Os -fno-ident -fno-asynchronous-unwind-tables
	strip build/grob
	lipo -info build/grob
