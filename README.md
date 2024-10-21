:toc: macro
:toc-title: Table of Contents

# ksf: Kanji Statistics Filter

![ksf](./ksf.png)

## Synopsis

Usage: ksf [OPTIONS] [FILES...]

## Description

Count and list the frequency of characters in text files.
If no file is specified, read from standard input.
If no option is specified, count all characters.
Only UTF-8 encoded files are supported.

## Version

ksf version 1.0.0 Last change: 2024/10/22-08:55:56. Hilofumi Yamamoto

## Options

--help Display this help message and exit.
--version Display version information and exit.
--count-kanji Count and list kanji characters.
--count-kanji-coverage Display kanji characters and their cumulative coverage.

## License

License: Apache 2.0
