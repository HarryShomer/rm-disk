TARGET = rm-disk

CC = g++
CFLAGS = -g -Wall -std=c++17

SRCDIR = src
OBJDIR = obj
BINDIR = bin

SOURCES  := $(wildcard $(SRCDIR)/*.cpp)
INCLUDES := $(wildcard $(SRCDIR)/*.h)
OBJECTS  := $(SOURCES:$(SRCDIR)/%.cpp=$(BINDIR)/%.o)

$(BINDIR)/$(TARGET): $(OBJECTS)
	@$(CC) $(OBJECTS) $(CFLAGS) -o $@
	@echo "Linking complete!"

$(OBJECTS): $(BINDIR)/%.o : $(SRCDIR)/%.cpp
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "Compiled "$<" successfully!"

clean:
	rm -f $(BINDIR)/rm-disk $(BINDIR)/*.o $(BINDIR)/*~