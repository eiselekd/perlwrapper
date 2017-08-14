#ifndef PERL_WRAPPER_H
#define PERL_WRAPPER_H

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

/* note: run and add to g++ compile
PERL_CCOPTS=$(shell perl -MExtUtils::Embed -e ccopts)
PERL_LDOPTS=$(shell perl -MExtUtils::Embed -e ldopts)
*/
#include <EXTERN.h>               /* from the Perl distribution     */
#include <perl.h>                 /* from the Perl distribution     */

class PerlContext {
    enum Globals_t { Globals }; // tag for "global variables"
public:

    explicit PerlContext()
    {
        /*XSINIT_t f = xs_init;*/
        int exitstatus, argc = 1;
        const char *argv_[2] = {"test",0}; char **argv = 0;
        const char *embedding_[] = { "", "-e", "0", 0 }; char **embedding = 0;
        argv = const_to_char(argv_);
        embedding = const_to_char(embedding_);

	//PERL_SYS_INIT3(&argc,&argv,&env);
	PERL_SYS_INIT(&argc,&argv);
        my_perl = perl_alloc();
        perl_construct(my_perl);

        perl_construct(my_perl);
        PL_perl_destruct_level = 0;
        perl_parse(my_perl, /*f*/ NULL, 3, embedding, NULL);
        PL_exit_flags |= PERL_EXIT_DESTRUCT_END;
        perl_run(my_perl);
    };

    ~PerlContext() noexcept
    {
        perl_destruct(my_perl);
        perl_free(my_perl);
	PERL_SYS_TERM();
    };


    template<typename... TData>
    void writeVariable(TData&&... data) noexcept {
        typedef typename std::decay<typename std::tuple_element<sizeof...(TData) - 1,std::tuple<TData...>>::type>::type RealDataType;

        setTable<RealDataType>(my_perl, Globals, std::forward<TData>(data)...);
    }




    template<typename TDataType, typename TData>
    static void setTable(PerlInterpreter *my_perl, Globals_t g, const char* index, TData&& data) noexcept
    {
        auto p1 = Pusher<typename std::decay<TDataType>::type>::push(my_perl, std::forward<TData>(data));
        //lua_setglobal(state, index);
        p1.release();
    }


    /**************************************************/
    /*                 PUSH OBJECT                    */
    /**************************************************/
    struct PushedObject {
        PushedObject(PerlInterpreter *my_perl_, int num_ = 1) : my_perl(my_perl_), num(num_) {}
        ~PushedObject() {  }

        PushedObject& operator=(const PushedObject&) = delete;
        PushedObject(const PushedObject&) = delete;
        PushedObject& operator=(PushedObject&& other) { std::swap(my_perl, other.my_perl); std::swap(num, other.num); return *this; }
        PushedObject(PushedObject&& other) : my_perl(other.my_perl), num(other.num) { other.num = 0; }

        PushedObject operator+(PushedObject&& other) && { PushedObject obj(my_perl, num + other.num); num = 0; other.num = 0; return obj; }
        void operator+=(PushedObject&& other) { assert(my_perl == other.my_perl); num += other.num; other.num = 0; }

        auto getState() const -> PerlInterpreter * { return my_perl; }
        auto getNum() const -> int { return num; }

        int release() { const auto n = num; num = 0; return n; }
        void pop() { num = 0; }
        void pop(int n) { num -= n; }

    private:
        PerlInterpreter *my_perl;
        int num = 0;
    };

    /**************************************************/
    /*                PUSH FUNCTIONS                  */
    /**************************************************/

    // the Pusher structures allow you to push a value on the stack
    //  - static const int minSize : minimum size on the stack that the value can have
    //  - static const int maxSize : maximum size on the stack that the value can have
    //  - static int push(const LuaContext&, ValueType) : pushes the value on the stack and returns the size on the stack

    // implementation for custom objects
    template<typename TType, typename = void>
    struct Pusher {
        static const int minSize = 1;
        static const int maxSize = 1;

        template<typename TType2>
        static PushedObject push(PerlInterpreter *my_perl, TType2&& value) noexcept {
            PushedObject obj{my_perl, 1};

            return obj;
        }
    };

private:
    PerlInterpreter *my_perl;


    static char **const_to_char(const char **a) {
        int i; char **c = 0;
        for(i = 0; a[i]; i++) ;
        /* using libc malloc, perl's malloc not available yet */
        c = (char **) /*libc_*/malloc((i+1) * sizeof(char *));
        for(i = 0; a[i]; i++)
            c[i] = strdup(a[i]);
        c[i] = 0;
        return c;
    }

    static void rel_const_to_char(char **a) {
        int i;
        for(i = 0; a[i]; i++)
            /*libc_*/free(a[i]);
        /*libc_*/free(a);
    }

};

// C function pointers
template<typename TReturnType, typename... TParameters>
struct PerlContext::Pusher<TReturnType (*)(TParameters...)>
{
    // using the function-pushing implementation
    typedef Pusher<TReturnType (TParameters...)>
    SubPusher;
    static const int minSize = SubPusher::minSize;
    static const int maxSize = SubPusher::maxSize;

    template<typename TType>
    static PushedObject push(PerlInterpreter *my_perl, TType value) noexcept {
        return SubPusher::push(my_perl, value);
    }
};


#endif

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
