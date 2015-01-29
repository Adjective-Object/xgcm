all: xgcm

xgcm: xgcm.o xgcm_parser.o xgcm_conf.o simple_hmap.o ini.o utils.o ini.o
	gcc -Wall -g $^ -o $@

ini.o:
	gcc -Wall -g -c ini/ini.c ini/ini.h

%.o: %.c %.h
	gcc -Wall -g -c $<

clean:
	rm *.o xgcm