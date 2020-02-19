INCLUDE:=-I../../include -I../../model -I../../api
LIBS:=-L../../build -lk8s -lcurl -lpthread -lssl -lz
CFLAGS:=-g

all:
	gcc main.c $(CFLAGS) $(INCLUDE) $(LIBS) -o cactivity
clean:
	rm cactivity
