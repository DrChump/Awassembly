#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

/*
* converts Awassembly into Awabytecode
* no dependencies
* supplied as is, without warranty
*/

#define AWATISM_LIST \
	X(nop) \
	X(prn) \
	X(pr1) \
	X(red) \
	X(r3d) \
	X(blo) \
	X(sbm) \
	X(pop) \
	X(dpl) \
	X(srn) \
	X(mrg) \
	X(4dd) \
	X(sub) \
	X(mul) \
	X(div) \
	X(cnt) \
	X(lbl) \
	X(jmp) \
	X(eql) \
	X(lss) \
	X(gr8)

#define AWASCII_ALNUM_LIST \
	X(A) \
	X(W) \
	X(a) \
	X(w) \
	X(J) \
	X(E) \
	X(L) \
	X(Y) \
	X(H) \
	X(O) \
	X(S) \
	X(I) \
	X(U) \
	X(M) \
	X(j) \
	X(e) \
	X(l) \
	X(y) \
	X(h) \
	X(o) \
	X(s) \
	X(i) \
	X(u) \
	X(m) \
	X(P) \
	X(C) \
	X(N) \
	X(T) \
	X(p) \
	X(c) \
	X(n) \
	X(t) \
	X(B) \
	X(D) \
	X(F) \
	X(G) \
	X(R) \
	X(b) \
	X(d) \
	X(f) \
	X(g) \
	X(r) \
	X(0) \
	X(1) \
	X(2) \
	X(3) \
	X(4) \
	X(5) \
	X(6) \
	X(7) \
	X(8) \
	X(9)

#define X(name) \
	AWAT_ ## name,
typedef enum {
	AWATISM_LIST
	AWAT_trm = 0x1F,
	AWAT_invalid,
} AWATISM;
#undef X

#define X(name) \
	AWAC_ ## name,
typedef enum {
	AWASCII_ALNUM_LIST
	AWAC_SPACE,
	AWAC_DOT,
	AWAC_COMMA,
	AWAC_EXCLAM,
	AWAC_QUOTE,
	AWAC_OPEN,
	AWAC_CLOSE,
	AWAC_TILDE,
	AWAC_UNDERSC,
	AWAC_SLASH,
	AWAC_SEMICOL,
	AWAC_NEWLINE,
} AWASCII;
#undef X

typedef enum {
	TOK_TISM,
	TOK_AWASCII,
	TOK_NUM,
	TOK_PRINT,
	TOK_OVER,
	TOK_STRING,
	TOK_EOF,
} TOKEN;

typedef struct {
	char *content;
	size_t count;
	size_t cursor;
} entire_file;

typedef struct {
	char *filename;
	entire_file ef;
	size_t linenum;
	AWATISM tism;
} lexer;

typedef struct {
	char *start;
	size_t len;
} sview;

typedef struct nlist nlist;
struct nlist {
	struct nlist *next;
	char *name;
	char *defn;
};

unsigned hash(char *s);
nlist *lookup(char *s);
nlist *install(char *name, char *defn);
bool undef(char *name);

void usage(char *progname);
bool read_entire_file(entire_file *ef, char *filename);
void free_entire_file(entire_file ef);
TOKEN gettoken(lexer *l, sview *tokstr);

char *awastr(int8_t val, int bits);
void init_lookup_table(void);
AWATISM tism_from_str(sview tstr);

bool print_stack_push(char *s);
char *print_stack_pop(void);
int print_stack_size(void);

char *convert_to_funnyspeak_if_possible(char *s);
char *sv_to_cstr(sview s);
void free_cstr_buf(void);

int main(int argc, char *argv[])
{
	char *progname = argv[0];
	char *outfile, *infile;
	outfile = infile = NULL;

	init_lookup_table();

	bool more_opts = true;
	while (--argc > 0) {
		++argv;
		if ((*argv)[0] == '-' && more_opts) {
			if (strcmp("-o", *argv) == 0 && outfile == NULL) {
				if (--argc > 0) ++argv;
				else usage(progname);
				outfile = *argv;
			} else if (strcmp("--", *argv) == 0) {
				more_opts = false;
			} else usage(progname);
		} else if (infile == NULL){
			infile = *argv;
		} else usage(progname);
	}
	if (infile == NULL) usage(progname);
	if (outfile == NULL) outfile = "awa.out";

	lexer l = {0};
	if (!read_entire_file(&l.ef, infile)) {
		fprintf(stderr, "An error occured when reading %s\n", infile);
		exit(1);
	}
	l.filename = infile;
	l.linenum = 1;
	TOKEN tok;
	sview op, arg;

	FILE *tmp = tmpfile();
	if (tmp == NULL) goto temp_perr;

#define TEMP_PRINT(...) \
	do { \
		if (fprintf(tmp, __VA_ARGS__) < 0) goto temp_perr; \
	} while (0)

	TEMP_PRINT("Awa");
	while ((tok = gettoken(&l, &op)) != TOK_EOF) {
		if (tok == TOK_TISM) {
			nlist *np = lookup(sv_to_cstr(op));
			if (np != NULL) {
				TEMP_PRINT("%s", np->defn);
				switch (l.tism) {
				case AWAT_blo: {
					int n;
					tok = gettoken(&l, &arg);
					if (tok == TOK_AWASCII) {
						np = lookup(sv_to_cstr(arg));
						if (np != NULL) {
							TEMP_PRINT("%s", np->defn);
						} else {
							fprintf(stderr, "%s:%zu: couldn't look up %.*s\n",
									l.filename, l.linenum, (int)arg.len, arg.start);
							exit(1);
						}
					} else if (tok == TOK_NUM && arg.len < 5 &&
							(n = atoi(sv_to_cstr(arg))) >= -128 && n <= 127) {
						TEMP_PRINT("%s", awastr(n, 8));
					} else {
						fprintf(stderr, "%s:%zu: expected 8 bit signed integer [-128,127] or AwaSCII char\n",
								l.filename, l.linenum);
						exit(1);
					}
					} break;
				case AWAT_sbm:
				case AWAT_srn:
				case AWAT_lbl:
				case AWAT_jmp: {
					int n;
					tok = gettoken(&l, &arg);
					if (tok == TOK_NUM && arg.len < 3 &&
							(n = atoi(sv_to_cstr(arg))) >= 0 && n <= 31) {
						TEMP_PRINT("%s", awastr(n, 5));
					} else {
						fprintf(stderr, "%s:%zu: expected 5 bit integer [0,31]\n",
								l.filename, l.linenum);
						exit(1);
					}
					} break;
				default:
					break;
				}
			} else {
				fprintf(stderr, "%s:%zu: couldn't look up %.*s\n",
						l.filename, l.linenum, (int)op.len, op.start);
				exit(1);
			}
		} else if (tok == TOK_PRINT) {
			tok = gettoken(&l, &arg);
			if (tok == TOK_STRING) {
				while (arg.len > 0) {
					size_t len;
					if (*arg.start == '\\')
						len = 2;
					else
						len = 1;
					assert(arg.len >= len);
					sview v = {.start = arg.start, .len = len};
					nlist *np = lookup(convert_to_funnyspeak_if_possible(sv_to_cstr(v)));
					if (np != NULL) {
						if (!print_stack_push(np->defn)) {
#define FLUSH() \
							char *printme; \
							int size = print_stack_size(); \
							while ((printme = print_stack_pop()) != NULL) { \
								TEMP_PRINT("%s", lookup("blo")->defn); \
								TEMP_PRINT("%s", printme); \
							} \
							TEMP_PRINT("%s", lookup("srn")->defn); \
							TEMP_PRINT("%s", awastr(size, 5)); \
							TEMP_PRINT("%s", lookup("prn")->defn);
							FLUSH();
							print_stack_push(np->defn);
						}
					} else {
						fprintf(stderr, "%s:%zu: couldn't look up %.*s\n",
								l.filename, l.linenum, (int)v.len, v.start);
						exit(1);
					}
					arg.len -= len;
					arg.start += len;
				}
				FLUSH();
			} else {
				fprintf(stderr, "%s:%zu: expected string\n", l.filename, l.linenum);
				exit(1);
			}
		} else if (tok == TOK_OVER) {
			// OVER 0 is the same as dpl
			int n;
			tok = gettoken(&l, &arg);
			if (tok == TOK_NUM && arg.len < 3 &&
					(n = atoi(sv_to_cstr(arg))) >= 0 && n <= 30) {
				for (int i = 0; i < n; i++)
					TEMP_PRINT("%s%s", lookup("sbm")->defn, awastr(n, 5)); // n times
				TEMP_PRINT("%s", lookup("dpl")->defn); // 1 time
				TEMP_PRINT("%s%s", lookup("sbm")->defn, awastr(n + 1, 5)); // 1 time
			} else {
				fprintf(stderr, "%s:%zu: expected 5 bit integer [0,30]\n",
						l.filename, l.linenum);
				exit(1);
			}
		} else {
			fprintf(stderr, "%s:%zu: expected operator\n", l.filename, l.linenum);
			exit(1);
		}
	}

	FILE *out = fopen(outfile, "w");
	if (out == NULL) goto out_perr;

	if (fseek(tmp, 0, SEEK_SET) == -1) goto temp_perr;

	// Copy from tempfile to outfile
	{
#define BUFSIZE 1024 * 1024
		char buf[BUFSIZE];
		size_t n;
		while ((n = fread(buf, sizeof(char), BUFSIZE, tmp)) > 0)
			if (fwrite(buf, sizeof(char), n, out) != n) {
				fprintf(stderr, "An error occured when writing to %s\n", outfile);
				exit(1);
			}
		if (ferror(tmp)) {
			fprintf(stderr, "An error occured when writing to %s\n", outfile);
			exit(1);
		}
	}

	if (fclose(tmp) == EOF) goto temp_perr;
	if (fclose(out) == EOF) goto out_perr;

	printf("Awawa\n");
	free_entire_file(l.ef);
	free_cstr_buf();

	return 0;

temp_perr:
	perror("TEMPFILE");
	exit(1);

out_perr:
	perror(outfile);
	exit(1);
}

char *getch(entire_file *ef)
{
	if (ef->cursor >= ef->count) return NULL;

	char *c = ef->content + ef->cursor;
	ef->cursor += 1;
	return c;
}

void ungetch(entire_file *ef)
{
	if (ef->cursor > 0)
		ef->cursor -= 1;
}

TOKEN gettoken(lexer *l, sview *tokstr)
{
	TOKEN res;
	char *c;

	while (true) {
		// skip whitespace
		while ((c = getch(&l->ef)) != NULL && isspace(*c))
			if (*c == '\n') l->linenum += 1;

		// skip comment
		if (c != NULL && *c == ';') {
			while ((c = getch(&l->ef)) != NULL && *c != '\n')
				;
			if (*c == '\n') l->linenum += 1;
		} else
			break;
	}

	if (c == NULL) {
		res = TOK_EOF;
	} else if (*c == '\'') {
		tokstr->start = c + 1;
		tokstr->len = 0;
		bool escaped = false;
		while ((c = getch(&l->ef)) != NULL && (escaped || *c != '\'')) {
			if (*c == '\\')
				escaped = !escaped;
			else
				escaped = false;
			tokstr->len += 1;
		}
		if (c == NULL)
			res = TOK_EOF;
		else
			res = TOK_AWASCII;
	} else if (*c == '"') {
		tokstr->start = c + 1;
		tokstr->len = 0;
		bool escaped = false;
		while ((c = getch(&l->ef)) != NULL && (escaped || *c != '"')) {
			if (*c == '\\')
				escaped = !escaped;
			else
				escaped = false;
			tokstr->len += 1;
		}
		if (c == NULL)
			res = TOK_EOF;
		else
			res = TOK_STRING;
	} else {
		bool numbers_only = true;
		tokstr->start = c;
		tokstr->len = 1;
		if (*c != '-' && *c != '+' && !isdigit(*c))
			numbers_only = false;
		while ((c = getch(&l->ef)) != NULL && !isspace(*c)) {
			if (!isdigit(*c))
				numbers_only = false;
			tokstr->len += 1;
		}
		ungetch(&l->ef);
		if (numbers_only)
			res = TOK_NUM;
		else {
			if (strncmp("PRINT", tokstr->start, tokstr->len) == 0)
				res = TOK_PRINT;
			else if (strncmp("OVER", tokstr->start, tokstr->len) == 0)
				res = TOK_OVER;
			else
				res = TOK_TISM;
		}
	}

	if (res == TOK_TISM)
		l->tism = tism_from_str(*tokstr);
	else
		l->tism = AWAT_invalid;

	return res;
}

bool read_entire_file(entire_file *ef, char *filename)
{
	FILE *f;
	if ((f = fopen(filename, "r")) == NULL) goto perr;
	if (fgetc(f) == EOF && ferror(f)) goto perr; /* Checking for directory before trying to allocate a billion bytes */
	if (fseek(f, 0, SEEK_END) == -1) goto perr;
	long o = ftell(f);
	if (o == -1) goto perr;
	ef->count = o;
	ef->content = malloc(ef->count * sizeof(char));
	ef->cursor = 0;
	if (fseek(f, 0, SEEK_SET) == -1) goto perr;
	size_t numbytes = fread(ef->content, sizeof(char), ef->count, f);
	if (numbytes != ef->count) return false;
	if (fclose(f) == EOF) goto perr;
	return true;
perr:
	perror(filename);
	return false;
}

void free_entire_file(entire_file ef)
{
	free(ef.content);
}

void usage(char *progname)
{
	fprintf(stderr, "Usage: %s [-o outfile] [--] infile\n", progname);
	exit(1);
}

#define HASHSIZE 101

static nlist *hashtab[HASHSIZE];

unsigned hash(char *s)
{
	unsigned hashval;

	for (hashval = 0; *s != '\0'; s++)
		hashval = *s + 31 * hashval;
	return hashval % HASHSIZE;
}

nlist *lookup(char *s)
{
	nlist *np;

	for (np = hashtab[hash(s)];  np != NULL; np = np->next)
		if (strcmp(s, np->name) == 0)
			return np;
	return NULL;
}

nlist *install(char *name, char *defn)
{
	nlist *np;
	unsigned hashval;

	if ((np = lookup(name)) == NULL) {
		np =  malloc(sizeof(*np));
		if (np == NULL || (np->name = strdup(name)) == NULL)
			return NULL;
		hashval = hash(name);
		np->next = hashtab[hashval];
		hashtab[hashval] = np;
	} else
		free(np->defn);
	if ((np->defn = strdup(defn)) == NULL)
		return NULL;
	return np;
}

bool undef(char *name)
{
	nlist *np, *prev, *first;

	first = hashtab[hash(name)];
	prev = first;
	for (np = first; np != NULL; prev = np, np = np->next)
		if (strcmp(name, np->name) == 0) {
			if (np == first)
				hashtab[hash(name)] = np->next;
			else
				prev->next = np->next;
			free(np->defn);
			free(np->name);
			free(np);
			return true;
		}
	return false;
}

char *awastr(int8_t val, int bits)
{
	static char res[100];
	res [0] = '\0';

	for (int i = 0; i < bits; i++) {
		if (val & (1 << (bits - 1 - i))) {
			if (i == 0)
				strcat(res, " ~");
			strcat(res, "wa");
		} else {
			strcat(res, " awa");
		}
	}

	return res;
}

void init_lookup_table(void)
{
#define X(name) \
	install(#name, awastr(AWAT_ ## name, 5));
	AWATISM_LIST
	install("trm", awastr(AWAT_trm, 5));
#undef X
#define X(name) \
	install(#name, awastr(AWAC_ ## name, 8));
	AWASCII_ALNUM_LIST
	install(" "  , awastr(AWAC_SPACE,   8));
	install("."  , awastr(AWAC_DOT,     8));
	install(","  , awastr(AWAC_COMMA,   8));
	install("!"  , awastr(AWAC_EXCLAM,  8));
	install("\\'", awastr(AWAC_QUOTE,   8));
	install("("  , awastr(AWAC_OPEN,    8));
	install(")"  , awastr(AWAC_CLOSE,   8));
	install("~"  , awastr(AWAC_TILDE,   8));
	install("_"  , awastr(AWAC_UNDERSC, 8));
	install("/"  , awastr(AWAC_SLASH,   8));
	install(";"  , awastr(AWAC_SEMICOL, 8));
	install("\\n", awastr(AWAC_NEWLINE, 8));
#undef X
}

AWATISM tism_from_str(sview tstr)
{
	if (strncmp("trm", tstr.start, tstr.len) == 0) return AWAT_trm;
#define X(name) \
	else if (strncmp(#name, tstr.start, tstr.len) == 0) return AWAT_ ## name;
	AWATISM_LIST
#undef X
	else return AWAT_invalid;
}

static char *cstr_buf;
static size_t cstr_buf_size;

#define MAX(a,b) (a>b?a:b)
char *sv_to_cstr(sview s)
{
	if (cstr_buf_size < s.len + 1) {
		if (cstr_buf_size == 0)
			cstr_buf_size = MAX(s.len + 1, 256);
		else
			cstr_buf_size = MAX(s.len + 1, cstr_buf_size * 2);
		cstr_buf = realloc(cstr_buf, cstr_buf_size);
	}
	strncpy(cstr_buf, s.start, s.len);
	cstr_buf[s.len] = '\0';
	return cstr_buf;
}

void free_cstr_buf(void)
{
	cstr_buf_size = 0;
	free(cstr_buf);
}

#define PS_SIZE 31
static char *printstack[PS_SIZE];
static int ps_pos = 0;

bool print_stack_push(char *s)
{
	if (ps_pos < PS_SIZE) {
		printstack[ps_pos] = s;
		ps_pos += 1;
		return true;
	} else {
		return false;
	}
}

char *print_stack_pop(void)
{
	if (ps_pos == 0)
		return NULL;
	else
		return printstack[--ps_pos];
}

int print_stack_size(void)
{
	return ps_pos;
}

char *convert_to_funnyspeak_if_possible(char *s)
{
	if (strcmp("k", s) == 0) return "c";
	else if (strcmp("K", s) == 0) return "C";
	else if (strcmp("q", s) == 0) return "c";
	else if (strcmp("Q", s) == 0) return "C";
	else if (strcmp("z", s) == 0) return "s";
	else if (strcmp("Z", s) == 0) return "S";
	else if (strcmp("v", s) == 0) return "b";
	else if (strcmp("V", s) == 0) return "B";
	else if (strcmp("x", s) == 0) return " ";
	else if (strcmp("X", s) == 0) return " ";
	else if (strcmp(":", s) == 0) return ";";
	else if (strcmp("\\\"", s) == 0) return "\\'";
	else return s;
}

