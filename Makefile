CC      = gcc
CFLAGS  = -g
RM      = rm -f





prova: prova.c
	$(CC) $(CFLAGS) -o prova prova.c -lOpenCL

clean veryclean:
	$(RM) Hello
