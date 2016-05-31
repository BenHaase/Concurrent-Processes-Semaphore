CC = gcc
CFLAGS = -g
TARGET1 = master
TARGET2 = slave
OBJS1 = master.o monitor.h
OBJS2 = slave.o monitor.h
LIBOBJS = monitor.o
LIBS = -lmonitor -lpthread
MYLIBS = libmonitor.a
LIBPATH = .
CLFILES = master slave master.o slave.o monitor.o libmonitor.a
.SUFFIXES: .c .o

all: master slave $(MYLIBS)
	

$(TARGET1): $(OBJS1) $(MYLIBS)
	$(CC) -o $@ -L. $(OBJS1) $(LIBS)

$(TARGET2): $(OBJS2) $(MYLIBS)
	$(CC) -o $@ -L. $(OBJS2) $(LIBS)

$(MYLIBS): $(LIBOBJS)
	ar -rs $@ $(LIBOBJS)

.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	/bin/rm -f *.o $(CLFILES) cstest
