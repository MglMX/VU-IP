all: serv1 client1 serv2 client2 serv3 client3 serv4 client4 talk

serv1: serv1/serv1.o
	gcc -o serv1/serv1 serv1/serv1.o serv1/wrapper.c serv1/keyvalue.c

client1: serv1/client.o
	gcc -o serv1/client serv1/client.c serv1/wrapper.c

serv2: serv2/serv2.o
	gcc -o serv2/serv2 serv2/serv2.o serv2/wrapper.c serv2/keyvalue.c

client2: serv2/client.o
	gcc -o serv2/client serv2/client.c serv2/wrapper.c

serv3: serv3/serv3.o
	gcc -o serv3/serv3 serv3/serv3.o serv3/wrapper.c serv3/keyvalue.c

client3: serv3/client.o
	gcc -o serv3/client serv3/client.c serv3/wrapper.c

serv4: serv4/serv4.o
	gcc -o serv4/serv4 serv4/serv4.o serv4/wrapper.c serv4/keyvalue.c -lpthread

client4: serv4/client.o
	gcc -o serv4/client serv4/client.c serv4/wrapper.c

talk: talk/talk.o
	gcc -o talk/talk talk/talk.c


clean:
	rm -f serv1/client serv1/serv1 serv2/client serv2/serv2 serv3/client serv3/serv3 serv4/client serv4/serv4 talk/talk
