CFLAGS=-fPIC -Wall -Wextra -O2 -g -Isrc -I jansson-2.7/src -I /home/vagrant/htslib
LDFLAGS=-L/usr/local/lib
LIBS=-ljansson -lcurl
RM=rm -f
TARGET_LIB=hfile_dnanexus.so

SRCS=$(wildcard src/*.c)
OBJS=$(patsubst %.c,%.o,$(SRCS))

TEST_SRCS=$(wildcard tests/*_tests.c)
TESTS=$(patsubst %.c,%,$(TEST_SRCS))

.PHONY: all
all: ${TARGET_LIB}

$(TARGET_LIB): $(OBJS)
	$(CC) -shared ${LDFLAGS} -o $@ $^ $(LIBS)

$(SRCS:.c=.d):%.d:%.c
	$(CC) $(CFLAGS) -MM $< >$@

include $(SRCS:.c=.d)

.PHONY: clean
clean:
	-${RM} ${TARGET_LIB} ${OBJS} $(SRCS:.c=.d) $(TESTS)

.PHONY: test
test: LDFLAGS += -L.
test: LDLIBS += $(TARGET_LIB) -ldl -lcurl -ljansson

test: $(TESTS)
