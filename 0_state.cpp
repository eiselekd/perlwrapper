#include "./perlwrapper.h"

int main (int argc, char **argv) {

    PerlContext c;
    PerlContext::Map_Type m;
    PerlContext::Scalar_Type str;

    c.load("p.pl");

    c.bind("$::sa", str);
    c.bind("%::ma", m);
    c.execute("$::sa = \"Hello\"; $::ma{'a'} = 1; print ('Ret:'.$::ma{'a'}.\"\n\");");
    assert(m["a"] == "1");
    assert(str == "Hello");





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
