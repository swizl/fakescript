#include "types.h"
#include "bison.h"
#include "fake.h"
#include "variant.h"
#include "paramstack.h"
#ifdef WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif
#include "buffer.h"

void fklog(const char * header, const char * file, const char * func, int pos, const char *fmt, ...)
{
	FILE *pLog = NULL;
	time_t clock1;
	struct tm * tptr;
	va_list ap;
	
	pLog = fopen("fakescript.log", "a+");
	if (pLog == NULL)
	{
		return;
	}
	
	clock1 = time(0);
	tptr = localtime(&clock1);

	fprintf(pLog, "===========================[%d.%d.%d, %d.%d.%d]%s:%d,%s:===========================\n%s", 
		tptr->tm_year+1990,tptr->tm_mon+1,
		tptr->tm_mday,tptr->tm_hour,tptr->tm_min,
		tptr->tm_sec,file,pos,func,header);

	va_start(ap, fmt);
	vfprintf(pLog, fmt, ap);
	fprintf(pLog, "\n\n");
	va_end(ap);

	fclose(pLog);
}

String fkget_token_name(int token)
{
#define TOKEN_SWITCH(x) case x: return #x;
	switch (token)
	{
		TOKEN_SWITCH(0)
		TOKEN_SWITCH(VAR_BEGIN)
		TOKEN_SWITCH(RETURN)
		TOKEN_SWITCH(BREAK)
		TOKEN_SWITCH(FUNC)
		TOKEN_SWITCH(WHILE)
		TOKEN_SWITCH(FTRUE)
		TOKEN_SWITCH(FFALSE)
		TOKEN_SWITCH(IF)
		TOKEN_SWITCH(THEN)
		TOKEN_SWITCH(ELSE)
		TOKEN_SWITCH(END)
		TOKEN_SWITCH(STRING_DEFINITION)
		TOKEN_SWITCH(IDENTIFIER)
		TOKEN_SWITCH(NUMBER)
		TOKEN_SWITCH(SINGLE_LINE_COMMENT)
		TOKEN_SWITCH(DIVIDE_MOD)
		TOKEN_SWITCH(ARG_SPLITTER)
		TOKEN_SWITCH(PLUS)
		TOKEN_SWITCH(MINUS)
		TOKEN_SWITCH(DIVIDE)
		TOKEN_SWITCH(MULTIPLY)
		TOKEN_SWITCH(ASSIGN)
		TOKEN_SWITCH(MORE)
		TOKEN_SWITCH(LESS)
		TOKEN_SWITCH(MORE_OR_EQUAL)
		TOKEN_SWITCH(LESS_OR_EQUAL)
		TOKEN_SWITCH(EQUAL)
		TOKEN_SWITCH(NOT_EQUAL)
		TOKEN_SWITCH(OPEN_BRACKET)
		TOKEN_SWITCH(CLOSE_BRACKET)
		TOKEN_SWITCH(AND)
		TOKEN_SWITCH(OR)
		TOKEN_SWITCH(FKFLOAT)
		TOKEN_SWITCH(PLUS_ASSIGN)
		TOKEN_SWITCH(MINUS_ASSIGN)
		TOKEN_SWITCH(DIVIDE_ASSIGN)
		TOKEN_SWITCH(MULTIPLY_ASSIGN)
		TOKEN_SWITCH(DIVIDE_MOD_ASSIGN)
		TOKEN_SWITCH(COLON)
		TOKEN_SWITCH(FOR)
		TOKEN_SWITCH(INC)
		TOKEN_SWITCH(FAKE)
		TOKEN_SWITCH(FKUUID)
		TOKEN_SWITCH(OPEN_SQUARE_BRACKET)
		TOKEN_SWITCH(CLOSE_SQUARE_BRACKET)
		TOKEN_SWITCH(FCONST)
		TOKEN_SWITCH(PACKAGE)
		TOKEN_SWITCH(INCLUDE)
		TOKEN_SWITCH(IDENTIFIER_DOT)
		TOKEN_SWITCH(IDENTIFIER_POINTER)
		TOKEN_SWITCH(STRUCT)
		TOKEN_SWITCH(IS)
		TOKEN_SWITCH(NOT)
		TOKEN_SWITCH(CONTINUE)
		TOKEN_SWITCH(YIELD)
		TOKEN_SWITCH(SLEEP)
		TOKEN_SWITCH(SWITCH)
		TOKEN_SWITCH(CASE)
		TOKEN_SWITCH(DEFAULT)
		TOKEN_SWITCH(NEW_ASSIGN)
		TOKEN_SWITCH(ELSEIF)
		TOKEN_SWITCH(RIGHT_POINTER)
	}
#undef TOKEN_SWITCH
	assert(0);
	return fkitoa(token);
}

void * safe_fkmalloc(fake * fk, size_t size)
{
	if (fk && size)
	{
		return fk->cfg.fkm(size);
	}
	return 0;
}

void safe_fkfree(fake * fk, const void * p)
{
	if (fk && p)
	{
		fk->cfg.fkf((void *)p);
	}
}

void seterror(fake * fk, efkerror err, const char *fmt, ...)
{
	fk->errorno = err;
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(fk->errorstr, sizeof(fk->errorstr) - 1, fmt, ap);
	va_end(ap);
	fk->errorstr[sizeof(fk->errorstr) - 1] = 0;
	if (fk->errorcb)
	{
		fk->errorcb(fk, fk->errorno, fk->errorstr);
	}
}

String fkstringeletoa(stringele * ele)
{
	return ele && ele->s ? ele->s : "";
}

String vartostring(const variant * v)
{
	String s;
	V_TOSTRING(v, s);
	return s;
}

const char * vartypetostring(int type)
{
	switch (type)
	{
	case variant::NIL:
		return "NIL";
	case variant::REAL:
		return "REAL";
	case variant::STRING:
		return "STRING";
	case variant::POINTER:
		return "POINTER";
	case variant::UUID:
		return "UUID";
	case variant::ARRAY:
		return "ARRAY";
	case variant::MAP:
		return "MAP";
	default:
		return "unknow";
	}
}

paramstack * getps(fake * fk)
{
	return &fk->ps;
}

char * stringdump(fake * fk, const char * src, size_t sz)
{
	char * s = (char*)safe_fkmalloc(fk, sz + 1);
	memcpy(s, src, sz);
	s[sz] = 0;
	return s;
}

uint32_t fkgetmstick()
{
#ifdef WIN32
	return ::GetTickCount();
#else
	struct timeval tv;
	if(::gettimeofday(&tv, 0) == 0)
	{
		uint64_t t = tv.tv_sec * 1000;
		t += tv.tv_usec / 1000;
		return t & 0xffffffff;
	}
	return 0;
#endif
}

String fkarraytoa(variant_array * va)
{
	if (ARRAY_RECUR(va->va))
	{
		return "ARRAY IN RECUR";
	}
	ARRAY_ENTER(va->va);
	
	String ret;
	ret += "[";
	for (int i = 0; i < (int)ARRAY_MAX_SIZE(va->va); i++)
	{
		pool<variant>::node * n = ARRAY_GET(va->va, i);
		if (n)
		{
			ret += vartostring(&(n->t));
		}
		else
		{
			ret += " ";
		}
		ret += ",";
	}
	ret += "]";
	
	ARRAY_LEAVE(va->va);
	
	return ret;
}

String fkmaptoa(variant_map * vm)
{
	if (HASHMAP_RECUR(vm->vm))
	{
		return "MAP IN RECUR";
	}
	HASHMAP_ENTER(vm->vm);
	
	String ret;
	ret += "{";
	int i = 0;
	for (const fkhashmap<variant, pool<variant>::node *>::ele * p = vm->vm.first(); p != 0; p = vm->vm.next())
	{
		const variant & key = p->k;
		const pool<variant>::node * value = *p->t;
		if (!i)
		{
			ret += "(";
		}
		else
		{
			ret += ",(";
		}
		ret += vartostring(&key);
		ret += ",";
		ret += vartostring(&(value->t));
		ret += ")";
		i++;
	}
	ret += "}";
	
	HASHMAP_LEAVE(vm->vm);
	
	return ret;
}

bool save_variant(fake * fk, const variant * v, buffer * b)
{
	if (!b->write((const char *)&(v->type), sizeof(v->type)))
	{
		return false;
	}
	
	if (v->type == variant::REAL ||
		v->type == variant::UUID ||
		v->type == variant::NIL)
	{
		return b->write((const char *)&(v->data), sizeof(v->data));
	}
	else if (v->type == variant::STRING)
	{
		const char * ss = v->data.str ? v->data.str->s : "";
		return save_string(fk, ss, b);
	}
	else if (v->type == variant::POINTER ||
		v->type == variant::ARRAY ||
		v->type == variant::MAP)
	{
		// 暂时不支持
	}
	
	return false;
}

bool load_variant(fake * fk, variant * v, buffer * b)
{
	if (!b->read((char *)&(v->type), sizeof(v->type)))
	{
		return false;
	}
	
	if (v->type == variant::REAL ||
		v->type == variant::UUID ||
		v->type == variant::NIL)
	{
		return b->read((char *)&(v->data), sizeof(v->data));
	}
	else if (v->type == variant::STRING)
	{
		String s;
		if (!load_string(fk, s, b))
		{
			return false;
		}

		*v = fk->sh.allocsysstr(s.c_str());
		return true;
	}
	else if (v->type == variant::POINTER ||
		v->type == variant::ARRAY ||
		v->type == variant::MAP)
	{
		// 暂时不支持
	}
	
	return false;
}

bool save_string(fake * fk, const char * str, buffer * b)
{
	int len = strlen(str);
	if (!b->write((const char *)&len, sizeof(len)))
	{
		return false;
	}
	return b->write(str, len);
}

bool load_string(fake * fk, String & str, buffer * b)
{
	int len = 0;
	if (!b->read((char *)&len, sizeof(len)))
	{
		return false;
	}
	char * ss = (char *)safe_fkmalloc(fk, len + 1);
	ss[len] = 0;
	if (!b->read(ss, len))
	{
		safe_fkfree(fk, ss);
		return false;
	}
	str = ss;
	return true;
}

