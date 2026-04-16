#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node {
    char op;
    char value[64];
    struct Node *left;
    struct Node *right;
} Node;

char *s;
int pos;
int parse_ok;
char error_msg[128];

Node *new_value(const char *value) {
    Node *node = malloc(sizeof(Node));
    if (!node) {
        parse_ok = 0;
        strcpy(error_msg, "Ошибка памяти");
        return NULL;
    }
    node->op = 0;
    strcpy(node->value, value);
    node->left = NULL;
    node->right = NULL;
    return node;
}

Node *new_op(char op, Node *left, Node *right) {
    Node *node = malloc(sizeof(Node));
    if (!node) {
        parse_ok = 0;
        strcpy(error_msg, "Ошибка памяти");
        return NULL;
    }
    node->op = op;
    node->value[0] = '\0';
    node->left = left;
    node->right = right;
    return node;
}

void free_tree(Node *node) {
    if (!node) return;
    free_tree(node->left);
    free_tree(node->right);
    free(node);
}

void skip_spaces(void) {
    while (isspace((unsigned char)s[pos])) {
        pos++;
    }
}

Node *parse_expr(void);

Node *parse_factor(void) {
    char token[64];
    int i = 0;
    int start;

    skip_spaces();

    if (s[pos] == '(') {
        Node *node;
        pos++;
        node = parse_expr();
        skip_spaces();
        if (s[pos] != ')') {
            parse_ok = 0;
            strcpy(error_msg, "Нет закрывающей скобки");
            free_tree(node);
            return NULL;
        }
        pos++;
        return node;
    }

    if (isalpha((unsigned char)s[pos]) || s[pos] == '_' || isdigit((unsigned char)s[pos])) {
        start = pos;
        while (isalnum((unsigned char)s[pos]) || s[pos] == '_' || s[pos] == '.') {
            pos++;
        }
        while (start < pos && i < 63) {
            token[i++] = s[start++];
        }
        token[i] = '\0';
        return new_value(token);
    }

    parse_ok = 0;
    strcpy(error_msg, "Неверное выражение");
    return NULL;
}

Node *parse_term(void) {
    Node *left = parse_factor();
    while (parse_ok) {
        Node *right;
        Node *tmp;
        char op;

        skip_spaces();
        op = s[pos];
        if (op != '*' && op != '/') break;
        pos++;

        right = parse_factor();
        if (!parse_ok) {
            free_tree(left);
            return NULL;
        }

        tmp = new_op(op, left, right);
        if (!tmp) {
            free_tree(left);
            free_tree(right);
            return NULL;
        }
        left = tmp;
    }
    return left;
}

Node *parse_expr(void) {
    Node *left = parse_term();
    while (parse_ok) {
        Node *right;
        Node *tmp;
        char op;

        skip_spaces();
        op = s[pos];
        if (op != '+' && op != '-') break;
        pos++;

        right = parse_term();
        if (!parse_ok) {
            free_tree(left);
            return NULL;
        }

        tmp = new_op(op, left, right);
        if (!tmp) {
            free_tree(left);
            free_tree(right);
            return NULL;
        }
        left = tmp;
    }
    return left;
}

Node *parse_string(char *text) {
    Node *root;

    s = text;
    pos = 0;
    parse_ok = 1;
    error_msg[0] = '\0';

    root = parse_expr();
    skip_spaces();

    if (!parse_ok) {
        free_tree(root);
        return NULL;
    }

    if (s[pos] != '\0' && s[pos] != '\n') {
        free_tree(root);
        parse_ok = 0;
        strcpy(error_msg, "Лишние символы в конце");
        return NULL;
    }

    return root;
}

void print_infix(Node *node) {
    if (!node) return;
    if (node->op == 0) {
        printf("%s", node->value);
        return;
    }
    printf("(");
    print_infix(node->left);
    printf(" %c ", node->op);
    print_infix(node->right);
    printf(")");
}

void print_tree(Node *node, int level) {
    int i;
    if (!node) return;

    print_tree(node->right, level + 1);
    for (i = 0; i < level; i++) printf("    ");
    if (node->op == 0) printf("%s\n", node->value);
    else printf("%c\n", node->op);
    print_tree(node->left, level + 1);
}

Node *transform(Node *node, int *count) {
    Node *a;
    Node *b;
    Node *c;
    Node *d;
    Node *new_num;
    Node *new_den;
    Node *new_node;

    if (!node) return NULL;

    node->left = transform(node->left, count);
    node->right = transform(node->right, count);

    if (node->op == '*' && node->left && node->right &&
        node->left->op == '/' && node->right->op == '/') {
        a = node->left->left;
        b = node->left->right;
        c = node->right->left;
        d = node->right->right;

        new_num = new_op('*', a, c);
        new_den = new_op('*', b, d);
        if (!new_num || !new_den) {
            free_tree(new_num);
            free_tree(new_den);
            return node;
        }

        new_node = new_op('/', new_num, new_den);
        if (!new_node) {
            free_tree(new_num);
            free_tree(new_den);
            return node;
        }

        node->left->left = NULL;
        node->left->right = NULL;
        node->right->left = NULL;
        node->right->right = NULL;
        free_tree(node->left);
        free_tree(node->right);
        free(node);

        (*count)++;
        return new_node;
    }

    return node;
}

int main(void) {
    char input[1024];
    Node *root;
    int count = 0;

    printf("Лабораторная работа 3, вариант 24\n");
    printf("Преобразование: (a/b) * (c/d) -> (a*c)/(b*d)\n\n");
    printf("Введите выражение: ");

    if (!fgets(input, sizeof(input), stdin)) {
        printf("Ошибка ввода\n");
        return 1;
    }

    root = parse_string(input);
    if (!root) {
        printf("Ошибка: %s\n", error_msg);
        return 1;
    }

    printf("\nИсходное выражение: ");
    print_infix(root);
    printf("\n\nИсходное дерево:\n");
    print_tree(root, 0);

    root = transform(root, &count);

    printf("\nКоличество преобразований: %d\n", count);
    printf("\nПреобразованное выражение: ");
    print_infix(root);
    printf("\n\nПреобразованное дерево:\n");
    print_tree(root, 0);

    free_tree(root);
    return 0;
}