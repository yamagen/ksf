:toc: macro
:toc-title: Table of Contents

# ksf: Kanji Statistics Filter

[![DOI](https://zenodo.org/badge/163961186.svg)](https://zenodo.org/badge/latestdoi/163961186)

![ksf](./ksf.png)

## Synopsis

Usage: ksf [OPTIONS] [FILES...]

## Description

Count and list the frequency of characters in text files.
If no file is specified, read from standard input.
If no option is specified, count all characters.
Only UTF-8 encoded files are supported.

## Version

ksf version 1.0.0 Last change: 2024/10/23-11:01:28. Hilofumi Yamamoto

## Options

- --help
  Display this help message and exit.
- --version
  Display version information and exit.
- --count-kanji
  Count and list kanji characters.
- --count-kanji-coverage
  Display kanji characters and their cumulative coverage.

## License

Apache 2.0

## Author

Hilo Yamamoto / Institute of Science Tokyo

## Examples

### Count kanji characters in a file

```shell
$ ksf --count-kanji large_test_input.txt | head -20

Number of files processed: 1
ASCII (Alphabet/Numbers): 580
Hiragana: 251
Katakana: 61
Kanji: 188
Symbols (punctuation): 38
Other characters: 0

Rank    Character       Count   Cumulative Percentage (%)
1       字      20      10.6383
2       文      12      17.0213
3       漢      12      23.4043
4       語      10      28.7234
5       日      8       32.9787
6       本      7       36.7021
7       章      6       39.8936
8       使      5       42.5532
9       読      5       45.2128
10      異      3       46.8085
11      含      3       48.4043
```

### Display kanji characters and their cumulative coverage

```shell
$ ksf --count-kanji-coverage large_test_input.txt | tail -25

Coverage Fractions for Observed Kanji
=====================================
Percentage          Required
of Text             Kanji
-------------------------------------
   10	                   1
   20	                   3
   30	                   5
   40	                   8
   50	                  13
   60	                  19
   70	                  28
   80	                  40
   90	                  59
   91	                  61
   92	                  62
   93	                  64
   94	                  66
   95	                  68
   96	                  70
   97	                  72
   98	                  74
   99	                  76
  100	                  77
```
