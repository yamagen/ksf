#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#define INITIAL_CHAR_COUNT 256  // 初期の文字数カウント
#define MAX_LINE_LENGTH 8192    // fgetsで読み取る最大行長

#define VERSION "1.0.0 Last change: 2024/10/22-07:59:42."  // プログラムのバージョン
#define AUTHOR "Hilofumi Yamamoto"  // プログラムの作者
#define PROGRAM "ksf"  // プログラム名
#define LICENSE "Apache 2.0"  // ライセンス情報

#define isutf8_1byte(a) ((a & 0x80) == 0)            // 1バイトASCII文字
#define isutf8_2byte(a) ((a & 0xE0) == 0xC0)         // 2バイト文字
#define isutf8_3byte(a) ((a & 0xF0) == 0xE0)         // 3バイト文字
#define isutf8_4byte(a) ((a & 0xF8) == 0xF0)         // 4バイト文字
#define isutf8_continuation_byte(a) ((a & 0xC0) == 0x80)  // 後続バイト

// ハッシュマップのエントリ構造
typedef struct {
    unsigned char utf8_char[5];  // UTF-8文字（最大4バイト + 終端）
    int count;                   // 出現回数
    bool is_kanji;               // 漢字かどうかのフラグ
} CharCount;

// 出現回数を保存する動的配列
CharCount *char_count;
int char_count_size = 0;
int char_count_capacity = INITIAL_CHAR_COUNT;

// オプションフラグ
bool count_kanji_only = false;          // 漢字リストのみを出力するか
bool count_kanji_coverage = false;      // 漢字カバー率を出力するか

// 配列を拡張する関数
void expand_char_count() {
    char_count_capacity *= 2;
    char_count = realloc(char_count, char_count_capacity * sizeof(CharCount));
    if (char_count == NULL) {
        perror("Failed to reallocate memory");
        exit(1);  // メモリ割り当て失敗時は即終了
    }
}

// UTF-8文字を配列に記録する関数
void record_char(const unsigned char *utf8_char, int len, bool is_kanji) {
    // 既存の文字があればカウントを増やす
    for (int i = 0; i < char_count_size; i++) {
        if (strncmp((const char *)char_count[i].utf8_char, (const char *)utf8_char, len) == 0) {
            char_count[i].count++;
            return;
        }
    }

    // 必要であれば配列を拡張
    if (char_count_size >= char_count_capacity) {
        expand_char_count();
    }

    // 新しい文字を配列に追加
    memcpy(char_count[char_count_size].utf8_char, utf8_char, len);
    char_count[char_count_size].utf8_char[len] = '\0';  // 終端文字を追加
    char_count[char_count_size].count = 1;
    char_count[char_count_size].is_kanji = is_kanji;    // 漢字かどうかのフラグを記録
    char_count_size++;
}

// 文字の頻度に基づいてソートする比較関数
int compare_char_count(const void *a, const void *b) {
    const CharCount *ca = (const CharCount *)a;
    const CharCount *cb = (const CharCount *)b;
    return cb->count - ca->count;  // 降順にソート
}

// バージョン情報を表示する関数
void print_version() {
    printf("%s version %s %s\n", PROGRAM, VERSION, AUTHOR);
}

// ヘルプメッセージを表示する関数
void print_help() {
    printf("Usage: %s [OPTIONS] [FILES...]\n", PROGRAM);
    print_version();
    printf("Count and list the frequency of characters in text files.\n");
    printf("If no file is specified, read from standard input.\n");
    printf("If no option is specified, count all characters.\n");
    // utf8のみのファイルが対象です。
    printf("Only UTF-8 encoded files are supported.\n");
    printf("Options:\n");
    printf("  --help                  Display this help message and exit.\n");
    printf("  --version               Display version information and exit.\n");
    printf("  --count-kanji           Count and list kanji characters.\n");
    printf("  --count-kanji-coverage  Display kanji characters and their cumulative coverage.\n");
    printf("License: %s\n", LICENSE);
    
}


// 文字種と各文字のカウントを数える関数のプロトタイプ
void count_character_types(const unsigned char *str, int *count_ascii, int *count_hiragana, int *count_katakana, int *count_kanji, int *count_symbols, int *count_other);

// 文字種と各文字のカウントを数える関数
void count_character_types(const unsigned char *str, int *count_ascii, int *count_hiragana, int *count_katakana, int *count_kanji, int *count_symbols, int *count_other) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (isutf8_1byte(str[i])) {
            // 1バイト文字（ASCII）
            unsigned char utf8_char[2] = {str[i], 0};
            record_char(utf8_char, 1, false);  // ASCII文字は漢字ではない
            (*count_ascii)++;
        } else if (isutf8_2byte(str[i])) {
            // 2バイト文字
            unsigned char utf8_char[3] = {str[i], str[i+1], 0};
            record_char(utf8_char, 2, false);  // 2バイト文字は漢字ではない
            (*count_other)++;
            i += 1;  // 2バイト文字なので次のバイトをスキップ
        } else if (isutf8_3byte(str[i])) {
            // 3バイト文字の処理
            unsigned char utf8_char[4] = {str[i], str[i+1], str[i+2], 0};
            unsigned int unicode_char = ((str[i] & 0x0F) << 12) | ((str[i+1] & 0x3F) << 6) | (str[i+2] & 0x3F);

            if (unicode_char >= 0x3040 && unicode_char <= 0x309F) {
                // ひらがな
                record_char(utf8_char, 3, false);
                (*count_hiragana)++;
            } else if (unicode_char >= 0x30A0 && unicode_char <= 0x30FF) {
                // カタカナ
                record_char(utf8_char, 3, false);
                (*count_katakana)++;
            } else if (unicode_char >= 0x4E00 && unicode_char <= 0x9FFF) {
                // 漢字の範囲: U+4E00 - U+9FFF
                record_char(utf8_char, 3, true);  // 漢字として記録
                (*count_kanji)++;
            } else if ((unicode_char >= 0x2000 && unicode_char <= 0x206F) ||  // 一般句読点
                       (unicode_char >= 0x3000 && unicode_char <= 0x303F)) {  // 全角句読点など
                record_char(utf8_char, 3, false);  // 記号として記録
                (*count_symbols)++;
            } else {
                record_char(utf8_char, 3, false);  // その他の3バイト文字
                (*count_other)++;
            }
            i += 2;  // 3バイト文字なので次の2バイトをスキップ
        } else if (isutf8_4byte(str[i])) {
            // 4バイト文字
            unsigned char utf8_char[5] = {str[i], str[i+1], str[i+2], str[i+3], 0};
            record_char(utf8_char, 4, false);  // 4バイト文字は漢字ではない
            (*count_other)++;
            i += 3;  // 4バイト文字なので次の3バイトをスキップ
        } else if (isutf8_continuation_byte(str[i])) {
            // 後続バイト（スキップ）
        } else {
            printf("Invalid UTF-8 byte: 0x%x\n", str[i]);
        }
    }
}

// 漢字のカバー率を出力する関数
void print_kanji_coverage(int total_kanji_count) {
    double cumulative_percentage = 0.0;
    int target_percentages[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100};
    int next_target_index = 0;
    int required_kanji = 0;

    printf("\nCoverage Fractions for Observed Kanji\n");
    printf("=====================================\n");
    printf("Percentage          Required\n");
    printf("of Text             Kanji\n");
    printf("-------------------------------------\n");

    for (int i = 0; i < char_count_size; i++) {
        if (char_count[i].is_kanji) {
            double percentage = (double)char_count[i].count / total_kanji_count * 100.0;
            cumulative_percentage += percentage;
            required_kanji++;

            // 指定されたカバー率に達したら出力
            while (next_target_index < sizeof(target_percentages) / sizeof(target_percentages[0]) &&
                   cumulative_percentage >= target_percentages[next_target_index]) {
                printf("%5d\t%20d\n", target_percentages[next_target_index], required_kanji);
                next_target_index++;
            }
        }
    }

    // 100%に達していない場合、強制的に100%として出力
    if (cumulative_percentage < 100.0) {
        printf("%5d\t%20d\n", 100, required_kanji);
    }
}

// ファイルまたは標準入力を1行ずつ処理する関数
void process_stream(FILE *stream, int *count_ascii, int *count_hiragana, int *count_katakana, int *count_kanji, int *count_symbols, int *count_other) {
    char line[MAX_LINE_LENGTH];
    
    // 1行ずつ読み込み
    while (fgets(line, sizeof(line), stream)) {
        count_character_types((const unsigned char *)line, count_ascii, count_hiragana, count_katakana, count_kanji, count_symbols, count_other);
    }
}

// 結果を出力する関数
void print_results(int count_ascii, int count_hiragana, int count_katakana, int count_kanji, int count_symbols, int count_other, int file_count, bool from_stdin) {
    // 処理したファイル数を出力
    if (from_stdin) {
        printf("Number of files processed: stdin\n");
    } else {
        printf("Number of files processed: %d\n", file_count);
    }

    // 全体のカウント結果を出力
    printf("ASCII (Alphabet/Numbers): %d\n", count_ascii);
    printf("Hiragana: %d\n", count_hiragana);
    printf("Katakana: %d\n", count_katakana);
    printf("Kanji: %d\n", count_kanji);
    printf("Symbols (punctuation): %d\n", count_symbols);
    printf("Other characters: %d\n", count_other);

    // ソートしてからリストを出力
    qsort(char_count, char_count_size, sizeof(CharCount), compare_char_count);

    // リストを出力
    printf("\nRank\tCharacter\tCount\tCumulative Percentage (%%)\n");
    int rank = 1;
    double cumulative_percentage = 0.0;

    // 分母の決定: --count-kanji の場合は漢字のみのカウントを使用
    int total_count = count_kanji_only ? count_kanji : (count_ascii + count_hiragana + count_katakana + count_kanji + count_symbols + count_other);

    for (int i = 0; i < char_count_size; i++) {
        // --count-kanji の場合は漢字のみを表示
        if (count_kanji_only && !char_count[i].is_kanji) {
            continue;  // 漢字以外はスキップ
        }

        // 累積割合を計算
        double percentage = (double)char_count[i].count / total_count * 100.0;
        cumulative_percentage += percentage;
        printf("%d\t%s\t%d\t%.4f\n", rank++, char_count[i].utf8_char, char_count[i].count, cumulative_percentage);
    }

    // --count-kanji-coverage が指定された場合に、漢字カバー率も出力
    if (count_kanji_coverage) {
        print_kanji_coverage(count_kanji);
    }
}




// コマンドライン引数を解析する関数
void parse_options(int argc, char *argv[], int *start_idx) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            print_help();  // ヘルプメッセージを表示
            exit(0);
        } else if (strcmp(argv[i], "--version") == 0) {
            print_version();  // バージョン情報を表示
            exit(0);
        } else if (strcmp(argv[i], "--count-kanji") == 0) {
            count_kanji_only = true;  // 漢字のみを出力する
        } else if (strcmp(argv[i], "--count-kanji-coverage") == 0) {
            count_kanji_coverage = true;  // 漢字カバー率を出力する
            count_kanji_only = true;      // coverageを出力する場合は漢字も出力する
        } else if (argv[i][0] != '-') {
            *start_idx = i;  // ファイル名の開始インデックス
            break;  // ファイル名の処理を開始
        }
    }
}

int main(int argc, char *argv[]) {
    int count_ascii = 0, count_hiragana = 0, count_katakana = 0, count_kanji = 0, count_symbols = 0, count_other = 0;
    bool from_stdin = false;
    FILE *file = NULL;
    int file_count = 0;

    // 初期メモリ割り当て
    char_count = malloc(INITIAL_CHAR_COUNT * sizeof(CharCount));
    if (char_count == NULL) {
        perror("Failed to allocate memory");
        return 1;
    }

    // オプションを解析
    int start_idx = argc;  // デフォルトはファイル名なし（標準入力使用）
    parse_options(argc, argv, &start_idx);

    // ファイルが指定されているか確認
    if (start_idx < argc) {
        // 複数のファイルを処理
        for (int i = start_idx; i < argc; i++) {
            file = fopen(argv[i], "r");
            if (file == NULL) {
                perror("Failed to open file");
                free(char_count);
                return 1;
            }
            // ファイルを処理
            process_stream(file, &count_ascii, &count_hiragana, &count_katakana, &count_kanji, &count_symbols, &count_other);
            fclose(file);
            file_count++;
        }
    } else {
        // ファイルが指定されていない場合、標準入力を処理
        from_stdin = true;
        process_stream(stdin, &count_ascii, &count_hiragana, &count_katakana, &count_kanji, &count_symbols, &count_other);
        file_count = 1;  // 標準入力でも1つのデータソースとしてカウント
    }

    // 結果を出力
    print_results(count_ascii, count_hiragana, count_katakana, count_kanji, count_symbols, count_other, file_count, from_stdin);

    // メモリ解放
    free(char_count);

    return 0;
}
