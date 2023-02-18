build:
	g++ main.cpp solver.cpp mapReduce.cpp -o tema1 -lpthread -lm -Wall 

clean:
	rm -rf tema1
