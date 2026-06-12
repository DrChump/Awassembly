#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
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
	TOK_TISM       = 1<<0,
	TOK_AWASCII    = 1<<1,
	TOK_NUM        = 1<<2,
	TOK_PRINT      = 1<<3,
	TOK_OVER       = 1<<4,
	TOK_SURF       = 1<<5,
	TOK_REPEAT     = 1<<6,
	TOK_REPEAT_END = 1<<7,
	TOK_ITERATOR   = 1<<8,
	TOK_STRING     = 1<<9,
	TOK_NAME       = 1<<10,
	TOK_EOF        = 1<<11,
} TOKEN;

typedef struct {
	char *start;
	size_t len;
} sview;

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

typedef struct sdag_node sdag_node;

typedef struct {
	sdag_node *iterator;
	uint64_t exptoks;
} parser;

typedef enum {
	ST_TISM     = 1<<0,
	ST_CONST    = 1<<1,
	ST_AWASCII  = 1<<2,
	ST_REPEAT   = 1<<3,
	ST_ITERATOR = 1<<4,
	ST_SURF     = 1<<5,
	ST_OVER     = 1<<6,
	ST_MACRO    = 1<<7,
	ST_STRING   = 1<<8,
	ST_PRINT    = 1<<9,
	ST_NAME     = 1<<10,
} NODE_TAG;

typedef struct {
	AWATISM tag;
	sdag_node *arg;
} st_tism;

typedef struct {
	int val;
} st_const;

typedef struct {
	char *awastr;
} st_awascii;

typedef struct {
	sdag_node *range_start;
	sdag_node *range_stop;
	sdag_node *iterator;
	sdag_node *body;
} st_repeat;

typedef struct {
	int val;
	sdag_node *name;
	sdag_node *next_it;
} st_iterator;

typedef struct {
	sview sv;
} st_name;

typedef struct {
	sdag_node *arg;
} st_surf;

typedef struct {
	sdag_node *arg;
} st_over;

typedef struct {
	sdag_node *arg;
} st_print;

typedef struct {
	sview sv;
} st_string;

typedef struct {
	//TODO: arguments
	sdag_node *def;
} st_macro;

struct sdag_node {
	char *filename;
	size_t linenum;
	NODE_TAG tag;
	union {
		st_tism tism;
		st_const constant;
		st_awascii awascii;
		st_repeat repeat;
		st_iterator iterator;
		st_surf surf;
		st_over over;
		st_macro macro;
		st_print print;
		st_string string;
		st_name name;
	} as;
	sdag_node *next;
};

typedef struct {
	uint64_t tags;
	int int_minval;
	int int_maxval;
	int int_awa_bits;
} expected_nodes;

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
void free_lookup_table();

void usage(char *progname);
bool read_entire_file(entire_file *ef, char *filename);
void free_entire_file(entire_file ef);
TOKEN gettoken(lexer *l, sview *tokstr);

char *awastr(int8_t val, int bits);
void init_lookup_table(void);
AWATISM tism_from_sv(sview tstr);
char *str_from_tism(AWATISM tism);

bool print_stack_push(char *s);
char *print_stack_pop(void);
int print_stack_size(void);

char *convert_to_funnyspeak_if_possible(char *s);
char *sv_to_cstr(sview s);
void free_cstr_buf(void);

sdag_node *parse(lexer *l, parser *p);
bool generate_bytecode(sdag_node *node, FILE *outfile, expected_nodes en);
sdag_node *new_sdag_node();
void free_sdag_nodes();
sdag_node *lookup_iterator_name(parser *p, sview name);

bool cstr_sv_cmp(char *cstr, sview sv);
char *tokstr(TOKEN t);

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

	parser p = {.exptoks = TOK_TISM | TOK_REPEAT | TOK_REPEAT_END | TOK_SURF | TOK_OVER | TOK_PRINT | TOK_EOF};
	sdag_node *root = parse(&l, &p);

	FILE *tmp = tmpfile();
	if (tmp == NULL) goto temp_perr;

#define TEMP_PRINT(...) \
	do { \
		if (fprintf(tmp, __VA_ARGS__) < 0) goto temp_perr; \
	} while (0)

	TEMP_PRINT("Awa");

	expected_nodes en = { .tags = ST_TISM | ST_REPEAT | ST_SURF | ST_OVER | ST_MACRO | ST_PRINT };
	if (!generate_bytecode(root, tmp, en)) {
		fprintf(stderr, "Failed to generate awabytecode\n");
		exit(1);
	}
	free_sdag_nodes();

	if (0) {

	TOKEN tok;
	sview op, arg;

	// for REPEAT macro
	bool rep_flag = false; // currently in a repeat loop
	int rep_start = 0; // iterator start num
	int rep_stop = 0; // iterator stop num
	int rep_delta = 1; // change of rep_start at every iteration
	size_t rep_pos = 0; // saved cursor pos in file to jump back to
	size_t rep_linenum = 0; // saved linenum in file to jump back to

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
					} else if (tok == TOK_NUM && arg.len <= 4 &&
							(n = atoi(sv_to_cstr(arg))) >= -128 && n <= 127) {
						TEMP_PRINT("%s", awastr(n, 8));
					} else if (tok == TOK_ITERATOR) {
						if (!rep_flag) {
							fprintf(stderr, "%s:%zu: cannot use ITERATOR outside of REPEAT/END block\n",
									l.filename, l.linenum);
							exit(1);
						}
						size_t it = rep_start;
						if (it >= 128) {
							fprintf(stderr, "%s:%zu: ITERATOR too big for 'blo'. (expected: [0,127], got: %zu)\n",
									l.filename, l.linenum, it);
							exit(1);
						}
						TEMP_PRINT("%s", awastr((int)it, 8));
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
					if (tok == TOK_NUM && arg.len <= 3 &&
							(n = atoi(sv_to_cstr(arg))) >= 0 && n <= 31) {
						TEMP_PRINT("%s", awastr(n, 5));
					} else if (tok == TOK_ITERATOR) {
						if (!rep_flag) {
							fprintf(stderr, "%s:%zu: cannot use ITERATOR outside of REPEAT/END block\n",
									l.filename, l.linenum);
							exit(1);
						}
						size_t it = rep_start;
						if (it >= 32) {
							fprintf(stderr, "%s:%zu: ITERATOR too big for '%s'. (expected: [0,31], got: %zu)\n",
									l.filename, l.linenum, np->name, it);
							exit(1);
						}
						TEMP_PRINT("%s", awastr((int)it, 5));
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
			if (tok == TOK_NUM && arg.len <= 3 &&
					(n = atoi(sv_to_cstr(arg))) >= 0 && n <= 30) {
				// empty block because of refactor
			} else if (tok == TOK_ITERATOR) {
				if (!rep_flag) {
					fprintf(stderr, "%s:%zu: cannot use ITERATOR outside of REPEAT/END block\n",
							l.filename, l.linenum);
					exit(1);
				}
				size_t it = rep_start;
				if (it >= 31) {
					fprintf(stderr, "%s:%zu: ITERATOR too big for 'OVER'. (expected: [0,30], got: %zu)\n",
							l.filename, l.linenum, it);
					exit(1);
				}
				n = (int)it;
			} else {
				fprintf(stderr, "%s:%zu: expected 5 bit integer [0,30]\n",
						l.filename, l.linenum);
				exit(1);
			}

			for (int i = 0; i < n; i++)
				TEMP_PRINT("%s%s", lookup("sbm")->defn, awastr(n, 5)); // n times
			TEMP_PRINT("%s", lookup("dpl")->defn); // 1 time
			TEMP_PRINT("%s%s", lookup("sbm")->defn, awastr(n + 1, 5)); // 1 time
		} else if (tok == TOK_SURF) {
			// SURF 0 has no effect
			int n;
			tok = gettoken(&l, &arg);
			if (tok == TOK_NUM && arg.len <= 3 &&
					(n = atoi(sv_to_cstr(arg))) >= 0 && n <= 31) {
				// empty block because of refactor
			} else if (tok == TOK_ITERATOR) {
				if (!rep_flag) {
					fprintf(stderr, "%s:%zu: cannot use ITERATOR outside of REPEAT/END block\n",
							l.filename, l.linenum);
					exit(1);
				}
				size_t it = rep_start;
				if (it >= 32) {
					fprintf(stderr, "%s:%zu: ITERATOR too big for 'SURF'. (expected: [0,31], got: %zu)\n",
							l.filename, l.linenum, it);
					exit(1);
				}
				n = (int)it;
			} else {
				fprintf(stderr, "%s:%zu: expected 5 bit integer [0,31]\n",
						l.filename, l.linenum);
				exit(1);
			}

			for (int i = 0; i < n; i++)
				TEMP_PRINT("%s%s", lookup("sbm")->defn, awastr(n, 5)); // n times
		} else if (tok == TOK_REPEAT) {
			if (rep_flag) {
				fprintf(stderr, "%s:%zu: nested REPEAT/END blocks are not allowed\n",
						l.filename, l.linenum);
				exit(1);
			}
			rep_flag = true;
			int n;
			// start num
			tok = gettoken(&l, &arg);
			if (tok == TOK_NUM && arg.len <= 11 &&
					(n = atoi(sv_to_cstr(arg))) >= 0 && n <= INT32_MAX) {
				rep_start = n;
			} else {
				fprintf(stderr, "%s:%zu: expected positive 32 bit integer [0,%d] for beginning of REPEAT range\n",
						l.filename, l.linenum, INT32_MAX);
				exit(1);
			}
			// stop num
			tok = gettoken(&l, &arg);
			if (tok == TOK_NUM && arg.len <= 11 &&
					(n = atoi(sv_to_cstr(arg))) >= 0 && n <= INT32_MAX) {
				rep_stop= n;
				rep_pos = l.ef.cursor;
				rep_linenum = l.linenum;
			} else {
				fprintf(stderr, "%s:%zu: expected positive 32 bit integer [0,%d] for end of REPEAT range\n",
						l.filename, l.linenum, INT32_MAX);
				exit(1);
			}
			rep_delta = (rep_start < rep_stop) ? 1 : -1;
		} else if (tok == TOK_REPEAT_END) {
			if (!rep_flag) {
				fprintf(stderr, "%s:%zu: END must be preceeded by REPEAT\n",
						l.filename, l.linenum);
				exit(1);
			}
			if (rep_start != rep_stop) {
				rep_start += rep_delta;
				l.ef.cursor = rep_pos;
				l.linenum = rep_linenum;
			} else
				rep_flag = false;
		} else {
			fprintf(stderr, "%s:%zu: expected operator\n", l.filename, l.linenum);
			exit(1);
		}
	}

	// never found token END after token REPEAT
	if (rep_flag) {
		fprintf(stderr, "%s:%zu: expected END after REPEAT\n", l.filename, rep_linenum);
		exit(1);
	}
	} // ENDIF

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
	free_lookup_table();

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
			if (cstr_sv_cmp("PRINT", *tokstr))
				res = TOK_PRINT;
			else if (cstr_sv_cmp("OVER", *tokstr))
				res = TOK_OVER;
			else if (cstr_sv_cmp("SURF", *tokstr))
				res = TOK_SURF;
			else if (cstr_sv_cmp("REPEAT", *tokstr))
				res = TOK_REPEAT;
			else if (cstr_sv_cmp("END", *tokstr))
				res = TOK_REPEAT_END;
			else if (cstr_sv_cmp("ITERATOR", *tokstr))
				res = TOK_ITERATOR;
			else {
				l->tism = tism_from_sv(*tokstr);
				if (l->tism == AWAT_invalid)
					res = TOK_NAME;
				else
					res = TOK_TISM;
			}
		}
	}

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

void free_lookup_table()
{
	nlist *np, *prev;
	for (int i = 0; i < HASHSIZE; i++) {
		for (np = hashtab[i]; np != NULL;) {
			prev = np;
			np = np->next;
			free(prev->defn);
			free(prev->name);
			free(prev);
		}
	}
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

bool cstr_sv_cmp(char *cstr, sview sv)
{
	if (strlen(cstr) != sv.len)
		return false;
	return strncmp(cstr, sv.start, sv.len) == 0;
}

AWATISM tism_from_sv(sview tstr)
{
	if (cstr_sv_cmp("trm", tstr)) return AWAT_trm;
#define X(name) \
	else if (cstr_sv_cmp(#name, tstr)) return AWAT_ ## name;
	AWATISM_LIST
#undef X
	else return AWAT_invalid;
}

char *str_from_tism(AWATISM tism)
{
	char *result = NULL;
	switch (tism) {
	case AWAT_trm:
		result = "trm";
		break;
#define X(name) \
	case AWAT_ ## name: \
		result = #name; \
		break;
	AWATISM_LIST
#undef X
	case AWAT_invalid:
		assert(false);
		break;
	}
	return result;
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
	else if (strcmp("-", s) == 0) return "~";
	else if (strcmp("\\\"", s) == 0) return "\\'";
	else return s;
}

typedef struct {
	void *data;
	size_t pos;
	size_t capacity;
} node_arena;
static node_arena arena = {0};

sdag_node *new_sdag_node()
{
	if (arena.capacity == 0) {
		arena.capacity = 500*1024*1024;
		arena.data = calloc(1, arena.capacity);
	}
	assert(arena.capacity - arena.pos >= sizeof(sdag_node));
	sdag_node *result = (sdag_node *)((char *)arena.data + arena.pos);
	arena.pos += sizeof(sdag_node);
	return result;
}

void free_sdag_nodes()
{
	free(arena.data);
}

#define ERROR_PARSE(...) \
	do { \
		fprintf(stderr, "%s:%zu: ERROR: ", l->filename, l->linenum); \
		fprintf(stderr, __VA_ARGS__); \
		fprintf(stderr, "\n"); \
		exit(1); \
	} while(0)

char *tokstr(TOKEN t)
{
	switch (t) {
	case TOK_TISM:        return "awatism instruction";
	case TOK_AWASCII:     return "awascii char";
	case TOK_NUM:         return "numeric constant";
	case TOK_PRINT:       return "PRINT macro";
	case TOK_OVER:        return "OVER macro";
	case TOK_SURF:        return "SURF macro";
	case TOK_REPEAT:      return "REPEAT";
	case TOK_REPEAT_END:  return "END";
	case TOK_ITERATOR:    return "ITERATOR";
	case TOK_STRING:      return "string literal";
	case TOK_NAME:        return "name (loop index)";
	case TOK_EOF:         return "end of file";
	default:              return "INVALID STRING LITERAL (This should never be displayed)";
	}
}

sdag_node *parse(lexer *l, parser *p)
{
	static int scope_depth = 0;
	sdag_node *node = NULL;
	sdag_node **cur = &node;

	TOKEN tok;
	sview op;
	uint64_t exptoks = p->exptoks; // copy, since it's subject to change
	bool done = false;
	scope_depth += 1;
	while (!done) {
		tok = gettoken(l, &op);
		if (!(tok & exptoks)) {
			char exptokstr[1000] = {0};
			for (int i = 0; i < 20; i++) {
				if (1<<i & exptoks) {
					strcat(exptokstr, "\n    ");
					strcat(exptokstr, tokstr(1<<i));
				}
			}
			ERROR_PARSE("Got %s, but expected one of %s", tokstr(tok), exptokstr);
		}

		if ((*cur) != NULL)
			cur = &((*cur)->next);

		if (tok != TOK_EOF && tok != TOK_REPEAT_END && tok != TOK_ITERATOR) {
			(*cur) = new_sdag_node();
			(*cur)->filename = l->filename;
			(*cur)->linenum = l->linenum;
		}

		switch (tok) {
		case TOK_EOF:
			if (scope_depth != 1)
				ERROR_PARSE("Reached end of file unexpectedly (Did you forget to END a loop?)");
			done = true;
			break;
		case TOK_REPEAT_END:
			if (scope_depth <= 1)
				ERROR_PARSE("Reached END keyword while already in top scope");
			p->iterator = p->iterator->as.iterator.next_it;
			done = true; // Up a level
			break;
		case TOK_TISM:
			(*cur)->tag = ST_TISM;
			(*cur)->as.tism.tag = l->tism;
			(*cur)->as.tism.arg = NULL;
			switch (l->tism) {
				case AWAT_blo:
				case AWAT_sbm:
				case AWAT_srn:
				case AWAT_lbl:
				case AWAT_jmp:
					p->exptoks = TOK_AWASCII | TOK_NUM | TOK_ITERATOR | TOK_NAME;
					(*cur)->as.tism.arg = parse(l, p);
					break;
				default:
					break;
			}
			break;
		case TOK_AWASCII:
			(*cur)->tag = ST_AWASCII;
			nlist *np = lookup(sv_to_cstr(op));
			if (!np)
				ERROR_PARSE("Failed to look up character: '%.*s'", (int)op.len, op.start);
			(*cur)->as.awascii.awastr = np->defn;
			done = true; // Up a level
			break;
		case TOK_NUM:
			(*cur)->tag = ST_CONST;
			assert(op.len <= 8); //TODO proper number parsing
			(*cur)->as.constant.val = atoi(sv_to_cstr(op));
			done = true; // Up a level
			break;
		case TOK_OVER:
			(*cur)->tag = ST_OVER;
			p->exptoks = TOK_NUM | TOK_ITERATOR | TOK_NAME;
			(*cur)->as.over.arg = parse(l, p);
			break;
		case TOK_SURF:
			(*cur)->tag = ST_SURF;
			p->exptoks = TOK_NUM | TOK_ITERATOR | TOK_NAME;
			(*cur)->as.surf.arg = parse(l, p);
			break;
		case TOK_STRING:
			(*cur)->tag = ST_STRING;
			(*cur)->as.string.sv = op;
			done = true; // Up a level
			break;
		case TOK_PRINT:
			(*cur)->tag = ST_PRINT;
			p->exptoks = TOK_STRING;
			(*cur)->as.print.arg = parse(l, p);
			break;
		case TOK_REPEAT:{
			(*cur)->tag = ST_REPEAT;

			sdag_node *it = new_sdag_node();
			it->tag = ST_ITERATOR;
			p->exptoks = TOK_NUM | TOK_NAME;
			sdag_node *iterator_or_range_start = parse(l, p);

			sdag_node *it_or_NULL = NULL;
			bool isname = (iterator_or_range_start->tag == ST_NAME);
			if (isname)
				it_or_NULL = lookup_iterator_name(p, iterator_or_range_start->as.name.sv);
			bool isregistered = (it_or_NULL != NULL);

			if (isname && !isregistered) {
				it->as.iterator.name = iterator_or_range_start;
				it->as.iterator.next_it = p->iterator;
				p->iterator = it;
				(*cur)->as.repeat.iterator = it;

				p->exptoks = TOK_NUM | TOK_ITERATOR | TOK_NAME;
				(*cur)->as.repeat.range_start = parse(l, p);
			} else {
				sdag_node *default_it_name = new_sdag_node();
				default_it_name->tag = ST_NAME;
				char *din = "ITERATOR";
				default_it_name->as.name.sv = (sview){.start = din, .len = strlen(din)};
				if (lookup_iterator_name(p, default_it_name->as.name.sv))
					ERROR_PARSE("Cannot have two nested loops with unnamed loop indices");

				it->as.iterator.name = default_it_name;
				it->as.iterator.next_it = p->iterator;
				p->iterator = it;
				(*cur)->as.repeat.iterator = it;

				(*cur)->as.repeat.range_start = iterator_or_range_start;
			}

			p->exptoks = TOK_NUM | TOK_ITERATOR | TOK_NAME;
			(*cur)->as.repeat.range_stop = parse(l, p);

			p->exptoks = exptoks;
			(*cur)->as.repeat.body = parse(l, p);
			} break;
		case TOK_ITERATOR:{
			char *defaultname = "ITERATOR";
			sdag_node *it = lookup_iterator_name(p, (sview){.start = defaultname, .len = strlen(defaultname)});
			if (!it)
				ERROR_PARSE("ITERATOR can only be used inside of loops with anonymous loop index");
			(*cur) = it;
			done = true; // Up a level
			} break;
		case TOK_NAME:{
			sdag_node *name = lookup_iterator_name(p, op);
			if (name) {
				(*cur) = name; // just return the existing iterator
			} else {
				(*cur)->tag = ST_NAME;
				(*cur)->as.name.sv = op;
			}
			done = true;
			} break;
		}
	}
	scope_depth -= 1;

	return node;
}

#define PRINT_GEN(...) \
	do { \
		if (fprintf(outfile, __VA_ARGS__) < 0) goto gen_perr; \
	} while (0)

#define ERROR_GEN(...) \
	do { \
		fprintf(stderr, "%s:%zu: ERROR: ", node->filename, node->linenum); \
		fprintf(stderr, __VA_ARGS__); \
		fprintf(stderr, "\n"); \
		exit(1); \
	} while(0)

#define WARN_GEN(...) \
	do { \
		fprintf(stderr, "%s:%zu: WARNING: ", node->filename, node->linenum); \
		fprintf(stderr, __VA_ARGS__); \
		fprintf(stderr, "\n"); \
	} while(0)

bool generate_bytecode(sdag_node *node, FILE *outfile, expected_nodes en)
{
	if (node == NULL) {
		if (en.tags == 0) return true; // nothing was expected and we got nothing
		else {
			// Try not to go here
			assert(false);
		}
	}

	for (; node != NULL; node = node->next) {
		assert(node->tag & en.tags); // TODO error message (might be unreachable)
		switch (node->tag) {
		case ST_TISM: {
			nlist *np = lookup(str_from_tism(node->as.tism.tag));
			assert(np);
			PRINT_GEN("%s", np->defn);
			expected_nodes en_next = { .tags = ST_CONST | ST_AWASCII | ST_ITERATOR };
			switch (node->as.tism.tag) {
				case AWAT_blo:
					en_next.int_minval = -128;
					en_next.int_maxval = 127;
					en_next.int_awa_bits = 8;
					assert(node->as.tism.arg != NULL);
					break;
				case AWAT_sbm:
				case AWAT_srn:
				case AWAT_lbl:
				case AWAT_jmp:
					en_next.int_minval = 0;
					en_next.int_maxval = 31;
					en_next.int_awa_bits = 5;
					assert(node->as.tism.arg != NULL);
					break;
				default:
					en_next.tags = 0;
					assert(node->as.tism.arg == NULL);
					break;
			}
			// give the shared iterator filename and linenumber for error reporting
			if (node->as.tism.arg != NULL && node->as.tism.arg->tag == ST_ITERATOR) {
				node->as.tism.arg->linenum = node->linenum;
				node->as.tism.arg->filename = node->filename;
			}
			if (!generate_bytecode(node->as.tism.arg, outfile, en_next)) return false;
			} break;
		case ST_CONST:
			if (en.int_minval <= node->as.constant.val
					&& node->as.constant.val <= en.int_maxval) {
				PRINT_GEN("%s", awastr(node->as.constant.val, en.int_awa_bits));
			} else {
				ERROR_GEN("Expected value in [%d, %d], got: %d", en.int_minval, en.int_maxval, node->as.constant.val);
			}
			break;
		case ST_AWASCII:
			PRINT_GEN("%s", node->as.awascii.awastr);
			break;
		case ST_REPEAT: {
			sdag_node *start = node->as.repeat.range_start;
			int startval = 0;
			if (start->tag == ST_CONST)
				startval = start->as.constant.val;
			else if (start->tag == ST_ITERATOR)
				startval = start->as.iterator.val;
			else {
				// TODO error message (unreachable?)
				assert(false);
			}

			sdag_node *stop = node->as.repeat.range_stop;
			int stopval = 0;
			if (stop->tag == ST_CONST)
				stopval = stop->as.constant.val;
			else if (stop->tag == ST_ITERATOR)
				stopval = stop->as.iterator.val;
			else {
				// TODO error message (unreachable?)
				assert(false);
			}

			sdag_node *iterator = node->as.repeat.iterator;
			assert(iterator);
			assert(iterator->tag == ST_ITERATOR);

			int delta = (startval > stopval) ? -1 : 1;

			if (node->as.repeat.body != NULL) {
				for (int i = startval; i <= stopval*delta; i += delta) {
					iterator->as.iterator.val = i;
					if (!generate_bytecode(node->as.repeat.body, outfile, en)) return false;
				}
			} else {
				WARN_GEN("Empty loop body");
			}

			} break;
		case ST_ITERATOR:
			// just check iterator against expected and print
			int val = node->as.iterator.val;
			if (en.int_minval <= val && val <= en.int_maxval)
				PRINT_GEN("%s", awastr(val, en.int_awa_bits));
			else {
				ERROR_GEN("Expected value in [%d, %d], got: %d", en.int_minval, en.int_maxval, val);
			}
			break;
		case ST_SURF: {
			NODE_TAG tag = node->as.over.arg->tag;
			int n;
			if (tag == ST_CONST)
				n = node->as.over.arg->as.constant.val;
			else if (tag == ST_ITERATOR)
				n = node->as.over.arg->as.iterator.val;
			else {
				// TODO error message (unreachable?)
				assert(false);
			}

			if (0 <= n && n <= 31) {
				for (int i = 0; i < n; i++)
					PRINT_GEN("%s%s", lookup("sbm")->defn, awastr(n, 5)); // n times
			} else {
				ERROR_GEN("Expected value in [%d, %d], got: %d", 0, 31, n);
			}
			} break;
		case ST_OVER: {
			NODE_TAG tag = node->as.over.arg->tag;
			int n;
			if (tag == ST_CONST)
				n = node->as.over.arg->as.constant.val;
			else if (tag == ST_ITERATOR)
				n = node->as.over.arg->as.iterator.val;
			else {
				// TODO error message (unreachable?)
				assert(false);
			}
			if (0 <= n && n <= 30) {
				for (int i = 0; i < n; i++)
					PRINT_GEN("%s%s", lookup("sbm")->defn, awastr(n, 5)); // n times
				PRINT_GEN("%s", lookup("dpl")->defn); // 1 time
				PRINT_GEN("%s%s", lookup("sbm")->defn, awastr(n + 1, 5)); // 1 time
			} else {
				ERROR_GEN("Expected value in [%d, %d], got: %d", 0, 30, n);
			}
			} break;
		case ST_MACRO:
			assert(false && "macro not implemented");
			break;
		case ST_STRING: {
			sview arg = node->as.string.sv;
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
#define PRINTSTACK_FLUSH() \
						char *printme; \
						int size = print_stack_size(); \
						while ((printme = print_stack_pop()) != NULL) { \
							PRINT_GEN("%s", lookup("blo")->defn); \
							PRINT_GEN("%s", printme); \
						} \
						PRINT_GEN("%s", lookup("srn")->defn); \
						PRINT_GEN("%s", awastr(size, 5)); \
						PRINT_GEN("%s", lookup("prn")->defn);
						PRINTSTACK_FLUSH();
						print_stack_push(np->defn);
					}
				} else {
					fprintf(stderr, "%s:%zu: couldn't look up %.*s\n",
							node->filename, node->linenum, (int)v.len, v.start);
					exit(1);
				}
				arg.len -= len;
				arg.start += len;
			}
			PRINTSTACK_FLUSH();
			} break;
		case ST_PRINT: {
			expected_nodes en_next = { .tags = ST_STRING };
			if (!generate_bytecode(node->as.print.arg, outfile, en_next)) return false;
			} break;
		case ST_NAME:
			assert(false && "printing st_name not implemented");
			break;
		}
	}

	return true;
gen_perr:
	perror("TEMPFILE");
	return false;
}

sdag_node *lookup_iterator_name(parser *p, sview name)
{
	sdag_node *result = NULL;
	for (sdag_node *node = p->iterator; node != NULL; node = node->as.iterator.next_it) {
		assert(node->tag == ST_ITERATOR);
		sview sv1 = node->as.iterator.name->as.name.sv;
		sview sv2 = name;
		if (sv1.len == sv2.len && strncmp(sv1.start, sv2.start, sv2.len) == 0) {
			result = node;
			break;
		}
	}
	return result;
}

