#include <iostream>
#include "chess.h"

const std::string START_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

void play(std::string fen, bool playerIsWhite, int depth) {
    Board board(fen);
    while (true) {
//        std::cout << board << '\n';
        if (board.whiteToMove ==  playerIsWhite) {
            Evaluation res = evaluateBoard(board, 1);
            if (res.pos.bestMovePath.empty()) {
                std::cout << evaluationValueToString(res.pos) << '\n';
                return;
            }
            std::string moveStr;
            std::cin >> moveStr;
            Move move = moveFromString(moveStr, board);
            board.doMove(move);
        } else {
            Evaluation res = evaluateBoard(board, depth);
            if (res.pos.bestMovePath.empty()) {
                std::cout << evaluationValueToString(res.pos) << '\n';
                return;
            }
            std::cout << res.stats << '\n';
            Move move = res.pos.bestMovePath[0];
            std::cout << move << std::endl;
            board.doMove(move);
        }
    }
}

int main(int argc, const char* argv[]) {
    if (argc < 4) {
        std::cout << "Usage: fen playerColor engineDepth";
    }

    std::string fen = argv[1];
    if (fen == "start") {
        fen = START_FEN;
    }

    bool playerIsWhite = std::string(argv[2]) == "w";
    int depth = std::stoi(argv[3]);

    play(fen, playerIsWhite, depth);

    return 0;

//    test();
//
//    uint64_t  bitBoard = 0;
//    int rank = 2;
//    int file = 4;
//
//    setBitBoardBit(bitBoard, rank, file);
//    std::cout << getBitIdx(rank, file) << '\n';
//    std::cout << getBitBoardBit(bitBoard, rank, file);

}