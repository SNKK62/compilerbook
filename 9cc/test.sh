#!/bin/bash
assert() {
    expected="$1"
    input="$2"
    func="$3"

    ./9cc "$input" > tmp.s

    # funcがある場合、mock用のfuncをリンクする
    if [ -n "$3" ]; then
      # make -f でsubdirのMakefileを指定したかったんだけど
      # カレントディレクトリと衝突？する
      # なので、ディレクトリを移動
      cd ./lib/mock
      make
      cd ../../
      # リンク
      cc -o tmp tmp.s ./lib/mock/mock.o
        ./tmp
    else
      cc -static -o tmp tmp.s
        ./tmp
        actual="$?"

        if [ "$actual" = "$expected" ]; then
          echo "$input => $actual"
        else
          echo "$iput => $expected expected, but got $actual"
          exit 1
    fi

    fi
}

assert 0 "int main(){0;}"
# assert 42 "42;"
# assert 21 "5+20-4;"
# assert 41 " 12 + 34 - 5 ;"
# assert 47 "5+6*7;"
# assert 15 "5*(9-6);"
assert 4 "int main(){(3+5) /2;}"
# assert 2 "-3+ 5;"
# assert 10 '- -10;'
# assert 10 '- - +10;'

assert 0 'int main(){0==1;}'
# assert 1 '42==42;'
# assert 1 '0!=1;'
# assert 0 '42!=42;'
#
# assert 1 '0<1;'
# assert 0 '1<1;'
# assert 0 '2<1;'
# assert 1 '0<=1;'
# assert 1 '1<=1;'
assert 0 'int main(){2<=1;}'

# assert 1 '1>0;'
# assert 0 '1>1;'
# assert 0 '1>2;'
# assert 1 '1>=0;'
# assert 1 '1>=1;'
# assert 0 '1>=2;'

assert 3 'int main(){int a, b; a = 1; b = 2; a + b;}'
# assert 6 'foo = 1; bar = 2 + 3; foo + bar;'
# assert 6 'foo = 1; bar = 2 + 3; return foo + bar;'
# assert 5 'foo = 1; bar = foo + 3; return foo + bar;'
#
# assert 1 'if (1 < 2) return 1;'
# assert 1 'if (1 < 2) return 1; else return 0;'
assert 0 'int main(){if (1 > 2) return 1; else return 0;}'

assert 3 'int main(){int i; i = 0; while (i < 3) i = i + 1; return i;}'

# assert 3 'b = 0; for(i = 0; i < 3; i = i + 1) b = b + 1; return b;'
# assert 3 'b = 0; i = 0; for(; i < 3; i = i + 1) b = b + 1; return b;'
assert 3 'int main(){int b; b = 0; for(; b < 3; ) b = b + 1; return b;}'

# assert 2 'a = 1; if (a < 2) {return 2;} else {return 3;}'
# assert 2 'a = 1; if (a < 2) {} return a+1;'
assert 1 'int main(){int a; a = 1; if (a < 2) {if (a > 2) {return 1;} else {return a;}} else {return 3;}}'

assert 0 'int main(){foo();}' "true"
# assert 0 'a = 1; if (a < 2) {foo();}' "true"
# assert 0 'a = 1; if (a > 2) {return 0;} else {foo();}' "true"

assert 0 'int main(){bar(1, 2, 3);}' "true"
# assert 0 'hoge(1, 2);' "true"

assert 4 'int main(){bar() * 2;} int bar() {return 2;}'
assert 10 'int main(){ int a; a = 1; sum(4, a) * 2;} int sum(int a, int b) {return a + b;}'
assert 13 'int main() {return fibonacci(7);} int fibonacci(int n){ if(n == 1) return 1; if(n == 2) return 1; return fibonacci(n - 1) + fibonacci(n - 2);}'

assert 3 'int main() {int x, *y; x = 3; y = &x; return *y;}'

assert 5 'int main() {int x, *y; y = &x; *y = 5; return x;}'
assert 10 'int main() {int x, *y, **z; y = &x; z = &y; **z = 5; return x + *y;}'

assert 4 'int main() {sizeof 1;}'
assert 4 'int main() {sizeof(1);}'
assert 4 'int main() {int x; sizeof x;}'
assert 4 'int main() {int x; sizeof(x);}'
assert 4 'int main() {int x; sizeof(x+3);}'
assert 8 'int main() {int *x; sizeof(x);}'
assert 8 'int main() {int *x; sizeof(-x);}'
assert 8 'int main() {int *x; sizeof(++x);}'
assert 8 'int main() {int *x, y; y = 2; x = &y; sizeof(x+3);}'
assert 4 'int main() {int *x; int y; y = 2; x = &y; sizeof(*x);}'

assert 0 'int main() {int x[5]; return 0;}'

assert 1 'int main() {int x[5]; *x = 1; return *x;}'
assert 3 'int main() {int a[2]; *a = 1; *(a + 1) = 2; int *p; p = a; return *p + *(p + 1);}'
assert 5 'int main() {int a[2]; a[0] = 3; a[1] = 2; return a[0] + a[1];}'

assert 5 'int a; int main() {a = 5; return a;}'
assert 3 'int a, b; int main() {a = 1; b = 2; return a + b;}'
assert 3 'int a[2]; int main() {a[0] = 1; a[1] = 2; return a[0] + a[1];}'
assert 2 'int a; int main(){a = 1; int a; a = 2; return a;}'

assert 3 'int main() {char a; a = 3; return a;}'
assert 3 'int main() {char x[3]; x[0] = -1; x[1] = 2; int y; y = 4; return x[0] + y;}'

assert 97 'int main() {char *p; p = "abc"; return p[0];}'
assert 98 'int main() {char *p; p = "abc"; return p[1];}'
assert 99 'int main() {char *p; p = "abc"; return p[2];}'

echo OK

