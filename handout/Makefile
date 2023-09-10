#-----------------------------------------------------------------------------
# Makefile
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# Choose a compiler and its options
#--------------------------------------------------------------------------
CC   = gcc -std=gnu99	# Use gcc for Zeus
OPTS = -Og -Wall -Werror -Wno-error=unused-variable -Wno-error=unused-function -pthread
DEBUG = -g # -g for GDB debugging

#--------------------------------------------------------------------
# Build Environment
#--------------------------------------------------------------------
SRCDIR=./src
OBJDIR=./obj
INCDIR=./inc
LIBDIR=./lib
BINDIR=.

#--------------------------------------------------------------------
# Build Files
#--------------------------------------------------------------------
SRCS=$(wildcard $(SRCDIR)/vm*.c)
HDRS=$(wildcard $(INCDIR)/*.h)

#--------------------------------------------------------------------
# Compiler Options
#--------------------------------------------------------------------
INCLUDE=$(addprefix -I,$(INCDIR))
LIBRARY=$(addprefix -L,$(OBJDIR))
SRCOBJS=${SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o}
INCS = $(wildcard $(INCDIR)/*.h)
OBJS=$(OBJDIR)/vm.o $(OBJDIR)/vm_cs.o $(OBJDIR)/vm_shell.o $(OBJDIR)/vm_support.o
HAKEOBJS=$(OBJDIR)/hake_sched.o
CFLAGS=$(OPTS) $(INCLUDE) $(LIBRARY) $(DEBUG)

HELPER_TARGETS=$(BINDIR)/slow_cooker $(BINDIR)/slow_door $(BINDIR)/slow_bug $(BINDIR)/slow_printer

#--------------------------------------------------------------------
# Build Recipies for the Executables (binary)
#--------------------------------------------------------------------
TARGET = $(BINDIR)/vm 
TARGET_LIB = $(LIBDIR)/vm_process.o

all: $(TARGET) helpers
lib: $(TARGET_LIB)

tester: $(TARGET) $(SRCDIR)/test_hake_sched.c $(OBJDIR)/vm_support.o $(OBJDIR)/hake_sched.o
	${CC} $(CFLAGS) -o $@ $(SRCDIR)/test_hake_sched.c $(OBJDIR)/vm_support.o $(OBJDIR)/hake_sched.o

helpers: $(HELPER_TARGETS)

$(BINDIR)/slow_cooker: $(OBJDIR)/slow_cooker.o
	${CC} ${CFLAGS} -o $@ $^

$(BINDIR)/slow_printer: $(OBJDIR)/slow_printer.o
	${CC} ${CFLAGS} -o $@ $^

$(BINDIR)/slow_door: $(OBJDIR)/slow_door.o
	${CC} ${CFLAGS} -o $@ $^

$(BINDIR)/slow_bug: $(OBJDIR)/slow_bug.o
	${CC} ${CFLAGS} -o $@ $^  

# Links the object files to create the target binary
$(TARGET): $(OBJS) $(HAKEOBJS) $(HDRS) $(INCDIR) $(OBJDIR)/libvm_sd.a
	${CC} ${CFLAGS} -o $@ $(OBJS) $(HAKEOBJS) -lvm_sd

# Links the object files to create the target binary
#$(OBJS): $(OBJDIR)/%.o : $(SRCDIR)/%.c 
$(OBJDIR)/%.o: $(SRCDIR)/%.c $(INCS)
	${CC} $(CFLAGS) -c -o $@ $<

# Links the object files to create the target binary
$(LIBDIR)/%.o: $(LIBDIR)/%.c $(INCS)
	${CC} $(CFLAGS) -c -o $@ $<
	ar -crs $(LIBDIR)/libvm_sd.a $@; mv $(LIBDIR)/libvm_sd.a $(OBJDIR)/libvm_sd.a

#--------------------------------------------------------------------
# Cleans the binaries
#--------------------------------------------------------------------
clean:
	rm -f $(OBJS) $(SRCOBJS) $(TARGET) $(HELPER_TARGETS) tester $(OBJDIR)/*.o $(LIBDIR)/*.o
