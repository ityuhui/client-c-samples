INCLUDE:=-I../../include -I../../model -I../../api
LIBS:=-L../../build -lk8s -lpthread -lssl -lz -lcurl
CFLAGS:=-g

all:
	gcc main.c $(CFLAGS) $(INCLUDE) $(LIBS) -o csecret
clean:
	rm csecret
