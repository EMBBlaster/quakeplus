/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "quakedef.h"


/*

*/

typedef struct
{
	int				s;
	dfunction_t		*f;
} prstack_t;

#define	MAX_STACK_DEPTH		32
prstack_t	pr_stack[MAX_STACK_DEPTH];
int			pr_depth;

#define	LOCALSTACK_SIZE		2048
int			localstack[LOCALSTACK_SIZE];
int			localstack_used;


bool	pr_trace;
dfunction_t	*pr_xfunction;
int			pr_xstatement;


int		pr_argc;

char *pr_opnames[] =
{
(char*)"DONE",

(char*)"MUL_F",
(char*)"MUL_V", 
(char*)"MUL_FV",
(char*)"MUL_VF",
 
(char*)"DIV",

(char*)"ADD_F",
(char*)"ADD_V", 
  
(char*)"SUB_F",
(char*)"SUB_V",

(char*)"EQ_F",
(char*)"EQ_V",
(char*)"EQ_S", 
(char*)"EQ_E",
(char*)"EQ_FNC",
 
(char*)"NE_F",
(char*)"NE_V", 
(char*)"NE_S",
(char*)"NE_E", 
(char*)"NE_FNC",
 
(char*)"LE",
(char*)"GE",
(char*)"LT",
(char*)"GT", 

(char*)"INDIRECT",
(char*)"INDIRECT",
(char*)"INDIRECT", 
(char*)"INDIRECT", 
(char*)"INDIRECT",
(char*)"INDIRECT", 

(char*)"ADDRESS", 

(char*)"STORE_F",
(char*)"STORE_V",
(char*)"STORE_S",
(char*)"STORE_ENT",
(char*)"STORE_FLD",
(char*)"STORE_FNC",

(char*)"STOREP_F",
(char*)"STOREP_V",
(char*)"STOREP_S",
(char*)"STOREP_ENT",
(char*)"STOREP_FLD",
(char*)"STOREP_FNC",

(char*)"RETURN",
  
(char*)"NOT_F",
(char*)"NOT_V",
(char*)"NOT_S", 
(char*)"NOT_ENT", 
(char*)"NOT_FNC", 
  
(char*)"IF",
(char*)"IFNOT",
  
(char*)"CALL0",
(char*)"CALL1",
(char*)"CALL2",
(char*)"CALL3",
(char*)"CALL4",
(char*)"CALL5",
(char*)"CALL6",
(char*)"CALL7",
(char*)"CALL8",
  
(char*)"STATE",
  
(char*)"GOTO", 
  
(char*)"AND",
(char*)"OR", 

(char*)"BITAND",
(char*)"BITOR"
};

char *PR_GlobalString (int ofs);
char *PR_GlobalStringNoContents (int ofs);


//=============================================================================

/*
=================
PR_PrintStatement
=================
*/
void PR_PrintStatement (dstatement_t *s)
{
	int		i;
	
	if ( (unsigned)s->op < sizeof(pr_opnames)/sizeof(pr_opnames[0]))
	{
		Con_Printf ((char*)"%s ",  pr_opnames[s->op]);
		i = strlen(pr_opnames[s->op]);
		for ( ; i<10 ; i++)
			Con_Printf ((char*)" ");
	}
		
	if (s->op == OP_IF || s->op == OP_IFNOT)
		Con_Printf ((char*)"%sbranch %i",PR_GlobalString(s->a),s->b);
	else if (s->op == OP_GOTO)
	{
		Con_Printf ((char*)"branch %i",s->a);
	}
	else if ( (unsigned)(s->op - OP_STORE_F) < 6)
	{
		Con_Printf ((char*)"%s",PR_GlobalString(s->a));
		Con_Printf ((char*)"%s", PR_GlobalStringNoContents(s->b));
	}
	else
	{
		if (s->a)
			Con_Printf ((char*)"%s",PR_GlobalString(s->a));
		if (s->b)
			Con_Printf ((char*)"%s",PR_GlobalString(s->b));
		if (s->c)
			Con_Printf ((char*)"%s", PR_GlobalStringNoContents(s->c));
	}
	Con_Printf ((char*)"\n");
}

/*
============
PR_StackTrace
============
*/
void PR_StackTrace (void)
{
	dfunction_t	*f;
	int			i;
	
	if (pr_depth == 0)
	{
		Con_Printf ((char*)"<NO STACK>\n");
		return;
	}
	
	pr_stack[pr_depth].f = pr_xfunction;
	for (i=pr_depth ; i>=0 ; i--)
	{
		f = pr_stack[i].f;
		
		if (!f)
		{
			Con_Printf ((char*)"<NO FUNCTION>\n");
		}
		else
			Con_Printf ((char*)"%12s : %s\n", pr_strings + f->s_file, pr_strings + f->s_name);		
	}
}


/*
============
PR_Profile_f

============
*/
void PR_Profile_f (void)
{
	dfunction_t	*f, *best;
	int			max;
	int			num;
	int			i;
	
	num = 0;	
	do
	{
		max = 0;
		best = NULL;
		for (i=0 ; i<progs->numfunctions ; i++)
		{
			f = &pr_functions[i];
			if (f->profile > max)
			{
				max = f->profile;
				best = f;
			}
		}
		if (best)
		{
			if (num < 10)
				Con_Printf ((char*)"%7i %s\n", best->profile, pr_strings+best->s_name);
			num++;
			best->profile = 0;
		}
	} while (best);
}


/*
============
PR_RunError

Aborts the currently executing function
============
*/
void PR_RunError (char *error, ...)
{
	va_list		argptr;
	char		string[1024];

	va_start (argptr,error);
	vsnprintf (string,1024,error,argptr);
	va_end (argptr);

	PR_PrintStatement (pr_statements + pr_xstatement);
	PR_StackTrace ();
	Con_Printf ((char*)"%s\n", string);
	
	pr_depth = 0;		// dump the stack so host_error can shutdown functions

	Host_Error ((char*)"Program error");
}

/*
============================================================================
PR_ExecuteProgram

The interpretation main loop
============================================================================
*/

/*
====================
PR_EnterFunction

Returns the new program statement counter
====================
*/
int PR_EnterFunction (dfunction_t *f)
{
	int		i, j, c, o;

	pr_stack[pr_depth].s = pr_xstatement;
	pr_stack[pr_depth].f = pr_xfunction;	
	pr_depth++;
	if (pr_depth >= MAX_STACK_DEPTH)
		PR_RunError ((char*)"stack overflow");

// save off any locals that the new function steps on
	c = f->locals;
	if (localstack_used + c > LOCALSTACK_SIZE)
		PR_RunError ((char*)"PR_ExecuteProgram: locals stack overflow\n");

	for (i=0 ; i < c ; i++)
		localstack[localstack_used+i] = ((int *)pr_globals)[f->parm_start + i];
	localstack_used += c;

// copy parameters
	o = f->parm_start;
	for (i=0 ; i<f->numparms ; i++)
	{
		for (j=0 ; j<f->parm_size[i] ; j++)
		{
			((int *)pr_globals)[o] = ((int *)pr_globals)[OFS_PARM0+i*3+j];
			o++;
		}
	}

	pr_xfunction = f;
	return f->first_statement - 1;	// offset the s++
}

/*
====================
PR_LeaveFunction
====================
*/
int PR_LeaveFunction (void)
{
	int		i, c;

	if (pr_depth <= 0)
		Sys_Error ((char*)"prog stack underflow");

// restore locals from the stack
	c = pr_xfunction->locals;
	localstack_used -= c;
	if (localstack_used < 0)
		PR_RunError ((char*)"PR_ExecuteProgram: locals stack underflow\n");

	for (i=0 ; i < c ; i++)
		((int *)pr_globals)[pr_xfunction->parm_start + i] = localstack[localstack_used+i];

// up stack
	pr_depth--;
	pr_xfunction = pr_stack[pr_depth].f;
	return pr_stack[pr_depth].s;
}


/*
====================
PR_ExecuteProgram
====================
*/
void PR_ExecuteProgram (func_t fnum)
{
	eval_t	*a, *b, *c;
	int			s;
	dstatement_t	*st;
	dfunction_t	*f, *newf;
	int		runaway;
	int		i;
	edict_t	*ed;
	int		exitdepth;
	eval_t	*ptr;

	if (!fnum || fnum >= progs->numfunctions)
	{
		if (pr_global_struct->self)
			ED_Print (PROG_TO_EDICT(pr_global_struct->self));
		Host_Error ((char*)"PR_ExecuteProgram: NULL function");
	}
	
	f = &pr_functions[fnum];

	runaway = 100000;
	pr_trace = false;

// make a stack frame
	exitdepth = pr_depth;

	s = PR_EnterFunction (f);
	
while (1)
{
	s++;	// next statement

	st = &pr_statements[s];
	a = (eval_t *)&pr_globals[st->a];
	b = (eval_t *)&pr_globals[st->b];
	c = (eval_t *)&pr_globals[st->c];
	
	if (!--runaway)
		PR_RunError ((char*)"runaway loop error");
		
	pr_xfunction->profile++;
	pr_xstatement = s;
	
	if (pr_trace)
		PR_PrintStatement (st);
		
	switch (st->op)
	{
	case OP_ADD_F:
		c->_float = a->_float + b->_float;
		break;
	case OP_ADD_V:
		c->vector[0] = a->vector[0] + b->vector[0];
		c->vector[1] = a->vector[1] + b->vector[1];
		c->vector[2] = a->vector[2] + b->vector[2];
		break;
		
	case OP_SUB_F:
		c->_float = a->_float - b->_float;
		break;
	case OP_SUB_V:
		c->vector[0] = a->vector[0] - b->vector[0];
		c->vector[1] = a->vector[1] - b->vector[1];
		c->vector[2] = a->vector[2] - b->vector[2];
		break;

	case OP_MUL_F:
		c->_float = a->_float * b->_float;
		break;
	case OP_MUL_V:
		c->_float = a->vector[0]*b->vector[0]
				+ a->vector[1]*b->vector[1]
				+ a->vector[2]*b->vector[2];
		break;
	case OP_MUL_FV:
		c->vector[0] = a->_float * b->vector[0];
		c->vector[1] = a->_float * b->vector[1];
		c->vector[2] = a->_float * b->vector[2];
		break;
	case OP_MUL_VF:
		c->vector[0] = b->_float * a->vector[0];
		c->vector[1] = b->_float * a->vector[1];
		c->vector[2] = b->_float * a->vector[2];
		break;

	case OP_DIV_F:
		c->_float = a->_float / b->_float;
		break;
	
	case OP_BITAND:
		c->_float = (int)a->_float & (int)b->_float;
		break;
	
	case OP_BITOR:
		c->_float = (int)a->_float | (int)b->_float;
		break;
	
		
	case OP_GE:
		c->_float = a->_float >= b->_float;
		break;
	case OP_LE:
		c->_float = a->_float <= b->_float;
		break;
	case OP_GT:
		c->_float = a->_float > b->_float;
		break;
	case OP_LT:
		c->_float = a->_float < b->_float;
		break;
	case OP_AND:
		c->_float = a->_float && b->_float;
		break;
	case OP_OR:
		c->_float = a->_float || b->_float;
		break;
		
	case OP_NOT_F:
		c->_float = !a->_float;
		break;
	case OP_NOT_V:
		c->_float = !a->vector[0] && !a->vector[1] && !a->vector[2];
		break;
	case OP_NOT_S:
		c->_float = !a->string || !pr_strings[a->string];
		break;
	case OP_NOT_FNC:
		c->_float = !a->function;
		break;
	case OP_NOT_ENT:
		c->_float = (PROG_TO_EDICT(a->edict) == sv.edicts);
		break;

	case OP_EQ_F:
		c->_float = a->_float == b->_float;
		break;
	case OP_EQ_V:
		c->_float = (a->vector[0] == b->vector[0]) &&
					(a->vector[1] == b->vector[1]) &&
					(a->vector[2] == b->vector[2]);
		break;
	case OP_EQ_S:
		c->_float = !strcmp(pr_strings+a->string,pr_strings+b->string);
		break;
	case OP_EQ_E:
		c->_float = a->_int == b->_int;
		break;
	case OP_EQ_FNC:
		c->_float = a->function == b->function;
		break;


	case OP_NE_F:
		c->_float = a->_float != b->_float;
		break;
	case OP_NE_V:
		c->_float = (a->vector[0] != b->vector[0]) ||
					(a->vector[1] != b->vector[1]) ||
					(a->vector[2] != b->vector[2]);
		break;
	case OP_NE_S:
		c->_float = strcmp(pr_strings+a->string,pr_strings+b->string);
		break;
	case OP_NE_E:
		c->_float = a->_int != b->_int;
		break;
	case OP_NE_FNC:
		c->_float = a->function != b->function;
		break;

//==================
	case OP_STORE_F:
	case OP_STORE_ENT:
	case OP_STORE_FLD:		// integers
	case OP_STORE_S:
	case OP_STORE_FNC:		// pointers
		b->_int = a->_int;
		break;
	case OP_STORE_V:
		b->vector[0] = a->vector[0];
		b->vector[1] = a->vector[1];
		b->vector[2] = a->vector[2];
		break;
		
	case OP_STOREP_F:
	case OP_STOREP_ENT:
	case OP_STOREP_FLD:		// integers
	case OP_STOREP_S:
	case OP_STOREP_FNC:		// pointers
		ptr = (eval_t *)((byte *)sv.edicts + b->_int);
		ptr->_int = a->_int;
		break;
	case OP_STOREP_V:
		ptr = (eval_t *)((byte *)sv.edicts + b->_int);
		ptr->vector[0] = a->vector[0];
		ptr->vector[1] = a->vector[1];
		ptr->vector[2] = a->vector[2];
		break;
		
	case OP_ADDRESS:
		ed = PROG_TO_EDICT(a->edict);
#ifdef PARANOID
		NUM_FOR_EDICT(ed);		// make sure it's in range
#endif
		if (ed == (edict_t *)sv.edicts && sv.state == ss_active)
			PR_RunError ((char*)"assignment to world entity");
		c->_int = (byte *)((int *)&ed->v + b->_int) - (byte *)sv.edicts;
		break;
		
	case OP_LOAD_F:
	case OP_LOAD_FLD:
	case OP_LOAD_ENT:
	case OP_LOAD_S:
	case OP_LOAD_FNC:
		ed = PROG_TO_EDICT(a->edict);
#ifdef PARANOID
		NUM_FOR_EDICT(ed);		// make sure it's in range
#endif
		a = (eval_t *)((int *)&ed->v + b->_int);
		c->_int = a->_int;
		break;

	case OP_LOAD_V:
		ed = PROG_TO_EDICT(a->edict);
#ifdef PARANOID
		NUM_FOR_EDICT(ed);		// make sure it's in range
#endif
		a = (eval_t *)((int *)&ed->v + b->_int);
		c->vector[0] = a->vector[0];
		c->vector[1] = a->vector[1];
		c->vector[2] = a->vector[2];
		break;
		
//==================

	case OP_IFNOT:
		if (!a->_int)
			s += st->b - 1;	// offset the s++
		break;
		
	case OP_IF:
		if (a->_int)
			s += st->b - 1;	// offset the s++
		break;
		
	case OP_GOTO:
		s += st->a - 1;	// offset the s++
		break;
		
	case OP_CALL0:
	case OP_CALL1:
	case OP_CALL2:
	case OP_CALL3:
	case OP_CALL4:
	case OP_CALL5:
	case OP_CALL6:
	case OP_CALL7:
	case OP_CALL8:
		pr_argc = st->op - OP_CALL0;
		if (!a->function)
			PR_RunError ((char*)"NULL function");

		newf = &pr_functions[a->function];

		if (newf->first_statement < 0)
		{	// negative statements are built in functions
			i = -newf->first_statement;
			if (i >= pr_numbuiltins)
				PR_RunError ((char*)"Bad builtin call number");
			pr_builtins[i] ();
			break;
		}

		s = PR_EnterFunction (newf);
		break;

	case OP_DONE:
	case OP_RETURN:
		pr_globals[OFS_RETURN] = pr_globals[st->a];
		pr_globals[OFS_RETURN+1] = pr_globals[st->a+1];
		pr_globals[OFS_RETURN+2] = pr_globals[st->a+2];
	
		s = PR_LeaveFunction ();
		if (pr_depth == exitdepth)
			return;		// all done
		break;
		
	case OP_STATE:
		ed = PROG_TO_EDICT(pr_global_struct->self);
#ifdef FPS_20
		ed->v.nextthink = pr_global_struct->time + 0.05;
#else
		ed->v.nextthink = pr_global_struct->time + 0.1;
#endif
		if (a->_float != ed->v.frame)
		{
			ed->v.frame = a->_float;
		}
		ed->v.think = b->function;
		break;
		
	default:
		PR_RunError ((char*)"Bad opcode %i", st->op);
	}
}

}
