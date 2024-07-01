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

Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *primary();
Node *unary();

// expr = equality
Node *expr(Token **rest, Token *tok) {
  return equality(rest, tok);
}

// equality = relatinal ("==" relational | "!=" relational)*
Node *equality(Token **rest, Token *tok) {
  Node *node = relational(rest, tok);

  for (;;) {
    if (consume(*rest, "==")) {
      *rest = (*rest)->next;
      node = new_node(ND_EQ, node, relational(rest, *rest));
      continue;
    }
    if (consume(*rest, "!=")) {
      *rest = (*rest)->next;
      node = new_node(ND_NE, node, relational(rest, *rest));
      continue;
    }

    return node;
  }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *relational(Token **rest, Token *tok) {
  Node *node = add(rest, tok);
  for (;;) {
    if (consume(*rest, "<")) {
      *rest = (*rest)->next;
      node = new_node(ND_LT, node, add(rest, *rest));
      continue;
    }
    if (consume(*rest, "<=")) {
      *rest = (*rest)->next;
      node = new_node(ND_LE, node, add(rest, *rest));
      continue;
    }
    if (consume(*rest, ">")) {
      *rest = (*rest)->next;
      node = new_node(ND_LT, add(rest, *rest), node);
      continue;
    }
    if (consume(*rest, ">=")) {
      *rest = (*rest)->next;
      node = new_node(ND_LE, add(rest, *rest), node);
      continue;
    }

    return node;
  }
}

// add = mul ("+" mul | "-" mul)*
Node *add(Token **rest, Token *tok) {
  Node *node = mul(rest, tok);

  for (;;) {
    if (consume(*rest, "+")) {
      *rest = (*rest)->next;
      node = new_node(ND_ADD, node, mul(rest, *rest));
      continue;
    }
    if (consume(*rest, "-")) {
      *rest = (*rest)->next;
      node = new_node(ND_SUB, node, mul(rest, *rest));
      continue;
    }

    return node;
  }
}

// mul = unary ("*" unary | "/" unary)*
Node *mul(Token **rest, Token *tok) {
  Node *node = unary(rest, tok);

  for (;;) {
    if (consume(*rest, "*")) {
      *rest = (*rest)->next;
      node = new_node(ND_MUL, node, unary(rest, *rest));
      continue;
    }
    if (consume(*rest, "/")) {
      *rest = (*rest)->next;
      node = new_node(ND_DIV, node, unary(rest, *rest));
      continue;
    }

    return node;
  }
}

// unary = ("+" | "-")? unary
Node *unary(Token **rest, Token *tok) {
  if (consume(tok, "+")) {
    *rest = tok->next;
    return unary(rest, *rest);
  }
  if (consume(tok, "-")) {
    *rest = tok->next;
    return new_node(ND_SUB, new_node_num(0), unary(rest, *rest));
  }
  return primary(rest, tok);
}

// primary = num | "(" expr ")"
Node *primary(Token **rest, Token *tok) {
  *rest = tok->next;
  // 次のトークンが"("なら，"(" expr ")"のはず
  if (consume(tok, "(")) {
    Node *node = expr(rest, *rest);
    expect(*rest, ")");
    *rest = (*rest)->next;
    return node;
  }
  // そうでなければ数値のはず
  return new_node_num(expect_number(tok));
}

Node *parse(Token *tok) {
  Token **p = &tok;
  Node *node = expr(p, tok);
  if (!at_eof(*p))
    error_at((*p)->str, "expected end of input");
  return node;
}

