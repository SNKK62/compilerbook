#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// tokenize.c

typedef enum
{
  TK_RESERVED, // symbol
  TK_IDENT,    // identifier
  TK_NUM,      // integer token
  TK_EOF,      // token for the end of input
  TK_INT,      // token for int
  TK_SIZEOF,   // token for sizeof
} TokenKind;

typedef struct Token Token;
struct Token
{
  TokenKind kind;
  Token *next;
  int val;
  char *str;
  int len;
};

typedef enum TypeKind {
  INT,
  PTR,
  ARRAY,
} TypeKind;
typedef struct Type Type;
struct Type {
  TypeKind ty;
  struct Type *ptr_to; // tyがPTRの場合のみ使う
  int array_size; // tyがARRAYの場合のみ使う
  int size;
  int depth;
};

typedef struct LVar LVar;
// ローカル変数の型
struct LVar {
  LVar *next; // 次の変数かNULL
  char *name; // 変数の名前
  int len;    // 名前の長さ
  int offset; // RBPからのオフセット
  Type *type; // 変数の型
};

typedef struct GVar GVar;
// グローバル変数の型
struct GVar {
  GVar *next; // 次の変数かNULL
  GVar *prev; // 前の変数かNULL
  char *name; // 変数の名前
  char label[10]; // 変数のラベル
  char *data; // 変数のデータ?
  int strlen;    // 名前の長さ
  int len; // データの長さ
  Type *type; // 変数の型
};

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
bool at_eof(Token *token);
Token *tokenize(char *p);

// parse.c

Type *new_type(TypeKind ty, Type *ptr_to);
bool consume(Token **token, char *op);
bool consume_ident(Token **token);
void expect(Token **token, char *op);
int expect_number(Token **token);
GVar *find_gvar(char *name, int len);
GVar *get_gvars();

typedef enum {
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_EQ, // ==
  ND_NE, // !=
  ND_LT, // <
  ND_LE, // <=
  ND_ADDR, // &
  ND_DEREF, // *
  ND_ASSIGN_DEREF, // 左辺のdereference
  ND_ASSIGN, // =
  ND_LVAR, // local variables
  ND_GVAR, // global variables
  ND_NUM, // integer
  ND_RETURN, // return
  ND_IF, // if
  ND_ELSE, // else
  ND_WHILE, // while
  ND_FOR, // for
  ND_BLOCK, // block
  ND_FUNC, // function
  ND_FUNC_DEF,      // definition of function
  ND_FUNC_DEF_END,  // end of function definition
  ND_LVAR_DEF, // definition of local variable
  ND_GVAR_DEF, // definition of global variable
} NodeKind;

typedef struct Node Node;
// 抽象構文木のノードの型
struct Node {
  NodeKind kind;
  Node *lhs;
  Node *rhs;
  int val; // kindがND_NUMの場合のみ使う
  int offset; // kindがND_LVARの場合のみ使う
  Type *type; // kindがND_LVAR, ND_DEREF, ND_ADD, ND_SUBの場合のみ使う
  char *funcName; // kindがND_FUNCの場合のみ使う
  char *name; // kindがND_GVARの場合のみ使う
  int len; // kindがND_GVARの場合のみ使う
  Node **argv; // kindがND_FUNCの場合のみ使う
  int argc; // kindがND_FUNCの場合のみ使う
  int stack_size; // kindがND_FUNCの場合のみ使う
};

Node **parse(Token *tok);

// gen.c

void gen(Node *node);

