# Nome do executável
TARGET = barbeiro_dorminhoco

# Compilador
CC = gcc

# Flags do compilador
CFLAGS = -pthread -Wall -Wextra

# Arquivo fonte
SRC = barbeiro_dorminhoco.c

# Regra padrão para compilar e gerar o executável
all: $(TARGET)

# Regra para gerar o executável
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# Regra para limpar os arquivos gerados
clean:
	rm -f $(TARGET)

# Regra para executar o programa
run: $(TARGET)
	./$(TARGET)

