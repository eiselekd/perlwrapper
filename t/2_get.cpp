#include "../perlwrapper.h"

int main (int argc, char **argv) {

    PerlContext c;

    std::map<std::string,std::string> a;
    c.require("perl_format.pl");
    c.bind("%a", a);

    c.execute("$a{'x'} = 1;");
    assert(a['x'] == "1");

    c.callPerl<int>("add", 1, 2);

    return 0;
}

/*
  Local Variables:
  compile-command:"gcc mem.c -o mem"
  mode:c++
  c-basic-offset:4
  c-file-style:"bsd"
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
