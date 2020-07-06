//
// Created by keega on 6/17/2020.
//

#ifndef CHESS_CHESS_H
#define CHESS_CHESS_H

#include <cmath>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <limits>
#include <chrono>

const uint8_t EMPTY = 16;
const uint8_t KING = 1;
const uint8_t QUEEN = 2;
const uint8_t ROOK = 3;
const uint8_t BISHOP = 4;
const uint8_t KNIGHT = 5;
const uint8_t PAWN = 6;
const uint8_t INVALID = 0xFFu;
const uint8_t CAPTURED = 0xFEu;

const uint8_t WHITE_LIST_START = 0;
const uint8_t BLACK_LIST_START = 17;
const uint8_t PADDING = 2;

const double QUEEN_WEIGHT = 10;
const double ROOK_WEIGHT = 5;
const double BISHOP_WEIGHT = 3.1;
const double KNIGHT_WEIGHT = 3;
const double PAWN_WEIGHT = 1;

#define adjRank(rank) ((int)(rank)+PADDING-1)
#define adjFile(file) ((char)(file)-'a'+PADDING)

#define unAdjRank(rank) ((int)(rank)-PADDING+1)
#define unAdjFile(file) ((char)((int)(file)+'a'-PADDING))

#define getBitIdx(rank, file) ((((rank)-PADDING)*8) + (file)-PADDING)
#define setNthBit(bitmap, n) ((bitmap) |= (1lu << (n)))
#define getNthBit(bitmap, n) (((bitmap) >> n) & 1lu)

#define setBitBoardBit(bitBoard, rank, file) (setNthBit(bitBoard, getBitIdx(rank, file)))
#define getBitBoardBit(bitBoard, rank, file) (getNthBit(bitBoard, getBitIdx(rank, file)))

uint8_t pieceTypeFromChar(char c);
char pieceTypeToChar(uint8_t pt);

struct PieceElement {
    uint8_t pieceType;
    uint8_t rank;
    uint8_t file;

    PieceElement(uint8_t pieceType, uint8_t rank, uint8_t file) : pieceType(pieceType), rank(rank), file(file) {}
    bool operator==(const PieceElement &rhs) const;
};

bool comparePieceElement(const PieceElement &p1, const PieceElement &p2);

struct Move {
    uint8_t pieceType;
    uint8_t startRank;
    uint8_t startFile;
    uint8_t destRank;
    uint8_t destFile;
    uint8_t captureType;
    uint8_t captureIdx;
    uint8_t promoteType;

    Move(){}

    Move(uint8_t pieceType, uint8_t startRank, uint8_t startFile, uint8_t destRank, uint8_t destFile,
         uint8_t captureType, uint8_t captureIdx, bool isPromote);

    bool operator==(const Move &rhs) const;
};

struct Board {
    std::vector<PieceElement> whitePieces;
    std::vector<PieceElement> blackPieces;
    uint8_t boardMap[12][12];
    bool whiteToMove;

//    uint64_t attackedSpaces;
//    int16_t moveCount;
//    int8_t whiteCastling;
//    int8_t blackCastling;
//    int16_t halfMoveCount;
//    uint16_t enPassant;

    explicit Board (std::string fen);
    Board(const Board &rhs);

    void doMove(const Move &m);
    void undoMove(const Move &m);

    PieceElement& pieceElementForBoardValue(uint8_t boardRes);
    const PieceElement& pieceElementForBoardValue(uint8_t boardRes) const;

    bool operator==(const Board &rhs) const;
    char getCharForBoardMapValue(int rank, int file) const;
    std::string toFen() const;
};

struct BoardContext {
    uint64_t pinned = 0;
    uint64_t absolutePinned = 0;

    explicit BoardContext(const Board &b);
    void updatePinnedPiecesForDirection(const Board &b, const PieceElement &k, int dRank, int dFile);
};

struct Statistics {
    long leafNodesReached = 0;
    long methodCalls = 0;
    long checkMateEvaluations = 0;
    long staleMateEvaluations = 0;
    long evaluationDurationMillis = 0;
};

struct PositionEvaluation {
    double value;
    std::vector<Move> bestMovePath;

    PositionEvaluation() = default;
    PositionEvaluation(double value, const std::vector<Move> &bestMovePath);
};

struct Evaluation {
    Statistics stats;
    PositionEvaluation pos;
};

std::ostream& operator<<(std::ostream &os, const Board &b);
std::ostream& operator<<(std::ostream &os, const PieceElement &pe);
std::ostream& operator<<(std::ostream &os, const std::vector<PieceElement> &pList);
std::ostream& operator<<(std::ostream &os, const Move &m);
std::ostream& operator<<(std::ostream &os, const std::vector<Move> &mList);
std::ostream& operator<<(std::ostream &os, const Statistics &s);


bool inCheck(const Board &b, bool isWhite);
double sumPieceList(const std::vector<PieceElement> &pieceList);
std::string evaluationValueToString(const PositionEvaluation &res);
Move moveFromString(const std::string &s, const Board &b);
Evaluation evaluateBoard(Board &b, int maxDepth);
void test();

void printBitBoard(uint64_t bitBoard);

#endif //CHESS_CHESS_H
