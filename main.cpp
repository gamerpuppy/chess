#include <iostream>
#include "chess.h"

const std::string START_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

void play(std::string fen, bool playerIsWhite, int depth) {
    Board board(fen);
    while (true) {
//        std::cout << board << '\n';
        if (board.whiteToMove && playerIsWhite) {
            EvaluationResult res = evaluateBoard(board, 1);
            if (res.bestMovePath.empty()) {
                std::cout << evaluationValueToString(res) << '\n';
                return;
            }
            std::string moveStr;
            std::cin >> moveStr;
            Move move = moveFromString(moveStr, board);
            board.doMove(move);
        } else {
            EvaluationResult res = evaluateBoard(board, depth);
            if (res.bestMovePath.empty()) {
                std::cout << evaluationValueToString(res) << '\n';
                return;
            }
            Move move = res.bestMovePath[0];
            std::cout << move << '\n';
            board.doMove(move);
        }
    }
}


int main(int argc, const char* argv[]) {
    if (argc < 4) {
        std::cout << "Usage: fen playerColor engineDepth";
    }
    std::string fen = argv[1];
    bool playerIsWhite = std::string(argv[2]) == "w";
    int depth = std::stoi(argv[3]);

    play(fen, playerIsWhite, depth);

    return 0;
}