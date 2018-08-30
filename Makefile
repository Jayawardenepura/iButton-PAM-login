TARGET=pam_module hmac-sha256 sha256
DEPS:=$(addsuffix .o, $(TARGET))
TARGET:=$(addsuffix .c, $(TARGET))

LIBS=libserialport libconfig  

SRCDIR=src

DEP_SO=identify
DEP_SO:=$(addsuffix .so, $(DEP_SO))

LIBFLAGS:=$(shell pkg-config --cflags --libs $(LIBS))	# be sure to see what it does

CONFIG=ibutton_pam
CONFIG:=$(addsuffix .config, $(CONFIG)) 

CC=gcc
CFLAGS=-fpic -fno-stack-protector -c
SOFLAGS=-shared -Xlinker -x 

PAM_PATH=/lib/security/
CONFIG_PATH=/etc

.PHONY: all clean install

help:				## display this message
	@echo Available options:
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-20s\033[0m %s\n", $$1, $$2}'

all: $(DEPS)

$(DEPS):
	$(CC) $(CFLAGS) $(addprefix $(SRCDIR)/, $(TARGET))
	$(CC) $(SOFLAGS) -o $(DEP_SO) $(DEPS) $(LIBFLAGS)

install: 
	cp $(DEP_SO) $(PAM_PATH)
	cp $(CONFIG) $(CONFIG_PATH)
%.o: %.c
	$(CC) $(CFLAGS) -c $<
clean:
	@echo Tidying things up...
	-rm -f *.o $(DEPSD)
	-rm -f *.so $(DEP_SO)