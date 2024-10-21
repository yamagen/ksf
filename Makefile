# コンパイラとフラグ
CC      = gcc
CFLAGS  = -Wall -O2
LDFLAGS = 
TARGET  = ksf
SRCS    = ksf.c
OBJS    = $(SRCS:.c=.o)

# バージョンとアーカイブ名
VERSION = 1.0.0
TARBALL = $(TARGET)-$(VERSION).tar.gz

# インストールディレクトリ (デフォルトは /usr/local/bin)
PREFIX  = /usr/local
BINDIR  = $(PREFIX)/bin

# デフォルトターゲット
all: $(TARGET)

# ターゲットの生成ルール
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

# .c から .o への変換ルール
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# クリーンアップ
clean:
	rm -f $(OBJS) $(TARGET)

# インストール
install: $(TARGET)
	install -d $(BINDIR)
	install $(TARGET) $(BINDIR)

# アンインストール
uninstall:
	rm -f $(BINDIR)/$(TARGET)

# デバッグ用
debug: CFLAGS += -g
debug: clean all

# tarball (ソースコードのtar.gzアーカイブを作成)
tarball:
	tar -czvf $(TARBALL) $(SRCS) Makefile README.md

.PHONY: all clean install uninstall debug tarball
