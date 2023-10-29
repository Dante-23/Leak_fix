TARGET:bin/exe
bin/exe:bin/main.o bin/ld_opps.o
	g++ -std=c++17 -o bin/exe -g bin/main.o bin/ld_opps.o
	rm bin/main.o bin/ld_opps.o
bin/main.o:main1.cpp
	g++ -std=c++17 -c main1.cpp -o bin/main.o
bin/ld_opps.o:src/ld_opps.cpp
	g++ -std=c++17 -c src/ld_opps.cpp -o bin/ld_opps.o
clean:
	rm bin/exe