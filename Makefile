# Distributed under the terms of the GNU General Public License v2
# $Header: /var/cvsroot/gentoo-projects/portage-utils/Makefile,v 1.1 2005/06/03 19:54:55 solar Exp $
####################################################################
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston,
# MA 02111-1307, USA.
####################################################################

####################################################
CFLAGS    := -Wall -Werror -Wformat=2 -Wmissing-declarations -Wwrite-strings -Wshadow -Wnested-externs -Wbad-function-cast
#CFLAGS   += -DEBUG -g
#LDFLAGS  :=-pie
DESTDIR    =
PREFIX    := $(DESTDIR)/usr
STRIP     := strip
MKDIR     := mkdir -p
CP        := cp

# Build with -Werror while emerging
ifneq ($(S),)
CFLAGS    += -Werror
endif
#####################################################
TARGETS    = q
OBJS       = ${TARGETS:%=%.o}
MPAGES     = ${TARGETS:%=man/%.1}
SOURCES    = ${OBJS:%.o=%.c}

all: $(OBJS) $(TARGETS)
	@:

debug: all
	@-/sbin/chpax  -permsx $(TARGETS)
	@-/sbin/paxctl -permsx $(TARGETS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

%: %.o
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

%.so: %.c
	$(CC) -shared -fPIC -o $@ $<

depend:
	$(CC) $(CFLAGS) -MM $(SOURCES) > .depend

clean:
	-rm -f $(OBJS) $(TARGETS)

distclean: clean
	-rm -f *~ core

install: all
	-$(STRIP) $(TARGETS)
	-$(MKDIR) $(PREFIX)/bin/ $(PREFIX)/share/man/man1/
	-$(CP) $(TARGETS) $(PREFIX)/bin/
	for mpage in $(MPAGES) ; do \
		[ -e $$mpage ] \
			&& cp $$mpage $(PREFIX)/share/man/man1/ || : ;\
	done

-include .depend
