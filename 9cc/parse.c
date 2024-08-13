#include "9cc.h"

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}


Node *new_node_num(int val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進めて
// 真を返す。それ以外の場合には偽を返す。
bool consume(Token **token, char *op)
{
  if ((*token)->kind != TK_RESERVED || strlen(op) != (*token)->len || memcmp((*token)->str, op, (*token)->len))
    return false;
  *token = (*token)->next;
  return true;
}

bool consume_ident(Token **token) {
  if ((*token)->kind == TK_IDENT) {
    *token = (*token)->next;
    return true;
  }
  return false;
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進める。
// それ以外の場合にはエラーを報告する。
void expect(Token **token, char *op)
{
  if ((*token)->kind != TK_RESERVED || strlen(op) != (*token)->len || memcmp((*token)->str, op, (*token)->len))
    error_at((*token)->str, "expected \"%s\"", op);
  *token = (*token)->next;
}

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
// それ以外の場合にはエラーを報告する。
int expect_number(Token **token)
{
  if ((*token)->kind != TK_NUM)
    error("数ではありません");
  int val = (*token)->val;
  *token = (*token)->next;
  return val;
}

bool at_eof(Token *token)
{
  return token->kind == TK_EOF;
}

void parse_argv(Token **tokenp, Node *node) {
  if (!consume(tokenp, ")")) {
    int i = 0;
    node->argv = calloc(6, sizeof(int));
    while(true) {
      Token *token = *tokenp;
      if(token->kind == TK_NUM) {
        node->argv[i] = token->val;
        i++;
        *tokenp = token->next;
      }
      if (consume(tokenp, ")")) {
        break;
      }
      expect(tokenp, ",");
    }
    node->argc = i;
  }
}

bool is_top_level = true;

Node *expect_func_definition(Token **tokenp)
{
  Token *tok = *tokenp;
  if(!consume_ident(tokenp)) error_at((*tokenp)->str, "トップレベルは関数しか書けません");

  expect(tokenp, "(");

  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_FUNC_DEF;
  node->funcName = calloc(1, tok->len);
  strncpy(node->funcName, tok->str, tok->len);

  // 引数をパース )も読み飛ばしてる
  parse_argv(tokenp, node);

  expect(tokenp, "{");

  return node;
}

LVar *locals = NULL;

// 変数を名前で検索する。見つからなかった場合はNULLを返す。
LVar *find_lvar(Token *tok) {
  for (LVar *var = locals; var; var = var->next) {
    if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
      return var;
  }
  return NULL;
}

void program(Token **tokenp);
Node *stmt(Token **tokenp);
Node *expr(Token **tokenp);
Node *assign(Token **tokenp);
Node *equality(Token **tokenp);
Node *relational(Token **tokenp);
Node *add(Token **tokenp);
Node *mul(Token **tokenp);
Node *unary(Token **tokenp);
Node *primary(Token **tokenp);

Node *code[100];

// program = stmt*
void program(Token **tokenp) {
  int i = 0;
  while (!at_eof(*tokenp)) {
    code[i++] = stmt(tokenp);
  }
  code[i] = NULL;
}

// stmt = expr ";" 
// | "{" stmt* "}"
// | "return" expr ";"
// | "if" "(" expr ")" stmt ("else" stmt)?
// | "while" "(" expr ")" stmt
// | "for" "(" expr? ";" expr? ";" expr? ")" stmt
Node *stmt(Token **tokenp) {
  Node *node;
  if(is_top_level) {
    node = expect_func_definition(tokenp);
    is_top_level = false;
    return node;
  }

  if (consume(tokenp, "{")) {
    node = calloc(1, sizeof(Node));

    Node *cur = calloc(1, sizeof(Node));
    node = cur;
    cur->kind = ND_BLOCK;
    cur->lhs = NULL;
    while (!consume(tokenp, "}")) {
      cur->lhs = stmt(tokenp);
      Node *next = calloc(1, sizeof(Node));
      next->kind = ND_BLOCK;
      next->lhs = NULL;
      cur->rhs = next;
      cur = next;
    }
    cur->rhs = NULL;
    return node;
  }

  if (consume(tokenp, "return")) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_RETURN;
    node->lhs = expr(tokenp);
    expect(tokenp, ";");
    return node;
  }

  if (consume(tokenp, "if")) {
    expect(tokenp, "(");
    Node *lhs = expr(tokenp);
    expect(tokenp, ")");
    Node *rhs = stmt(tokenp);
    if (consume(tokenp, "else")) {
      rhs = new_node(ND_ELSE, rhs, stmt(tokenp));
    }
    node = new_node(ND_IF, lhs, rhs);
    return node;
  } 

  if (consume(tokenp, "while")) {
    expect(tokenp, "(");
    Node *lhs = expr(tokenp);
    expect(tokenp, ")");
    Node *rhs = stmt(tokenp);
    node = new_node(ND_WHILE, lhs, rhs);
    return node;
  }

  if (consume(tokenp, "for")) {
    expect(tokenp, "(");
    Node *lhs = NULL;
    Node *rhs = NULL;
    Node *ternary= NULL;
    if (!consume(tokenp, ";")) {
      lhs = expr(tokenp);
      expect(tokenp, ";");
    }
    if (!consume(tokenp, ";")) {
      rhs = expr(tokenp);
      expect(tokenp, ";");
    }
    if (!consume(tokenp, ")")) {
      ternary = expr(tokenp);
      expect(tokenp, ")");
    }
    node = new_node(ND_FOR, lhs, new_node(ND_FOR, rhs, new_node(ND_FOR, ternary, stmt(tokenp))));
    return node;
  }

  if(consume(tokenp, "}")) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_FUNC_DEF_END;
    is_top_level = true;
    return node;
  }

  node = expr(tokenp);
  expect(tokenp, ";");
  return node;
}

// expr = assign
Node *expr(Token **tokenp) {
  return assign(tokenp);
}

// assign = equality ("=" equality)?
Node *assign(Token **tokenp) {
  Node *node = equality(tokenp);
  if (consume(tokenp, "="))
    node = new_node(ND_ASSIGN, node, equality(tokenp));
  return node;
}

// equality = relatinal ("==" relational | "!=" relational)*
Node *equality(Token **tokenp) {
  Node *node = relational(tokenp);

  for (;;) {
    if (consume(tokenp, "==")) {
      node = new_node(ND_EQ, node, relational(tokenp));
      continue;
    }
    if (consume(tokenp, "!=")) {
      node = new_node(ND_NE, node, relational(tokenp));
      continue;
    }

    return node;
  }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *relational(Token **tokenp) {
  Node *node = add(tokenp);
  for (;;) {
    if (consume(tokenp, "<")) {
      node = new_node(ND_LT, node, add(tokenp));
      continue;
    }
    if (consume(tokenp, "<=")) {
      node = new_node(ND_LE, node, add(tokenp));
      continue;
    }
    if (consume(tokenp, ">")) {
      node = new_node(ND_LT, add(tokenp), node);
      continue;
    }
    if (consume(tokenp, ">=")) {
      node = new_node(ND_LE, add(tokenp), node);
      continue;
    }

    return node;
  }
}

// add = mul ("+" mul | "-" mul)*
Node *add(Token **tokenp) {
  Node *node = mul(tokenp);

  for (;;) {
    if (consume(tokenp, "+")) {
      node = new_node(ND_ADD, node, mul(tokenp));
      continue;
    }
    if (consume(tokenp, "-")) {
      node = new_node(ND_SUB, node, mul(tokenp));
      continue;
    }

    return node;
  }
}

// mul = unary ("*" unary | "/" unary)*
Node *mul(Token **tokenp) {
  Node *node = unary(tokenp);

  for (;;) {
    if (consume(tokenp, "*")) {
      node = new_node(ND_MUL, node, unary(tokenp));
      continue;
    }
    if (consume(tokenp, "/")) {
      node = new_node(ND_DIV, node, unary(tokenp));
      continue;
    }

    return node;
  }
}

// unary = ("+" | "-")? unary | pimary
Node *unary(Token **tokenp) {
  if (consume(tokenp, "+")) {
    return unary(tokenp);
  }
  if (consume(tokenp, "-")) {
    return new_node(ND_SUB, new_node_num(0), unary(tokenp));
  }
  return primary(tokenp);
}

// primary = num | ident ("(" (exp (, expr)*)? ")")? | "(" expr ")" 
Node *primary(Token **tokenp) {
  // 次のトークンが"("なら，"(" expr ")"のはず
  if (consume(tokenp, "(")) {
    Node *node = expr(tokenp);
    expect(tokenp, ")");
    return node;
  }

  Token *tok = *tokenp;
  if (consume_ident(tokenp)) {
    if (consume(tokenp, "(")) {
      Node *node = calloc(1, sizeof(Node));
      node->kind = ND_FUNC;
      node->funcName = calloc(1, tok->len);
      strncpy(node->funcName, tok->str, tok->len);;

      parse_argv(tokenp, node);

      return node;
    }


    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LVAR;

    LVar *lvar = find_lvar(tok);
    if (lvar) {
      node->offset = lvar->offset;
    } else {
      lvar = calloc(1, sizeof(LVar));
      lvar->next = locals;
      lvar->name = tok->str;
      lvar->len = tok->len;
      if (locals) {
        lvar->offset = locals->offset + 8;
      } else {
        lvar->offset = 8;
      }
      node->offset = lvar->offset;
      locals = lvar;
    }
    return node;
  }



  // そうでなければ数値のはず
  return new_node_num(expect_number(tokenp));
}

Node **parse(Token *tok) {
  Token **tokenp = &tok;
  program(tokenp);
  if (!at_eof(*tokenp))
    error_at((*tokenp)->str, "expected end of input");
  return code;
}

