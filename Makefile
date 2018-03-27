# If you don't use CC 
CC       = gcc 

# Edit this line if you don't want yeahwm to install under /usr.
# Note that $(DESTDIR) is used by the Debian build process.
prefix = $(DESTDIR)/usr

XROOT    = /usr/X11R6
INCLUDES = -I$(XROOT)/include -I/usr/include/openmotif

LDPATH   = -L$(XROOT)/lib
LIBS     = -lX11

# Configure yeahwm by editing the following DEFINES lines.

# To support shaped windows properly, uncomment the following two lines:
DEFINES += -DSHAPE
LIBS	+= -lXext
#Xinerama

LIBS	+= -lXinerama

LIBS += -lxosd

# Uncomment to compile in certain text messages like help.  You want this too
# unless you *really* want to trim the bytes.
DEFINES += -DSTDIO



# Print whatever debugging messages I've left in this release.
#DEFINES += -DDEBUG	# miscellaneous debugging

# ----- You shouldn't need to change anything under this line ------ #

version = 0
revision = 3
subrev = 5

distdir = yeahwm-$(version).$(revision).$(subrev)
disttar = yeahwm_$(version).$(revision).$(subrev).tar.gz


disttar_test = yeahwm_test_$(version).$(revision).$(subrev).tar.gz
distdir_test = yeahwm_test-$(version).$(revision).$(subrev)

#DEFINES += -DSANITY	# active sanity checks
#DEFINES += -DXDEBUG	# show some X calls

DEFINES += -DVERSION=\"$(version).$(revision).$(subrev)\" $(DEBIAN)
CFLAGS  += $(INCLUDES) $(DEFINES)  -Wall  -Os
#CFLAGS  += $(INCLUDES) $(DEFINES) -g -Wall
CFLAGS  +=  -Wstrict-prototypes -Wpointer-arith -Wcast-align -Wcast-qual -Wshadow -Waggregate-return -Wnested-externs -Winline -Wwrite-strings -Wundef
LDFLAGS +=  $(LDPATH) $(LIBS)

HEADERS  = yeahwm.h
SRCS     = client.c events.c main.c misc.c new.c screen.c 
OBJS     = $(SRCS:.c=.o)

all: yeahwm

yeahwm: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)
	strip yeahwm

allinone:
	cat yeahwm.h $(SRCS) | sed 's/^#include.*yeahwm.*$$//' > allinone.c
	$(CC) $(CFLAGS) -o yeahwm allinone.c $(LDFLAGS)
	rm allinone.c
	strip yeahwm

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $<

doinstall:
	if [ -f yeahwm.exe ]; then mv yeahwm.exe yeahwm; fi
	mkdir -p $(prefix)/bin $(prefix)/share/man/man1
	install -s yeahwm $(prefix)/bin
	#install yeahwm.1 $(prefix)/share/man/man1
	#gzip -9 $(prefix)/share/man/man1/yeahwm.1

install: doinstall

dist: clean
	mkdir /tmp/$(distdir)
	cp -a . /tmp/$(distdir)/
	cd /tmp && tar cfz $(disttar) $(distdir) --exclude=$(distdir)/RCS --exclude=$(distdir)/tags --exclude=$(distdir)/.\#\* --exclude=$(distdir)/*.c~  --exclude=$(distdir)/cscope.out && rm -rf $(distdir)
	mv /tmp/$(disttar) ..
	ncftpput -f ~/.phrat / README  ../yeahwm_$(version).$(revision).$(subrev).tar.gz
	
dist_test: clean
	mkdir /tmp/$(distdir_test)
	cp -a . /tmp/$(distdir_test)/
	cd /tmp && tar cfz $(disttar_test) $(distdir_test) --exclude=$(distdir_test)/RCS --exclude=$(distdir_test)/tags --exclude=$(distdir_test)/.\#\* && rm -rf $(distdir_test)
	mv /tmp/$(disttar_test) ..
	

debuild: dist
	-cd ..; rm -rf $(distdir)/
	cd ..; tar xfz $(disttar)
	cp -a debian ../$(distdir)/
	rm -rf ../$(distdir)/debian/CVS/
	cd ../$(distdir); debuild

clean:
	rm -f yeahwm yeahwm.exe $(OBJS)
