#include "9cc.h"

// 関数に引数を渡す際の第1引数から第6までは、どのレジスタにセットするかはきまってるので、配列で保持しておく。
char map_func_argv_register[6][4] = {
  "rdi",
  "rsi",
  "rdx",
  "rcx",
  "r8",
  "r9",
};

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
      {
        //int argc = sizeof(node->argv) / sizeof(int);
        //for(int i = 0; i < argc; i++) {
        //  printf("  push %d\n", node->argv[i]);
        //}

        // 関数呼び出しの際はRSPの値が16の倍数になっていることを前提としている関数がある
        // なので、RSPの値が16の倍数ではない場合、調整する
        int rsp_label_id = gen_label();
        printf("  mov rax, rsp\n");
        printf("  mov r10, 16\n");
        printf("  cqo\n");
        printf("  div r10\n");
        printf("  cmp rdx, 0\n");
        printf("  je .Lrsp%d\n", rsp_label_id);
        printf("  sub rsp, 8\n");

         // 関数呼び出しから戻ったときに、rspが調整されているかどうかを判別するために使う
        printf("  mov r11, 1\n");
        printf(".Lrsp%d:\n", rsp_label_id);

        // 第一引数はrdiレジスタ、、、のように決まってるみたい
        for(int i = 0; i < node->argc; i++) {
          gen(node->argv[i]);
          printf("  pop rax\n");
          printf("  mov %s, rax\n", map_func_argv_register[i]);
        }

        printf("  call %s\n", node->funcName);

        // rspが調整されている場合、元に戻す
        int rsp_restore_label_id = gen_label();
        printf("  cmp r11, 0\n");
        printf("  je .LrspRestore%d\n", rsp_restore_label_id);
        printf("  mov r11, 0\n");
        printf("  add rsp, 8\n");
        printf(".LrspRestore%d:\n", rsp_restore_label_id);

        // 関数を呼び出した結果、raxに関数の結果が残っている
        // それをスタックに残す
        printf("  push rax\n");

        return;
      }
    case ND_FUNC_DEF:
      {
        printf("%s:\n", node->funcName);

        // プロローグ処理
        printf("  push rbp\n");
        printf("  mov rbp, rsp\n");
        // 変数26個分の領域を確保する
        printf("  sub rsp, 208\n");

        // 第一引数はrdiレジスタ、、、のように決まってるみたい
        for(int i = 0; i < node->argc; i++) {
          gen_lval(node->argv[i]);
          printf("  pop rax\n");
          printf("  mov [rax], %s\n", map_func_argv_register[i]);
        }

        return;
      }
    case ND_FUNC_DEF_END:
      {
        // ここのアセンブリが実行されるパターンは、関数でreturnしていないとき
        // returnしていないときはNULLを返したほうがいいんだろうけど、
        // ひとまず直前の式の結果がスタックトップにあると思うので、それを返すようにする
        printf("  pop rax\n");

        // エピローグ
        printf("  mov rsp, rbp\n");
        printf("  pop rbp\n");
        printf("  ret\n");

        return;
      }
    case ND_ASSIGN:
      gen_lval(node->lhs);
      gen(node->rhs);

      printf("  pop rdi\n");
      printf("  pop rax\n");
      printf("  mov [rax], rdi\n");
      /* printf("  push rdi\n"); */
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

      // スタックトップに式全体の値が残っているはずなので
      // それをRAXにロードして関数からの返り値とする
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

