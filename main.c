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

    // Node *node = expr();
    // Node *node = program();
    Node *node = stmt();

    // アセンブリ冒頭部分
    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    gen(node);

    printf("  pop rax\n");
    printf("  ret\n");
    return 0;
}
