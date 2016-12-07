/*
 * This file was generated automatically by ExtUtils::ParseXS version 3.18 from the
 * contents of Preproc.xs. Do not edit this file, edit Preproc.xs instead.
 *
 *    ANY CHANGES MADE HERE WILL BE LOST!
 *
 */

#line 1 "Preproc.xs"
#/* Verilog.xs -- Verilog Booter  -*- C++ -*-
#*********************************************************************
#*
#* DESCRIPTION: Verilog::Preproc Perl XS interface
#*
#* Author: Wilson Snyder <wsnyder@wsnyder.org>
#*
#* Code available from: http://www.veripool.org/
#*
#*********************************************************************
#*
#* Copyright 2000-2015 by Wilson Snyder.  This program is free software;
#* you can redistribute it and/or modify it under the terms of either the GNU
#* Lesser General Public License Version 3 or the Perl Artistic License Version 2.0.
#*
#* This program is distributed in the hope that it will be useful,
#* but WITHOUT ANY WARRANTY; without even the implied warranty of
#* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#* GNU General Public License for more details.
#*
#* You should have received a copy of the Perl Artistic License
#* along with this module; see the file COPYING.  If not, see
#* www.cpan.org
#*
#***********************************************************************
#* Note with C++ XS libraries, the CLASS parameter is implied...
#***********************************************************************/

/* Mine: */
#include "VPreProc.h"
#include <deque>

/* Perl */
extern "C" {
# include "EXTERN.h"
# include "perl.h"
# include "XSUB.h"
}

#ifdef open
# undef open	/* Perl 64 bit on solaris has a nasty hack that redefines open */
#endif

class VFileLineXs;

#//**********************************************************************
#// Preprocessor derived classes, so we can override the callbacks to call perl.

class VPreProcXs : public VPreProc {
public:
    SV*		m_self;	// Class called from (the hash, not SV pointing to the hash)
    deque<VFileLineXs*> m_filelineps;

    VPreProcXs() : VPreProc() {}
    virtual ~VPreProcXs();

    // Callback methods
    virtual void comment(string filename);	// Comment for keepComments=>sub
    virtual void include(string filename);	// Request a include file be processed
    virtual void define(string name, string value, string params); // `define with parameters
    virtual void undef(string name);		// Remove a definition
    virtual void undefineall();			// Remove all non-command-line definitions
    virtual bool defExists(string name);	// Return true if define exists
    virtual string defParams(string name);	// Return parameter list if define exists
    virtual string defValue(string name);	// Return value of given define (should exist)
    virtual string defSubstitute(string substitute);	// Return value to substitute for given post-parameter value

    void call(string* rtnStrp, int params, const char* method, ...);
    void unreadback(char* text);
};

class VFileLineXs : public VFileLine {
    VPreProcXs*	m_vPreprocp;		// Parser handling the errors
public:
    VFileLineXs(VPreProcXs* pp) : VFileLine(true), m_vPreprocp(pp) { if (pp) pushFl(); }
    virtual ~VFileLineXs() { }
    virtual VFileLine* create(const string& filename, int lineno) {
	VFileLineXs* filelp = new VFileLineXs(m_vPreprocp);
	filelp->init(filename, lineno);
	return filelp;
    }
    virtual void error(const string& msg);	// Report a error at given location
    void setPreproc(VPreProcXs* pp) {
	m_vPreprocp=pp;
	pushFl(); // The very first construction used pp=NULL, as pp wasn't created yet so make it now
    }
    // Record the structure so we can delete it later
    void pushFl() { m_vPreprocp->m_filelineps.push_back(this); }
};

#//**********************************************************************
#// Overrides error handling virtual functions to invoke callbacks

void VFileLineXs::error(const string& msg) {
    static string holdmsg; holdmsg = msg;
    m_vPreprocp->call(NULL, 1,"error",holdmsg.c_str());
}

#//**********************************************************************
#// VPreProcXs functions

VPreProcXs::~VPreProcXs() {
    for (deque<VFileLineXs*>::iterator it=m_filelineps.begin(); it!=m_filelineps.end(); ++it) {
	delete *it;
    }
}

#//**********************************************************************
#// Overrides of virtual functions to invoke callbacks

void VPreProcXs::comment(string cmt) {
    static string holdcmt; holdcmt = cmt;
    call(NULL, 1,"comment",holdcmt.c_str());
}
void VPreProcXs::include(string filename) {
    static string holdfilename; holdfilename = filename;
    call(NULL, 1,"include",holdfilename.c_str());
}
void VPreProcXs::undef(string define) {
    static string holddefine; holddefine = define;
    call(NULL, 1,"undef", holddefine.c_str());
}
void VPreProcXs::undefineall() {
    call(NULL, 0,"undefineall");
}
void VPreProcXs::define(string define, string value, string params) {
    static string holddefine; holddefine = define;
    static string holdvalue; holdvalue = value;
    static string holdparams; holdparams = params;
    // 4th argument is cmdline; always undef from here
    call(NULL, 3,"define", holddefine.c_str(), holdvalue.c_str(), holdparams.c_str());
}
bool VPreProcXs::defExists(string define) {
    return defParams(define)!="";
}
string VPreProcXs::defParams(string define) {
    static string holddefine; holddefine = define;
    string paramStr;
    call(&paramStr, 1,"def_params", holddefine.c_str());
    return paramStr;
}
string VPreProcXs::defValue(string define) {
    static string holddefine; holddefine = define;
    string valueStr;
    call(&valueStr, 1,"def_value", holddefine.c_str());
    return valueStr;
}
string VPreProcXs::defSubstitute(string subs) {
    static string holdsubs; holdsubs = subs;
    string outStr;
    call(&outStr, 1, "def_substitute", holdsubs.c_str());
    return outStr;
}

void VPreProcXs::call (
    string* rtnStrp,	/* If non-null, load return value here */
    int params,		/* Number of parameters.  Negative frees the parameters */
    const char* method,	/* Name of method to call */
    ...)		/* Arguments to pass to method's @_ */
{
    // Call $perlself->method (passedparam1, parsedparam2)
    va_list ap;
    va_start(ap, method);
    {
	dSP;				/* Initialize stack pointer */
	ENTER;				/* everything created after here */
	SAVETMPS;			/* ...is a temporary variable. */
	PUSHMARK(SP);			/* remember the stack pointer */
	SV* selfsv = newRV_inc(m_self);	/* $self-> */
	XPUSHs(sv_2mortal(selfsv));

	while (params--) {
	    char* text = va_arg(ap, char *);
	    SV* sv;
	    if (text) {
		sv = sv_2mortal(newSVpv (text, 0));
	    } else {
		sv = &PL_sv_undef;
	    }
	    XPUSHs(sv);			/* token */
	}

	PUTBACK;			/* make local stack pointer global */

	if (rtnStrp) {
	    int rtnCount = perl_call_method ((char*)method, G_SCALAR);
	    SPAGAIN;			/* refresh stack pointer */
	    if (rtnCount > 0) {
		SV* sv = POPs;
		//printf("RTN %ld %d %s\n", SvTYPE(sv),SvTRUE(sv),SvPV_nolen(sv));
#ifdef SvPV_nolen	// Perl 5.6 and later
		*rtnStrp = SvPV_nolen(sv);
#else
		*rtnStrp = SvPV(sv,PL_na);
#endif
	    }
	    PUTBACK;
	} else {
	    perl_call_method ((char*)method, G_DISCARD | G_VOID);
	}

	FREETMPS;			/* free that return value */
	LEAVE;				/* ...and the XPUSHed "mortal" args.*/
    }
    va_end(ap);
}

#//**********************************************************************

#line 220 "Preproc.c"
#ifndef PERL_UNUSED_VAR
#  define PERL_UNUSED_VAR(var) if (0) var = var
#endif

#ifndef dVAR
#  define dVAR		dNOOP
#endif


/* This stuff is not part of the API! You have been warned. */
#ifndef PERL_VERSION_DECIMAL
#  define PERL_VERSION_DECIMAL(r,v,s) (r*1000000 + v*1000 + s)
#endif
#ifndef PERL_DECIMAL_VERSION
#  define PERL_DECIMAL_VERSION \
	  PERL_VERSION_DECIMAL(PERL_REVISION,PERL_VERSION,PERL_SUBVERSION)
#endif
#ifndef PERL_VERSION_GE
#  define PERL_VERSION_GE(r,v,s) \
	  (PERL_DECIMAL_VERSION >= PERL_VERSION_DECIMAL(r,v,s))
#endif
#ifndef PERL_VERSION_LE
#  define PERL_VERSION_LE(r,v,s) \
	  (PERL_DECIMAL_VERSION <= PERL_VERSION_DECIMAL(r,v,s))
#endif

/* XS_INTERNAL is the explicit static-linkage variant of the default
 * XS macro.
 *
 * XS_EXTERNAL is the same as XS_INTERNAL except it does not include
 * "STATIC", ie. it exports XSUB symbols. You probably don't want that
 * for anything but the BOOT XSUB.
 *
 * See XSUB.h in core!
 */


/* TODO: This might be compatible further back than 5.10.0. */
#if PERL_VERSION_GE(5, 10, 0) && PERL_VERSION_LE(5, 15, 1)
#  undef XS_EXTERNAL
#  undef XS_INTERNAL
#  if defined(__CYGWIN__) && defined(USE_DYNAMIC_LOADING)
#    define XS_EXTERNAL(name) __declspec(dllexport) XSPROTO(name)
#    define XS_INTERNAL(name) STATIC XSPROTO(name)
#  endif
#  if defined(__SYMBIAN32__)
#    define XS_EXTERNAL(name) EXPORT_C XSPROTO(name)
#    define XS_INTERNAL(name) EXPORT_C STATIC XSPROTO(name)
#  endif
#  ifndef XS_EXTERNAL
#    if defined(HASATTRIBUTE_UNUSED) && !defined(__cplusplus)
#      define XS_EXTERNAL(name) void name(pTHX_ CV* cv __attribute__unused__)
#      define XS_INTERNAL(name) STATIC void name(pTHX_ CV* cv __attribute__unused__)
#    else
#      ifdef __cplusplus
#        define XS_EXTERNAL(name) extern "C" XSPROTO(name)
#        define XS_INTERNAL(name) static XSPROTO(name)
#      else
#        define XS_EXTERNAL(name) XSPROTO(name)
#        define XS_INTERNAL(name) STATIC XSPROTO(name)
#      endif
#    endif
#  endif
#endif

/* perl >= 5.10.0 && perl <= 5.15.1 */


/* The XS_EXTERNAL macro is used for functions that must not be static
 * like the boot XSUB of a module. If perl didn't have an XS_EXTERNAL
 * macro defined, the best we can do is assume XS is the same.
 * Dito for XS_INTERNAL.
 */
#ifndef XS_EXTERNAL
#  define XS_EXTERNAL(name) XS(name)
#endif
#ifndef XS_INTERNAL
#  define XS_INTERNAL(name) XS(name)
#endif

/* Now, finally, after all this mess, we want an ExtUtils::ParseXS
 * internal macro that we're free to redefine for varying linkage due
 * to the EXPORT_XSUB_SYMBOLS XS keyword. This is internal, use
 * XS_EXTERNAL(name) or XS_INTERNAL(name) in your code if you need to!
 */

#undef XS_EUPXS
#if defined(PERL_EUPXS_ALWAYS_EXPORT)
#  define XS_EUPXS(name) XS_EXTERNAL(name)
#else
   /* default to internal */
#  define XS_EUPXS(name) XS_INTERNAL(name)
#endif

#ifndef PERL_ARGS_ASSERT_CROAK_XS_USAGE
#define PERL_ARGS_ASSERT_CROAK_XS_USAGE assert(cv); assert(params)

/* prototype to pass -Wmissing-prototypes */
STATIC void
S_croak_xs_usage(pTHX_ const CV *const cv, const char *const params);

STATIC void
S_croak_xs_usage(pTHX_ const CV *const cv, const char *const params)
{
    const GV *const gv = CvGV(cv);

    PERL_ARGS_ASSERT_CROAK_XS_USAGE;

    if (gv) {
        const char *const gvname = GvNAME(gv);
        const HV *const stash = GvSTASH(gv);
        const char *const hvname = stash ? HvNAME(stash) : NULL;

        if (hvname)
            Perl_croak(aTHX_ "Usage: %s::%s(%s)", hvname, gvname, params);
        else
            Perl_croak(aTHX_ "Usage: %s(%s)", gvname, params);
    } else {
        /* Pants. I don't think that it should be possible to get here. */
        Perl_croak(aTHX_ "Usage: CODE(0x%"UVxf")(%s)", PTR2UV(cv), params);
    }
}
#undef  PERL_ARGS_ASSERT_CROAK_XS_USAGE

#ifdef PERL_IMPLICIT_CONTEXT
#define croak_xs_usage(a,b)    S_croak_xs_usage(aTHX_ a,b)
#else
#define croak_xs_usage        S_croak_xs_usage
#endif

#endif

/* NOTE: the prototype of newXSproto() is different in versions of perls,
 * so we define a portable version of newXSproto()
 */
#ifdef newXS_flags
#define newXSproto_portable(name, c_impl, file, proto) newXS_flags(name, c_impl, file, proto, 0)
#else
#define newXSproto_portable(name, c_impl, file, proto) (PL_Sv=(SV*)newXS(name, c_impl, file), sv_setpv(PL_Sv, proto), (CV*)PL_Sv)
#endif /* !defined(newXS_flags) */

#line 362 "Preproc.c"

XS_EUPXS(XS_Verilog__Preproc__new); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Verilog__Preproc__new)
{
    dVAR; dXSARGS;
    if (items != 7)
       croak_xs_usage(cv,  "CLASS, SELF, keepcmt, keepwhite, linedir, pedantic, synthesis");
    {
	SV *	SELF = ST(1)
;
	int	keepcmt = (int)SvIV(ST(2))
;
	int	keepwhite = (int)SvIV(ST(3))
;
	int	linedir = (int)SvIV(ST(4))
;
	int	pedantic = (int)SvIV(ST(5))
;
	int	synthesis = (int)SvIV(ST(6))
;
	char *	CLASS = (char *)SvPV_nolen(ST(0))
;
	VPreProcXs *	RETVAL;
#line 225 "Preproc.xs"
{
    if (CLASS) {}  /* Prevent unused warning */
    if (!SvROK(SELF)) { warn("${Package}::$func_name() -- SELF is not a hash reference"); }
    VFileLineXs* filelinep = new VFileLineXs(NULL/*ok,for initial*/);
    VPreProcXs* preprocp = new VPreProcXs();
    filelinep->setPreproc(preprocp);
    preprocp->m_self = SvRV(SELF);
    preprocp->keepComments(keepcmt);
    preprocp->keepWhitespace(keepwhite);
    preprocp->lineDirectives(linedir);
    preprocp->pedantic(pedantic);
    preprocp->synthesis(synthesis);
    preprocp->configure(filelinep);
    RETVAL = preprocp;
}
#line 402 "Preproc.c"
	ST(0) = sv_newmortal();
    // SELF->{_cthis} = THIS
    if( sv_isobject(SELF) && (SvTYPE(SvRV(SELF)) == SVt_PVHV) ) {
	SV **svp = hv_fetch ((HV*)SvRV(SELF), "_cthis", 6, 1);
	sv_setiv(*svp, PTR2IV( RETVAL ));
	XSRETURN_UNDEF;
    } else {
        warn( "Verilog::Preproc::_new() -- RETVAL is not a Verilog::Preproc object" );
        XSRETURN_UNDEF;
    }
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Verilog__Preproc__DESTROY); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Verilog__Preproc__DESTROY)
{
    dVAR; dXSARGS;
    if (items != 1)
       croak_xs_usage(cv,  "THIS");
    {
	VPreProcXs *    THIS = NULL;
    if( sv_isobject(ST(0)) && (SvTYPE(SvRV( ST(0) )) == SVt_PVHV) ) {
        SV **svp = hv_fetch ((HV*)SvRV(( ST(0) )), "_cthis", 6, 0);
	THIS = NULL;
	if (svp) { THIS = INT2PTR(VPreProcXs *,SvIV( *svp )); }
    }
    if (!THIS || !dynamic_cast<VPreProcXs*>(THIS)) {
        warn( "Verilog::Preproc::_DESTROY() -- THIS is not a Verilog::Preproc object" );
        XSRETURN_UNDEF;
    }
;
#line 249 "Preproc.xs"
{
    delete THIS;
}
#line 440 "Preproc.c"
    }
    XSRETURN_EMPTY;
}


XS_EUPXS(XS_Verilog__Preproc__debug); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Verilog__Preproc__debug)
{
    dVAR; dXSARGS;
    if (items != 2)
       croak_xs_usage(cv,  "THIS, level");
    {
	int	level = (int)SvIV(ST(1))
;
	VPreProcXs *    THIS = NULL;
    if( sv_isobject(ST(0)) && (SvTYPE(SvRV( ST(0) )) == SVt_PVHV) ) {
        SV **svp = hv_fetch ((HV*)SvRV(( ST(0) )), "_cthis", 6, 0);
	THIS = NULL;
	if (svp) { THIS = INT2PTR(VPreProcXs *,SvIV( *svp )); }
    }
    if (!THIS || !dynamic_cast<VPreProcXs*>(THIS)) {
        warn( "Verilog::Preproc::_debug() -- THIS is not a Verilog::Preproc object" );
        XSRETURN_UNDEF;
    }
;
#line 260 "Preproc.xs"
{
    THIS->debug(level);
}
#line 470 "Preproc.c"
    }
    XSRETURN_EMPTY;
}


XS_EUPXS(XS_Verilog__Preproc_lineno); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Verilog__Preproc_lineno)
{
    dVAR; dXSARGS;
    if (items != 1)
       croak_xs_usage(cv,  "THIS");
    {
	VPreProcXs *    THIS = NULL;
    if( sv_isobject(ST(0)) && (SvTYPE(SvRV( ST(0) )) == SVt_PVHV) ) {
        SV **svp = hv_fetch ((HV*)SvRV(( ST(0) )), "_cthis", 6, 0);
	THIS = NULL;
	if (svp) { THIS = INT2PTR(VPreProcXs *,SvIV( *svp )); }
    }
    if (!THIS || !dynamic_cast<VPreProcXs*>(THIS)) {
        warn( "Verilog::Preproc::lineno() -- THIS is not a Verilog::Preproc object" );
        XSRETURN_UNDEF;
    }
;
	int	RETVAL;
	dXSTARG;
#line 271 "Preproc.xs"
{
    if (!THIS) XSRETURN_UNDEF;
    RETVAL = (THIS->fileline()->lineno());
}
#line 501 "Preproc.c"
	XSprePUSH; PUSHi((IV)RETVAL);
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Verilog__Preproc_filename); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Verilog__Preproc_filename)
{
    dVAR; dXSARGS;
    if (items != 1)
       croak_xs_usage(cv,  "THIS");
    {
	VPreProcXs *    THIS = NULL;
    if( sv_isobject(ST(0)) && (SvTYPE(SvRV( ST(0) )) == SVt_PVHV) ) {
        SV **svp = hv_fetch ((HV*)SvRV(( ST(0) )), "_cthis", 6, 0);
	THIS = NULL;
	if (svp) { THIS = INT2PTR(VPreProcXs *,SvIV( *svp )); }
    }
    if (!THIS || !dynamic_cast<VPreProcXs*>(THIS)) {
        warn( "Verilog::Preproc::filename() -- THIS is not a Verilog::Preproc object" );
        XSRETURN_UNDEF;
    }
;
	SV *	RETVAL;
#line 284 "Preproc.xs"
{
    if (!THIS) XSRETURN_UNDEF;
    string ret = THIS->fileline()->filename();
    RETVAL = newSVpv(ret.c_str(), ret.length());
}
#line 533 "Preproc.c"
	ST(0) = RETVAL;
	sv_2mortal(ST(0));
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Verilog__Preproc_unreadback); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Verilog__Preproc_unreadback)
{
    dVAR; dXSARGS;
    if (items != 2)
       croak_xs_usage(cv,  "THIS, text");
    {
	char*	text = (char *)SvPV_nolen(ST(1))
;
	VPreProcXs *    THIS = NULL;
    if( sv_isobject(ST(0)) && (SvTYPE(SvRV( ST(0) )) == SVt_PVHV) ) {
        SV **svp = hv_fetch ((HV*)SvRV(( ST(0) )), "_cthis", 6, 0);
	THIS = NULL;
	if (svp) { THIS = INT2PTR(VPreProcXs *,SvIV( *svp )); }
    }
    if (!THIS || !dynamic_cast<VPreProcXs*>(THIS)) {
        warn( "Verilog::Preproc::unreadback() -- THIS is not a Verilog::Preproc object" );
        XSRETURN_UNDEF;
    }
;
#line 299 "Preproc.xs"
{
    if (!THIS) XSRETURN_UNDEF;
    THIS->insertUnreadback((string)text);
}
#line 566 "Preproc.c"
    }
    XSRETURN_EMPTY;
}


XS_EUPXS(XS_Verilog__Preproc_getall); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Verilog__Preproc_getall)
{
    dVAR; dXSARGS;
    if (items < 1 || items > 2)
       croak_xs_usage(cv,  "THIS, approx_chunk=0");
    {
	size_t	approx_chunk;
	VPreProcXs *    THIS = NULL;
    if( sv_isobject(ST(0)) && (SvTYPE(SvRV( ST(0) )) == SVt_PVHV) ) {
        SV **svp = hv_fetch ((HV*)SvRV(( ST(0) )), "_cthis", 6, 0);
	THIS = NULL;
	if (svp) { THIS = INT2PTR(VPreProcXs *,SvIV( *svp )); }
    }
    if (!THIS || !dynamic_cast<VPreProcXs*>(THIS)) {
        warn( "Verilog::Preproc::getall() -- THIS is not a Verilog::Preproc object" );
        XSRETURN_UNDEF;
    }
;
	SV *	RETVAL;

	if (items < 2)
	    approx_chunk = 0;
	else {
	    approx_chunk = (size_t)SvUV(ST(1))
;
	}
#line 312 "Preproc.xs"
{
    static string holdline;
    if (!THIS || THIS->isEof()) XSRETURN_UNDEF;
    string lastline = THIS->getall(approx_chunk);
    holdline = lastline;	/* Stash it so c_str() doesn't disappear immediately */
    if (holdline=="" && THIS->isEof()) XSRETURN_UNDEF;
    RETVAL = newSVpv(lastline.c_str(), lastline.length());
}
#line 608 "Preproc.c"
	ST(0) = RETVAL;
	sv_2mortal(ST(0));
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Verilog__Preproc_getline); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Verilog__Preproc_getline)
{
    dVAR; dXSARGS;
    if (items != 1)
       croak_xs_usage(cv,  "THIS");
    {
	VPreProcXs *    THIS = NULL;
    if( sv_isobject(ST(0)) && (SvTYPE(SvRV( ST(0) )) == SVt_PVHV) ) {
        SV **svp = hv_fetch ((HV*)SvRV(( ST(0) )), "_cthis", 6, 0);
	THIS = NULL;
	if (svp) { THIS = INT2PTR(VPreProcXs *,SvIV( *svp )); }
    }
    if (!THIS || !dynamic_cast<VPreProcXs*>(THIS)) {
        warn( "Verilog::Preproc::getline() -- THIS is not a Verilog::Preproc object" );
        XSRETURN_UNDEF;
    }
;
	SV *	RETVAL;
#line 329 "Preproc.xs"
{
    static string holdline;
    if (!THIS || THIS->isEof()) XSRETURN_UNDEF;
    string lastline = THIS->getline();
    holdline = lastline;	/* Stash it so c_str() doesn't disappear immediately */
    if (holdline=="" && THIS->isEof()) XSRETURN_UNDEF;
    RETVAL = newSVpv(lastline.c_str(), lastline.length());
}
#line 644 "Preproc.c"
	ST(0) = RETVAL;
	sv_2mortal(ST(0));
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Verilog__Preproc_eof); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Verilog__Preproc_eof)
{
    dVAR; dXSARGS;
    if (items != 1)
       croak_xs_usage(cv,  "THIS");
    {
	VPreProcXs *    THIS = NULL;
    if( sv_isobject(ST(0)) && (SvTYPE(SvRV( ST(0) )) == SVt_PVHV) ) {
        SV **svp = hv_fetch ((HV*)SvRV(( ST(0) )), "_cthis", 6, 0);
	THIS = NULL;
	if (svp) { THIS = INT2PTR(VPreProcXs *,SvIV( *svp )); }
    }
    if (!THIS || !dynamic_cast<VPreProcXs*>(THIS)) {
        warn( "Verilog::Preproc::eof() -- THIS is not a Verilog::Preproc object" );
        XSRETURN_UNDEF;
    }
;
	int	RETVAL;
	dXSTARG;
#line 346 "Preproc.xs"
{
    RETVAL = THIS->isEof();
}
#line 676 "Preproc.c"
	XSprePUSH; PUSHi((IV)RETVAL);
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Verilog__Preproc__open); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Verilog__Preproc__open)
{
    dVAR; dXSARGS;
    if (items != 2)
       croak_xs_usage(cv,  "THIS, filename");
    {
	const char *	filename = (const char *)SvPV_nolen(ST(1))
;
	VPreProcXs *    THIS = NULL;
    if( sv_isobject(ST(0)) && (SvTYPE(SvRV( ST(0) )) == SVt_PVHV) ) {
        SV **svp = hv_fetch ((HV*)SvRV(( ST(0) )), "_cthis", 6, 0);
	THIS = NULL;
	if (svp) { THIS = INT2PTR(VPreProcXs *,SvIV( *svp )); }
    }
    if (!THIS || !dynamic_cast<VPreProcXs*>(THIS)) {
        warn( "Verilog::Preproc::_open() -- THIS is not a Verilog::Preproc object" );
        XSRETURN_UNDEF;
    }
;
	int	RETVAL;
	dXSTARG;
#line 359 "Preproc.xs"
{
    if (!THIS) XSRETURN_UNDEF;
    THIS->openFile(filename);
    RETVAL = 1;
}
#line 711 "Preproc.c"
	XSprePUSH; PUSHi((IV)RETVAL);
    }
    XSRETURN(1);
}

#ifdef __cplusplus
extern "C"
#endif
XS_EXTERNAL(boot_Verilog__Preproc); /* prototype to pass -Wmissing-prototypes */
XS_EXTERNAL(boot_Verilog__Preproc)
{
    dVAR; dXSARGS;
#if (PERL_REVISION == 5 && PERL_VERSION < 9)
    char* file = (char*)__FILE__;
#else
    char* file = (char*)__FILE__;
#endif

    PERL_UNUSED_VAR(cv); /* -W */
    PERL_UNUSED_VAR(items); /* -W */
#ifdef XS_APIVERSION_BOOTCHECK
    XS_APIVERSION_BOOTCHECK;
#endif
    XS_VERSION_BOOTCHECK;

        (void)newXSproto_portable((char*)"Verilog::Preproc::_new", XS_Verilog__Preproc__new, file, (char*)"$$$$$$");
        (void)newXSproto_portable((char*)"Verilog::Preproc::_DESTROY", XS_Verilog__Preproc__DESTROY, file, (char*)"$");
        (void)newXSproto_portable((char*)"Verilog::Preproc::_debug", XS_Verilog__Preproc__debug, file, (char*)"$$");
        (void)newXSproto_portable((char*)"Verilog::Preproc::lineno", XS_Verilog__Preproc_lineno, file, (char*)"$");
        (void)newXSproto_portable((char*)"Verilog::Preproc::filename", XS_Verilog__Preproc_filename, file, (char*)"$");
        (void)newXSproto_portable((char*)"Verilog::Preproc::unreadback", XS_Verilog__Preproc_unreadback, file, (char*)"$$");
        (void)newXSproto_portable((char*)"Verilog::Preproc::getall", XS_Verilog__Preproc_getall, file, (char*)"$;$");
        (void)newXSproto_portable((char*)"Verilog::Preproc::getline", XS_Verilog__Preproc_getline, file, (char*)"$");
        (void)newXSproto_portable((char*)"Verilog::Preproc::eof", XS_Verilog__Preproc_eof, file, (char*)"$");
        (void)newXSproto_portable((char*)"Verilog::Preproc::_open", XS_Verilog__Preproc__open, file, (char*)"$$");
#if (PERL_REVISION == 5 && PERL_VERSION >= 9)
  if (PL_unitcheckav)
       call_list(PL_scopestack_ix, PL_unitcheckav);
#endif
    XSRETURN_YES;
}
