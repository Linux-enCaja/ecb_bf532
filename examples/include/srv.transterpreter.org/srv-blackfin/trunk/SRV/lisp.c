/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * lisp.c: LISP interpreter for Surveyor SRV1 / SVS
 *
 * derived from "Scheme In One Defun" (SIOD) by George J. Carrette
 * and modified for robotics application by Surveyor Corporation
 *
 * COPYRIGHT (c) 2005-2008 by Surveyor Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details (www.gnu.org/licenses)
 *
 * Original SIOD source code and documentatino is found here - 
 *      http://people.delphiforums.com/gjc/siod.html
 * This is the original Paradigm Associates copyright notice:
 *
 * COPYRIGHT (c) 1989 BY                             
 * PARADIGM ASSOCIATES INCORPORATED, CAMBRIDGE, MASSACHUSETTS.    
 * ALL RIGHTS RESERVED   
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of Paradigm Associates Inc
 * not be used in advertising or publicity pertaining to distribution of
 * the software without specific, written prior permission.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <cdefBF537.h>
#include "srv.h"
#include "malloc.h"
#include "print.h"
#include "string.h"
#include "setjmp.h"
#include "i2c.h"
#include "colors.h"
#include "lisp.h"

// #define VERBOSE

LISP lheap_1,lheap_2;
LISP lheap,lheap_end,lheap_org;

char ugetc;  // buffer for unreadchar() character
int ugetf;  // flag to indicate that unreadchar was called
int lheap_size;
int old_lheap_used;
int which_lheap;
int gc_status_flag;
char tkbuffer[TKBUFFERN];
int errjmp[41];
int lquit;

LISP oblist;
LISP truth;
LISP eof_val;
LISP sym_errobj;
LISP sym_progn;
LISP sym_lambda;
LISP sym_quote;
LISP unbound_marker;
struct gc_protected *protected_registers;

int lisp_mode;  /*  0 = interactive mode, 1 = execute from buffer */
char *fp;

void lisp(char *fbuf) /* lisp main program */
{
  lisp_mode = (int)fbuf;   /*  if 0, run interactive mode, else execute from buffer */
  fp = fbuf;
  if (lisp_mode == 0)
    printf("starting interactive LISP;  hit ESC to exit\n\r");
  else
    printf("running LISP program from flash buffer\n\r");

  ugetc = 0;  
  ugetf = 0;  
  gc_status_flag = 1;
  lquit = 0;
  lheap_size = 100000;

  init_storage();
  init_subrs();
  our_subrs();
  print_hs_1();
  errjmp[40] = 0;
  setjmp(errjmp);
  if (errjmp[40]) {
    if (lquit) {
      printf("\n\rgoodbye ...\n\r");
      return;
    }
  }
  repl();
}

void print_hs_1()
{
  printf("lheap_size = %d cells, %d bytes\n\r",
        lheap_size, lheap_size*sizeof(struct obj));
}

void print_hs_2()
{
  printf("heap_1 = 0x%X, heap_2 = 0x%x, free stack = %d\n\r",
        lheap_1, lheap_2, stack_remaining());
}


void repl() 
{LISP x,cw;
 while(1)
 {
    if ((gc_status_flag) || lheap >= lheap_end)
    {
      gc();
      #ifdef VERBOSE
        printf("GC: %d compressed to %d, %d free\n\r",
             old_lheap_used,lheap-lheap_org,lheap_end-lheap);
      #endif
    }
    printf("> ");
    x = lread();
    if EQ(x,eof_val) break;
    cw = lheap;
    x = leval(x,NIL);
    #ifdef VERBOSE
      printf("Eval used %d cons\n\r", lheap-cw);
    #endif
    lprint(x);
  }
}


void err(message,x)
 char *message; LISP x;
{
 if NNULLP(x) 
   printf("ERROR: %s (see errobj)\n\r",message);
 else 
   printf("ERROR: %s\n\r",message); 
 //print_hs_2();
 errjmp[40] = 1;
 longjmp(errjmp, 1);
}


LISP lerr(message,x)
     LISP message,x;
{
 if NTYPEP(message,tc_symbol) err("argument to error not a symbol",message);
 err(PNAME(message),x);
 return(NIL);
}


LISP cons(x,y)
     LISP x,y;
{LISP z;
 if ((z = lheap) >= lheap_end) err("ran out of storage",NIL);
 lheap = z+1;
 (*z).gc_mark = 0;
 (*z).type = tc_cons;
 CAR(z) = x;
 CDR(z) = y;
 return(z);}

LISP consp(x)
     LISP x;
{
 if TYPEP(x,tc_cons) return(truth); else return(NIL);
}

LISP car(x)
     LISP x;
{
 switch TYPE(x)
   {case tc_nil:
      return(NIL);
    case tc_cons:
      return(CAR(x));
    default:
      err("wta to car",x);}}

LISP cdr(x)
     LISP x;
{
 switch TYPE(x)
   {case tc_nil:
      return(NIL);
    case tc_cons:
      return(CDR(x));
    default:
      err("wta to cdr",x);}}


LISP setcar(cell,value)
     LISP cell, value;
{
 if NTYPEP(cell,tc_cons) err("wta to setcar",cell);
 return(CAR(cell) = value);
}

LISP setcdr(cell,value)
     LISP cell, value;
{
 if NTYPEP(cell,tc_cons) err("wta to setcdr",cell);
 return(CDR(cell) = value);
}

LISP flocons(x)
 int x;
{LISP z;
 if ((z = lheap) >= lheap_end) err("ran out of storage",NIL);
 lheap = z+1;
 (*z).gc_mark = 0;
 (*z).type = tc_flonum;
 (*z).storage_as.flonum.data = x;
 return(z);}

LISP numberp(x)
     LISP x;
{
 if TYPEP(x,tc_flonum) return(truth); else return(NIL);
}

LISP plus(x,y)
     LISP x,y;
{
 if NTYPEP(x,tc_flonum) err("wta(1st) to plus",x);
 if NTYPEP(y,tc_flonum) err("wta(2nd) to plus",y);
 return(flocons(FLONM(x)+FLONM(y)));
}

LISP ltimes(x,y)
 LISP x,y;
{
 if NTYPEP(x,tc_flonum) err("wta(1st) to times",x);
 if NTYPEP(y,tc_flonum) err("wta(2nd) to times",y);
 return(flocons(FLONM(x)*FLONM(y)));
}

LISP difference(x,y)
 LISP x,y;
{
 if NTYPEP(x,tc_flonum) err("wta(1st) to difference",x);
 if NTYPEP(y,tc_flonum) err("wta(2nd) to difference",y);
 return(flocons(FLONM(x)-FLONM(y)));
}

LISP quotient(x,y)
 LISP x,y;
{
 if NTYPEP(x,tc_flonum) err("wta(1st) to quotient",x);
 if NTYPEP(y,tc_flonum) err("wta(2nd) to quotient",y);
 return(flocons(FLONM(x)/FLONM(y)));
}

LISP greaterp(x,y)
 LISP x,y;
{
 if NTYPEP(x,tc_flonum) err("wta(1st) to greaterp",x);
 if NTYPEP(y,tc_flonum) err("wta(2nd) to greaterp",y);
 if (FLONM(x)>FLONM(y)) return(truth);
 return(NIL);
}

LISP lessp(x,y)
 LISP x,y;
{
 if NTYPEP(x,tc_flonum) err("wta(1st) to lessp",x);
 if NTYPEP(y,tc_flonum) err("wta(2nd) to lessp",y);
 if (FLONM(x)<FLONM(y)) return(truth);
 return(NIL);
}

LISP equalp(x,y)
 LISP x,y;
{
 if NTYPEP(x,tc_flonum) err("wta(1st) to lessp",x);
 if NTYPEP(y,tc_flonum) err("wta(2nd) to lessp",y);
 if (FLONM(x)==FLONM(y)) return(truth);
 return(NIL);
}

LISP eq(x,y)
 LISP x,y;
{
 if EQ(x,y) return(truth); else return(NIL);
}

LISP eql(x,y)
 LISP x,y;
{
 if EQ(x,y) return(truth); else 
 if NTYPEP(x,tc_flonum) return(NIL); else
 if NTYPEP(y,tc_flonum) return(NIL); else
 if (FLONM(x) == FLONM(y)) return(truth);
 return(NIL);
}

LISP symcons(pname,vcell)
 char *pname; LISP vcell;
{
 LISP z;
 if ((z = lheap) >= lheap_end) err("ran out of storage",NIL);
 lheap = z+1;
 (*z).gc_mark = 0;
 (*z).type = tc_symbol;
 PNAME(z) = pname;
 VCELL(z) = vcell;
 return(z);
}

LISP symbolp(x)
     LISP x;
{
 if TYPEP(x,tc_symbol) return(truth); else return(NIL);
}

LISP symbol_boundp(x,env)
 LISP x,env;
{
 LISP tmp;
 if NTYPEP(x,tc_symbol) err("not a symbol",x);
 tmp = envlookup(x,env);
 if NNULLP(tmp) return(truth);
 if EQ(VCELL(x),unbound_marker) return(NIL); else return(truth);
}

LISP symbol_value(x,env)
 LISP x,env;
{
 LISP tmp;
 if NTYPEP(x,tc_symbol) err("not a symbol",x);
 tmp = envlookup(x,env);
 if NNULLP(tmp) return(CAR(tmp));
 tmp = VCELL(x);
 if EQ(tmp,unbound_marker) err("unbound variable",x);
 return(tmp);
}

LISP cintern_soft(name)
 char *name;
{
 LISP l;
 for(l=oblist;NNULLP(l);l=CDR(l))
   if (strcmp(name,PNAME(CAR(l))) == 0) return(CAR(l));
 return(NIL);
}

LISP cintern(name)
 char *name;
{
 LISP sym;
 sym = cintern_soft(name);
 if(sym) return(sym);
 sym = symcons(name,unbound_marker);
 oblist = cons(sym,oblist);
 return(sym);
}

char * must_malloc(size)
     unsigned int size;
{
 char *tmp;
 tmp = (char *) malloc(size);
 if (tmp == (char *)NULL) err("failed to allocate storage from system",NIL);
 return(tmp);
}

LISP rintern(name)
 char *name;
{
 LISP sym;
 char *newname;
 sym = cintern_soft(name);
 if(sym) return(sym);
 newname = must_malloc(strlen(name)+1);
 strcpy(newname,name);
 sym = symcons(newname,unbound_marker);
 oblist = cons(sym,oblist);
 return(sym);
}

LISP subrcons(type,name,f)
 int type; char *name; LISP (*f)();
{
 LISP z;
 if ((z = lheap) >= lheap_end) err("ran out of storage",NIL);
 lheap = z+1;
 (*z).gc_mark = 0;
 (*z).type = type;
 (*z).storage_as.subr.name = name;
 (*z).storage_as.subr.f = f;
 return(z);
}

LISP closure(env,code)
     LISP env,code;
{
 LISP z;
 if ((z = lheap) >= lheap_end) err("ran out of storage",NIL);
 lheap = z+1;
 (*z).gc_mark = 0;
 (*z).type = tc_closure;
 (*z).storage_as.closure.env = env;
 (*z).storage_as.closure.code = code;
 return(z);
}

void gc_protect(location)
 LISP *location;
{
 struct gc_protected *reg;
 reg = (struct gc_protected *) must_malloc(sizeof(struct gc_protected));
 (*reg).location = location;
 (*reg).next = protected_registers;
  protected_registers = reg;
 }

void scan_registers()
{
 struct gc_protected *reg;
 for(reg = protected_registers; reg; reg = (*reg).next) {
   *((*reg).location) = gc_relocate(*((*reg).location));
 }
}

void init_storage()
{
 oblist = NIL;
 truth = NIL;
 eof_val = NIL;
 sym_errobj = NIL;
 sym_progn = NIL;
 sym_lambda = NIL;
 sym_quote = NIL;
 unbound_marker = NIL;
 protected_registers = NULL;

 lheap_1 = (LISP) must_malloc(sizeof(struct obj)*lheap_size);
 lheap_2 = (LISP) must_malloc(sizeof(struct obj)*lheap_size);
 lheap = lheap_1;
 which_lheap = 1;
 lheap_org = lheap;
 lheap_end = lheap + lheap_size;
 //old_lheap_used = 0;
 unbound_marker = cons(cintern("**unbound-marker**"),NIL);
 gc_protect(&unbound_marker);
 eof_val = cons(cintern("eof"),NIL);
 gc_protect(&eof_val);
 truth = cintern("t");
 gc_protect(&truth);
 setvar(truth,truth,NIL);
 setvar(cintern("nil"),NIL,NIL);
 setvar(cintern("let"),cintern("let-internal-macro"),NIL);
 sym_errobj = cintern("errobj");
 gc_protect(&sym_errobj);
 setvar(sym_errobj,NIL,NIL);
 sym_progn = cintern("begin");
 gc_protect(&sym_progn);
 sym_lambda = cintern("lambda");
 gc_protect(&sym_lambda);
 sym_quote = cintern("quote");
 gc_protect(&sym_quote);
 gc_protect(&oblist);
}

void init_subr(name,type,fcn)
 char *name; int type; LISP (*fcn)();
{
 setvar(cintern(name),subrcons(type,name,fcn),NIL);
}

LISP assq(x,alist)
     LISP x,alist;
{
 LISP l,tmp;
 for(l=alist;TYPEP(l,tc_cons);l=CDR(l))
   {tmp = CAR(l);
    if (TYPEP(tmp,tc_cons) && EQ(CAR(tmp),x)) return(tmp);}
 if EQ(l,NIL) return(NIL);
 err("improper list to assq",alist);
}

LISP gc_relocate(x)
     LISP x;
{
 LISP new;
 if EQ(x,NIL) return(NIL);
 if ((*x).gc_mark == 1) return(CAR(x));
 switch TYPE(x)
   {case tc_flonum:
      new = flocons(FLONM(x));
      break;
    case tc_cons:
      new = cons(CAR(x),CDR(x));
      break;
    case tc_symbol:
      new = symcons(PNAME(x),VCELL(x));
      break;
    case tc_closure:
      new = closure((*x).storage_as.closure.env,
            (*x).storage_as.closure.code);
      break;
    case tc_subr_0:
    case tc_subr_1:
    case tc_subr_2:
    case tc_subr_3:
    case tc_lsubr:
    case tc_fsubr:
    case tc_msubr:
      new = subrcons(TYPE(x),
             (*x).storage_as.subr.name,
             (*x).storage_as.subr.f);
      break;
    default: err("BUG IN GARBAGE COLLECTOR gc_relocate",NIL);}
 (*x).gc_mark = 1;
 CAR(x) = new;
 return(new);
}

LISP get_newspace()
{
 LISP newspace;
 if (which_lheap == 1)
   {newspace = lheap_2;
    which_lheap = 2;}
 else
   {newspace = lheap_1;
    which_lheap = 1;}
 lheap = newspace;
 lheap_org = lheap;
 lheap_end = lheap + lheap_size;
 return(newspace);
}

void scan_newspace(newspace)
     LISP newspace;
{
 LISP ptr;
 for(ptr=newspace; ptr < lheap; ++ptr)
   {
    switch TYPE(ptr)
      {
       case tc_cons:
       case tc_closure:
         CAR(ptr) = gc_relocate(CAR(ptr));
         CDR(ptr) = gc_relocate(CDR(ptr));
         break;
       case tc_symbol:
         VCELL(ptr) = gc_relocate(VCELL(ptr));
         break;
       default:
         break;
      }
   }
}
      
void gc()
{
 LISP newspace;
 old_lheap_used = lheap - lheap_org;
 newspace = get_newspace();
 scan_registers();
 scan_newspace(newspace);
}

LISP gc_status(args)
     LISP args;
{
  if NNULLP(args) {
    if NULLP(car(args))
      gc_status_flag = 0; 
    else
      gc_status_flag = 1;
  }
  if (gc_status_flag)
    printf("garbage collection is on\n\r"); 
  else
    printf("garbage collection is off\n\r");
  printf("%d allocated %d free\n\r",lheap - lheap_org, lheap_end - lheap);
  return(NIL);
}

LISP leval_args(l,env)
     LISP l,env;
{
 LISP result,v1,v2,tmp;
 if NULLP(l) return(NIL);
 if NTYPEP(l,tc_cons) err("bad syntax argument list",l);
 result = cons(leval(CAR(l),env),NIL);
 for(v1=result,v2=CDR(l);
     TYPEP(v2,tc_cons);
     v1 = tmp, v2 = CDR(v2))
  {tmp = cons(leval(CAR(v2),env),NIL);
   CDR(v1) = tmp;}
 if NNULLP(v2) err("bad syntax argument list",l);
 return(result);
}

LISP extend_env(actuals,formals,env)
 LISP actuals,formals,env;
{
 if TYPEP(formals,tc_symbol)
    return(cons(cons(cons(formals,NIL),cons(actuals,NIL)),env));
 return(cons(cons(formals,actuals),env));
}

LISP envlookup(var,env)
 LISP var,env;
{
 LISP frame,al,fl,tmp;
 for(frame=env;TYPEP(frame,tc_cons);frame=CDR(frame))
   {tmp = CAR(frame);
    if NTYPEP(tmp,tc_cons) err("damaged frame",tmp);
    for(fl=CAR(tmp),al=CDR(tmp);
    TYPEP(fl,tc_cons);
    fl=CDR(fl),al=CDR(al))
      {if NTYPEP(al,tc_cons) err("too few arguments",tmp);
       if EQ(CAR(fl),var) return(al);}}
 if NNULLP(frame) err("damaged env",env);
 return(NIL);
}

LISP leval(x,env)
 LISP x,env;
{
  LISP tmp;

loop:
  switch TYPE(x) {
    case tc_symbol:
      tmp = envlookup(x,env);
      if (tmp) return(CAR(tmp));
      tmp = VCELL(x);
      if EQ(tmp,unbound_marker) err("unbound variable",x);
      return(tmp);
    case tc_cons:
      tmp = leval(CAR(x),env);
      switch TYPE(tmp) {
        case tc_subr_0:
          return(SUBRF(tmp)());
        case tc_subr_1:
          return(SUBRF(tmp)(leval(car(CDR(x)),env)));
        case tc_subr_2:
          return(SUBRF(tmp)(leval(car(CDR(x)),env),
                 leval(car(cdr(CDR(x))),env)));
        case tc_subr_3:
          return(SUBRF(tmp)(leval(car(CDR(x)),env),
                 leval(car(cdr(CDR(x))),env),
                 leval(car(cdr(cdr(CDR(x)))),env)));
        case tc_lsubr:
          return(SUBRF(tmp)(leval_args(CDR(x),env)));
        case tc_fsubr:
          return(SUBRF(tmp)(CDR(x),env));
        case tc_msubr:
          if NULLP(SUBRF(tmp)(&x,&env)) return(x);
          goto loop;
        case tc_closure:
          env = extend_env(leval_args(CDR(x),env),
                car((*tmp).storage_as.closure.code),
                (*tmp).storage_as.closure.env);
          x = cdr((*tmp).storage_as.closure.code);
          goto loop;
        case tc_symbol:
          x = cons(tmp,cons(cons(sym_quote,cons(x,NIL)),NIL));
          x = leval(x,NIL);
          goto loop;
        default:
          err("bad function",tmp);
       }
       default:
         return(x);
  }
}

LISP setvar(var,val,env)
 LISP var,val,env;
{
 LISP tmp;
 if NTYPEP(var,tc_symbol) err("wta(non-symbol) to setvar",var);
 tmp = envlookup(var,env);
 if NULLP(tmp) return(VCELL(var) = val);
 return(CAR(tmp)=val);
}

LISP leval_setq(args,env)
 LISP args,env;
{
 return(setvar(car(args),leval(car(cdr(args)),env),env));
}

LISP syntax_define(args)
 LISP args;
{
 if TYPEP(car(args),tc_symbol) return(args);
 return(syntax_define(
        cons(car(car(args)),
    cons(cons(sym_lambda,
         cons(cdr(car(args)),
          cdr(args))),
         NIL))));
}
      
LISP leval_define(args,env)
 LISP args,env;
{
 LISP tmp,var,val;
 tmp = syntax_define(args);
 var = car(tmp);
 if NTYPEP(var,tc_symbol) err("wta(non-symbol) to define",var);
 val = leval(car(cdr(tmp)),env);
 tmp = envlookup(var,env);
 if NNULLP(tmp) return(CAR(tmp) = val);
 if NULLP(env) return(VCELL(var) = val);
 tmp = car(env);
 setcar(tmp,cons(var,car(tmp)));
 setcdr(tmp,cons(val,cdr(tmp)));
 return(val);
}
 
LISP leval_if(pform,penv)
 LISP *pform,*penv;
{
 LISP args,env;
 args = cdr(*pform);
 env = *penv;
 if NNULLP(leval(car(args),env)) 
    *pform = car(cdr(args)); else *pform = car(cdr(cdr(args)));
 return(truth);
}

LISP leval_while(args,env)
 LISP args, env;
{
 LISP l;
 while NNULLP(leval(car(args),env))
   for(l=cdr(args);NNULLP(l);l=cdr(l))
     leval(car(l),env);
 return(NIL);
}

//LISP leval_cond(args,env)  // this is broken
// LISP args, env;
//{
// while (1) {
//   if NNULLP(leval(car(args),env))
//     return(truth);
//   args=cdr(args);
//   if NULLP(args)
//     return(NIL);
// }
//}

LISP leval_lambda(args,env)
 LISP args,env;
{
 LISP body;
 if NULLP(cdr(cdr(args)))
   body = car(cdr(args));
  else body = cons(sym_progn,cdr(args));
 return(closure(env,cons(arglchk(car(args)),body)));
}
                         
LISP leval_progn(pform,penv)
 LISP *pform,*penv;
{
 LISP env,l,next;
 env = *penv;
 l = cdr(*pform);
 next = cdr(l);
 while(NNULLP(next)) {leval(car(l),env);l=next;next=cdr(next);}
 *pform = car(l); 
 return(truth);
}

LISP leval_or(pform,penv)
 LISP *pform,*penv;
{
 LISP env,l,next,val;
 env = *penv;
 l = cdr(*pform);
 next = cdr(l);
 while(NNULLP(next))
   {val = leval(car(l),env);
    if NNULLP(val) {*pform = val; return(NIL);}
    l=next;next=cdr(next);}
 *pform = car(l); 
 return(truth);
}

LISP leval_and(pform,penv)
 LISP *pform,*penv;
{
 LISP env,l,next;
 env = *penv;
 l = cdr(*pform);
 if NULLP(l) {*pform = truth; return(NIL);}
 next = cdr(l);
 while(NNULLP(next))
   {if NULLP(leval(car(l),env)) {*pform = NIL; return(NIL);}
    l=next;next=cdr(next);}
 *pform = car(l); 
 return(truth);
}

LISP leval_let(pform,penv)
 LISP *pform,*penv;
{
 LISP env,l;
 l = cdr(*pform);
 env = *penv;
 *penv = extend_env(leval_args(car(cdr(l)),env),car(l),env);
 *pform = car(cdr(cdr(l)));
 return(truth);
}

LISP lreverse(l)
 LISP l;
{
 LISP n,p;
 n = NIL;
 for(p=l;NNULLP(p);p=cdr(p)) n = cons(car(p),n);
 return(n);
}

LISP let_macro(form)
 LISP form;
{
 LISP p,fl,al,tmp;
 fl = NIL;
 al = NIL;
 for(p=car(cdr(form));NNULLP(p);p=cdr(p))
  {tmp = car(p);
   if TYPEP(tmp,tc_symbol) {fl = cons(tmp,fl); al = cons(NIL,al);}
   else {fl = cons(car(tmp),fl); al = cons(car(cdr(tmp)),al);}}
 p = cdr(cdr(form));
 if NULLP(cdr(p)) p = car(p); else p = cons(sym_progn,p);
 setcdr(form,cons(lreverse(fl),cons(lreverse(al),cons(p,NIL))));
 setcar(form,cintern("let-internal"));
 return(form);
}
   
LISP leval_quote(args,env)
 LISP args,env;
{
 return(car(args));
}

LISP leval_tenv(args,env)
 LISP args,env;
{
 return(env);
}

LISP lprint(exp)
 LISP exp;
{
 printf("\n\r");
 lprin1(exp);
 printf("\n\r");
 return(NIL);
}

LISP lprin1(exp)
 LISP exp;
{
 LISP tmp;
 switch TYPE(exp) {
   case tc_nil:
      printf("()");
      break;
   case tc_cons:
      printf("(");
      lprin1(car(exp));
      for(tmp=cdr(exp);TYPEP(tmp,tc_cons);tmp=cdr(tmp))
        {printf(" ");lprin1(car(tmp));}
      if NNULLP(tmp) 
        {printf(" . ");lprin1(tmp);}
      printf(")");
      break;
    case tc_flonum:
      printf("%d",FLONM(exp));
      break;
    case tc_symbol:
      printf("%s",PNAME(exp));
      break;
    case tc_subr_0:
    case tc_subr_1:
    case tc_subr_2:
    case tc_subr_3:
    case tc_lsubr:
    case tc_fsubr:
    case tc_msubr:
      printf("#<SUBR(%d) %s>",TYPE(exp),(*exp).storage_as.subr.name);
      break;
    case tc_closure:
      printf("#<CLOSURE ");
      lprin1(car((*exp).storage_as.closure.code));
      printf(" ");
      lprin1(cdr((*exp).storage_as.closure.code));
      printf(">");
      break;}
 return(NIL);
}

char readchar()
{
  if (lisp_mode == 0) {  // read from console
    if (ugetf) {
      ugetf = 0; 
      return (ugetc);
    }
    while ((ugetc=getch()) == 0) // filter out any nulls from the console
      continue;
    putchar(ugetc);
    return ugetc;
  } else {
    if (ugetf) {
      ugetf = 0; 
      return *fp++;
    }
    putchar(*fp);
    return *fp++;    // read from buffer
  }
}

void unreadchar(c)
 char c;
{
  ugetf = 1;
  if (lisp_mode == 0) {ugetc = c; return;}
  fp--;
}

LISP lread()
{
 return(lreadf());
}

int flush_ws(eoferr)
 char *eoferr;
{
 int c;
 while(1) {
    c = readchar();
    if (c==0x3b) {      // semicolon is used for comments
      while ((c = readchar())) {
        if ((c==0x0a) || (c==0x0d)) {  // skip until \n or \r
          unreadchar(c);
          break;
        }
      }
    }
    if ((c==0x00) || (c==0x1b) || (c==-1)) {
      lquit = 1; 
      err("EOF", NIL);
    }
    if ((c==0x20) || (c==0x0a) || (c==0x0d) || (c==0x09)) 
      continue;
    return(c);
  }
}

LISP lreadf()
{
 int c;
 c = flush_ws((char *)NULL);
 if ((c==0x00) || (c==0x1b) || (c==-1)) {lquit = 1; err("EOF", NIL);}
 unreadchar(c);
 return(lreadr());
}

LISP lreadr()
{
 int c,j;
 char *p;
 c = flush_ws("end of file inside read");
 switch (c)
   {case '(':
      return(lreadparen());
    case ')':
      err("unexpected close paren",NIL);
    case '\'':
      return(cons(sym_quote,cons(lreadr(),NIL)));}
 p = tkbuffer;
 *p++ = c;
 for(j = 1; j<TKBUFFERN; ++j)
   {c = readchar();
    if ((c==0x00) || (c==0x1b) || (c==-1)) {lquit = 1; err("EOF", NIL);}
    if ((c==0x20) || (c==0x0a) || (c==0x0d) || (c==0x09)) return(lreadtk(j));
    if (strchr("()'",c)) {unreadchar(c);return(lreadtk(j));}
    *p++ = c;}
 err("token larger than TKBUFFERN",NIL);
}

LISP lreadparen()
{
 int c;
 LISP tmp;
 c = flush_ws("end of file inside list");
 if (c == ')') return(NIL);
 unreadchar(c);
 tmp = lreadr();
 return(cons(tmp,lreadparen()));
}

LISP lreadtk(j)
 int j;
{
 char *p;
 p = tkbuffer;
 p[j] = 0;
 if (*p == '-') p+=1;
 { int adigit = 0;
   while(isdigit(*p)) {p+=1; adigit=1;}
   if (*p=='.') {
     p += 1;
     while(isdigit(*p)) {p+=1; adigit=1;}}
   if (!adigit) goto a_symbol; }
 if (*p=='e') {
   p+=1;
   if (*p=='-'||*p=='+') p+=1;
   if (!isdigit(*p)) goto a_symbol; else p+=1;
   while(isdigit(*p)) p+=1; }
 if (*p) goto a_symbol;
 return(flocons(atoi(tkbuffer)));
 a_symbol:
 return(rintern(tkbuffer));
}
      
LISP copy_list(x)
 LISP x;
{
 if NULLP(x) return(NIL);
 return(cons(car(x),copy_list(cdr(x))));
}

LISP oblistfn()
{
 return(copy_list(oblist));
}

LISP lconsole()
{
  lisp_mode = 0;
  printf("starting interactive LISP;  hit ESC to exit\n\r");
  return(truth);
}


LISP quit()
{
 errjmp[40] = 1;
 lquit = 1;
 longjmp(errjmp, 1);
 return(NIL);
}

LISP ldump()
{
  print_hs_2();
  return(NIL);
}

LISP nullp(x)
 LISP x;
{
 if EQ(x,NIL) return(truth); else return(NIL);
}

LISP arglchk(x)
 LISP x;
{
 LISP l;
 if TYPEP(x,tc_symbol) return(x);
 for(l=x;TYPEP(l,tc_cons);l=CDR(l));
 if NNULLP(l) err("improper formal argument list",x);
 return(x);
}


void init_subrs()
{
// tc_subr_0
 init_subr("read",tc_subr_0,lread);
 init_subr("oblist",tc_subr_0,oblistfn);
 init_subr("console",tc_subr_0,lconsole);
 init_subr("quit",tc_subr_0,quit);
 init_subr("dump",tc_subr_0,ldump);
// tc_subr_1
 init_subr("not",tc_subr_1,nullp);
 init_subr("null?",tc_subr_1,nullp);
 init_subr("pair?",tc_subr_1,consp);
 init_subr("symbol?",tc_subr_1,symbolp);
 init_subr("number?",tc_subr_1,numberp);
 init_subr("copy-list",tc_subr_1,copy_list);
 init_subr("print",tc_subr_1,lprint);
 init_subr("car",tc_subr_1,car);
 init_subr("cdr",tc_subr_1,cdr);
 init_subr("let-internal-macro",tc_subr_1,let_macro);
 init_subr("reverse",tc_subr_1,lreverse);
// tc_subr_2
 init_subr("error",tc_subr_2,lerr);
 init_subr("env-lookup",tc_subr_2,envlookup);
 init_subr("symbol-bound?",tc_subr_2,symbol_boundp);
 init_subr("symbol-value",tc_subr_2,symbol_value);
 init_subr("cons",tc_subr_2,cons);
 init_subr("set-car!",tc_subr_2,setcar);
 init_subr("set-cdr!",tc_subr_2,setcdr);
 init_subr("+",tc_subr_2,plus);
 init_subr("-",tc_subr_2,difference);
 init_subr("*",tc_subr_2,ltimes);
 init_subr("/",tc_subr_2,quotient);
 init_subr(">",tc_subr_2,greaterp);
 init_subr("<",tc_subr_2,lessp);
 init_subr("=",tc_subr_2,equalp);
 init_subr("eq?",tc_subr_2,eq);
 init_subr("eqv?",tc_subr_2,eql);
 init_subr("assq",tc_subr_2,assq);
 init_subr("eval",tc_subr_2,leval);
// tc_subr3
 init_subr("set-symbol-value!",tc_subr_3,setvar);
// tc_lsubr
 init_subr("gc-status",tc_lsubr,gc_status);
// tc_fsubr
 init_subr("define",tc_fsubr,leval_define);
 init_subr("lambda",tc_fsubr,leval_lambda);
 init_subr("while",tc_fsubr,leval_while);
 init_subr("set!",tc_fsubr,leval_setq);
 init_subr("quote",tc_fsubr,leval_quote);
 init_subr("the-environment",tc_fsubr,leval_tenv);
// tc_msubr
 init_subr("if",tc_msubr,leval_if);
 init_subr("begin",tc_msubr,leval_progn);
  // init_subr("cond",tc_msubr,leval_cond);
 init_subr("or",tc_msubr,leval_or);
 init_subr("and",tc_msubr,leval_and);
 init_subr("let-internal",tc_msubr,leval_let);
}


/* This is cfib, for compiled fib, based on
    (define (standard-fib x)
       (if (< x 2)
         x
         (+ (standard-fib (- x 1))
        (standard-fib (- x 2)))))  
*/

LISP my_one, my_two;

LISP cfib(x)
     LISP x;
{
  if NNULLP(lessp(x,my_two))
    return(x);
  else
    return(plus(cfib(difference(x,my_one)),
           cfib(difference(x,my_two))));
}

LISP lrand(LISP x)  // return rand() % (x+1)
{return(flocons(rand() % ((unsigned int)FLONM(x)+1)));}

LISP lrange(LISP x)  // return laser range
{return(flocons(laser_range(0)));}

LISP ltime()  // return RTC value
{return(flocons(readRTC()));}

LISP lsignal()  // check for kbhit, return t or nil
{
  unsigned char ch;
  if (getchar(&ch))
    return(truth);
  return(NIL);
}

LISP ldelay(LISP x) // delay x milliseconds
{delayMS(FLONM(x)); return x;}

LISP linput()  // return 0-9 from console input
{return(flocons(getch() & 0x0F));}

LISP lasers(LISP x) // 1 == on, 0 == off
{
  if (FLONM(x)) 
    *pPORTHIO |= 0x0380;
  else
    *pPORTHIO &= 0xFC7F;
  return x;
}

extern int pwm1_init, pwm1_mode;
LISP lrobot(LISP x) // 8=forward  2=back  4=left  6=right  5=stop
{
  if (!pwm1_init) {
    initPWM();
    pwm1_init = 1;
    pwm1_mode = PWM_PWM;
  }
  switch (FLONM(x)) {
    case 8:
      setPWM(50,50);
      break;
    case 2:
      setPWM(-50,-50);
      break;
    case 4:
      setPWM(75,-75);
      delayMS(200);
      setPWM(0,0);
      break;
    case 6:
      setPWM(-75,75);
      delayMS(200);
      setPWM(0,0);
      break;
    case 5:
      setPWM(0,0);
      break;
  }
  return(x);
}

LISP lcompass()  // return reading from HMC6352 I2C compass
{
  unsigned char i2c_data[2];
  unsigned int ix;
    
  i2c_data[0] = 0x41;  // read compass twice to clear last reading
  i2cread(0x21, (unsigned char *)i2c_data, 2, SCCB_ON);
  i2c_data[0] = 0x41;
  i2cread(0x21, (unsigned char *)i2c_data, 2, SCCB_ON);
  ix = ((i2c_data[0] << 8) + i2c_data[1]) / 10;
  return(flocons(ix));
}

extern int sonar_data[];
LISP lping(LISP x) // read sonar module
{
  unsigned int i;
  i = FLONM(x);
  if ((i<1) || (i>4))
    return(flocons(0));
  ping_sonar();
  return (flocons(sonar_data[i] / 100));
}

extern unsigned int ymax[], ymin[], umax[], umin[], vmax[], vmin[];
LISP lcolor(LISP cbin, LISP clist) 
{
    int ix;
    ix = FLONM(cbin);  // color bin index
    if (ix > MAX_COLORS) err("invalid color bin", cbin);
    ymin[ix] = FLONM(car(clist));
    ymax[ix] = FLONM(car(cdr(clist)));
    umin[ix] = FLONM(car(cdr(cdr(clist))));
    umax[ix] = FLONM(car(cdr(cdr(cdr(clist)))));
    vmin[ix] = FLONM(car(cdr(cdr(cdr(cdr(clist))))));
    vmax[ix] = FLONM(car(cdr(cdr(cdr(cdr(cdr(clist)))))));
    return(cbin);
}    

extern unsigned int blobx1[], blobx2[], bloby1[], bloby2[], blobcnt[];
extern unsigned int imgWidth, imgHeight;
LISP lblob(LISP cbin) 
{
    int ix;
    ix = FLONM(cbin);  // color bin index
    if (ix > MAX_COLORS) err("invalid color bin", cbin);
    move_image((unsigned char *)DMA_BUF1, (unsigned char *)DMA_BUF2,  // grab new frame
            (unsigned char *)FRAME_BUF, imgWidth, imgHeight); 
    vblob((unsigned char *)FRAME_BUF, ix);
    return (cons(flocons(blobcnt[0]),   // return list comprised of blobcnt,
        cons(flocons((blobx1[0]+blobx2[0])/2),            // x midpoint
        cons(flocons((bloby1[0]+bloby2[0])/2), NIL))));   // and y midpoint
}

void our_subrs()
{
  my_one = flocons((int) 1);
  my_two = flocons((int) 2);
  gc_protect(&my_one);
  gc_protect(&my_two);
// tc_subr_0
  init_subr("time",tc_subr_0,ltime);
  init_subr("input",tc_subr_0,linput);
  init_subr("compass",tc_subr_0,lcompass);
  init_subr("signal",tc_subr_0,lsignal);
// tc_subr_1
  init_subr("cfib",tc_subr_1,cfib);
  init_subr("rand",tc_subr_1,lrand);
  init_subr("range",tc_subr_1,lrange);
  init_subr("delay",tc_subr_1,ldelay);
  init_subr("lasers",tc_subr_1,lasers);
  init_subr("robot",tc_subr_1,lrobot);
  init_subr("ping",tc_subr_1,lping);
  init_subr("blob",tc_subr_1,lblob);
// tc_subr_2
  init_subr("color",tc_subr_2,lcolor);
}

