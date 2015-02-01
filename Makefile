all: xgcm

LINK_FLAGS=

xgcm: xgcm.o xgcm_traversal.o xgcm_parser.o xgcm_conf.o ini.o\
		utils.o ini.o simple_hmap.o string_buffer.o
	gcc -Wall -g ${LINK_FLAGS} $^ -o $@

ini.o:
	gcc -Wall -g ${LINK_FLAGS} -c ini/ini.c ini/ini.h

%.o: %.c %.h
	gcc -Wall -g ${LINK_FLAGS} -c $<

clean:
	rm *.o xgcm