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

pushであればRSPのアドレスをインクリメントして更新しそのアドレスに値を格納する。

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
