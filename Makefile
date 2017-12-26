all: test

PERL_CCOPTS=$(shell export PATH=$(CURDIR)/staticperl.build/bin:$$PATH; perl -MExtUtils::Embed -e ccopts)
PERL_LDOPTS=$(shell export PATH=$(CURDIR)/staticperl.build/bin:$$PATH; perl -MExtUtils::Embed -e ldopts)

test:
	g++ -g  -std=c++17 $(PERL_CCOPTS)  0_state.cpp -o 0_state.exe $(PERL_LDOPTS); ./0_state.exe

prepare:
	sudo apt install libperl-dev

clean:
	rm -f *.o *.exe


# build in $(CURDIR)/staticperl.build
staticperl:
	wget http://cvs.schmorp.de/App-Staticperl/bin/staticperl;
	cp staticperl staticperl.tmp
	cat staticperl.tmp | sed -e 's/-g /-g -fPIC $(M32_OPT) /' \
			   | sed -e 's@STATICPERL=~/.staticperl@STATICPERL=$(CURDIR)/staticperl.build@' \
                           | sed -e 's/PERL_LDFLAGS=/PERL_LDFLAGS="$(M32_OPT)" /' > staticperl
	chmod a+x staticperl
	sh staticperl clean
	sh staticperl build
	sh staticperl install

gtags:
	cd staticperl.build/src/perl; ls *.c *.h | gtags -i -f -
