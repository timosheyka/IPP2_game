/** @file
 * Implementation of game's engine
 * 
 * @author Tsimafei Lukashevich
 * @copyright University of Warsaw
 * @date 2023
*/

#include "game.h"

// Players limit
#define MAX_PLAYERS 35

/** @brief Representation of board's square
 * player - number of player on this field
 * parent_id - number that represents that area it belongs to
*/
struct pair {
    uint32_t player;
    uint32_t parent_id;
};
typedef struct pair pair_t;

/** @brief Representation of player
 * boundary - number of free fields around player's areas
 * busy_areas - number of areas that player used in the game
 * completed_moves - number of pawns that player set on the board
*/
struct player {
    uint64_t boundary;
    uint32_t busy_areas;
    uint64_t completed_moves;
};
typedef struct player player_t;

/** @brief Representation of game's engine 
 * width - board's width
 * height - board's height
 * areas - number that limits creating independent areas
 * 
 * board - 2d array of pairs that stores information about the fields
 * neighbours - 4 element array that stores information about <x,y> neighbours
 * 
 * players - array of players participating in the game
 * players_num - number of players participating in the game
*/
struct game {
    pair_t ** board;
    player_t * players;
    uint32_t * neighbours;

    uint32_t width;
    uint32_t height;
    uint32_t areas;
    uint32_t players_num;
};
typedef struct game game_t;

game_t * game_new(uint32_t width, uint32_t height,
                    uint32_t players, uint32_t areas) {
    if (!width || !height || !players || !areas) { return NULL; }
    if (players > MAX_PLAYERS) { return NULL; }

    game_t * g = (game_t *) malloc(sizeof(game_t));
    if (g == NULL) { return NULL; }

    g->width = width;
    g->height = height;
    g->areas = areas;

    g->players_num = players;
    g->players = (player_t *) calloc(g->players_num, sizeof(player_t));
    if (g->players == NULL) { return NULL; }
    g->neighbours = (uint32_t *) calloc(4, sizeof(uint32_t));
    if (g->neighbours == NULL) { return NULL; }

    g->board = (pair_t **) malloc(width * sizeof(pair_t *));
    if (g->board == NULL) { return NULL; }

    for (size_t i = 0; i < width; i++) {
        g->board[i] = (pair_t *) calloc(height, sizeof(pair_t));
        if (g->board[i] == NULL) { return NULL; }
    }

    return g;
}

void game_delete(game_t *g) {
    if (g == NULL) { return; }

    for (size_t i = 0; i < g->width; i++) { free(g->board[i]); }
    free(g->board);

    free(g->neighbours);
    free(g->players);
    free(g);
}

/** @brief valid_coordinate 
 * defines whether the fields exists
 * @param[in] width - board width
 * @param[in] height - board height
 * @param[in] x - column's number
 * @param[in] y - row's number 
 * @return @p true if fields exists and 
 * @p false if coordinates are incorrect
*/
static bool valid_coordinate(uint32_t width, uint32_t height,
                                uint32_t x, uint32_t y) {
    return !(x >= width || y >= height);
}

/** @brief isSurrounded.
 * Calculates symbol of fields that connects to <x,y> 
 * @param[in] board - representation of game board
 * @param[in] width - board width
 * @param[in] height - board height
 * @param[in] player - player's number
 * @param[in] x - column's number
 * @param[in] y - row's number  
 * @return number of player's fields around <x,y>
*/
static uint32_t isSurrounded(pair_t ** const board, uint64_t width,
                    uint64_t height, uint64_t player, uint64_t x, uint64_t y) {
    uint32_t around = 0;
    if (x > 0 && valid_coordinate(width, height, x - 1, y)) {
        if (board[x - 1][y].player == player) { around++; }
    }
    if (x + 1 < width && valid_coordinate(width, height, x + 1, y)) {
        if (board[x + 1][y].player == player) { around++; }
    }
    if (y > 0 && valid_coordinate(width, height, x, y - 1)) {
        if (board[x][y - 1].player == player) { around++; }
    }
    if (y + 1 < height && valid_coordinate(width, height, x, y + 1)) {
        if (board[x][y + 1].player == player) { around++; }
    }
    return around;
}

/** @brief common_free_fields.
 * Calculates fields that are in a "1" distance with field <x,y>
 * @param[in] board - representation of game board
 * @param[in] width - board width
 * @param[in] height - board height
 * @param[in] player - player's number
 * @param[in] x - column's number
 * @param[in] y - row's number  
 * @return number of player's fields in a distance
*/
static uint32_t common_free_fields(pair_t ** const board, uint32_t width,
                    uint32_t height, uint32_t player, uint32_t x, uint32_t y) {
    uint32_t common = 0;
    if (x > 1 && valid_coordinate(width, height, x - 2, y)) {
        if (board[x - 2][y].player == player) { common++; }
    }
    if (x + 2 < width && valid_coordinate(width, height, x + 2, y)) {
        if (board[x + 2][y].player == player) { common++; }
    }
    if (y > 1 && valid_coordinate(width, height, x, y - 2)) {
        if (board[x][y - 2].player == player) { common++; }
    }
    if (y + 2 < height && valid_coordinate(width, height, x, y + 2)) {
        if (board[x][y + 2].player == player) { common++; }
    }
    return common;
}

/** @brief update_strangers_boundary 
 * decreases player's boundary whether it connects to
 * different player's field
 * @param[in] g - pointer to game structure
 * @param[in] player - player's number
 * @param[in] x - column's number
 * @param[in] y - row's number 
*/
static void update_strangers_boundary(game_t *g, uint32_t player,
                                        uint32_t x, uint32_t y) {
    uint32_t stranger = g->board[x][y].player;
    if (!stranger) { return; }
    if (valid_coordinate(g->width, g->height, x, y) && stranger != player) {
        g->players[stranger - 1].boundary--;
    }
}

/** @brief update_strangers.
 * Helper to @ref update_strangers_boundary.
 * @param[in] g - pointer to game structure
 * @param[in] player - player's number
 * @param[in] x - column's number
 * @param[in] y - row's number
*/
static void update_strangers(game_t *g, uint32_t player, uint32_t x, uint32_t y) {
    if (x > 0) { update_strangers_boundary(g, player, x - 1, y); }
    if (x + 1 < g->width) { update_strangers_boundary(g, player, x + 1, y); }
    if (y > 0) { update_strangers_boundary(g, player, x, y - 1); }
    if (y + 1 < g->height) { update_strangers_boundary(g, player, x, y + 1); }
}


/** @brief diagonal_neighbours.
 * Calculates existing fields on the diagonals of <x,y>
 * @param[in] g - pointer to game structure
 * @param[in] player - player's number
 * @param[in] x - column's number
 * @param[in] y - row's number
 * @return non-zero diagonal fields
*/
static uint32_t diagonal_neighbours(game_t *g, uint32_t player,
                                        uint32_t x, uint32_t y) {
    uint32_t diagonal = 0;
    if (x > 0) {
        if (y > 0) {
            if (g->board[x - 1][y - 1].player == player) {
                if (g->board[x][y - 1].player == 0) { diagonal++; }
                if (g->board[x - 1][y].player == 0) { diagonal++; }
            }
        }
    }
    if (x + 1 < g->width) {
        if (y + 1 < g->height) {
            if (g->board[x + 1][y + 1].player == player) {
                if (g->board[x][y + 1].player == 0) { diagonal++; }
                if (g->board[x + 1][y].player == 0) { diagonal++; }
            }
        }
    }
    return diagonal;
}

/** @brief different_areas.
 * Calcutes how many different areas are in <x,y> surroundings
 * @param[in] neighbours - array of surrounding fields
 * @param[in] around - number of non-zero fields around <x,y>
 * @return different_areas result
*/
static uint32_t different_areas(const uint32_t * neighbours, uint32_t around) {
    if (around == 4) { return around; }

    uint32_t areas = 0;
    for (int i = 0; i < 3; i++) {
        for (int j = i + 1; j < 4 && areas < around; j++) {
            if (neighbours[i] && neighbours[j] && neighbours[i] != neighbours[j]) {
                areas++;
            }
        }
    }
    return areas == 0 ? 1 : areas;
}

/** @brief BFS (breadth first search).
 * Updates id for united area
 * @param[in] g - pointer to game structure
 * @param[in] id - id of given field
 * @param[in] player - player's number
 * @param[in] x - column's number
 * @param[in] y - row's number
*/
static void BFS(game_t* g, uint32_t id, uint32_t player, uint32_t x, uint32_t y) {
    if (!valid_coordinate(g->width, g->height, x, y)) { return; }
    if (g->board[x][y].player == player && g->board[x][y].parent_id != id) {
        g->board[x][y].parent_id = id;
        BFS(g, id, player, x - 1, y);
        BFS(g, id, player, x + 1, y);
        BFS(g, id, player, x, y - 1);
        BFS(g, id, player, x, y + 1);
    }
}

/** @brief set_id.
 * defines new id for united area
 * @param[in] neighbours - array of surrounding fields
 * @return new id
*/
static uint32_t set_id(uint32_t * neighbours) {
    if (neighbours[0]) { return neighbours[0]; }
    if (neighbours[1]) { return neighbours[1]; }
    if (neighbours[2]) { return neighbours[2]; }
    if (neighbours[3]) { return neighbours[3]; }
    return 0;
}

/** @brief find_neighbours.
 * Search if fields around belongs to same player and writes field id
 * @param[in] g - pointer to game structure
 * @param[in] player - player's number
 * @param[in] x - column's number
 * @param[in] y - row's number
*/
static void find_neighbours(game_t *g, uint32_t player, uint32_t x, uint32_t y) {
    g->neighbours[0] = (x > 0 && g->board[x - 1][y].player == player)
                         ? g->board[x - 1][y].parent_id : 0;
    g->neighbours[1] = (x + 1 < g->width && g->board[x + 1][y].player == player)
                         ? g->board[x + 1][y].parent_id : 0;
    g->neighbours[2] = (y > 0 && g->board[x][y - 1].player == player)
                         ? g->board[x][y - 1].parent_id : 0;
    g->neighbours[3] = (y + 1 < g->height && g->board[x][y + 1].player == player)
                         ? g->board[x][y + 1].parent_id : 0;
}

bool game_move(game_t *g, uint32_t player, uint32_t x, uint32_t y) {
    // game structure correctness
    if (g == NULL || g->players == NULL || g->players_num < player) { return false; }
    // coordinates correctness
    if (!valid_coordinate(g->width, g->height, x, y)) { return false; }
    // free field
    if (g->board[x][y].player != 0) { return false; }

    uint32_t around = isSurrounded(g->board, g->width, g->height, player, x, y);

    if (!around) {
        if (g->players[player - 1].busy_areas == g->areas) {
            return false;
        } else {
            // is an "island"
            g->board[x][y].player = player;
            g->players[player - 1].busy_areas++;
            g->board[x][y].parent_id = g->players[player - 1].busy_areas;

            g->players[player - 1].completed_moves++;
            g->players[player - 1].boundary += 
                        isSurrounded(g->board, g->width, g->height, 0, x, y);
            g->players[player - 1].boundary -= 
                        common_free_fields(g->board, g->width, g->height, player, x, y);
            g->players[player - 1].boundary -= 
                        diagonal_neighbours(g, player, x, y);

            update_strangers(g, player, x, y);

            return true;
        }
    }

    find_neighbours(g, player, x, y);

    if (different_areas(g->neighbours, around) == 1) {
        if (around > 1) { g->players[player - 1].busy_areas -= around - 1; }
        // adjust to existing area
        for (int i = 0; i < 4; i++) {
            if (g->neighbours[i]) {
                g->board[x][y].player = player;
                g->board[x][y].parent_id = g->neighbours[i];
                g->players[player - 1].boundary--;
                break;
            }
        }
    } else {
        // union at least 2 areas

        g->board[x][y].player = player;
        g->board[x][y].parent_id = set_id(g->neighbours);
        g->players[player - 1].busy_areas -= 
                    different_areas(g->neighbours, around) - 1;

        g->players[player - 1].boundary--;

        BFS(g, g->board[x][y].parent_id, player, x - 1, y);
        BFS(g, g->board[x][y].parent_id, player, x + 1, y);
        BFS(g, g->board[x][y].parent_id, player, x, y - 1);
        BFS(g, g->board[x][y].parent_id, player, x, y + 1);
    }

    // update game's info

    g->players[player - 1].completed_moves++;

    g->players[player - 1].boundary += 
                isSurrounded(g->board, g->width, g->height, 0, x, y);
    g->players[player - 1].boundary -= 
                common_free_fields(g->board, g->width, g->height, player, x, y);
    g->players[player - 1].boundary -= 
                diagonal_neighbours(g, player, x, y);

    update_strangers(g, player, x, y);

    return true;
}

uint64_t game_busy_fields(game_t const *g, uint32_t player) {
    return (g == NULL || g->players == NULL || g->players_num < player)
           ? 0 : g->players[player - 1].completed_moves;
}

uint64_t game_free_fields(game_t const *g, uint32_t player) {
    if (g == NULL || g->players == NULL || g->players_num < player) { return 0; }

    player_t player_tmp = g->players[player - 1];
    if (player_tmp.busy_areas == g->areas) {
        return player_tmp.boundary;
    } else {
        uint64_t all_fields = (uint64_t) g->height * (uint64_t) g->width;
        uint64_t busy_fields_all = 0;
        for (uint32_t p = 0; p < g->players_num; p++) {
            busy_fields_all += (uint64_t) g->players[p].completed_moves;
        }
        return all_fields - busy_fields_all;
    }
}

uint32_t game_board_width(game_t const *g) {
    return g == NULL ? 0 : g->width;
}

uint32_t game_board_height(game_t const *g) {
    return g == NULL ? 0 : g->height;
}

uint32_t game_players(game_t const *g) {
    return g == NULL ? 0 : g->players_num;
}

char game_player(game_t const *g, uint32_t player) {
    if (g == NULL || game_players(g) < player || player == 0) { return '.'; }
    return player <= 9 ? player + '0' : player - 10 + 'a';
}

char * game_board(game_t const *g) {
    if (g == NULL) {
        return NULL;
    }

    char * board = (char *) malloc((uint64_t) ((uint64_t) g->width + 1)
                                   * (uint64_t) g->height + 1);

    if (board == NULL) {
        errno = ENOMEM;
        return NULL;
    }
    else {
        uint64_t idx = 0;
        for (uint32_t i = g->height - 1; i + 1 > 0; i--) {
            for (uint32_t j = 0; j < g->width; j++) {
                board[idx] = game_player(g, g->board[j][i].player); idx++;
            }
            board[idx] = '\n'; idx++;
        }
        board[(uint64_t) ((uint64_t) g->width + 1) * (uint64_t) g->height] = '\0';

        return board;
    }
}