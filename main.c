#include <stdio.h>
#include <wchar.h>
#include <wctype.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <locale.h>

#define MAX(a, b) ((a)>(b)?(a):(b))
#define STR_EQ(s, v) (strcmp((s), (v)) == 0)

bool flag_newline = false;
bool flag_word = false;
bool flag_character = false;
bool flag_byte = false;
bool flag_max_line_length = false;
bool flag_argend = false;

uint32_t files_read = 0;
uint32_t total_count_line = 0;
uint32_t total_count_word = 0;
uint32_t total_count_char = 0;
uint32_t total_count_byte = 0;
uint32_t total_max_line_length = 0;

void print_help();
void wc_parse_file_arg(char* arg);
void wc_open_file(const char* name);
void wc_parse_file(FILE *f);

int main(int argc, char** argv) {
    setlocale(LC_ALL, "");

    for (int i = 1; i < argc; ++i) {
        if (!flag_argend) {
            if (STR_EQ(argv[i], "--help")) {
                print_help();
                return 0;
            }
            if (STR_EQ(argv[i], "--version")) {
                printf("wc 1.48.8\n"
                       "Copyleft (C) MMXVIII t1meshift\n"
                       "License Unlicense: <http://unlicense.org/>\n"
                       "This is free software: you are free to change and redistribute it.\n"
                       "There is NO WARRANTY, to the extent permitted by law.\n\n"
                       "Written by Yury Kurlykov.\n");
                return 0;
            }
            if (STR_EQ(argv[i], "--bytes")) {
                flag_byte = true;
            } else if (STR_EQ(argv[i], "--chars")) {
                flag_character = true;
            } else if (STR_EQ(argv[i], "--lines")) {
                flag_newline = true;
            } else if (STR_EQ(argv[i], "--words")) {
                flag_word = true;
            } else if (STR_EQ(argv[i], "--max-line-length")) {
                flag_max_line_length = true;
            } else if ((strlen(argv[i]) > 1) && (argv[i][0] == '-') && (argv[i][1] != '-')) {
                size_t keys = strlen(argv[i]);
                for (size_t j = 1; j < keys; ++j) {
                    switch (argv[i][j]) {
                        case 'c':
                            flag_byte = true;
                            break;
                        case 'm':
                            flag_character = true;
                            break;
                        case 'l':
                            flag_newline = true;
                            break;
                        case 'L':
                            flag_max_line_length = true;
                            break;
                        case 'w':
                            flag_word = true;
                            break;
                        default:
                            fprintf(stderr, "wc: invalid option '%c'\nTry \"%s --help\" --help for details.\n",
                                    argv[i][j], argv[0]);
                            return 0;
                    }
                }
            } else {
                flag_argend = true;
                wc_parse_file_arg(argv[i]);
            }
        } else {
            wc_parse_file_arg(argv[i]);
        }
    }

    if (!flag_argend) {
        wc_parse_file(stdin);
        printf("\n");
    }

    if (files_read > 1)
    {
        if (flag_newline) {
            printf("%7u ", total_count_line);
        }
        if (flag_word) {
            printf("%7u ", total_count_word);
        }
        if (flag_character) {
            printf("%7u ", total_count_char);
        }
        if (flag_byte) {
            printf("%7u ", total_count_byte);
        }
        if (flag_max_line_length) {
            printf("%7u ", total_max_line_length);
        }
        printf("total\n");
    }
    return 0;
}

void print_help() {
    printf("Usage: wc [OPTION]... [FILE]...\n\n"
           "Print newline, word, and byte counts for each FILE, and a total line if\n"
           "more than one FILE is specified.  A word is a non-zero-length sequence of\n"
           "characters delimited by white space.\n"
           "\n"
           "With no FILE, or when FILE is -, read standard input.\n"
           "\n"
           "The options below may be used to select which counts are printed, always in\n"
           "the following order: newline, word, character, byte, maximum line length.\n"
           "  -c, --bytes            print the byte counts\n"
           "  -m, --chars            print the character counts\n"
           "  -l, --lines            print the newline counts\n"
           "  -L, --max-line-length  print the maximum display width\n"
           "  -w, --words            print the word counts\n"
           "      --help     display this help and exit\n"
           "      --version  output version information and exit\n");
}

void wc_parse_file_arg(char* arg) {
    if (STR_EQ(arg, "-")) {
        wc_parse_file(stdin);
        printf("%s\n", arg);
    } else {
        wc_open_file(arg);
    }
}

void wc_open_file(const char* name) {
    FILE* f = fopen(name, "r");
    if (f == NULL) {
        char err_prefix[276] = "";
        sprintf(err_prefix, "wc: %s", name);
        perror(err_prefix);
        return;
    }
    wc_parse_file(f);
    printf("%s\n", name);
    fclose(f);
}

void wc_parse_file(FILE *f) {
    uint32_t count_line = 0;
    uint32_t count_word = 0;
    uint32_t count_char = 0;
    uint32_t count_byte = 0;
    uint32_t max_line_length = 0;

    bool is_word = false;
    uint32_t line_length = 0;
    wint_t c;
    while ((c = getwc(f)) != WEOF) {
        int byte_cnt = 1;
        wint_t ct = c;
        while ((ct >>= 8) != 0)
            ++byte_cnt;

        count_byte += byte_cnt;
        ++count_char;
        ++line_length;

        if (iswspace(c) != 0) {
            if (is_word) {
                is_word = false;
            }
        } else {
            if (!is_word) {
                is_word = true;
                ++count_word;
            }
        }

        if (c == 10) { // Line break
            max_line_length = MAX(max_line_length, line_length-1);
            line_length = 0;
            ++count_line;
        }
    }

    if ((max_line_length == 0) && (count_char > 0))
        max_line_length = line_length;

    if (!flag_newline && !flag_word && !flag_character && !flag_byte && !flag_max_line_length) {
        // set -lwc flags
        flag_newline = true;
        flag_word = true;
        flag_byte = true;
    }
    if (flag_newline) {
        printf("%7u ", count_line);
        total_count_line += count_line;
    }
    if (flag_word) {
        printf("%7u ", count_word);
        total_count_word += count_word;
    }
    if (flag_character) {
        printf("%7u ", count_char);
        total_count_char += count_char;
    }
    if (flag_byte) {
        printf("%7u ", count_byte);
        total_count_byte += count_byte;
    }
    if (flag_max_line_length) {
        printf("%7u ", max_line_length);
        total_max_line_length = MAX(total_max_line_length, max_line_length);
    }
    ++files_read;
}