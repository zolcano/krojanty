# Variables de Configuration
CC = gcc
SRC_DIR = src
OBJ_DIR = build
DOC_DIR = documentation
TEST_DIR = tests
TARGET = game

# Argument par défaut pour 'make run'. Peut être surchargé en ligne de commande.
# Exemple : make run ARGS="-s 5555"
ARGS ?= -l

# Fichiers Source

# --- Fichiers pour l'application principale ---
# On exclut la logique pure (testée séparément) si elle n'est pas nécessaire directement
# Note : ia.c et jeu_logique.c sont nécessaires, donc on les inclut.
# Le filtrage est plus utile si certains fichiers sont VRAIMENT que pour les tests.
# Pour ce projet, tous les fichiers .c de src/ sont nécessaires à l'application.
SRCS_APP = $(wildcard $(SRC_DIR)/*.c)
OBJS_APP = $(SRCS_APP:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

#  Fichiers pour les tests unitaires 
# Test pour la logique du jeu (jeu_logique.c)
TEST_JEU_SRCS = $(TEST_DIR)/test_jeu.c $(SRC_DIR)/jeu_logique.c
TEST_JEU_TARGET = test_runner_jeu

# Test pour l'IA (ia.c)
TEST_IA_SRCS = $(TEST_DIR)/test_ia.c $(SRC_DIR)/ia.c
TEST_IA_TARGET = test_runner_ia

# Flags de Compilation et de Liaison

# Flags pour l'application principale (GTK4)
PKG_CFLAGS = $(shell pkg-config --cflags gtk4)
PKG_LIBS = $(shell pkg-config --libs gtk4)

CFLAGS = -Wall -Wextra -Iinclude $(PKG_CFLAGS)
LIBS = $(PKG_LIBS) -lpthread

# Flags spécifiques pour la compilation des tests (avec couverture de code)
TEST_CFLAGS = -Wall -Wextra -Iinclude $(PKG_CFLAGS) -fprofile-arcs -ftest-coverage
TEST_LIBS = --coverage


# Cibles Principales
.PHONY: all run clean docs tests coverage tests-jeu tests-ia

all: $(TARGET)

# Cible pour l'application principale
$(TARGET): $(OBJS_APP)
	$(CC) $^ -o $@ $(LIBS)

# Règle générique pour compiler les fichiers sources en fichiers objets
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Cible pour exécuter l'application
run: $(TARGET)
	./$(TARGET) $(ARGS)

# Cibles de Test et de Couverture

# Cible pour lancer tous les tests
tests: tests-jeu tests-ia

# Cible pour le test de la logique du jeu
tests-jeu: $(TEST_JEU_TARGET)
	./$(TEST_JEU_TARGET)

$(TEST_JEU_TARGET): $(TEST_JEU_SRCS)
	$(CC) $(TEST_CFLAGS) $^ -o $@ $(TEST_LIBS)

# Cible pour le test de l'IA
tests-ia: $(TEST_IA_TARGET)
	./$(TEST_IA_TARGET)

$(TEST_IA_TARGET): $(TEST_IA_SRCS)
	$(CC) $(TEST_CFLAGS) $^ -o $@ $(TEST_LIBS)

# Cible pour générer le rapport de couverture de code
coverage: tests
	@echo "Génération du rapport de couverture..."
	@mkdir -p coverage_report
	@lcov --capture --directory . --output-file coverage.info --rc lcov_branch_coverage=1
	@lcov --ignore-errors unused --remove coverage.info '/usr/*' '*/tests/*' --output-file coverage.info --rc lcov_branch_coverage=1
	@genhtml coverage.info --output-directory coverage_report --branch-coverage
	@echo "Rapport généré dans 'coverage_report/index.html'"


# Cibles Utilitaires

# Cible de nettoyage complète
clean:
	rm -rf $(OBJ_DIR) $(TARGET) $(TEST_JEU_TARGET) $(TEST_IA_TARGET) *.gcda *.gcno coverage.info coverage_report documentation/html documentation/latex

# Cible pour générer la documentation avec Doxygen
docs:
	@echo "Génération de la documentation"
	@(cd $(DOC_DIR) && doxygen Doxyfile)