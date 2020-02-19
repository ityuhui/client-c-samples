CLIENT_C_DIR:=/root/c_k8s_api_client
INCLUDE:=-I$(CLIENT_C_DIR)/include -I$(CLIENT_C_DIR)/model -I$(CLIENT_C_DIR)/api
LIBS:=-L$(CLIENT_C_DIR)/build -lkubernetes -lcurl -lpthread -lssl -lz
CFLAGS:=-g

all:
	gcc main.c $(CFLAGS) $(INCLUDE) $(LIBS) -o create_pod_bin
clean:
	rm ./create_pod_bin
