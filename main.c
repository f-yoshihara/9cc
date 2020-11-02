#include "9cc.h"

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        error("invalid args");
        return 1;
    }

    user_input = argv[1];
    token = tokenize(argv[1]);
    program();

    // Node *node = expr();
    // Node *node = program();
    // Node *node = stmt();

    // アセンブリ冒頭部分
    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    // prologue
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, 208\n");

    for (int i = 0; code[i]; i++)
    {
        gen(code[i]);
        printf("  pop rax\n");
    }

    // epilogue
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return 0;
}
