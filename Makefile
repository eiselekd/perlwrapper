all:

PERL_CCOPTS=$(shell perl -MExtUtils::Embed -e ccopts)
PERL_LDOPTS=$(shell perl -MExtUtils::Embed -e ldopts)

test:
	g++ -g  -std=c++14 $(PERL_CCOPTS)  t/0_state.cpp -o 0_state.exe $(PERL_LDOPTS)

prepare:
	sudo apt install libperl-dev

clean:
	rm -f *.o *.exe
