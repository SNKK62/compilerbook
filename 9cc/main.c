#include "9cc.h"

static char *escape(char *s, int len) {
  char *buf = malloc(len * 4);
  char *p = buf;
  for (int i = 0; i < len; i++) {
    if (s[i] == '\\') {
      *p++ = '\\';
      *p++ = '\\';
    } else if (isgraph(s[i]) || s[i] == ' ') {
      *p++ = s[i];
    } else {
      sprintf(p, "\\%03o", s[i]);
      p += 4;
    }
  }
  *p = '\0';
  return buf;
}

int main(int argc, char **argv)
{
  if (argc != 2)
  {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  // トークナイズしてパースする
  Token *token = tokenize(argv[1]);
  Node **code = parse(token);

  // アセンブリの前半部分の出力
  printf(".intel_syntax noprefix\n");

  printf(".data\n");
  GVar *globals = get_gvars();
  for (GVar *var = globals; var; var = var->next) {
    printf("%s:\n", var->label);
    printf("  .ascii \"%s\"\n", escape(var->data, var->len));
  }
  printf(".text\n");

  printf(".global main\n");

  // コード生成
  for (int i = 0; code[i]; i++)
  {
    gen(code[i]);
  }

  return 0;
}
