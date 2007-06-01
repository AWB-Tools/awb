tool_PROGRAMS= apm-find-replace

apm_find_replace_SOURCES = apm_find_replace_head.pl                  \
                           apm_find_replace.ui apm_find_replace.ui.h \
                           apm_find_replace_main.pl

apm-find-replace$(EXEEXT): apm_find_replace_head.pl apm_find_replace.ui.pm apm_find_replace_main.pl
	cat apm_find_replace_head.pl apm_find_replace.ui.pm apm_find_replace_main.pl >$@
	chmod +x $@

apm_find_replace.ui.pm: apm_find_replace.ui apm_find_replace.ui.h

%.ui.pm: %.ui
	puic -o $@ $<

CLEANFILES = *.ui.pm

