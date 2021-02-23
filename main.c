#include <stdio.h>
#include <assert.h>
#include "ass4.h"
#define MAX_FEN_LEN 100
#define MAX_PGN_LEN 10
#define MAX_GAME_LEN 200

void readGame(const char* filename, char fen[], char pgn[][MAX_PGN_LEN], int* gameLength) {
    FILE* input = fopen(filename, "r");
    fscanf(input, "%s\n", fen);
    for (*gameLength = 0; fscanf(input, "%s\n", pgn[*gameLength]) != EOF; ++ * gameLength);
    fclose(input);
}

void printGame(char fen[], char pgn[][MAX_PGN_LEN], int gameLength) {
    printf("fen: %s\npgn:\n", fen);
    for (int i = 0; i < gameLength; i++) {
        printf("%s", pgn[i]);
        printf("\n");
    }
}

int main(int argc, char const* argv[]) {
    //assert(argc == 2);
    int gameLength;
    char fen[MAX_FEN_LEN];
    char pgn[MAX_GAME_LEN][MAX_PGN_LEN];
    readGame("\\8 board cases - splitted\\long-2.txt", fen, pgn, &gameLength);
    // printGame(fen, pgn, gameLength);

    char board[SIZE][SIZE];
    createBoard(board, fen);
    printBoard(board);

    for (int i = 0; i < gameLength; i++) {
        printf("%d %s: %s\n", (i + 2) / 2, i % 2 ? "Black" : "White", pgn[i]);
        if (makeMove(board, pgn[i], !(i % 2))) {
            printBoard(board);
        }
        else {
            printf("Illegal Move!\n");
            break;
        }
    }
    return 0;
}
