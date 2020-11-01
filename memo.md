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
