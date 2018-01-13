all: serialKnn blockingKnn nonBlockingKnn

serialKnn:serialKnn.c
	gcc serialKnn.c -o serialKnn -O3 -lm
	
blockingKnn:blockingKnn.c
	mpicc blockingKnn.c -o blockingKnn -O3 -lm

nonBlockingKnn:nonBlockingKnn.c
	mpicc nonBlockingKnn.c -o nonBlockingKnn -O3 -lm
	
clear:
	rm serialKnn blockingKnn nonBlockingKnn
