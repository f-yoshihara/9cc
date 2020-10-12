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
        if (*p == '+' || *p == '-')
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

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        error("invalid args");
        return 1;
    }

    user_input = argv[1];

    token = tokenize(argv[1]);

    // アセンブリ冒頭部分
    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    printf("  mov rax, %ld\n", expect_number());

    while (!at_eof())
    {
        if (consume('+'))
        {
            // + の次は数値が繋がるはず
            printf("  add rax, %ld\n", expect_number());
            continue;
        }

        // + 以外であれば - であるはず
        expect('-');
        printf("  sub rax, %ld\n", expect_number());
    }

    printf("  ret\n");
    return 0;
}
