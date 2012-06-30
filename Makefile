
myftp_server: myftp_server.cpp
	g++ myftp_server.cpp -o myftp_server
	
myftp_client: myftp_client.cpp
	g++ myftp_client.cpp -o myftp_client
	
clean:
	rm myftp_client myftp_server
	

