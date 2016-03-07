

CC=gcc
CFLAGS=-g -O2 -Wall -Wextra -Isrc $(OPTFLAGS)
LIBS=-ldl -lpthread $(OPTLIBS)
PREFIX?=/usr/local
LINK=gcc -o

SOURCES=$(wildcard src/**/*.c src/*.c)
OBJECTS=$(patsubst %.c,objs/%.o,$(SOURCES))

# for tests
OBJECTS_WITHOUT_ygxqqx=$(filter-out objs/src/ygxqqx.o,$(OBJECTS))

TEST_SRC=$(wildcard tests/*_test.c)
TESTS=$(patsubst %.c,%,$(TEST_SRC))

TARGET=objs/ygxqqx

all: $(TARGET) tests
ygxqqx: $(TARGET)

dev: CFLAGS=-g -Wall -Isrc -Wextra $(OPTFLAGS)
dev: all

$(TARGET): build $(OBJECTS)
	$(LINK) $(TARGET) $(OBJECTS) $(LIBS)

objs/%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

tests/%: tests/%.c 
	$(CC) $(CFLAGS) -o $@ $< $(OBJECTS_WITHOUT_ygxqqx) $(LIBS)

build:
	@mkdir -p objs/src

# 当前目录下有tests这个文件或目录，所以如果不加PHONY，则总是最新的，不会执行下面的指令
.PHONY: tests
tests: $(TESTS)
	@echo "compile test succeed"
	

valgrind:
	VALGRIND="valgrind --log-file=/tmp/valgrind-%p.log" $(MAKE)

# The Cleaner
clean:
	rm -rf objs $(TESTS)
	rm -f tests/tests.log
	rm -rf `find . -name "*.dSYM" -print`

# The Install
install: all
	install -d $(DESTDIR)/$(PREFIX)/lib/
	install $(TARGET) $(DESTDIR)/$(PREFIX)/lib/
