#include "9cc.h"

void gen_lval(Node *node) {
  if (node->kind != ND_LVAR)
    error("代入の左辺値が変数ではありません");

  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->offset);
  printf("  push rax\n");
}

int gen_label() {
  int static id = 0;
  return id++;
}

void gen(Node *node) {
  switch(node->kind) {
    case ND_NUM:
      printf("  push %d\n", node->val);
      return;
    case ND_LVAR:
      gen_lval(node);
      printf("  pop rax\n");
      printf("  mov rax, [rax]\n");
      printf("  push rax\n");
      return;
    case ND_FUNC:
      printf("  call %s\n", node->funcName);
      return;
    case ND_ASSIGN:
      gen_lval(node->lhs);
      gen(node->rhs);

      printf("  pop rdi\n");
      printf("  pop rax\n");
      printf("  mov [rax], rdi\n");
      printf("  push rdi\n");
      return;
    case ND_BLOCK:
      while (node) {
        if (node->lhs) {
          gen(node->lhs);
          if (node->rhs) {
            printf("  pop rax\n");
          }
        }
        node = node->rhs;
      }
      return;
    case ND_RETURN:
      gen(node->lhs);

      printf("  pop rax\n");
      printf("  mov rsp, rbp\n");
      printf("  pop rbp\n");
      printf("  ret\n");
      return;
    case ND_IF:
      gen(node->lhs);

      printf("  pop rax\n");
      printf("  cmp rax, 0\n");

      if (node->rhs->kind == ND_ELSE) {
        int else_id = gen_label();
        printf("  je  .Lelse%d\n", else_id);
        gen(node->rhs->lhs);
        int end_id = gen_label();
        printf("  jmp  .Lend%d\n", end_id);
        printf(".Lelse%d:\n", else_id);
        gen(node->rhs->rhs);
        printf(".Lend%d:\n", end_id);
        return;
      }

      int end_id = gen_label();
      printf("  je  .Lend%d\n", end_id);
      gen(node->rhs);
      printf(".Lend%d:\n", end_id);
      return;
    case ND_WHILE:
      {
        int begin_id = gen_label();
        printf(".Lbegin%d:\n", begin_id);
        gen(node->lhs);

        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        int end_id = gen_label();
        printf("  je  .Lend%d\n", end_id);
        gen(node->rhs);

        printf("  jmp .Lbegin%d\n", begin_id);
        printf(".Lend%d:\n", end_id);
        return;
      }
    case ND_FOR:
      {
        int begin_id = gen_label();
        int end_id = gen_label();
        if (node->lhs) gen(node->lhs);
        printf(".Lbegin%d:\n", begin_id);
        if (node->rhs->lhs) gen(node->rhs->lhs);

        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je  .Lend%d\n", end_id);

        gen(node->rhs->rhs->rhs);

        if (node->rhs->rhs->lhs) gen(node->rhs->rhs->lhs);

        printf("  jmp .Lbegin%d\n", begin_id);
        printf(".Lend%d:\n", end_id);
        return;
      }
    }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind) {
    case ND_ADD:
      printf("  add rax, rdi\n");
      break;
    case ND_SUB:
      printf("  sub rax, rdi\n");
      break;
    case ND_MUL:
      printf("  imul rax, rdi\n");
      break;
    case ND_DIV:
      printf("  cqo\n");
      printf("  idiv rdi\n");
      break;
    case ND_EQ:
      printf("  cmp rax, rdi\n");
      printf("  sete al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_NE:
      printf("  cmp rax, rdi\n");
      printf("  setne al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_LT:
      printf("  cmp rax, rdi\n");
      printf("  setl al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_LE:
      printf("  cmp rax, rdi\n");
      printf("  setle al\n");
      printf("  movzb rax, al\n");
      break;
  }
  printf("  push rax\n");
}

