/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * lisp.h: LISP interpreter for Surveyor SRV1 / SVS
 *
 * derived from "Scheme In One Day" (SIOD) by George J. Carrette
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


struct obj
{short gc_mark;
 short type;
 union {struct {struct obj * car;
		struct obj * cdr;} cons;
	struct {int data; int dummy;} flonum;
	struct {char *pname;
		struct obj * vcell;} symbol;
	struct {char *name;
		struct obj * (*f)();} subr;
	struct {struct obj *env;
		struct obj *code;} closure;}
 storage_as;};

#define CAR(x) ((*x).storage_as.cons.car)
#define CDR(x) ((*x).storage_as.cons.cdr)
#define PNAME(x) ((*x).storage_as.symbol.pname)
#define VCELL(x) ((*x).storage_as.symbol.vcell)
#define SUBRF(x) (*((*x).storage_as.subr.f))
#define FLONM(x) ((*x).storage_as.flonum.data)

#define NIL ((struct obj *) 0)
#define EQ(x,y) ((x) == (y))
#define NEQ(x,y) ((x) != (y))
#define NULLP(x) EQ(x,NIL)
#define NNULLP(x) NEQ(x,NIL)

#define TYPE(x) (((x) == NIL) ? 0 : ((*(x)).type))

#define TYPEP(x,y) (TYPE(x) == (y))
#define NTYPEP(x,y) (TYPE(x) != (y))

#define NULL 0

#define tc_nil    0
#define tc_cons   1
#define tc_flonum 2
#define tc_symbol 3
#define tc_subr_0 4
#define tc_subr_1 5
#define tc_subr_2 6
#define tc_subr_3 7
#define tc_lsubr  8
#define tc_fsubr  9
#define tc_msubr  10
#define tc_closure 11

typedef struct obj* LISP;


#define TKBUFFERN 100

LISP cons(), car(), cdr(), setcar();
LISP setcdr(),consp();

LISP symcons(),rintern(), cintern();
LISP cintern_soft();
LISP symbolp();

LISP flocons();
LISP plus(),ltimes(),difference();
LISP quotient(), greaterp(), lessp(), equalp();

LISP eq(),eql(),numberp();
LISP assq();

LISP lread(),leval(),lprint(),lprin1();

LISP subrcons();
LISP closure();

LISP leval_define(),leval_lambda(),leval_if(), leval_while(), leval_cond();
LISP leval_progn(),leval_setq(),leval_let(),let_macro();
LISP leval_args(),extend_env(),setvar();
LISP leval_quote(),leval_and(),leval_or();
LISP oblistfn(),copy_list(), ldump();
LISP gc_relocate(),get_newspace(),gc_status();
LISP leval_tenv(),lerr(),quit(),nullp();
LISP symbol_boundp(),symbol_value();
LISP envlookup(),arglchk(),lreverse();
LISP lreadr(),lreadparen(),lreadtk(),lreadf(), lconsole();

struct gc_protected
{LISP *location;
 struct gc_protected *next;};

void gc_protect();
void init_subr();
char readchar();
void unreadchar(char);
void print_hs_1();
void init_storage();
void init_subrs();
void our_subrs();
void repl();
void gc();
void lisp(char *);

