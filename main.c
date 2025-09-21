/*
written by kenichi sasagawa
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <setjmp.h>
#include "lisp15.h"

cell heap[HEAPSIZE];
int stack[STACKSIZE];
int argstk[STACKSIZE];
token stok = { GO, OTHER };

jmp_buf buf;
int cell_hash_table[HASHTBSIZE];


int main(void)
{
    printf("LISP 1.5\n");
    initcell();
    initsubr();
    int ret = setjmp(buf);

  repl:
    if (ret == 0)
	while (1) {
	    printf("> ");
	    fflush(stdout);
	    fflush(stdin);
	    print(eval(read()));
	    printf("\n");
	    fflush(stdout);
    } else if (ret == 1) {
	ret = 0;
	goto repl;
    } else
	return 0;
}

//-----------------------------
void initcell(void)
{
    int addr, x;

    for (addr = 0; addr < HEAPSIZE; addr++) {
	heap[addr].flag = FRE;
	heap[addr].cdr = addr + 1;
    }
    hp = 0;
    fc = HEAPSIZE;

    for (x = 0; x < HASHTBSIZE; x++)
	cell_hash_table[x] = NIL;

    // Address 0 is NIL; set up the environment registers.
    // This is the initial environment."
    ep = makesym("nil");
    assocsym(makesym("nil"), NIL);
    assocsym(makesym("t"), makesym("t"));

    sp = 0;
    ap = 0;
}

int freshcell(void)
{
    int res;

    res = hp;
    hp = heap[hp].cdr;
    SET_CDR(res, 0);
    fc--;
    return (res);
}

// Done via deep-bind. If the symbol is not found, register it.
// If found, store the value there.
void bindsym(int sym, int val)
{
    int addr;

    addr = assoc(sym, ep);
    if (addr == 0)
	assocsym(sym, val);
    else
	SET_CDR(addr, val);
}

// Variable binding
// For local variables, bindings are stacked on top of previous ones.
// Therefore, even if the same variable name exists, it is not overwritten.
void assocsym(int sym, int val)
{
    ep = cons(cons(sym, val), ep);
}



// The environment is represented as an association list like this:
// env = ((sym1 . val1) (sym2 . val2) ... (t . t) (nil . 0))
// Use `assoc` to find the value corresponding to a symbol.
// If not found, it returns -1.
int findsym(int sym)
{
    int addr;

    addr = assoc(sym, ep);

    if (addr == 0)
	return (-1);
    else
	return (cdr(addr));
}

// Used to ensure the uniqueness of symbols
int getsym(char *name, int index)
{
    int addr;

    addr = cell_hash_table[index];

    while (addr != NIL) {
	if (strcmp(name, GET_NAME(car(addr))) == 0)
	    return (car(addr));
	else
	    addr = cdr(addr);
    }
    return (0);
}

int addsym(char *name, int index)
{
    int addr, res;

    addr = cell_hash_table[index];
    addr = cons(res = makesym1(name), addr);
    cell_hash_table[index] = addr;
    return (res);
}

int makesym1(char *name)
{
    int addr;

    addr = freshcell();
    SET_TAG(addr, SYM);
    SET_NAME(addr, name);
    return (addr);
}


int hash(char *name)
{
    int res;

    res = 0;
    while (*name != NUL) {
	res = res + (int) *name;
	name++;
    }
    return (res % HASHTBSIZE);
}

//-------for debug------------------    
void cellprint(int addr)
{
    switch (GET_FLAG(addr)) {
    case FRE:
	printf("FRE ");
	break;
    case USE:
	printf("USE ");
	break;
    }
    switch (GET_TAG(addr)) {
    case EMP:
	printf("EMP    ");
	break;
    case NUM:
	printf("NUM    ");
	break;
    case SYM:
	printf("SYM    ");
	break;
    case LIS:
	printf("LIS    ");
	break;
    case SUBR:
	printf("SUBR   ");
	break;
    case FSUBR:
	printf("FSUBR  ");
	break;
    case FUNC:
	printf("FUNC   ");
	break;
    case MACRO:
	printf("MACRO  ");
	break;
    }
    printf("%07d ", GET_CAR(addr));
    printf("%07d ", GET_CDR(addr));
    printf("%07d ", GET_BIND(addr));
    printf("%s \n", GET_NAME(addr));
}
   
void heapdump(int start, int end)
{
    int i;

    printf("addr    F   TAG   CAR     CDR    BIND   NAME\n");
    for (i = start; i <= end; i++) {
	printf("%07d ", i);
	cellprint(i);
    }
}

//---------GC-----------
void gbc(void)
{
    int addr;

    printf("enter GBC free=%d\n", fc);
    fflush(stdout);
    gbcmark();
    gbcsweep();
    fc = 0;
    for (addr = 0; addr < HEAPSIZE; addr++) {
	if (IS_EMPTY(addr))
	    fc++;
    }
    printf("exit GBC free=%d\n", fc);
    fflush(stdout);
}


void markoblist(void)
{
    int addr;

    addr = ep;
    while (!(nullp(addr))) {
	MARK_CELL(addr);
	addr = cdr(addr);
    }
    MARK_CELL(0);
}

void markcell(int addr)
{
    if (USED_CELL(addr))
	return;

    MARK_CELL(addr);

    if (car(addr) != 0) {
	markcell(car(addr));
    }
    if (cdr(addr) != 0) {
	markcell(cdr(addr));
    }

    if ((GET_BIND(addr) != 0) && IS_FUNC(addr))
	markcell(GET_BIND(addr));

    if ((GET_BIND(addr) != 0) && IS_MACRO(addr))
	markcell(GET_BIND(addr));


}

void gbcmark(void)
{
    int addr, i;

    //Mark oblist
    markoblist();
    // Mark the cells linked from the oblist.
    addr = ep;
    while (!(nullp(addr))) {
	markcell(car(addr));
	addr = cdr(addr);
    }
    // Mark the cells that are bound from the argstack.
    for (i = 0; i < ap; i++)
	markcell(argstk[i]);

    // Mark the cells linked from the symbol hash table.
    for (i = 0; i < HASHTBSIZE; i++)
	markcell(cell_hash_table[i]);
}

void gbcsweep(void)
{
    int addr;

    addr = 0;
    while (addr < HEAPSIZE) {
	if (USED_CELL(addr))
	    NOMARK_CELL(addr);
	else {
	    clrcell(addr);
	    SET_CDR(addr, hp);
	    hp = addr;
	}
	addr++;
    }
}

void clrcell(int addr)
{
    SET_TAG(addr, EMP);
    free(heap[addr].name);
    heap[addr].name = NULL;
    SET_CAR(addr, 0);
    SET_CDR(addr, 0);
    SET_BIND(addr, 0);
}

// If the number of free cells falls below a certain threshold, trigger GBC.
void checkgbc(void)
{
    if (fc < FREESIZE)
	gbc();
}

//--------------list---------------------

int car(int addr)
{
    return (GET_CAR(addr));
}

int caar(int addr)
{
    return (car(car(addr)));
}

int cdar(int addr)
{
    return (cdr(car(addr)));
}

int cdr(int addr)
{
    return (GET_CDR(addr));
}

int cadr(int addr)
{
    return (car(cdr(addr)));
}

int caddr(int addr)
{
    return (car(cdr(cdr(addr))));
}

int cadar(int addr)
{
    return (car(cdr(car(addr))));
}


int cons(int car, int cdr)
{
    int addr;

    addr = freshcell();
    SET_TAG(addr, LIS);
    SET_CAR(addr, car);
    SET_CDR(addr, cdr);
    return (addr);
}

int assoc(int sym, int lis)
{
    if (nullp(lis))
	return (0);
    else if (eqp(sym, caar(lis)))
	return (car(lis));
    else
	return (assoc(sym, cdr(lis)));
}

int length(int addr)
{
    int len = 0;

    while (!(nullp(addr))) {
	len++;
	addr = cdr(addr);
    }
    return (len);
}

int list(int arglist)
{
    if (nullp(arglist))
	return (NIL);
    else
	return (cons(car(arglist), list(cdr(arglist))));
}

int append(int x, int y)
{
    if (nullp(x))
	return (y);
    else
	return (cons(car(x), append(cdr(x), y)));
}

//----------------------------------------

int makenum(int num)
{
    int addr;

    addr = freshcell();
    SET_TAG(addr, NUM);
    SET_NUMBER(addr, num);
    return (addr);
}


int makesym(char *name)
{
    int index, res;

    index = hash(name);
    if ((res = getsym(name, index)) != 0)
	return (res);
    else
	return (addsym(name, index));
}


int makefunc(int addr)
{
    int val;

    val = freshcell();
    SET_TAG(val, FUNC);
    SET_BIND(val, addr);
    SET_CDR(val, 0);
    return(val);
}



// Stack. Used to save the environment pointer (EP).
void push(int pt)
{
    stack[sp++] = pt;
}

int pop(void)
{
    return (stack[--sp]);
}

// Push/pop for the arglist stack
void argpush(int addr)
{
    argstk[ap++] = addr;
}

void argpop(void)
{
    --ap;
}

//-------read()--------

void gettoken(void)
{
    char c;
    int pos;

    if (stok.flag == BACK) {
	stok.flag = GO;
	return;
    }

    if (stok.ch == ')') {
	stok.type = RPAREN;
	stok.ch = NUL;
	return;
    }

    if (stok.ch == '(') {
	stok.type = LPAREN;
	stok.ch = NUL;
	return;
    }

    c = getchar();
    while ((c == SPACE) || (c == EOL) || (c == TAB))
	c = getchar();

    switch (c) {
    case '(':
	stok.type = LPAREN;
	break;
    case ')':
	stok.type = RPAREN;
	break;
    case '\'':
	stok.type = QUOTE;
	break;
    case '.':
	stok.type = DOT;
	break;
    case '`':
	stok.type = BACKQUOTE;
	break;
    case ',':
	stok.type = COMMA;
	break;
    case '@':
	stok.type = ATMARK;
	break;
    default:{
	    pos = 0;
	    stok.buf[pos++] = c;
	    while (((c = getchar()) != EOL) && (pos < BUFSIZE) &&
		   (c != SPACE) && (c != '(') && (c != ')') &&
		   (c != '`') && (c != ',') && (c != '@'))
		stok.buf[pos++] = c;

	    stok.buf[pos] = NUL;
	    stok.ch = c;
	    if (numbertoken(stok.buf)) {
		stok.type = NUMBER;
		break;
	    }
	    if (symboltoken(stok.buf)) {
		stok.type = SYMBOL;
		break;
	    }
	    stok.type = OTHER;
	}
    }
}

int numbertoken(char buf[])
{
    int i;
    char c;

    if (((buf[0] == '+') || (buf[0] == '-'))) {
	if (buf[1] == NUL)
	    return (0);		// case {+,-} => symbol
	i = 1;
	while ((c = buf[i]) != NUL)
	    if (isdigit(c))
		i++;		// case {+123..., -123...}
	    else
		return (0);
    } else {
	i = 0;			// {1234...}
	while ((c = buf[i]) != NUL)
	    if (isdigit(c))
		i++;
	    else
		return (0);
    }
    return (1);
}

int symboltoken(char buf[])
{
    int i;
    char c;

    if (isdigit(buf[0]))
	return (0);

    i = 0;
    while ((c = buf[i]) != NUL)
	if ((isalpha(c)) || (isdigit(c)) || (issymch(c)))
	    i++;
	else
	    return (0);

    return (1);
}

int issymch(char c)
{
    switch (c) {
    case '!':
    case '?':
    case '+':
    case '-':
    case '*':
    case '/':
    case '=':
    case '<':
    case '>':
	return (1);
    default:
	return (0);
    }
}


int read(void)
{
    gettoken();
    switch (stok.type) {
    case NUMBER:
	return (makenum(atoi(stok.buf)));
    case SYMBOL:
	return (makesym(stok.buf));
    case QUOTE:
	return (cons(makesym("quote"), cons(read(), NIL)));
    case BACKQUOTE:
	return (cons(makesym("quasi-quote"), cons(read(), NIL)));
    case COMMA:{
	    gettoken();
	    if (stok.type == ATMARK)
		return (cons
			(makesym("unquote-splicing"), cons(read(), NIL)));
	    else {
		stok.flag = BACK;
		return (cons(makesym("unquote"), cons(read(), NIL)));
	    }
	}
    case LPAREN:
	return (readlist());
    default:
	break;
    }
    error(CANT_READ_ERR, "read", NIL);
    return (0);
}

int readlist(void)
{
    int car, cdr;

    gettoken();
    if (stok.type == RPAREN)
	return (NIL);
    else if (stok.type == DOT) {
	cdr = read();
	if (atomp(cdr))
	    gettoken();
	return (cdr);
    } else {
	stok.flag = BACK;
	car = read();
	cdr = readlist();
	return (cons(car, cdr));
    }
}

//-----print------------------
void print(int addr)
{
    switch (GET_TAG(addr)) {
    case NUM:
	printf("%d", GET_NUMBER(addr));
	break;
    case SYM:
	printf("%s", GET_NAME(addr));
	break;
    case SUBR:
	printf("<subr>");
	break;
    case FSUBR:
	printf("<fsubr>");
	break;
    case FUNC:
	printf("<function>");
	break;
    case MACRO:
	printf("<macro>");
	break;
    case LIS:{
	    printf("(");
	    printlist(addr);
	    break;
	}
    default:
	printf("<undef>");
	break;
    }
}


void printlist(int addr)
{
    if (IS_NIL(addr))
	printf(")");
    else if ((!(listp(cdr(addr)))) && (!(nullp(cdr(addr))))) {
	print(car(addr));
	printf(" . ");
	print(cdr(addr));
	printf(")");
    } else {
	print(GET_CAR(addr));
	if (!(IS_NIL(GET_CDR(addr))))
	    printf(" ");
	printlist(GET_CDR(addr));
    }
}

//--------eval---------------        
int eval(int addr)
{
    int res;

    if (atomp(addr)) {
	if (numberp(addr))
	    return (addr);
	if (symbolp(addr)) {
	    res = findsym(addr);
	    if (res == -1)
		error(CANT_FIND_ERR, "eval", addr);
	    else
		return (res);
	}
    } else if (listp(addr)) {
	if ((symbolp(car(addr))) && (HAS_NAME(car(addr), "quasi-quote")))
	    return (eval(quasi_transfer2(cadr(addr), 0)));
	if (numberp(car(addr)))
	    error(ARG_SYM_ERR, "eval", addr);
	if (subrp(car(addr)))
	    return (apply(eval(car(addr)), evlis(cdr(addr))));
	if (fsubrp(car(addr)))
	    return (apply(eval(car(addr)), cdr(addr)));
	if (functionp(car(addr)))
	    return (apply(eval(car(addr)), evlis(cdr(addr))));
	if (macrop(car(addr)))
	    return (apply(eval(car(addr)), cdr(addr)));

    }
    error(CANT_FIND_ERR, "eval", addr);
    return (0);
}

int apply(int func, int args)
{
    int varlist, body, res, macrofunc;

    res = NIL;
	switch (GET_TAG(func)) {
	case SUBR:
	    return ((GET_SUBR(func)) (args));
	case FSUBR:
	    return ((GET_SUBR(func)) (args));
	case FUNC:{
		varlist = car(GET_BIND(func));
		body = cdr(GET_BIND(func));
		bindarg(varlist, args);
		while (!(IS_NIL(body))) {
		    res = eval(car(body));
		    body = cdr(body);
		}
		unbind();
		return (res);
	    }
	case MACRO:{
		macrofunc = GET_BIND(func);
		varlist = car(GET_BIND(macrofunc));
		body = cdr(GET_BIND(macrofunc));
		bindarg(varlist, args);
		while (!(IS_NIL(body))) {
		    res = eval(car(body));
		    body = cdr(body);
		}
		unbind();
		res = eval(res);
		return (res);
	    }
	default:
	    error(ILLEGAL_OBJ_ERR, "apply", func);
	}
    return (0);
}

void bindarg(int varlist, int arglist)
{
    int arg1, arg2;

    push(ep);
    while (!(IS_NIL(varlist))) {
	arg1 = car(varlist);
	arg2 = car(arglist);
	assocsym(arg1, arg2);
	varlist = cdr(varlist);
	arglist = cdr(arglist);
    }
}

void unbind(void)
{
    ep = pop();
}


int evlis(int addr)
{
    int car_addr, cdr_addr;

    argpush(addr);
    checkgbc();
    if (IS_NIL(addr)) {
	argpop();
	return (addr);
    } else {
	car_addr = eval(car(addr));
	argpush(car_addr);
	cdr_addr = evlis(cdr(addr));
	argpop();
	argpop();
	return (cons(car_addr, cdr_addr));
    }
}


int atomp(int addr)
{
    if ((IS_NUMBER(addr)) || (IS_SYMBOL(addr)))
	return (1);
    else
	return (0);
}

int numberp(int addr)
{
    if (IS_NUMBER(addr))
	return (1);
    else
	return (0);
}

int symbolp(int addr)
{
    if (IS_SYMBOL(addr))
	return (1);
    else
	return (0);
}

int listp(int addr)
{
    if (IS_LIST(addr) || IS_NIL(addr))
	return (1);
    else
	return (0);
}

int nullp(int addr)
{
    if (IS_NIL(addr))
	return (1);
    else
	return (0);
}


int eqp(int addr1, int addr2)
{
    if (addr1 == addr2)
	return (1);
    else if ((numberp(addr1)) && (numberp(addr2))
	     && ((GET_NUMBER(addr1)) == (GET_NUMBER(addr2))))
	return (1);
    else if ((symbolp(addr1)) && (symbolp(addr2))
	     && (SAME_NAME(addr1, addr2)))
	return (1);
    else
	return (0);
}



int subrp(int addr)
{
    int val;

    val = findsym(addr);
    if (val != -1)
	return (IS_SUBR(val));
    else
	return (0);
}

int fsubrp(int addr)
{
    int val;

    val = findsym(addr);
    if (val != -1)
	return (IS_FSUBR(val));
    else
	return (0);
}

int functionp(int addr)
{
    int val;

    val = findsym(addr);
    if (val != -1)
	return (IS_FUNC(val));
    else
	return (0);
}

int macrop(int addr)
{
    int val;

    val = findsym(addr);
    if (val != -1)
	return (IS_MACRO(val));
    else
	return (0);
}


//-------エラー処理------
void error(int errnum, char *fun, int arg)
{
    switch (errnum) {
    case CANT_FIND_ERR:{
	    printf("%s can't find difinition of ", fun);
	    print(arg);
	    break;
	}

    case CANT_READ_ERR:{
	    printf("%s can't read of ", fun);
	    break;
	}

    case ILLEGAL_OBJ_ERR:{
	    printf("%s got an illegal object ", fun);
	    print(arg);
	    break;
	}

    case ARG_SYM_ERR:{
	    printf("%s require symbol but got ", fun);
	    print(arg);
	    break;
	}

    case ARG_NUM_ERR:{
	    printf("%s require number but got ", fun);
	    print(arg);
	    break;
	}

    case ARG_LIS_ERR:{
	    printf("%s require list but got ", fun);
	    print(arg);
	    break;
	}

    case ARG_LEN0_ERR:{
	    printf("%s require 0 arg but got %d", fun, length(arg));
	    break;
	}

    case ARG_LEN1_ERR:{
	    printf("%s require 1 arg but got %d", fun, length(arg));
	    break;
	}

    case ARG_LEN2_ERR:{
	    printf("%s require 2 args but got %d", fun, length(arg));
	    break;
	}

    case ARG_LEN3_ERR:{
	    printf("%s require 3 args but got %d", fun, length(arg));
	    break;
	}

    case MALFORM_ERR:{
	    printf("%s got malformed args ", fun);
	    print(arg);
	    break;
	}
    }
    printf("\n");
    longjmp(buf, 1);
}

void checkarg(int test, char *fun, int arg)
{
    switch (test) {
    case NUMLIST_TEST:
	if (isnumlis(arg))
	    return;
	else
	    error(ARG_NUM_ERR, fun, arg);
    case SYMBOL_TEST:
	if (symbolp(arg))
	    return;
	else
	    error(ARG_SYM_ERR, fun, arg);
    case NUMBER_TEST:
	if (numberp(arg))
	    return;
	else
	    error(ARG_NUM_ERR, fun, arg);
    case LIST_TEST:
	if (listp(arg))
	    return;
	else
	    error(ARG_LIS_ERR, fun, arg);
    case LEN0_TEST:
	if (length(arg) == 0)
	    return;
	else
	    error(ARG_LEN0_ERR, fun, arg);
    case LEN1_TEST:
	if (length(arg) == 1)
	    return;
	else
	    error(ARG_LEN1_ERR, fun, arg);
    case LEN2_TEST:
	if (length(arg) == 2)
	    return;
	else
	    error(ARG_LEN2_ERR, fun, arg);
    case LEN3_TEST:
	if (length(arg) == 3)
	    return;
	else
	    error(ARG_LEN3_ERR, fun, arg);
    }
}

int isnumlis(int arg)
{
    while (!(IS_NIL(arg)))
	if (numberp(car(arg)))
	    arg = cdr(arg);
	else
	    return (0);
    return (1);
}



//--------builtin functions------------
//define subr
void defsubr(char *symname, int (*func)(int))
{
    bindfunc(symname, SUBR, func);
}

//define fsubr
void deffsubr(char *symname, int (*func)(int))
{
    bindfunc(symname, FSUBR, func);
}

void bindfunc(char *name, tag tag, int (*func)(int))
{
    int sym, val;

    sym = makesym(name);
    val = freshcell();
    SET_TAG(val, tag);
    SET_SUBR(val, func);
    SET_CDR(val, 0);
    bindsym(sym, val);
}

void bindfunc1(char *name, int addr)
{
    int sym, val;

    sym = makesym(name);
    val = freshcell();
    SET_TAG(val, FUNC);
    SET_BIND(val, addr);
    SET_CDR(val, 0);
    bindsym(sym, val);
}

void bindmacro(char *name, int addr)
{
    int sym, val1, val2;

    sym = makesym(name);
    val1 = freshcell();
    SET_TAG(val1, FUNC);
    SET_BIND(val1, addr);
    SET_CDR(val1, 0);
    val2 = freshcell();
    SET_TAG(val2, MACRO);
    SET_BIND(val2, val1);
    SET_CDR(val2, 0);
    bindsym(sym, val2);
}

void initsubr(void)
{
    defsubr("plus", f_plus);
    defsubr("minus", f_minus);
    defsubr("times", f_mult);
    defsubr("quotient", f_quotient);
    defsubr("add1", f_add1);
    defsubr("sub1", f_sub1);
    defsubr("quit", f_quit);
    defsubr("hdmp", f_heapdump);
    defsubr("car", f_car);
    defsubr("cdr", f_cdr);
    defsubr("cons", f_cons);
    defsubr("list", f_list);
    defsubr("length", f_length);
    defsubr("append", f_append);
    defsubr("eq", f_eq);
    defsubr("null", f_nullp);
    defsubr("atom", f_atomp);
    defsubr("oblist", f_oblist);
    defsubr("gbc", f_gbc);
    defsubr("read", f_read);
    defsubr("eval", f_eval);
    defsubr("apply", f_apply);
    defsubr("print", f_print);
    defsubr("prin1", f_prin1);
    //defsubr("=", f_numeqp);
    defsubr("greaterp", f_greater);
    //defsubr(">=", f_eqgreater);
    defsubr("smallerp", f_smaller);
    //defsubr("<=", f_eqsmaller);
    defsubr("zerop", f_zerop);
    defsubr("onep", f_onep);
    defsubr("numberp", f_numberp);
    defsubr("symbolp", f_symbolp);
    defsubr("listp", f_listp);
    defsubr("assoc", f_assoc);
    defsubr("set", f_set);
    deffsubr("quote", f_quote);
    deffsubr("setq", f_setq);
    deffsubr("defun", f_defun);
    deffsubr("lambda", f_lambda);
    deffsubr("macro", f_defmacro);
    deffsubr("if", f_if);
    deffsubr("begin", f_begin);
    deffsubr("cond", f_cond);
}

//--subr----

int f_plus(int arglist)
{
    int arg, res;

    checkarg(NUMLIST_TEST, "plus", arglist);
    res = 0;
    while (!(IS_NIL(arglist))) {
	arg = GET_NUMBER(car(arglist));
	arglist = cdr(arglist);
	res = res + arg;
    }
    return (makenum(res));
}

int f_add1(int arglist)
{
    int number1;

    checkarg(NUMLIST_TEST, "add1", arglist);
    number1 = GET_NUMBER(car(arglist));
    return (makenum(number1+1));
}

int f_sub1(int arglist)
{
    int number1;

    checkarg(NUMLIST_TEST, "sub1", arglist);
    number1 = GET_NUMBER(car(arglist));
    return (makenum(number1-1));
}

int f_minus(int arglist)
{
    int arg, res;

    checkarg(NUMLIST_TEST, "minus", arglist);
    res = GET_NUMBER(car(arglist));
    arglist = cdr(arglist);
    while (!(IS_NIL(arglist))) {
	arg = GET_NUMBER(car(arglist));
	arglist = cdr(arglist);
	res = res - arg;
    }
    return (makenum(res));
}

int f_mult(int arglist)
{
    int arg, res;

    checkarg(NUMLIST_TEST, "times", arglist);
    res = GET_NUMBER(car(arglist));
    arglist = cdr(arglist);
    while (!(IS_NIL(arglist))) {
	arg = GET_NUMBER(car(arglist));
	arglist = cdr(arglist);
	res = res * arg;
    }
    return (makenum(res));
}

int f_quotient(int arglist)
{
    int arg, res;

    checkarg(NUMLIST_TEST, "quotient", arglist);
    res = GET_NUMBER(car(arglist));
    arglist = cdr(arglist);
    while (!(IS_NIL(arglist))) {
	arg = GET_NUMBER(car(arglist));
	arglist = cdr(arglist);
	res = res / arg;
    }
    return (makenum(res));
}

int f_quit(int arglist)
{
    int addr;

    checkarg(LEN0_TEST, "quit", arglist);
    for (addr = 0; addr < HEAPSIZE; addr++) {
	free(heap[addr].name);
    }
    printf("- good bye -\n");
    longjmp(buf, 2);
}

int f_heapdump(int arglist)
{
    int arg1;

    checkarg(LEN1_TEST, "hdmp", arglist);
    arg1 = GET_NUMBER(car(arglist));
    heapdump(arg1, arg1 + 10);
    return (T);
}

int f_car(int arglist)
{
    int arg1;

    checkarg(LEN1_TEST, "car", arglist);
    arg1 = car(arglist);
    return (car(arg1));
}

int f_cdr(int arglist)
{
    int arg1;

    checkarg(LEN1_TEST, "cdr", arglist);
    arg1 = car(arglist);
    return (cdr(arg1));
}

int f_cons(int arglist)
{
    int arg1, arg2;

    checkarg(LEN2_TEST, "cons", arglist);
    arg1 = car(arglist);
    arg2 = cadr(arglist);
    return (cons(arg1, arg2));
}

int f_eq(int arglist)
{
    int arg1, arg2;

    checkarg(LEN2_TEST, "eq", arglist);
    arg1 = car(arglist);
    arg2 = cadr(arglist);
    if (eqp(arg1, arg2))
	return (T);
    else
	return (NIL);
}

int f_nullp(int arglist)
{
    int arg;

    checkarg(LEN1_TEST, "null", arglist);
    arg = car(arglist);
    if (nullp(arg))
	return (T);
    else
	return (NIL);
}

int f_atomp(int arglist)
{
    int arg;

    checkarg(LEN1_TEST, "atom", arglist);
    arg = car(arglist);
    if (atomp(arg))
	return (T);
    else
	return (NIL);
}

int f_length(int arglist)
{
    checkarg(LEN1_TEST, "length", arglist);
    checkarg(LIST_TEST, "length", car(arglist));
    return (makenum(length(car(arglist))));
}

int f_list(int arglist)
{
    return (list(arglist));
}

int f_assoc(int arglist)
{
    int arg1,arg2;

    checkarg(LEN2_TEST, "assoc", arglist);
    checkarg(SYMBOL_TEST, "assoc", car(arglist));
    checkarg(LIST_TEST, "assoc", cadr(arglist));
    arg1 = car(arglist);
    arg2 = cadr(arglist);
    return(assoc(arg1,arg2));
}

int f_append(int arglist)
{
    checkarg(LEN2_TEST, "append", arglist);
    return (append(car(arglist), cadr(arglist)));
}

int f_numeqp(int arglist)
{
    int num1, num2;

    checkarg(LEN2_TEST, "=", arglist);
    checkarg(NUMLIST_TEST, "=", arglist);
    num1 = GET_NUMBER(car(arglist));
    num2 = GET_NUMBER(cadr(arglist));

    if (num1 == num2)
	return (T);
    else
	return (NIL);
}

int f_symbolp(int arglist)
{
    if (symbolp(car(arglist)))
	return (T);
    else
	return (NIL);
}

int f_numberp(int arglist)
{
    if (numberp(car(arglist)))
	return (T);
    else
	return (NIL);
}

int f_listp(int arglist)
{
    if (listp(car(arglist)))
	return (T);
    else
	return (NIL);
}

int f_onep(int arglist)
{
    int num1;

    checkarg(LEN1_TEST, "onep", arglist);
    checkarg(NUMLIST_TEST, "onep", arglist);
    num1 = GET_NUMBER(car(arglist));

    if (num1 == 1)
	return (T);
    else
	return (NIL);
}

int f_zerop(int arglist)
{
    int num1;

    checkarg(LEN1_TEST, "zerop", arglist);
    checkarg(NUMLIST_TEST, "zerop", arglist);
    num1 = GET_NUMBER(car(arglist));

    if (num1 == 0)
	return (T);
    else
	return (NIL);
}


int f_smaller(int arglist)
{
    int num1, num2;

    checkarg(LEN2_TEST, "<", arglist);
    checkarg(NUMLIST_TEST, "<", arglist);
    num1 = GET_NUMBER(car(arglist));
    num2 = GET_NUMBER(cadr(arglist));

    if (num1 < num2)
	return (T);
    else
	return (NIL);
}

int f_eqsmaller(int arglist)
{
    int num1, num2;

    checkarg(LEN2_TEST, "<=", arglist);
    checkarg(NUMLIST_TEST, "<=", arglist);
    num1 = GET_NUMBER(car(arglist));
    num2 = GET_NUMBER(cadr(arglist));

    if (num1 <= num2)
	return (T);
    else
	return (NIL);
}

int f_greater(int arglist)
{
    int num1, num2;

    checkarg(LEN2_TEST, ">", arglist);
    checkarg(NUMLIST_TEST, ">", arglist);
    num1 = GET_NUMBER(car(arglist));
    num2 = GET_NUMBER(cadr(arglist));

    if (num1 > num2)
	return (T);
    else
	return (NIL);
}


int f_eqgreater(int arglist)
{
    int num1, num2;

    checkarg(LEN2_TEST, ">=", arglist);
    checkarg(NUMLIST_TEST, ">=", arglist);
    num1 = GET_NUMBER(car(arglist));
    num2 = GET_NUMBER(cadr(arglist));

    if (num1 >= num2)
	return (T);
    else
	return (NIL);
}

int f_oblist(int arglist)
{
    int addr, addr1, res;

    checkarg(LEN0_TEST, "oblist", arglist);
    res = NIL;
    addr = ep;
    while (!(nullp(addr))) {
	addr1 = caar(addr);
	res = cons(addr1, res);
	addr = cdr(addr);
    }
    return (res);
}

int f_gbc(int arglist)
{
    gbc();
    return (T);
}

int f_read(int arglist)
{
    checkarg(LEN0_TEST, "read", arglist);
    return (read());
}

int f_print(int arglist)
{
    checkarg(LEN1_TEST, "print", arglist);
    print(car(arglist));
    printf("\n");
    return (T);
}

int f_prin1(int arglist)
{
    checkarg(LEN1_TEST, "prin1", arglist);
    print(car(arglist));
    return (T);
}

int f_eval(int arglist)
{
    checkarg(LEN1_TEST, "eval", arglist);
    return (eval(car(arglist)));
}

int f_apply(int arglist)
{
    checkarg(LEN2_TEST, "apply", arglist);
    //checkarg(SYMBOL_TEST, "apply", car(arglist));
    checkarg(LIST_TEST, "apply", cadr(arglist));
    int arg1, arg2;

    arg1 = car(arglist);
    arg2 = cadr(arglist);
    return (apply(arg1, arg2));
}

int f_set(int arglist)
{
    int arg1, arg2;

    checkarg(LEN2_TEST, "set", arglist);
    checkarg(SYMBOL_TEST, "set", car(arglist));
    arg1 = car(arglist);
    arg2 = cadr(arglist);
    bindsym(arg1, arg2);
    return (T);
}



//--FSUBR-----------
int f_quote(int arglist)
{
    int arg1;

    checkarg(LEN1_TEST, "quote", arglist);
    arg1 = car(arglist);
    return(arg1);
}

int f_setq(int arglist)
{
    int arg1, arg2;

    checkarg(LEN2_TEST, "setq", arglist);
    checkarg(SYMBOL_TEST, "setq", car(arglist));
    arg1 = car(arglist);
    arg2 = eval(cadr(arglist));
    bindsym(arg1, arg2);
    return (T);
}

int f_defun(int arglist)
{
    int arg1, arg2;

    checkarg(LEN3_TEST, "defun", arglist);
    checkarg(SYMBOL_TEST, "defun", car(arglist));
    checkarg(LIST_TEST, "defun", cadr(arglist));
    checkarg(LIST_TEST, "defun", caddr(arglist));
    arg1 = car(arglist);
    arg2 = cdr(arglist);
    bindfunc1(GET_NAME(arg1), arg2);
    return (T);
}

int f_lambda(int arglist)
{

    checkarg(LEN2_TEST, "lambda", arglist);
    checkarg(LIST_TEST, "lambda", car(arglist));
    checkarg(LIST_TEST, "lambda", cdr(arglist));
    return(makefunc(arglist));
}

int f_defmacro(int arglist)
{
    int arg1, arg2;

    checkarg(LEN3_TEST, "defmacro", arglist);
    checkarg(SYMBOL_TEST, "defmacro", car(arglist));
    checkarg(LIST_TEST, "defmacro", cadr(arglist));
    checkarg(LIST_TEST, "defmacro", caddr(arglist));
    arg1 = car(arglist);
    arg2 = cdr(arglist);
    bindmacro(GET_NAME(arg1), arg2);
    return (T);
}



int f_if(int arglist)
{
    int arg1, arg2, arg3;

    checkarg(LEN3_TEST, "if", arglist);
    arg1 = car(arglist);
    arg2 = cadr(arglist);
    arg3 = car(cdr(cdr(arglist)));

    if (!(nullp(eval(arg1))))
	return (eval(arg2));
    else
	return (eval(arg3));
}

int f_cond(int arglist)
{
    int arg1, arg2, arg3;

    if (nullp(arglist))
	return (NIL);

    arg1 = car(arglist);
    checkarg(LIST_TEST, "cond", arg1);
    arg2 = car(arg1);
    arg3 = cdr(arg1);

    if (!(nullp(eval(arg2))))
	return (f_begin(arg3));
    else
	return (f_cond(cdr(arglist)));
}

int f_begin(int arglist)
{
    int res;

    res = NIL;
    while (!(nullp(arglist))) {
	res = eval(car(arglist));
	arglist = cdr(arglist);
    }
    return (res);
}

//--------準クオート---------------
int quasi_transfer1(int x)
{
    if (nullp(x))
	return (NIL);
    else if (atomp(x))
	return (list2(makesym("quote"), x));
    else if (listp(x) && eqp(caar(x), makesym("unquote")))
	return (list3(makesym("cons"), cadar(x), quasi_transfer1(cdr(x))));
    else if (listp(x) && eqp(caar(x), makesym("unquote-splicing")))
	return (list3
		(makesym("append"), cadar(x), quasi_transfer1(cdr(x))));
    else
	return (list3
		(makesym("cons"), quasi_transfer1(car(x)),
		 quasi_transfer1(cdr(x))));
}

int list2(int x, int y)
{
    return (cons(x, cons(y, NIL)));
}

int list3(int x, int y, int z)
{
    return (cons(x, cons(y, cons(z, NIL))));
}


int quasi_transfer2(int x, int n)
{
    if (nullp(x))
	return (NIL);
    else if (atomp(x))
	return (list2(makesym("quote"), x));
    else if (listp(x) && eqp(car(x), makesym("unquote")) && n == 0)
	return (cadr(x));
    else if (listp(x) && eqp(car(x), makesym("unquote-splicing"))
	     && n == 0)
	return (cadr(x));
    else if (listp(x) && eqp(car(x), makesym("quasi-quote")))
	return (list3(makesym("list"),
		      list2(makesym("quote"), makesym("quasi-quote")),
		      quasi_transfer2(cadr(x), n + 1)));
    else if (listp(x) && eqp(caar(x), makesym("unquote")) && n == 0)
	return (list3
		(makesym("cons"), cadar(x), quasi_transfer2(cdr(x), n)));
    else if (listp(x) && eqp(caar(x), makesym("unquote-splicing"))
	     && n == 0)
	return (list3
		(makesym("append"), cadar(x),
		 quasi_transfer2(cdr(x), n - 1)));
    else if (listp(x) && eqp(caar(x), makesym("unquote")))
	return (list3(makesym("cons"),
		      list3(makesym("list"),
			    list2(makesym("quote"), makesym("unquote")),
			    quasi_transfer2(cadar(x), n - 1)),
		      quasi_transfer2(cdr(x), n)));
    else if (listp(x) && eqp(caar(x), makesym("unquote-splicing")))
	return (list3(makesym("cons"),
		      list3(makesym("list"),
			    list2(makesym("quote"),
				  makesym("unquote-splicing")),
			    quasi_transfer2(cadar(x), n - 1)),
		      quasi_transfer2(cdr(x), n)));
    else
	return (list3
		(makesym("cons"), quasi_transfer2(car(x), n),
		 quasi_transfer2(cdr(x), n)));
}
