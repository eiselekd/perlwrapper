#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstring>
#include <functional>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <random>
#include <set>
#include <stdexcept>
#include <string>
#include <sstream>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <boost/any.hpp>
#include <boost/mpl/distance.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/optional.hpp>
#include <boost/variant.hpp>
#include <boost/type_traits.hpp>

#include "./perlwrapper.h"

int main (int argc, char **argv) {

    PerlContext c;
    PerlContext::Map_Type m;
    PerlContext::Scalar_Type str;

    c.load("p.pl");

    /* use perl's tied scalar/hash to catch perl write/read with a c++ variable */
    c.bind("$::sa", str);
    c.bind("%::ma", m);

    c.execute("$::sa = \"Hello\"; $::ma{'a'} = 1; print ('Ret:'.$::ma{'a'}.\"\n\");");
    assert(m["a"] == "1");
    assert(str == "Hello");

    /* call out from perl */

    c.bind("&::fa", [](int v, int w, int x) { std::cout << v << std::endl; });

    c.execute("fa(1);");





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
