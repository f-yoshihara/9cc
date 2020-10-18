#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// tokenの種類
typedef enum
{
    TK_RESERVED, // 記号
    TK_NUM,      // 整数トークン
    TK_EOF,      // 入力の終わりを示すトークン
} TokenKind;

typedef struct Token Token;

// token型
struct Token
{
    TokenKind kind; // tokenの型
    Token *next;    // 次のtoken
    int val;        // kindがTK_NUMの場合、その数値
    char *str;      // token文字列
};

Token *token;

// 入力値
char *user_input;

void error_at(char *loc, char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, "");
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void error(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// 次のトークンが期待している記号の時にはトークンを一つ読み進めて真を返す。
// それ以外の場合は偽を返す。
bool consume(char op)
{
    if (token->kind != TK_RESERVED || token->str[0] != op)
    {
        return false;
    }
    token = token->next;
    return true;
}

// 次のトークンが期待している記号のときにはトークンを一つ読み進める。
// それ以外の場合にはエラーを報告する。
void expect(char op)
{
    if (token->kind != TK_RESERVED || token->str[0] != op)
    {
        error_at(token->str, "'%c'ではありません", op);
    }
    token = token->next;
}

// 次のトークンが数値の場合、その数値を返しトークンを一つ読み進める。
// それ以外の場合にはエラーを報告する。
int expect_number()
{
    if (token->kind != TK_NUM)
    {
        error_at(token->str, "not number");
    }
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof()
{
    return token->kind == TK_EOF;
}

// 新しいtokenを作成してcurにつなげる
Token *new_token(TokenKind kind, Token *cur, char *str)
{
    // メモリを確保する
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    cur->next = tok;
    return tok;
}

// 入力文字列pをトークン化してToken型の値へのポインタを返す
Token *tokenize(char *p)
{
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p)
    {
        // printf("%ld\n", p);
        if (isspace(*p))
        {
            p++;
            continue;
        }
        // 記号の場合
        if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')')
        {
            // 記号のトークンを作成してcurとpを進める
            cur = new_token(TK_RESERVED, cur, p++);
            continue;
        }
        // 数値の場合
        if (isdigit(*p))
        {
            // 数値のトークンを作成してcurを進める
            cur = new_token(TK_NUM, cur, p);
            // 進めたcurのvalにlong型の数値をいれる
            cur->val = strtol(p, &p, 10);
            // printf("%ld\n", cur->val);
            continue;
        }
        error_at(p, "cannot tokenize");
    }
    new_token(TK_EOF, cur, p);
    return head.next;
}

typedef enum
{
    ND_ADD, // +
    ND_SUB, // -
    ND_MUL, // *
    ND_DIV, // /
    ND_NUM, // int
} NodeKind;

typedef struct Node Node;

struct Node
{
    NodeKind kind; // ノードの型
    Node *lhs;     // 左辺
    Node *rhs;     // 右辺
    int val;       // 値（kindがND_NUMの場合のみ使用）
};

Node *new_node(NodeKind kind, Node *lhs, Node *rhs)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

Node *expr();
Node *mul();
Node *primary();

Node *expr()
{
    Node *node = mul();
    for (;;)
    {
        if (consume('+'))
        {
            node = new_node(ND_ADD, node, expr());
        }
        else if (consume('-'))
        {
            node = new_node(ND_SUB, node, expr());
        }
        return node;
    }
}

Node *mul()
{
    Node *node = primary();
    for (;;)
    {
        if (consume('*'))
        {
            node = new_node(ND_MUL, node, expr());
        }
        else if (consume('/'))
        {
            node = new_node(ND_DIV, node, expr());
        }
        return node;
    }
}

Node *primary()
{
    for (;;)
    {
        if (consume('('))
        {
            Node *node = expr();
            expect(')');
            return node;
        }
        return new_node_num(expect_number());
    }
}

// ノードを受け取ってスタックマシンのように計算するための
// アセンブリを出力する
void gen(Node *node)
{
    if (node->kind == ND_NUM)
    {
        printf("  push %d\n", node->val);
        return;
    }
    gen(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");

    switch (node->kind)
    {
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
    }
    printf("  push rax\n");
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        error("invalid args");
        return 1;
    }

    user_input = argv[1];
    token = tokenize(argv[1]);

    Node *node = expr();

    // アセンブリ冒頭部分
    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    gen(node);

    printf("  pop rax\n");
    printf("  ret\n");
    return 0;
}
