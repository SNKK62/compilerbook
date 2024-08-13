#include "9cc.h"

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
  printf(".globl main\n");

  // コード生成
  for (int i = 0; code[i]; i++)
  {
    gen(code[i]);
  }

  return 0;
}
