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

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
bool consume(Token **token, char *op);
bool consume_ident(Token **token);
void expect(Token **token, char *op);
int expect_number(Token **token);
bool at_eof(Token *token);
Token *tokenize(char *p);

// parse.c

typedef enum {
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_EQ, // ==
  ND_NE, // !=
  ND_LT, // <
  ND_LE, // <=
  ND_ASSIGN, // =
  ND_LVAR, // local variables
  ND_NUM, // integer
} NodeKind;

typedef struct Node Node;
// 抽象構文木のノードの型
struct Node {
  NodeKind kind;
  Node *lhs;
  Node *rhs;
  int val;
  int offset; // kindがND_LVARの場合のみ使う
};

Node **parse(Token *tok);

// gen.c

void gen(Node *node);

