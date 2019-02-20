all: server client
server: $(OBJ)
	gcc dhcp_server.c -lm -o dhcp_server

client: 
	gcc -o client client.c

.PHONY : clean
clean:
	rm dhcp_server client
