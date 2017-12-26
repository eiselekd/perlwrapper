#include "../perlwrapper.h"

template <typename RET>
RET evalto(const char *str){
    RET v;
    return v;
}

int main (int argc, char **argv) {

    PerlContext c;

    std::function<int (int,int)> f = [](int a, int b) ->int{ return a+b; };
    c.writeFunction<int(int,int)>("add", f);
    c.execute("add();");

    assert(c.evalToInt("add(1,2)") == 3);

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
