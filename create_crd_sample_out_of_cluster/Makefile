INCLUDE:=-I../../c_k8s_api_client/include -I../../c_k8s_api_client/model -I../../c_k8s_api_client/api
LIBS:=-L../../c_k8s_api_client/build -lkubernetes -lpthread -lssl -lz -lcurl
CFLAGS:=-g

all:
	gcc main.c $(CFLAGS) $(INCLUDE) $(LIBS) -o cactivityOutCluster
clean:
	rm cactivityOutCluster
