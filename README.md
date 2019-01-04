# ksf: Kanji Statistic Filter
Time-stamp: "2019-01-04 17:30:44 yamagen"

<img src="ksf.png" alt="ksf" border=0>

Kanji, Kana, etc. Count Program
Hilofumi Yamamoto
文字種集計プログラム 'ksf'<br>

## 説明
日本語の文字種集計プログラムです。
使い方は簡単です。プログラム名の後ろにファイル名を書くだけです。

```
% ksf textfile 
```

- make だけでコンパイルできます。
- 詳しいオプションは、-hでごらんください。
- デフォルトは、EUCですが、-sをつけるとシフトJISも使えます。
- make sjis でコンパイルすると、シフトJISがデフォルトで使えます。


## オプション

```
  -c  show only kanji frequency count
  -t  print only summary table
  -f  show only kanji fraction data
  -e  show only table explanation
  -h  print this help
  -v  print ksf version
```

## 出力例

```
% ksf bochan.euc
1       云      517     1.9410
2       出      423     3.5291
3       事      371     4.9219
4       一      368     6.3035
5       来      356     7.6400
6       人      342     8.9240
7       御      289     10.0090
8       見      279     11.0565
9       何      238     11.9500
10      様      229     12.8097
(略)
1770    逑      1       99.9587
1771    錮      1       99.9625
1772    鎬      1       99.9662
1773    閾      1       99.9700
1774    靄      1       99.9737
1775    靠      1       99.9775
1776    飩      1       99.9812
1777    饂      1       99.9850
1778    饒      1       99.9887
1779    髯      1       99.9925
1780    鬢      1       99.9962
1781    鼈      1       100.0000

files                1
kanji(total)     26636  (30.6%)
kanji(unique)     1781
kanji(once)        416  (23.4% of unique)
hiragana         52767  (60.6%)
katakana          1068  ( 1.2%)
Arabic               0  ( 0.0%)
Roman                0  ( 0.0%)
Cyrillic             0  ( 0.0%)
Greek                0  ( 0.0%)
graphic              0  ( 0.0%)
punctuation       5293  ( 6.1%)
symbols           1058  ( 1.2%)
space              208  ( 0.2%)
other                0  ( 0.0%)


Coverage Fractions for Observed Kanji
=====================================
Percentage        Required
of Text           Kanji
-------------------------------------
   10                  7
   20                 20
   30                 36
   40                 65
   50                107
   60                174
   70                271
   80                429
   90                721
   91                766
   92                816
   93                869
   94                932
   95                999
   96               1085
   97               1174
   98               1307
   99               1515
  100               1781
```

## その他

- 改編前のプログラム名はkazoeru.cです。
- このプログラムは、MIT Licenseでお願いします。

(c) 2010-19 Hilofumi Yamamoto
