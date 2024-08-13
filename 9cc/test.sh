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
      cc -o tmp tmp.s
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

assert 0 "main(){0;}"
# assert 42 "42;"
# assert 21 "5+20-4;"
# assert 41 " 12 + 34 - 5 ;"
# assert 47 "5+6*7;"
# assert 15 "5*(9-6);"
assert 4 "main(){(3+5) /2;}"
# assert 2 "-3+ 5;"
# assert 10 '- -10;'
# assert 10 '- - +10;'

assert 0 'main(){0==1;}'
# assert 1 '42==42;'
# assert 1 '0!=1;'
# assert 0 '42!=42;'
#
# assert 1 '0<1;'
# assert 0 '1<1;'
# assert 0 '2<1;'
# assert 1 '0<=1;'
# assert 1 '1<=1;'
assert 0 'main(){2<=1;}'

# assert 1 '1>0;'
# assert 0 '1>1;'
# assert 0 '1>2;'
# assert 1 '1>=0;'
# assert 1 '1>=1;'
# assert 0 '1>=2;'

assert 3 'main(){a = 1; b = 2; a + b;}'
# assert 6 'foo = 1; bar = 2 + 3; foo + bar;'
# assert 6 'foo = 1; bar = 2 + 3; return foo + bar;'
# assert 5 'foo = 1; bar = foo + 3; return foo + bar;'
#
# assert 1 'if (1 < 2) return 1;'
# assert 1 'if (1 < 2) return 1; else return 0;'
assert 0 'main(){if (1 > 2) return 1; else return 0;}'

assert 3 'main(){i = 0; while (i < 3) i = i + 1; return i;}'

# assert 3 'b = 0; for(i = 0; i < 3; i = i + 1) b = b + 1; return b;'
# assert 3 'b = 0; i = 0; for(; i < 3; i = i + 1) b = b + 1; return b;'
assert 3 'main(){b = 0; for(; b < 3; ) b = b + 1; return b;}'

# assert 2 'a = 1; if (a < 2) {return 2;} else {return 3;}'
# assert 2 'a = 1; if (a < 2) {} return a+1;'
assert 1 'main(){a = 1; if (a < 2) {if (a > 2) {return 1;} else {return a;}} else {return 3;}}'

assert 0 'main(){foo();}' "true"
# assert 0 'a = 1; if (a < 2) {foo();}' "true"
# assert 0 'a = 1; if (a > 2) {return 0;} else {foo();}' "true"

assert 0 'main(){bar(1, 2, 3);}' "true"
# assert 0 'hoge(1, 2);' "true"

# TODO: 未実装
# assert 10 'main(){a = 1; sum(1, 4) * 2;} sum(a, b) {return a + b;}'

echo OK

