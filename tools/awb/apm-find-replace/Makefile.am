tool_PROGRAMS= apm-find-replace

apm_find_replace_SOURCES = apm_find_replace_head.pl  \
                           apm_find_replace.ui     \
                           apm_find_replace.pm     \
                           apm_find_replace_main.pl

apm-find-replace$(EXEEXT): apm_find_replace_head.pl apm_find_replace.ui.pm apm_find_replace.pm apm_find_replace_main.pl
	cat apm_find_replace_head.pl apm_find_replace.ui.pm apm_find_replace.pm apm_find_replace_main.pl >$@
	chmod +x $@

apm_find_replace.ui.pm: apm_find_replace.ui

%.ui.pm: %.ui
	puic4 -o $@ $<


#
# Convert perl documentation to man page and install
#

PERLRUN = perl
MAN1EXT = 1
MANPERM = 644
MAN1DIR=$(mandir)/man1
POD2MAN_EXE = $(PERLRUN) "-MExtUtils::Command::MM" -e pod2man "--"
POD2MAN = $(POD2MAN_EXE) --section=$(MAN1EXT) --perm_rw=$(MANPERM)

install-data-hook:
	$(mkinstalldirs) $(DESTDIR)$(MAN1DIR)
	$(POD2MAN) apm-find-replace $(DESTDIR)$(MAN1DIR)/apm-find-replace.$(MAN1EXT)



CLEANFILES = *.ui.pm

