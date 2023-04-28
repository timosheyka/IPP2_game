/** @file
 * Przykładowe użycie silnika gry
 *
 * @author Marcin Peczarski <marpe@mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 2023
 */

/**
 * W tym pliku nawet w wersji release chcemy korzystać z asercji.
 */
#ifdef NDEBUG
#undef NDEBUG
#endif

#include "game.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Tak ma wyglądać plansza po wykonaniu całego poniższego przykładu.
 */
static const char board[] =
  "1.........\n"
  "..........\n"
  "..........\n"
  "......2...\n"
  ".....1....\n"
  "..........\n"
  "..........\n"
  "1.........\n"
  "1222......\n"
  "1.........\n";

/** @brief Testuje silnik gry.
 * Przeprowadza przykładowe testy silnika gry.
 * @return Zero, gdy wszystkie testy przebiegły poprawnie,
 * a w przeciwnym przypadku kod błędu.
 */
int main() {
  game_t *g;

  g = game_new(0, 0, 0, 0);
  assert(g == NULL);

  g = game_new(10, 10, 2, 3);
  assert(g != NULL);

  assert(game_move(g, 1, 0, 0));
  assert(game_busy_fields(g, 1) == 1);
  assert(game_busy_fields(g, 2) == 0);
  assert(game_free_fields(g, 1) == 99);
  assert(game_free_fields(g, 2) == 99);
  assert(game_move(g, 2, 3, 1));
  assert(game_busy_fields(g, 1) == 1);
  assert(game_busy_fields(g, 2) == 1);
  assert(game_free_fields(g, 1) == 98);
  assert(game_free_fields(g, 2) == 98);
  assert(game_move(g, 1, 0, 2));
  assert(game_move(g, 1, 0, 9));
  assert(!game_move(g, 1, 5, 5));
  assert(game_free_fields(g, 1) == 6);
  assert(game_move(g, 1, 0, 1));
  assert(game_free_fields(g, 1) == 95);
  assert(game_move(g, 1, 5, 5));
  assert(!game_move(g, 1, 6, 6));
  assert(game_busy_fields(g, 1) == 5);
  assert(game_free_fields(g, 1) == 10);
  assert(game_move(g, 2, 2, 1));
  assert(game_move(g, 2, 1, 1));
  assert(game_free_fields(g, 1) == 9);
  assert(game_free_fields(g, 2) == 92);
  assert(!game_move(g, 2, 0, 1));
  assert(game_move(g, 2, 6, 6));
  assert(game_busy_fields(g, 1) == 5);
  assert(game_free_fields(g, 1) == 9);
  assert(game_busy_fields(g, 2) == 4);
  assert(game_free_fields(g, 2) == 91);

  char *p = game_board(g);
  assert(p);
  assert(strcmp(p, board) == 0);
  printf(p);
  free(p);

  game_delete(g);
  return 0;
}
