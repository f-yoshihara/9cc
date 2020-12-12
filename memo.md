# 開発memo

## 20201010

### 最小実装

- アセンプリを出力するcのコード(9cc.c)を書く
- 9cc.cをgccでコンパイルする
- 9ccを実行する
- アセンプリファイル(tmp.s)が生成される
- tmp.sをアセンブルする
- tmpを実行する

### make

https://www.usptomo.com/PAGE=20120228GCC

 sudo yum install glibc-static

## 20201011

### ポインタ

ポインタとは変数などの値が格納されているアドレスを値とする変数のこと。

下記で二つの*が使用されているが、これら二つの意味は異なる。
宣言や初期化の際に使われる*はそれがポインタであることを示すためのもの。
参照の時に使用される*はそのポインタが指し示す値を意味する。

```c
char *p;
b = *p;
```

### token

> a piece of paper with a particular amount of money printed on it that can be exchanged in a shop for goods of that value:

[cambridge dictionary](https://dictionary.cambridge.org/ja/dictionary/english/token)

### 再帰下降構文解析

```ebnf
expr    = mul ("+" mul | "-" mul)*
mul     = primary ("*" primary | "/" primary)*
primary = num | "(" expr ")"
```

### スタックマシン

x86-64はスタックマシンではなくレジスタマシン

レジスタマシンでスタックマシンをエミュレートするには

スタックの先頭を指すレジスタ（スタックポインタ）を用意する

x86-64のpushやpopは暗黙的にRSPが指し示すメモリのアドレスにアクセスしつつアドレスの更新を行う。

pushであればRSPのアドレスをデクリメントして更新しそのアドレスに値を格納する。

popであればRSPのアドレスから値を取得してアドレスをデクリメントして更新する。

```c
struct Node
{
    NodeKind kind; // ノードの型
    Node *lhs;     // 左辺
    Node *rhs;     // 右辺
    int val;       // 値（kindがND_NUMの場合のみ使用）
};
```

```s
// 2*3
push 2
push 3
pop rdi
pop rax
mul rax, rdi
push rax
```

### 比較演算子を導入する

```ebnf
expr       = equality
equality   = relational ("==" relational | "!=" relational)*
relational = add ("<" add | "<=" add | ">" add | ">=" add)*
add        = mul ("+" mul | "-" mul)*
mul        = unary ("*" unary | "/" unary)*
unary      = ("+" | "-")? primary
primary    = num | "(" expr ")"
```

### アセンブリでの比較演算

```s
pop rdi
pop rax
cmp rax, rdi
sete al
movzb rax, al
```

cmpで指定のレジスタの値を比較することができる。同一である場合は1、それ以外は0を特定のレジスタにセットする。sete命令では指定のレジスタに比較演算の結果の値をロードする。
seteは8ビットレジスタしか指定することができない。alはraxの下位8ビットである。
movzbでraxの上位56ビットをクリアする。

### ファイル分割

- 9cc.h: ヘッダファイル
- main.c: main関数
- parse.c: パーサ
- codegen.c: コードジェネレータ
- container.c: ベクタ、マップ、およびそのテストコード

### ローカル変数

Cの変数はメモリ上に存在し変数はアドレスに名前をつけたものと考えることができる。ローカル変数はスタックに置く。

アセンブリのcall命令はリターンアドレスにスタックを積む。
リターンアドレスとは関数が呼ばれた際に関数から帰った際の戻り先のアドレスである。関数から処理が帰った際は常にリターンアドレスの一番上の値を参照するようになっている。

現在の関数フレームの開始位置を常に指しているレジスタのことをベースレジスタと呼ぶ。そこに入っている値をベースポインタと呼ぶ。x86-64ではRBPがベースレジスタとして使用される。

関数フレームはリターンアドレスの上にスタックされる。
関数フレーム（アクティベーションレコード）とは関数呼び出しごとに用意されるメモリ領域のことである。ここを基準にスコープの管理を行う。ローカル変数はベースポインタからの相対的な距離（バイト）で指定する。

関数フレームの位置がわからなくならないように常に関数フレームを常に指すレジスタが用意されており、それをベースレジスタと呼ぶ。

x86-64ではRBPレジスタがベースレジスタとして使用されている。

RSPはスタックマシンのレジスタとして使用されている

#### 関数の実装

##### プロローグ

push: RSPのアドレスをデクリメントして更新しそのアドレスに値を格納する。
mov: データをコピーする

1. rspで保持されているアドレスをインクリメントしてそのアドレスにその時点のrbpが保持する値を格納する
2. rbpにrspの値をコピーする(1をしたのでrbpを変更してよくなる)
3. rspから16を引く(ローカル変数のための領域を確保する)

```s
push rbp
mov rbp, rsp
sub rsp, 16
```

x86-64のスタックはアドレスの大きい方から小さい方に成長する。

#### エピローグ

1. スタックポインタを前のベースポインタに戻す
2. ベースポインタにrspの指し示す値を格納してrspの値をインクリメントする
3. rspをインクリメントしてアドレスを取り出しそこに処理を戻す

`ret`はアドレスをポップしてそこにジャンプする。

```s
mov rsp, rbp
pop rbp
ret
```

#### トークナイザ実装

```c
// mからnまでを足す
sum(m, n) {
  acc = 0;
  for (i = m; i <= n; i = i + 1)
    acc = acc + i;
  return acc;
}

main() {
  return sum(1, 10); // 55を返す
}
```

#### パーサ変更

##### EBNF復習

εは何もないことを示す。

書き方	意味
A*	Aの0回以上の繰り返し
A?	Aまたはε
A | B	AまたはB
( ... )	グループ化

```ebnf
expr       = equality
equality   = relational ("==" relational | "!=" relational)*
relational = add ("<" add | "<=" add | ">" add | ">=" add)*
add        = mul ("+" mul | "-" mul)*
mul        = unary ("*" unary | "/" unary)*
unary      = ("+" | "-")? primary
primary    = num | "(" expr ")"
```

↑を↓に修正する。

```ebnf
program    = stmt*
stmt       = expr ";"
expr       = assign
assign     = equality ("=" assign)?
equality   = relational ("==" relational | "!=" relational)*
relational = add ("<" add | "<=" add | ">" add | ">=" add)*
add        = mul ("+" mul | "-" mul)*
mul        = unary ("*" unary | "/" unary)*
unary      = ("+" | "-")? primary
primary    = num | ident | "(" expr ")"
```

#### 左辺値と右辺値

代入式の左辺にくることができるのはメモリのアドレスを指定する式のみ。
アドレスを既知の変数からの固定のオフセットで指定することは許されていない。

左辺に置くことのできる値のことを左辺値、右辺に置くことのできる値のことを右辺値と呼ぶ。変数についてはそれが左辺に置かれている場合はその値（アドレス）を取得しそこに右辺の値をストアする。
右辺に置かれている場合はその値（アドレス）にストアされてる値をロードする。

#### アドレスから値をロードする方法

アセンブリの構文ではレジスタを[]で囲むとそこに格納されている値をアドレスとみなし、そのアドレスの格納されている値を扱うという意味になる。

```s
# メモリから値をロードする
mov dst, [src]
```

```s
# メモリに値をストアする
mov [dst], src
```

```s
pop rax
```

```s
mov rax, [rsp]
add rsp, 8
```

```s
push rax
```

```s
sub rsp, 8
mov [rsp], rax
```

```c
Token head;
head.next = NULL;
Token *cur = &head;
```

初期化されていない変数を参照しようとすると`segmentation fault`が発生する。

初期化されているか否かは下記のように判定可能。

```c
if(var){
  // 初期化されている
}else{
  // 初期化されていない
}
```

### return文

```bnf
program = stmt*
stmt    = expr ";"
        | "return" expr ";"
...
```

文字列のポインタに[]をつけるとポインタからのオフセット分の位置の値を見てくれる

```c
void gen(Node *node) {
  if (node->kind == ND_RETURN) {
    gen(node->lhs); // gen(node->rhs);
    printf("  pop rax\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return;
  }
}
```

間違い？

### 制御構文

- if
- while
- for

```ebnf
program = stmt*
stmt    = expr ";"
        | "if" "(" expr ")" stmt ("else" stmt)?
        | "while" "(" expr ")" stmt
        | "for" "(" expr? ";" expr? ";" expr? ")" stmt
        | "return" expr ";"
        | ...
...
```

nodeに必要なのは実際にアセンブリの出力が必要な部分のみ。
()などは計算の順番を制御するためだけに必要なので不要。

->アロー関数はポインタを構造する

pop rax

### je

je jump if equal

https://qiita.com/DQNEO/items/76a99445e3adde72eb2d

### for 文

- tokenize
- node 作成
- gen

  Aをコンパイルしたコード
.LbeginXXX:
  Bをコンパイルしたコード
  pop rax
  cmp rax, 0
  je  .LendXXX
  Dをコンパイルしたコード
  Cをコンパイルしたコード
  jmp .LbeginXXX
.LendXXX:

```c
A;
begin:
if(B==0){
  goto end;
}
D;
C;
goto begin;
end:
```

### ブロック文

ブロックとは{}で囲まれた複数の文のこと。
正式には「compound statement」と呼ばれるが通常は単にブロックと呼ばれる。

{}で囲うことにより一塊の意味のあるプログラムとすることができる。

> 関数本体も実はブロックです。文法上、関数本体は必ずブロックでなければならないことになっています。関数の定義の{ ... }は、実はifやwhileの後に書く{ ... }と構文的には同じなのです。

関数の本体部分もブロックである。制御構文の{}と関数の{}には違いがない。

```ebnf
program = stmt*
stmt    = expr ";"
        | "{" stmt* "}"
        | "if" "(" expr ")" stmt ("else" stmt)?
        | "while" "(" expr ")" stmt
        | "for" "(" expr? ";" expr? ";" expr? ")" stmt
        | "return" expr ";"
        | ...
...
```

TK_BEGIN_BLOCK必要か？

assert 10 'a=0; for (i=0; i<5; i=i+1) {a=a+1; a=a+1;} return a;'

### ベクタ

一次元で同じ型の配列のことをベクタと呼ぶ。

今回は{}内のステートメントのNodeのポインタの配列を持つようにする。

stmt*はすでに実装したことがあるので同じようにする

### 関数呼び出し

foo()のような引数なしの関数呼び出しを認識できるようにして、これをcall fooにコンパイルする。

```ebnf
primary = num
        | ident ("(" ")")?
        | "(" expr ")"
```

関数名も識別子の一つなのでidentで認識する。
続いて`(`があるか否かでそれが関数名なのか変数名なのかを判断する。

### Makefile

ターゲット: 依存ファイル1 依存ファイル2

ターゲットというバイナリファイルを作成しようとする。
依存ファイルは0個のファイルを指定することができる。

```Makefile
9cc: $(OBJS)
		$(CC) -o 9cc $(OBJS) $(LDFLAGS)

$(OBJS): 9cc.h

test: 9cc
		./test.sh

clean:
		rm -f 9cc *.o *~ tmp*

.PHONY: test clean		
```

`.PHONY`は予約語でこれをターゲットとして依存ファイルを指定すると`make`の引数として指定された場合にはルールを実行してもらえるようになる。

### test.sh

```sh
# 9ccに引数を渡してアセンブリをtmp.sに出力
./9cc "$2" > tmp.s
# 作成したアセンブリからtmpという名前の実行ファイルを作成する（-oオプションで実行ファイルの名前を指定できる）
cc -o tmp tmp.s
# 実行ファイルを実行する
./tmp
```

今回はtmpに別のcファイル（foo.c）から作成したオブジェクトファイルをリンクして実行できるようにしたい。
foo.cで定義した関数をtmpから呼び出すことができれば良い。

```sh
./9cc "foo();" > tmp.s
gcc tmp.s foo.c -o tmp
./tmp
```

```s
.intel_syntax noprefix
.globl main
main:
  push rbp
  mov rbp, rsp
  sub rsp, 208
  push 0
  pop rax
  mov rsp, rbp
  pop rbp
  ret

```
