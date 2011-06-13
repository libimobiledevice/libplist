TARGET = cnary
LIBRARY = libcnary.a
OBJECTS = cnary.o libcnary.a
LIBRARY_OBJECTS = node.o list.o iterator.o node_list.o node_iterator.o
CFLAGS=-g -I./include -I/opt/local/include -mmacosx-version-min=10.5 -arch i386 -isysroot /Developer/SDKs/MacOSX10.5.sdk
LDFLAGS=-L/opt/local/lib -framework CoreFoundation -mmacosx-version-min=10.5 -arch i386 -isysroot /Developer/SDKs/MacOSX10.5.sdk -Wl,-no_compact_linkedit


%.o: %.c
	$(CC) -o $(@) -c $(^) $(CFLAGS)

$(LIBRARY): $(LIBRARY_OBJECTS)
	$(AR) rs $(@) $(^)
	
$(TARGET): $(OBJECTS)
	$(CC) -o $(@) $(^) $(CFLAGS) $(LDFLAGS)

all: $(TARGET)

clean:
	rm -rf $(TARGET) $(LIBRARY) $(OBJECTS) $(LIBRARY_OBJECTS)