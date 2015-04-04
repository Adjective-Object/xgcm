all: xgcm

LINK_FLAGS= -lm -llua

xgcm: xgcm.o xgcm_traversal.o xgcm_parser.o xgcm_conf.o ini.o\
		utils.o simple_ll.o string_buffer.o
	gcc -Wall -g ${LINK_FLAGS} $^ -o $@

%.o: %.c %.h
	gcc -Wall -g ${LINK_FLAGS} -c $<

clean:
	rm *.o xgcm
