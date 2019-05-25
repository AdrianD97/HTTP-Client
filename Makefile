FLAGS = -Wall

build: tasks.cpp parson.c client.h helpers.h request.h parson.h
	g++ $(FLAGS) tasks.cpp parson.c -o tasks

run:
	valgrind ./tasks

clean:
	rm tasks

