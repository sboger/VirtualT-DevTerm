# ===============================================
# Makefile for VirtualT.  Notice that the order
# of the fltk_images and fltk libraries in the
# LIBFILES definition is order dependent.
#
# This Makefile builds silently except to echo
# the current compile file.  To see the actual
# build commands, comment out the .SILENT:
# line below.
# ===============================================
.SILENT:

-include $(shell uname).mk

# ====================================
# Define base gcc compiler environment
# ====================================
SRCDIR		=	 src
OBJDIR		=    obj
DEPDIR		=    .dep

# Optimize only if not debug mode
ifeq ($(DEBUG),)
OPTIMIZE	=	-O2
endif

CFLAGS		+=	 -I $(SRCDIR)/FLU $(OPTIMIZE) $(DEBUG) -fsigned-char
CPPFLAGS	+=	 -I $(SRCDIR) $(OPTIMIZE) $(DEBUG)
VIRTUALT	=	 virtualt
CLIENT		=	 vt_client
ifndef CC
  CC		=    $(CROSS_COMPILE)gcc
else
  ifdef CROSS_COMPILE
    CC		=    $(CROSS_COMPILE)gcc
  endif
endif

# =============================
# Find the FLTK libs
# =============================
FLTKCONFIG  =   $(shell which fltk-config)
ifneq ($(FLTKCONFIG),)
FLTKLIB     =   $(shell $(FLTKCONFIG) --use-images --libs --ldflags)
CFLAGS      +=  $(shell $(FLTKCONFIG) --use-images --cflags)
CPPFLAGS    +=  $(shell $(FLTKCONFIG) --use-images --cxxflags)
endif
POSTBUILD	=   $(FLTKCONFIG) --use-images --post

ifeq ($(FLTKLIB),)
ifneq ($(FLTKDIR),)
FLTKLIB     =   $(FLTKDIR)/lib/libfltk.a
CFLAGS      +=  -I$(FLTKDIR)
CPPFLAGS    +=  -I$(FLTKDIR)
endif
endif

# ================================
# Define our linker flags and libs
# ================================
LDFLAGS		+=	$(shell $(FLTKCONFIG) --use-images --ldflags) -L/usr/X11R6/lib
LIBFILES	=	-lstdc++ $(FLTKLIB) -lm -lc -lX11 -lpthread -ldl -ljpeg -lpng -lXft -lXinerama -lfontconfig -lXext

# =============================
# Defines for MacOSX builds
# =============================
MACLDFLAGS	=	$(shell $(FLTKCONFIG) --use-images --ldflags) -arch i386 -arch ppc
MACLIBFILES	=	-lstdc++ `$(FLTKCONFIG) --use-images --ldstaticflags --use-images` --lm -lpthread

# ====================================
# Define all source files for VirtualT
# ====================================
VT_SOURCES	=   $(wildcard $(SRCDIR)/*.c) $(wildcard $(SRCDIR)/*.cpp)
VT_EXCLUDES =   inet_pton.c clientsocket.cpp vt_client_main.cpp
SOURCES		=   $(filter-out $(VT_EXCLUDES),$(patsubst $(SRCDIR)/%,%,$(VT_SOURCES)))

# =================================
# Define source files for vt_client
# =================================
CLIENT_SRC	=	clientsocket.cpp vt_client_main.cpp socket.cpp

# =============================
# Define the object files
# =============================
OBJTMP		=	$(SOURCES:.c=.o)
OBJTMP2		=	$(OBJTMP:.cpp=.o)
OBJECTS		=	$(patsubst %,$(OBJDIR)/%,$(OBJTMP2))

# Objects for the vt_client
CLIENT_OTMP	=	$(CLIENT_SRC:.cpp=.o)
CLIENT_OBJS	=	$(patsubst %,$(OBJDIR)/%,$(CLIENT_OTMP))

# ==========================================
# Declare auto-generated dependencies rules
# ==========================================
OTMP        =   $(OBJECTS) $(CLIENT_OBJS)
DEPS        =   $(patsubst $(OBJDIR)/%.o,$(DEPDIR)/%.d,$(OTMP))

# ===========================================================
# Rule for building all exectuables.  Must be the 1st target.
# ===========================================================
all:	init $(VIRTUALT) $(CLIENT)

.PHONY: init
init:
	@mkdir -p $(DEPDIR)
	@mkdir -p $(OBJDIR)

#Include our built dependencies
-include $(DEPS)

# ========================
# Rule to build VirtualT
# ========================
$(VIRTUALT):	$(OBJECTS)
ifndef FLTKLIB
	@echo "FLTKDIR environment variable must be set first!"
	@echo "Or install the FLTK libraries"
	exit 1
else
	# Test if FLTK libraries built
	for ONE_FLTKLIB in $(FLTKLIB); do \
		if ! test -f $(ONE_FLTKLIB); then \
			echo "Please ensure the FLTK, JPEG, PNG and ZLIB libraries and run make again"; \
			exit 1; \
		fi; \
	done
	@echo "Linking" $(VIRTUALT)
	if test -f /Developer/Tools/Rez; then \
		g++ $(MACLDFLAGS) $(OBJECTS) $(MACLIBFILES) -o $@ ; \
	else \
		$(CC) $(LDFLAGS) $(OBJECTS) $(LIBFILES) -o $@ ; \
	fi;
	cd ..

	# If bulding on MacOS, post the resource file to the executable
	if test -f /Developer/Tools/Rez; then \
		$(POSTBUILD) $(VIRTUALT); \
	fi;
endif

# ========================
# Rule to build vt_client
# ========================
$(CLIENT):		$(CLIENT_OBJS)
ifndef FLTKLIB
	@echo "FLTKDIR environment variable must be set first!"
	@echo "Or install the FLTK libraries"
	exit 1
else
	@echo "Linking" $(CLIENT)
	if test -f /Developer/Tools/Rez; then \
		g++ $(MACLDFLAGS) $(CLIENT_OBJS) $(MACLIBFILES) -o $@ ; \
	else \
		$(CC) $(LDFLAGS) $(CLIENT_OBJS) $(LIBFILES) -o $@ ; \
	fi
	cd ..
endif

# ============================================================
# Rule for compiling source files.  Define rule for CPP first.
# ============================================================
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
ifndef FLTKLIB
	@echo "FLTKDIR environment variable must be set first!"
	@echo "Or install the FLTK libraries"
	exit 1
else
	@echo "Compiling" $*.cpp
	$(CC) $(CPPFLAGS) -c $< -o $@
	@$(CC) -MM -MT $(OBJDIR)/$*.o $(CPPFLAGS) $(SRCDIR)/$*.cpp > $(DEPDIR)/$*.d
endif

# Now the rule for C files
$(OBJDIR)/%.o: $(SRCDIR)/%.c
ifndef FLTKLIB
	@echo "FLTKDIR environment variable must be set first!"
	@echo "Or install the FLTK libraries"
	exit 1
else
	@echo "Compiling" $*.c
	$(CC) $(CFLAGS) -c $< -o $@
	@$(CC) -MM -MT $(OBJDIR)/$*.o $(CFLAGS) $(SRCDIR)/$*.c > $(DEPDIR)/$*.d
endif

# =============================
# Rule to clean all build files
# =============================
.PHONY: clean
clean:
	@echo "=== cleaning ===";
	@echo "Objects..."
	@-rm -rf $(OBJDIR) $(DEPDIR)
	@echo "Executables..."
	-rm -f virtualt 
	-rm -rf vt_client

# ================================================
# Provide info for building FLTK, Tiger, Leopard versions
# Windows, Linux, etc.
# ================================================
.PHONY: info
info:
	@echo
	@echo "Virtual T make Info"
	@echo "==================="
	@echo "  To build VirtualT, you must first build the FLTK libraries"
	@echo "  and the FLTK sub-components zlib, png and jpeg.  For MacOSX"
	@echo "  if you are tyring to build a universal build, you should"
	@echo "  configure fltk with"
	@echo
	@echo "        ./configure --with-archflags=\"-arch i386 -arch ppc\""
	@echo
	@echo "  When compiling FLTK for a version of OSX earlier than the"
	@echo "  running version, us a configure line something smilar to:"
	@echo
	@echo "        MACOSX_DEPLOYMENT_TARGET=10.4 ./configure CXX=\"/usr/bin/g++-4.0\" \\"
	@echo "           CC=\"/usr/bin/gcc-4.0\" \\"
	@echo "           CPP=\"/usr/bin/cpp-4.0\" \\"
	@echo "           CFLAGS=\"-isysroot /Developer/SDKs/MacOSX10.4u.sdk\" \\"
	@echo "           --with-archflags=\"-arch i386 -arch ppc\""
	@echo

