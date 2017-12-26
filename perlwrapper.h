#pragma once

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
#include <iostream>
#include <fstream>
#include <streambuf>

#define DO_DBG_PRINTF
#ifdef DO_DBG_PRINTF
#define DBG_PRINTF(format, ...) printf (format, ##__VA_ARGS__)
#else
#define DBG_PRINTF(format, ...) do {} while(0)
#endif

/* note: run and add to g++ compile
PERL_CCOPTS=$(shell perl -MExtUtils::Embed -e ccopts)
PERL_LDOPTS=$(shell perl -MExtUtils::Embed -e ldopts)
*/
#include <EXTERN.h>               /* from the Perl distribution     */
#include <perl.h>                 /* from the Perl distribution     */
#include "XSUB.h"

XS(XS_some_func);

int         perl_free (pTHX_ SV *sv, MAGIC* mg)
{
    printf("Cv free called \n");
    return 0;
}

STATIC MGVTBL my_vtbl = { 0, 0, 0, 0, perl_free, 0, 0, 0 };
struct my_priv_data_t {
    int a;
};

struct my_priv_data_t a{0xfafa};

class PerlContext;





class PerlContext {
private:
    PerlInterpreter *my_perl = nullptr;
public:

    template <typename Val>
    struct tie_data {
        class PerlContext *ctx;
        Val &v;
    };

    typedef std::map<std::string, std::string> Map_Type;
    typedef std::string Scalar_Type;
    typedef tie_data<Map_Type> Map_Tie;
    typedef tie_data<std::string> Scalar_Tie;

    int num;
    enum Globals_t { Globals }; // tag for "global variables"

    CV *PerlWrapper_Hash_STORE;
    CV *PerlWrapper_Hash_FETCH;
    CV *PerlWrapper_Scalar_STORE;
    CV *PerlWrapper_Scalar_FETCH;

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

        PerlWrapper_Hash_STORE = newXS("PerlWrapperHash::STORE", XS_PerlWrapper_HASH_STORE, __FILE__);
        PerlWrapper_Hash_FETCH = newXS("PerlWrapperHash::FETCH", XS_PerlWrapper_HASH_FETCH, __FILE__);
        PerlWrapper_Scalar_STORE = newXS("PerlWrapperScalar::STORE", XS_PerlWrapper_SCALAR_STORE, __FILE__);
        PerlWrapper_Scalar_FETCH = newXS("PerlWrapperScalar::FETCH", XS_PerlWrapper_SCALAR_FETCH, __FILE__);


    };

    ~PerlContext() noexcept
    {
        if (my_perl == nullptr)
            return;
        PL_perl_destruct_level = 1;
        perl_destruct(my_perl);
        perl_free(my_perl);
	PERL_SYS_TERM();

        printf("Exit PerlContext\n");
    };

    PerlContext &operator=(const PerlContext &other) = delete;
    PerlContext(PerlContext &&other) : my_perl(other.my_perl) { other.my_perl = nullptr; }

    bool load(const char *f) {
        std::ifstream infile(f);
        std::string str((std::istreambuf_iterator<char>(infile)),std::istreambuf_iterator<char>());
        return execute(str);
    }

    bool execute(const std::string &str) {
        SV *r = eval_pv( str.c_str(), TRUE);
    }

    /* https://perldoc.perl.org/perlguts.html#Stashes-and-Globs */
    bool bind(std::string str, Map_Type &m) {
        const char *n = str.c_str(); char c;
        c = (*n++);
        if (c != '%')
            return false;
        HV *hash = get_hv(n, GV_ADD);
        HV *inter = newHV();
        SV *tie = newRV_noinc((SV*)inter);
        HV *stash = gv_stashpv("PerlWrapperHash", GV_ADD);
        sv_bless(tie, stash);
        hv_magic(hash, (GV*)tie, PERL_MAGIC_tied);

        MAGIC *mg = sv_magicext((SV *)tie,
                                0,
                                PERL_MAGIC_ext,
                                &my_vtbl,
                                (const char*)(new Map_Tie{this, m}),
                                sizeof(Map_Tie));

    }

    bool bind(std::string str, std::string &m) {

        const char *n = str.c_str(); char c;
        c = (*n++);
        if (c != '$')
            return false;
        SV *sv = get_sv(n, GV_ADD);
        SV *inter = newSVpv(m.c_str(),0);
        SV *tie = newRV_noinc((SV*)inter);
        HV *stash = gv_stashpv("PerlWrapperScalar", GV_ADD);
        sv_bless(tie, stash);
        hv_magic(sv, (GV*)tie, PERL_MAGIC_tiedscalar);

        MAGIC *mg = sv_magicext((SV *)tie,
                                0,
                                PERL_MAGIC_ext,
                                &my_vtbl,
                                (const char*)(new Scalar_Tie{this, m}),
                                sizeof(Map_Tie));
    }

    static XSPROTO(XS_PerlWrapper_HASH_STORE)
    {
        int cnt = 0; SV *self, *keysv, *valsv;
        dXSARGS;

        PERL_UNUSED_ARG(cv);
        if(items != 3)
            croak("Usage: PerlWrapper::Store(self, key, val)");

        self = ST(0);
        keysv = ST(1);
        valsv = ST(2);
        SP -= items;

        MAGIC *mg;
        if ((mg = mg_findext((SV*)self, PERL_MAGIC_ext, &my_vtbl))) {
            Map_Tie *priv = (Map_Tie *)mg->mg_ptr;

            STRLEN key_len, val_len;
            const char *key = SvPV(keysv, key_len);
            const char *val = SvPV(valsv, val_len);
            std::string k = std::string(key, key_len);
            std::string v = std::string(val, val_len);

            DBG_PRINTF("set: %p['%s'] = '%s'\n", self, key, val);

            priv->v[k] = v;

        } else {
            DBG_PRINTF("get: %p ! no magic\n", self);
        }
        XSRETURN(cnt);
    }

    static XSPROTO(XS_PerlWrapper_HASH_FETCH)
    {
        int cnt = 0; SV *self, *keysv;
        dXSARGS;

        PERL_UNUSED_ARG(cv);
        if(items != 2)
            croak("Usage: PerlWrapper::Fetch(self, key)");

        self = ST(0);
        keysv = ST(1);
        SP -= items;

        MAGIC *mg;
        if ((mg = mg_findext((SV*)self, PERL_MAGIC_ext, &my_vtbl))) {
            Map_Tie *priv = (Map_Tie *)mg->mg_ptr;

            STRLEN key_len, val_len;
            const char *key = SvPV(keysv, key_len);
            std::string k = std::string(key, key_len);

            if (priv->v.find(k) == priv->v.end()) {
                DBG_PRINTF("self: %p['%s'] note present (created)\n", self, key);
            }

            std::string v = priv->v[k];
            DBG_PRINTF("get: %p['%s'] : '%s'\n", self, key, v.c_str());

            XPUSHs(sv_2mortal(newSVpv(v.c_str(),0)));
            cnt++;
        } else {
            XPUSHs(&PL_sv_undef);
            cnt++;
            DBG_PRINTF("get: %p ! no magic\n", self);
        }

        XSRETURN(cnt);
    }


    static XSPROTO(XS_PerlWrapper_SCALAR_STORE)
    {
        int cnt = 0; SV *self, *keysv, *valsv;
        dXSARGS;

        PERL_UNUSED_ARG(cv);
        if(items != 2)
            croak("Usage: PerlWrapper::Store(self, key, val)");

        self = ST(0);
        valsv = ST(1);
        SP -= items;

        MAGIC *mg;
        if ((mg = mg_findext((SV*)self, PERL_MAGIC_ext, &my_vtbl))) {
            Scalar_Tie *priv = (Scalar_Tie *)mg->mg_ptr;

            STRLEN val_len;
            const char *val = SvPV(valsv, val_len);
            std::string v = std::string(val, val_len);

            DBG_PRINTF("set: %p = '%s'\n", self, val);

            priv->v = v;

        } else {
            DBG_PRINTF("get: %p ! no magic\n", self);
        }
        XSRETURN(cnt);
    }

    static XSPROTO(XS_PerlWrapper_SCALAR_FETCH)
    {
        int cnt = 0; SV *self, *keysv;
        dXSARGS;

        PERL_UNUSED_ARG(cv);
        if(items != 1)
            croak("Usage: PerlWrapper::Fetch(self, key)");

        self = ST(0);
        SP -= items;

        MAGIC *mg;
        if ((mg = mg_findext((SV*)self, PERL_MAGIC_ext, &my_vtbl))) {
            Scalar_Tie *priv = (Scalar_Tie *)mg->mg_ptr;

            STRLEN key_len, val_len;

            std::string v = priv->v;
            DBG_PRINTF("get: %p : '%s'\n", self, v.c_str());

            XPUSHs(sv_2mortal(newSVpv(v.c_str(),0)));
            cnt++;
        } else {
            XPUSHs(&PL_sv_undef);
            cnt++;
            DBG_PRINTF("get: %p ! no magic\n", self);
        }

        XSRETURN(cnt);
    }







private:


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


XS(XS_some_func)
{
    dXSARGS;
    char *str_from_perl, *str_from_c;
    MAGIC *mg;
    if ((mg = mg_findext((SV*)cv, PERL_MAGIC_ext, &my_vtbl))) {
        /* this is really ours, not another module's PERL_MAGIC_ext */
        struct my_priv_data_t *priv = (struct my_priv_data_t *)mg->mg_ptr;
        printf("magic %x\n", priv->a);
    } else {
        printf("no magic\n");
    }

    /* get SV*s from the stack usign ST(x) and friends, do stuff to them */
    printf("Test\n");

    /* do your c thing calling back to your application, or whatever */

    /* pack up the c retval into an sv again and return it on the stack */
    XSRETURN(1);
}


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
