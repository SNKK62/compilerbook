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

Node *expr(Token **tokenp);
Node *equality(Token **tokenp);
Node *relational(Token **tokenp);
Node *add(Token **tokenp);
Node *mul(Token **tokenp);
Node *primary(Token **tokenp);
Node *unary(Token **tokenp);

// expr = equality
Node *expr(Token **tokenp) {
  return equality(tokenp);
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

// unary = ("+" | "-")? unary
Node *unary(Token **tokenp) {
  if (consume(tokenp, "+")) {
    return unary(tokenp);
  }
  if (consume(tokenp, "-")) {
    return new_node(ND_SUB, new_node_num(0), unary(tokenp));
  }
  return primary(tokenp);
}

// primary = num | "(" expr ")"
Node *primary(Token **tokenp) {
  // 次のトークンが"("なら，"(" expr ")"のはず
  if (consume(tokenp, "(")) {
    Node *node = expr(tokenp);
    expect(tokenp, ")");
    return node;
  }
  // そうでなければ数値のはず
  return new_node_num(expect_number(tokenp));
}

Node *parse(Token *tok) {
  Token **tokenp = &tok;
  Node *node = expr(tokenp);
  if (!at_eof(*tokenp))
    error_at((*tokenp)->str, "expected end of input");
  return node;
}

