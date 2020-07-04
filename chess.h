//
// Created by keega on 6/17/2020.
//

#ifndef CHESS_CHESS_H
#define CHESS_CHESS_H

#include <stdint-gcc.h>
#include <string>
#include <vector>
#include <assert.h>
#include <iostream>
#include <algorithm>
#include <limits>

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

#define adjRank(rank) ((int)(rank)+PADDING-1)
#define adjFile(file) ((char)(file)-'a'+PADDING)

#define unAdjRank(rank) ((int)(rank)-PADDING+1)
#define unAdjFile(file) ((char)((int)(file)+'a'-PADDING))


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
    bool isPromote;

    Move(){}

    Move(uint8_t pieceType, uint8_t startRank, uint8_t startFile, uint8_t destRank, uint8_t destFile,
         uint8_t captureType, uint8_t captureIdx, bool isPromote);
};

struct Board {
    std::vector<PieceElement> whitePieces;
    std::vector<PieceElement> blackPieces;
    uint8_t boardMap[12][12];
    bool whiteToMove;

//    int16_t moveCount;
//    int8_t whiteCastling;
//    int8_t blackCastling;
//    int16_t halfMoveCount;
//    uint16_t enPassant;

    explicit Board (std::string fen);
    Board(const Board &rhs);

    bool operator==(const Board &rhs) const;
    void doMove(const Move &m);
    void undoMove(const Move &m);
    char getCharForBoardMapValue(int rank, int file) const;
};

struct EvaluationResult {
    double value;
    std::vector<Move> bestMovePath;
    EvaluationResult(double value, const std::vector<Move> &bestMovePath);
};

std::ostream& operator<<(std::ostream &os, const Board &b);
std::ostream& operator<<(std::ostream &os, const PieceElement &pe);
std::ostream& operator<<(std::ostream &os, const std::vector<PieceElement> &pList);
std::ostream& operator<<(std::ostream &os, const Move &m);
std::ostream& operator<<(std::ostream &os, const std::vector<Move> &mList);


double sumPieceList(const std::vector<PieceElement> &pieceList);
EvaluationResult evaluateBoard(Board &b, int maxDepth);
std::string evaluationValueToString(const EvaluationResult &res);
Move moveFromString(const std::string &s, const Board &b);

#endif //CHESS_CHESS_H
