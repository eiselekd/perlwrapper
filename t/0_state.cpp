#include "../perlwrapper.h"

int main (int argc, char **argv) {

    PerlContext c;

    struct Foo {
        static int increment(int x)
        {
            return x + 1;
        }
    };

    c.writeVariable("f", &Foo::increment);

    //c.executeCode<int>("return h(8)");

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
