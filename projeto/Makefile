CC = gcc
CFLAGS = -Wall -Wextra -O2 -fopenmp

SRC_ATM = analyzer_par_atomic.c
SRC_PAD = analyzer_par_atomic_padded.c
SRC_CRT = analyzer_par_critical.c
SRC_LCK = analyzer_par_lock.c
SRC_SEQ = analyzer_seq.c
SRC_INCLUDE = hash_table.c

TARGET_SEQ = analyzer_seq
TARGET_ATM = analyzer_par_atomic
TARGET_CRT = analyzer_par_critical
TARGET_LCK = analyzer_par_lock
TARGET_PAD = analyzer_par_atomic_padded
TARGET_EXT = .out

TARGETS = \
	$(TARGET_SEQ)$(TARGET_EXT) \
	$(TARGET_ATM)$(TARGET_EXT) \
	$(TARGET_CRT)$(TARGET_EXT) \
	$(TARGET_LCK)$(TARGET_EXT) \
	$(TARGET_PAD)$(TARGET_EXT)

# Alvo principal: compila todos os executáveis
all:
	$(CC) $(CFLAGS) $(SRC_SEQ) $(SRC_INCLUDE) -o $(TARGET_SEQ)$(TARGET_EXT)
	$(CC) $(CFLAGS) $(SRC_ATM) $(SRC_INCLUDE) -o $(TARGET_ATM)$(TARGET_EXT)
	$(CC) $(CFLAGS) $(SRC_CRT) $(SRC_INCLUDE) -o $(TARGET_CRT)$(TARGET_EXT)
	$(CC) $(CFLAGS) $(SRC_LCK) $(SRC_INCLUDE) -o $(TARGET_LCK)$(TARGET_EXT)

# Regra para o Sequencial
analyzer_seq: analyzer_seq.c hash_table.c
	$(CC) $(CFLAGS) $(SRC_SEQ) $(SRC_INCLUDE) -o $(TARGET_SEQ)$(TARGET_EXT)

# Regra para Atomics
analyzer_par_atomic: analyzer_par_atomic.c hash_table.c
	$(CC) $(CFLAGS) $(SRC_ATM) $(SRC_INCLUDE) -o $(TARGET_ATM)$(TARGET_EXT)

# Regra para Critical
analyzer_par_critical: analyzer_par_critical.c hash_table.c
	$(CC) $(CFLAGS) $(SRC_CRT) $(SRC_INCLUDE) -o $(TARGET_CRT)$(TARGET_EXT)

# Regra para Bucket Lock
analyzer_par_lock: analyzer_par_lock.c hash_table.c
	$(CC) $(CFLAGS) $(SRC_LCK) $(SRC_INCLUDE) -o $(TARGET_LCK)$(TARGET_EXT)

# Regra para Padded
analyzer_par_pad: analyzer_par_atomic_padded.c hash_table.c
	$(CC) $(CFLAGS) $(SRC_PAD) $(SRC_INCLUDE) -o $(TARGET_PAD)$(TARGET_EXT)

# Limpa os arquivos executáveis
clean:
	rm -f $(TARGET_SEQ)$(TARGET_EXT)
	rm -f $(TARGET_CRT)$(TARGET_EXT)
	rm -f $(TARGET_LCK)$(TARGET_EXT)
	rm -f $(TARGET_ATM)$(TARGET_EXT)
	rm -f $(TARGET_PAD)$(TARGET_EXT)