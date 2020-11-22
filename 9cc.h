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
    TK_IDENT,    //  識別子
    TK_NUM,      // 整数トークン
    TK_EOF,      // 入力の終わりを示すトークン
    TK_RETURN,   // return
    TK_IF,       // if
    TK_ELSE,     // else
    TK_WHILE,    // while
    TK_FOR,      // for
} TokenKind;

typedef struct Token Token;

// token型
struct Token
{
    TokenKind kind; // tokenの型
    Token *next;    // 次のtoken
    int val;        // kindがTK_NUMの場合、その数値
    char *str;      // token文字列
    int len;        // トークンの長さ
};

Token *token;

typedef struct LVar LVar;

// ローカル変数の型
struct LVar
{
    LVar *next; // 次の変数かNULL
    char *name; // 変数の名前
    int len;    // 名前の長さ
    int offset; // RBPからのオフセット
};

// ローカル変数
LVar *locals;

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

bool starts_with(char *str1, char *str2)
{
    return memcmp(str1, str2, strlen(str2)) == 0;
}

// 次のトークンが期待している記号の時にはトークンを一つ読み進めて真を返す。
// それ以外の場合は偽を返す。
bool consume(char *op)
{
    if (token->kind != TK_RESERVED ||
        strlen(op) != token->len ||
        memcmp(token->str, op, token->len))
    {
        return false;
    }
    token = token->next;
    return true;
}

Token *consume_ident()
{
    if (token->kind != TK_IDENT)
    {
        return false;
    }
    Token *result = token;
    token = token->next;
    return result;
}

Token *consume_keyword(TokenKind kind)
{
    if (token->kind != kind)
    {
        return false;
    }
    Token *result = token;
    token = token->next;
    return result;
}

Token *consume_return()
{
    return consume_keyword(TK_RETURN);
}

Token *consume_if()
{
    return consume_keyword(TK_IF);
}

Token *consume_else()
{
    return consume_keyword(TK_ELSE);
}

Token *consume_while()
{
    return consume_keyword(TK_WHILE);
}

Token *consume_for()
{
    return consume_keyword(TK_FOR);
}

// 次のトークンが期待している記号のときにはトークンを一つ読み進める。
// それ以外の場合にはエラーを報告する。
void expect(char *op)
{
    if (token->kind != TK_RESERVED || !starts_with(token->str, op))
    {
        error_at(token->str, "'%c'ではありません", *op);
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
Token *new_token(TokenKind kind, Token *cur, char *str, int len)
{
    // メモリを確保する
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;

    return tok;
}

// 識別子を構成しうる要素かどうかを判断する
int is_alnum(char c)
{
    return ('a' <= c && c <= 'z') ||
           ('A' <= c && c <= 'Z') ||
           ('0' <= c && c <= '9') ||
           (c == '_');
}

// 入力文字列pをトークン化してToken型の値へのポインタを返す
Token *tokenize(char *p)
{
    Token head;
    head.next = NULL;
    Token *cur = &head;
    int len = 0;

    while (*p)
    {
        // printf("%ld\n", p);
        if (isspace(*p))
        {
            p++;
            continue;
        }
        if (starts_with(p, "==") || starts_with(p, "!=") ||
            starts_with(p, "<=") || starts_with(p, ">="))
        {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }
        if (strchr("+-*/()<>=;{}", *p))
        {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }

        // 数値の場合
        if (isdigit(*p))
        {
            // 数値のトークンを作成してcurを進める
            cur = new_token(TK_NUM, cur, p, 0);
            // 進めたcurのvalにlong型の数値をいれる
            cur->val = strtol(p, &p, 10);
            continue;
        }

        if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6]) && cur->kind != TK_IDENT)
        {
            cur = new_token(TK_RETURN, cur, p, 6);
            p += 6;
            continue;
        }

        if (strncmp(p, "if", 2) == 0 && !is_alnum(p[2]) && cur->kind != TK_IDENT)
        {
            cur = new_token(TK_IF, cur, p, 2);
            p += 2;
            continue;
        }
        if (strncmp(p, "else", 4) == 0 && !is_alnum(p[4]) && cur->kind != TK_IDENT)
        {
            cur = new_token(TK_ELSE, cur, p, 4);
            p += 4;
            continue;
        }
        if (strncmp(p, "while", 5) == 0 && !is_alnum(p[5]) && cur->kind != TK_IDENT)
        {
            cur = new_token(TK_WHILE, cur, p, 5);
            p += 5;
            continue;
        }
        if (strncmp(p, "for", 3) == 0 && !is_alnum(p[3]) && cur->kind != TK_IDENT)
        {
            cur = new_token(TK_FOR, cur, p, 3);
            p += 3;
            continue;
        }

        // 小文字のアルファベットである場合
        if ('a' <= *p && *p <= 'z')
        {
            if (cur->kind == TK_IDENT)
            {
                cur->len = ++len;
                p++;
            }
            else
            {
                cur = new_token(TK_IDENT, cur, p++, len = 1);
            }
            continue;
        }

        error_at(p, "cannot tokenize");
    }
    new_token(TK_EOF, cur, p, 0);
    return head.next;
}

typedef enum
{
    ND_ADD,    // +
    ND_SUB,    // -
    ND_MUL,    // *
    ND_DIV,    // /
    ND_ASSIGN, // =
    ND_LVAR,   // local variable
    ND_EQ,     // ==
    ND_NE,     // !=
    ND_LT,     // <
    ND_LE,     // <=
    ND_NUM,    // int
    ND_RETURN, // return
    ND_IF,     // if
    ND_WHILE,  // while
    ND_FOR,    // for
    ND_BLOCK,  // block
} NodeKind;

typedef struct Node Node;

struct Node
{
    NodeKind kind; // ノードの型
    Node *lhs;     // 左辺
    Node *rhs;     // 右辺
    int val;       // 値（kindがND_NUMの場合のみ使用）
    int offset;

    // 条件式
    Node *cond;
    // ifブロック内の文を入れ子にする
    Node *then;
    // elseブロック内の文を入れ子にする
    Node *ethen;
    // for(init;cond;inc)
    Node *init;
    Node *inc;
    Node *block[100];
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

LVar *find_lvar(Token *tok)
{
    for (LVar *var = locals; var; var = var->next)
    {
        if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
        {
            return var;
        }
    }
    return NULL;
}

// 識別子
Node *new_node_ident(Token *tok)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LVAR;
    LVar *lvar = find_lvar(tok);
    if (lvar)
    {
        node->offset = lvar->offset;
    }
    else
    {
        lvar = calloc(1, sizeof(LVar));
        lvar->next = locals;
        lvar->name = tok->str;
        lvar->len = tok->len;
        lvar->offset = ((locals) ? locals->offset : 0) + 8;
        node->offset = lvar->offset;
        locals = lvar;
    }
    return node;
}

Node *program();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

Node *code[100];

Node *program()
{
    int i = 0;
    while (!at_eof())
    {
        code[i++] = stmt();
    }
    code[i] = NULL;
}

Node *stmt()
{
    Node *node;
    if (consume_return())
    {
        node = new_node(ND_RETURN, node, expr());
        expect(";");
    }
    else if (consume("{"))
    {
        int i = 0;
        node = new_node(ND_BLOCK, node, NULL);
        while (!consume("}"))
        {
            node->block[i++] = stmt();
        }
        node->block[i] = NULL;
    }
    else if (consume_if())
    {
        node = new_node(ND_IF, node, NULL);
        expect("(");
        node->cond = expr();
        expect(")");
        node->then = stmt();
        if (consume_else())
        {
            node->ethen = stmt();
        }
    }
    else if (consume_while())
    {
        node = new_node(ND_WHILE, node, NULL);
        expect("(");
        node->cond = expr();
        expect(")");
        node->then = stmt();
    }
    else if (consume_for())
    {
        node = new_node(ND_FOR, node, NULL);
        expect("(");
        if (!consume(";"))
        {
            node->init = expr();
            expect(";");
        }
        if (!consume(";"))
        {
            node->cond = expr();
            expect(";");
        }
        if (!consume(";"))
        {
            node->inc = expr();
        }
        expect(")");
        node->then = stmt();
    }
    else
    {
        node = expr();
        expect(";");
    }

    return node;
}

Node *expr()
{
    return assign();
}

Node *assign()
{
    Node *node = equality();
    if (consume("="))
    {
        node = new_node(ND_ASSIGN, node, assign());
    }
    else
    {
        return node;
    }
}

Node *equality()
{
    Node *node = relational();
    for (;;)
    {
        if (consume("=="))
        {
            node = new_node(ND_EQ, node, relational());
        }
        else if (consume("!="))
        {
            node = new_node(ND_NE, node, relational());
        }
        else
        {
            return node;
        }
    }
}

Node *relational()
{
    Node *node = add();
    for (;;)
    {
        if (consume("<"))
        {
            node = new_node(ND_LT, node, add());
        }
        else if (consume("<="))
        {
            node = new_node(ND_LE, node, add());
        }
        else if (consume(">"))
        {
            node = new_node(ND_LT, add(), node);
        }
        else if (consume(">="))
        {
            node = new_node(ND_LE, add(), node);
        }
        else
        {
            return node;
        }
    }
}

Node *add()
{
    Node *node = mul();
    for (;;)
    {
        if (consume("+"))
        {
            node = new_node(ND_ADD, node, mul());
        }
        else if (consume("-"))
        {
            node = new_node(ND_SUB, node, mul());
        }
        else
        {
            return node;
        }
    }
}

Node *mul()
{
    Node *node = unary();
    for (;;)
    {
        if (consume("*"))
        {
            node = new_node(ND_MUL, node, unary());
        }
        else if (consume("/"))
        {
            node = new_node(ND_DIV, node, unary());
        }
        else
        {
            return node;
        }
    }
}

Node *unary()
{
    if (consume("+"))
    {
        return primary();
    }
    else if (consume("-"))
    {
        return new_node(ND_SUB, new_node_num(0), primary());
    }
    else
    {
        return primary();
    }
}

Node *primary()
{
    for (;;)
    {
        if (consume("("))
        {
            Node *node = expr();
            expect(")");
            return node;
        }
        Token *tok = consume_ident();
        if (tok)
        {
            return new_node_ident(tok);
        }
        return new_node_num(expect_number());
    }
}

// 対象の変数のアドレスがスタックに積まれている状態にする
void gen_lval(Node *node)
{
    if (node->kind != ND_LVAR)
    {
        error("lval is not valiable");
    }
    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", node->offset);
    printf("  push rax\n");
}

void instr(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    printf("\t");
    printf(fmt, ap);
    printf("\n");
}

void label(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    printf(fmt, ap);
    printf("\n");
}

int label_num = 0;

// ノードを受け取ってスタックマシンのように計算するための
// アセンブリを出力する
void gen(Node *node)
{
    switch (node->kind)
    {
    case ND_NUM:
        printf("  push %d\n", node->val);
        return;
    // 変数が右辺に来ている場合に有効
    case ND_LVAR:
        gen_lval(node);
        // 変数のアドレスをraxにロード
        printf("  pop rax\n");
        // 変数の値をraxにコピー
        printf("  mov rax, [rax]\n");
        // 変数の値をスタックに積む
        printf("  push rax\n");
        return;
    // 代入演算子なので変数が左辺にある
    case ND_ASSIGN:
        gen_lval(node->lhs);
        gen(node->rhs);
        // 右辺の値をrdiにpopする
        printf("  pop rdi\n");
        // 変数のアドレスをraxにpopする
        printf("  pop rax\n");
        // 右辺の値をraxのアドレスにストアする
        printf("  mov [rax], rdi\n");
        // 右辺の値をスタックに積む
        printf("  push rdi\n");
        return;
    case ND_RETURN:
        gen(node->rhs);
        return;
    case ND_BLOCK:
        for (int i = 0; node->block[i]; i++)
        {
            gen(node->block[i]);
        }
        return;
    case ND_IF:
        // 条件文内の計算を行う
        gen(node->cond);
        printf("  pop rax\n");
        // 条件文の結果を0と比較する
        printf("  cmp rax, 0\n");
        if (node->ethen) // elseがある場合
        {
            printf("  je .Lels%d\n", label_num);
            gen(node->then);
            printf("  jmp .Lend%d\n", label_num);
            printf(".Lels%d:\n", label_num);
            gen(node->ethen);
        }
        else // elseがない場合
        {
            printf("  je .Lend%d\n", label_num);
            gen(node->then);
        }
        printf(".Lend%d:\n", label_num++);
        return;
    case ND_WHILE:
        printf(".Lbegin%d:\n", label_num);
        gen(node->cond);
        instr("pop rax");
        instr("cmp rax, 0");
        printf("  je .Lend%d\n", label_num);
        gen(node->then);
        printf("  jmp .Lbegin%d\n", label_num);
        printf(".Lend%d:\n", label_num++);
        return;
    case ND_FOR:
        gen(node->init);
        printf(".Lbegin%d:\n", label_num);
        gen(node->cond);
        instr("pop rax");
        instr("cmp rax, 0");
        printf("  je .Lend%d\n", label_num);
        gen(node->then);
        gen(node->inc);
        printf("  jmp .Lbegin%d\n", label_num);
        printf(".Lend%d:\n", label_num++);
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
