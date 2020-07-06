
#include "chess.h"

uint8_t pieceTypeFromChar(char c) {
    switch (c) {
        case 'k': return KING;
        case 'q': return QUEEN;
        case 'r': return ROOK;
        case 'b': return BISHOP;
        case 'n': return KNIGHT;
        case 'p': return PAWN;
        default: return INVALID;
    }
}

char pieceTypeToChar(uint8_t pt) {
    switch (pt) {
        case KING: return 'k';
        case QUEEN: return 'q';
        case ROOK: return 'r';
        case BISHOP: return 'b';
        case KNIGHT: return 'n';
        case PAWN: return 'p';
        case EMPTY: return 'e';
        default: return 'i';
    }
}

double getPieceScore(uint8_t pieceType) {
    switch (pieceType) {
        case QUEEN: return QUEEN_WEIGHT;
        case ROOK: return ROOK_WEIGHT;
        case BISHOP: return BISHOP_WEIGHT;
        case KNIGHT: return KNIGHT_WEIGHT;
        case PAWN: return PAWN_WEIGHT;
        default: return 0;
    }
}

double sumPieceList(const std::vector<PieceElement> &pieceList) {
    double sum = 0;
    for (const PieceElement &pe : pieceList) {
        sum += getPieceScore(pe.pieceType);
    }
    return sum;
}

double getPieceScoreChange(const Move &m) {
    if (m.captureType == 0 && m.promoteType == 0) {
        return 0;
    }

    if (m.captureType == 0) {
        if (m.promoteType == 0) {
            return getPieceScore(m.captureType);
        }
    } else {
        if (m.promoteType == 0) {
            return getPieceScore(m.promoteType) - getPieceScore(PAWN_WEIGHT);
        } else {
            return getPieceScore(m.captureType) + getPieceScore(m.promoteType) - getPieceScore(PAWN_WEIGHT);
        }
    }
}

std::ostream& operator<<(std::ostream &os, const Board &b) {
    for (int r = adjRank(8); r >= adjRank(1); r--) {
        for (int f = adjFile('a'); f <= adjFile('h'); f++) {
            os << b.getCharForBoardMapValue(r,f);
        }
        os << '\n';
    }
    return os;
}

bool spacesAreOnDiag(int sRank, int sFile, int tRank, int tFile) {
    return std::abs(sRank - tRank) == std::abs(sFile - tFile);
}

bool spacesAreOnDirectPath(int sRank, int sFile, int tRank, int tFile) {
    return sRank == tRank || sFile == tFile;
}

bool isDiagPathToSpace(const Board &b, int sRank, int sFile, int tRank, int tFile) {
    int dist = std::abs(tRank-sRank);
    int dRank = (tRank - sRank) / dist;
    int dFile = (tFile - sFile) / dist;

    int cRank = sRank;
    int cFile = sFile;
    while (true) {
        cRank += dRank;
        cFile += dFile;
        if (cRank == tRank) {
            return true;
        }
        if (b.boardMap[cRank][cFile] != EMPTY) {
            return false;
        }
    }
}

bool isDirectPathToSpace(const Board &b, int sRank, int sFile, int tRank, int tFile) {
    int dRank = sRank == tRank ? 0 : (tRank - sRank) / std::abs(tRank-sRank);
    int dFile = sFile == tFile ? 0 : (tFile - sFile) / std::abs(tFile-sFile);

    int cRank = sRank;
    int cFile = sFile;
    while (true) {
        cRank += dRank;
        cFile += dFile;

        if (cRank == tRank && cFile == tFile) {
            return true;
        }
        if (b.boardMap[cRank][cFile] != EMPTY) {
            return false;
        }
    }
}

bool isKnightAttacking(int sRank, int sFile, int tRank, int tFile) {
    int rDist = std::abs(tRank-sRank);
    int fDist = std::abs(tFile-sFile);
    if (rDist == 1) {
        return fDist == 2;
    } else if (rDist == 2) {
        return fDist == 1;
    } else {
        return false;
    }
}

bool isPawnAttacking(bool isWhite, int sRank, int sFile, int tRank, int tFile) {
    if (isWhite) {
        return std::abs(tFile-sFile) == 1 && tRank-sRank == 1;
    } else {
        return std::abs(tFile-sFile) == 1 && tRank-sRank == -1;
    }
}

bool isQueenAttacking(const Board &b, int sRank, int sFile, int tRank, int tFile) {
    return (spacesAreOnDiag(sRank, sFile, tRank, tFile) && isDiagPathToSpace(b, sRank, sFile, tRank, tFile))
           || (spacesAreOnDirectPath(sRank, sFile, tRank, tFile) && isDirectPathToSpace(b, sRank, sFile, tRank, tFile));
}

bool isAttacking(const Board &b, bool isWhite, const PieceElement &pe, int tRank, int tFile) {
    switch (pe.pieceType) {
        case KING:
            return std::abs(pe.rank - tRank) <= 1 && std::abs(pe.file - tFile) <= 1;
        case QUEEN:
            {
                bool res = isQueenAttacking(b, pe.rank, pe.file, tRank, tFile);
                return res;
            }
        case ROOK:
            return (spacesAreOnDirectPath(pe.rank, pe.file, tRank, tFile) && isDirectPathToSpace(b, pe.rank, pe.file, tRank, tFile));
        case BISHOP:
            return (spacesAreOnDiag(pe.rank, pe.file, tRank, tFile) && isDiagPathToSpace(b, pe.rank, pe.file, tRank, tFile));
        case KNIGHT:
            return isKnightAttacking(pe.rank, pe.file, tRank, tFile);
        case PAWN:
            return isPawnAttacking(isWhite, pe.rank, pe.file, tRank, tFile);
        default:
            return false;
    }
}

inline void tryAddMove(std::vector<Move> &moves, const Board &b, int sRank, int sFile, int tRank, int tFile, uint8_t pieceType) {
    uint8_t boardRes = b.boardMap[tRank][tFile];
    if (boardRes == INVALID) {
        return;
    }
    if (boardRes == EMPTY) {
        moves.emplace_back(Move(pieceType, sRank, sFile, tRank, tFile, EMPTY, 0, false));
        return;
    }

    if (b.whiteToMove) {
        if (boardRes >= BLACK_LIST_START) {
            uint8_t captureIdx = boardRes-BLACK_LIST_START;
            moves.emplace_back(Move(pieceType, sRank, sFile, tRank, tFile, b.blackPieces[captureIdx].pieceType, captureIdx, false));
        }
    } else {
        if (boardRes < BLACK_LIST_START) {
            uint8_t captureIdx = boardRes-WHITE_LIST_START;
            moves.emplace_back(Move(pieceType, sRank, sFile, tRank, tFile, b.whitePieces[captureIdx].pieceType, captureIdx, false));
        }
    }
}

void addMovesForKing(std::vector<Move> &moves, Board &b, int sRank, int sFile) {
    for (int dRank = -1; dRank <= 1; dRank++) {
        for (int dFile = -1; dFile <= 1; dFile++) {
            int cRank = sRank + dRank;
            int cFile = sFile + dFile;
            tryAddMove(moves, b, sRank, sFile, cRank, cFile, KING);
        }
    }
}

void getMovesForPath(std::vector<Move> &moves, const Board &b, int sRank, int sFile, uint8_t pieceType, int dRank, int dFile) {
    int cRank = sRank + dRank;
    int cFile = sFile + dFile;
    while(true) {
        uint8_t boardRes = b.boardMap[cRank][cFile];

        if (boardRes == EMPTY) {
            moves.emplace_back(Move(pieceType, sRank, sFile, cRank, cFile, EMPTY, 0, false));
            cRank += dRank;
            cFile += dFile;
            continue;
        }

        if (boardRes == INVALID) {
            return;
        }

        if (b.whiteToMove) {
            if (boardRes >= BLACK_LIST_START) {
                uint8_t captureIdx = boardRes-BLACK_LIST_START;
                moves.emplace_back(Move(pieceType, sRank, sFile, cRank, cFile, b.blackPieces[captureIdx].pieceType, captureIdx,  false));
            }
        } else {
            if (boardRes < BLACK_LIST_START) {
                uint8_t captureIdx = boardRes-WHITE_LIST_START;
                moves.emplace_back(Move(pieceType, sRank, sFile, cRank, cFile, b.whitePieces[boardRes-WHITE_LIST_START].pieceType, captureIdx, false));
            }
        }
        return;
    }
}

void addMovesForDiagonals(std::vector<Move> &moves, const Board &b, int sRank, int sFile, uint8_t pieceType, bool isPinned) {
    if (isPinned) {
        const PieceElement &k = b.whiteToMove ? b.whitePieces[0] : b.blackPieces[0];
        if (k.rank - sRank == 0 || k.file - sFile == 0) {
            return;
        }
        int dRank = (k.rank - sRank) / std::abs(k.rank - sRank);
        int dFile = (k.file - sFile) / std::abs(k.file - sFile);
        getMovesForPath(moves, b, sRank, sFile, pieceType, dRank, dFile);
        getMovesForPath(moves, b, sRank, sFile, pieceType, -dRank, -dFile);

    } else {
        getMovesForPath(moves, b, sRank, sFile, pieceType, -1, -1);
        getMovesForPath(moves, b, sRank, sFile, pieceType, -1, 1);
        getMovesForPath(moves, b, sRank, sFile, pieceType, 1, -1);
        getMovesForPath(moves, b, sRank, sFile, pieceType, 1, 1);
    }

}

void addMovesForDirectPath(std::vector<Move> &moves, const Board &b, int sRank, int sFile, uint8_t pieceType, bool isPinned) {
    if (isPinned) {
        const PieceElement &k = b.whiteToMove ? b.whitePieces[0] : b.blackPieces[0];
        if (k.rank - sRank == 0) {
            getMovesForPath(moves, b, sRank, sFile, pieceType, 0, -1);
            getMovesForPath(moves, b, sRank, sFile, pieceType, 0, 1);
        } else {
            getMovesForPath(moves, b, sRank, sFile, pieceType, -1, 0);
            getMovesForPath(moves, b, sRank, sFile, pieceType, 1, 0);
        }
    } else {
        getMovesForPath(moves, b, sRank, sFile, pieceType, -1, 0);
        getMovesForPath(moves, b, sRank, sFile, pieceType, 1, 0);
        getMovesForPath(moves, b, sRank, sFile, pieceType, 0, -1);
        getMovesForPath(moves, b, sRank, sFile, pieceType, 0, 1);
    }
}

void addMovesForQueen(std::vector<Move> &moves, const Board &b, int sRank, int sFile, bool isPinned) {
    addMovesForDiagonals(moves, b, sRank, sFile, QUEEN, isPinned);
    addMovesForDirectPath(moves, b, sRank, sFile, QUEEN, isPinned);
}

void addMovesForKnight(std::vector<Move> &moves, const Board &b, int sRank, int sFile) {
    for (int dRank = -2; dRank <= 2; dRank++) {
        if (dRank == 0) {
            continue;
        }

        int dFile = std::abs(dRank) == 2 ? 1 : 2;
        tryAddMove(moves, b, sRank, sFile, sRank+dRank, sFile-dFile, KNIGHT);
        tryAddMove(moves, b, sRank, sFile, sRank+dRank, sFile+dFile, KNIGHT);
    }
}

void addMovesForPawnCapture(std::vector<Move> &moves, const Board &b, int sRank, int sFile, int dRank, int dFile) {
    int tFile = sFile+dFile;
    int tRank = sRank+dRank;

    uint8_t boardRes = b.boardMap[tRank][tFile];
    if (boardRes == INVALID || boardRes == EMPTY) {
        return;
    }

    if (b.whiteToMove) {
        if (boardRes >= BLACK_LIST_START) {
            bool isPromote = tRank == adjRank(8);
            uint8_t captureIdx = boardRes-BLACK_LIST_START;
            moves.emplace_back(Move(PAWN, sRank, sFile, tRank, tFile, b.blackPieces[captureIdx].pieceType, captureIdx, isPromote));
        }
    } else {
        if (boardRes < BLACK_LIST_START) {
            bool isPromote = tRank == adjRank(1);
            uint8_t captureIdx = boardRes-WHITE_LIST_START;
            moves.emplace_back(Move(PAWN, sRank, sFile, tRank, tFile, b.whitePieces[captureIdx].pieceType, captureIdx, isPromote));
        }
    }
}

void addMovesForPawnAdvance(std::vector<Move> &moves, const Board &b, int sRank, int sFile, int dRank) {
    uint8_t boardRes = b.boardMap[sRank + dRank][sFile];
    if (boardRes != EMPTY) {
        return;
    }
    if (b.whiteToMove) {
        if (sRank > adjRank(2)) {
            bool isPromote = sRank + dRank == PADDING + 8 - 1;
            if (isPromote) {
                moves.emplace_back(Move(PAWN, sRank, sFile, sRank + dRank, sFile, EMPTY, 0, QUEEN));
                moves.emplace_back(Move(PAWN, sRank, sFile, sRank + dRank, sFile, EMPTY, 0, BISHOP));
                moves.emplace_back(Move(PAWN, sRank, sFile, sRank + dRank, sFile, EMPTY, 0, KNIGHT));
            } else {
                moves.emplace_back(Move(PAWN, sRank, sFile, sRank + dRank, sFile, EMPTY, 0, false));
            }
        } else {
            moves.emplace_back(Move(PAWN, sRank, sFile, sRank + dRank, sFile, EMPTY, 0, false));
            if (b.boardMap[sRank + (dRank * 2)][sFile] == EMPTY) {
                moves.emplace_back(Move(PAWN, sRank, sFile, sRank + (dRank * 2), sFile, EMPTY, 0, false));
            }
        }

    } else {
        if (sRank < adjRank(7)) {
            bool isPromote = sRank + dRank == PADDING + 8 - 1;
            if (isPromote) {
                moves.emplace_back(Move(PAWN, sRank, sFile, sRank + dRank, sFile, EMPTY, 0, QUEEN));
                moves.emplace_back(Move(PAWN, sRank, sFile, sRank + dRank, sFile, EMPTY, 0, BISHOP));
                moves.emplace_back(Move(PAWN, sRank, sFile, sRank + dRank, sFile, EMPTY, 0, KNIGHT));
            } else {
                moves.emplace_back(Move(PAWN, sRank, sFile, sRank + dRank, sFile, EMPTY, 0, false));
            }

        } else {
            moves.emplace_back(Move(PAWN, sRank, sFile, sRank + dRank, sFile, EMPTY, 0, false));
            if (b.boardMap[sRank + (dRank * 2)][sFile] == EMPTY) {
                moves.emplace_back(PAWN, sRank, sFile, sRank + (dRank * 2), sFile, EMPTY, 0, false);
            }
        }
    }
}

void addMovesForPawn(std::vector<Move> &moves, const Board &b, int sRank, int sFile, bool isPinned) {
    int dRank = b.whiteToMove ? 1 : -1;
    addMovesForPawnAdvance(moves, b, sRank, sFile, dRank);
    addMovesForPawnCapture(moves, b, sRank, sFile, dRank, -1);
    addMovesForPawnCapture(moves, b, sRank, sFile, dRank, 1);
}

void addMovesForPiece(std::vector<Move> &moves, Board &b, const PieceElement &pe, bool isPinned) {
    switch (pe.pieceType) {
        case KING:
            addMovesForKing(moves, b, pe.rank, pe.file);
            break;
        case QUEEN:
            addMovesForQueen(moves, b, pe.rank, pe.file, isPinned);
            break;
        case ROOK:
            addMovesForDirectPath(moves, b, pe.rank, pe.file, ROOK, isPinned);
            break;
        case BISHOP:
            addMovesForDiagonals(moves, b, pe.rank, pe.file, BISHOP, isPinned);
            break;
        case KNIGHT:
            addMovesForKnight(moves, b, pe.rank, pe.file);
            break;
        case PAWN:
            addMovesForPawn(moves, b, pe.rank, pe.file, isPinned);
            break;
        default:
            break;
    }
}

bool pawnCanAttack(int pRank, int pFile, int tRank, int tFile, bool isWhite) {
    if (isWhite) {
        return tRank-pRank == 1 && std::abs(tFile-pFile) == 1;
    } else {
        return tRank-pRank == -1 && std::abs(tFile-pFile) == 1;
    }
}

bool directionHasCheck(const Board &b, int sRank, int sFile, int dRank, int dFile, bool kingIsWhite) {
    int cRank = sRank;
    int cFile = sFile;
    while (true) {
        cRank += dRank;
        cFile += dFile;

        uint8_t res = b.boardMap[cRank][cFile];
        if (res == EMPTY) {
            continue;
        } else if (res == INVALID) {
            return false;
        } else {
            bool pieceIsWhite = res < BLACK_LIST_START;
            if (pieceIsWhite == kingIsWhite) {
                return false;
            } else {
                uint8_t pieceType = pieceIsWhite ?
                                    b.whitePieces[res-WHITE_LIST_START].pieceType :
                                    b.blackPieces[res-BLACK_LIST_START].pieceType;
                if (pieceType == KNIGHT) {
                    return false;
                } else if (pieceType == PAWN) {
                    return pawnCanAttack(cRank, cFile, sRank, sFile, pieceIsWhite);
                } else {
                    return true;
                }
            }
        }
    }
}

bool checkForPieceType(const Board &b, uint8_t targetPieceType, int rank, int file, bool targetColorIsWhite) {
    uint8_t res = b.boardMap[rank][file];
    if (res == EMPTY || res == INVALID) {
        return false;
    } else {
        bool pieceIsWhite = res < BLACK_LIST_START;
        if (pieceIsWhite != targetColorIsWhite) {
            return false;
        }

        uint8_t pieceType = pieceIsWhite ?
                            b.whitePieces[res-WHITE_LIST_START].pieceType :
                            b.blackPieces[res-BLACK_LIST_START].pieceType;

        return pieceType == targetPieceType;
    }
}

bool knightCanAttackPosition(const Board &b, int rank, int file, bool targetColorIsWhite) {
    for (int dRank = -2; dRank <= 2; dRank++) {
        if (dRank == 0) {
            continue;
        }

        int dFile = std::abs(dRank) == 2 ? 1 : 2;
        if (checkForPieceType(b, KNIGHT, rank+dRank, file-dFile, targetColorIsWhite) ||
                checkForPieceType(b, KNIGHT, rank+dRank, file+dFile, targetColorIsWhite)) {
            return true;
        }
    }
    return false;
}

bool inCheck(const Board &b, bool isWhite) {
    const PieceElement &king(isWhite ? b.whitePieces[0] : b.blackPieces[0]);

    return
            directionHasCheck(b, king.rank, king.file, 1, 1, isWhite) ||
            directionHasCheck(b, king.rank, king.file, 1, 0, isWhite) ||
            directionHasCheck(b, king.rank, king.file, 1, -1, isWhite) ||
            directionHasCheck(b, king.rank, king.file, 0, 1, isWhite) ||
            directionHasCheck(b, king.rank, king.file, 0, -1, isWhite) ||
            directionHasCheck(b, king.rank, king.file, -1, 1, isWhite) ||
            directionHasCheck(b, king.rank, king.file, -1, 0, isWhite) ||
            directionHasCheck(b, king.rank, king.file, -1, -1, isWhite) ||
            knightCanAttackPosition(b, king.rank, king.file, !isWhite);
}

std::vector<Move> getMoves(Board &b, const BoardContext &bc) {
    std::vector<Move> moves;
    for (PieceElement pe : b.whiteToMove ? b.whitePieces : b.blackPieces) {
        int pinIdx = getBitIdx(pe.rank, pe.file);
        if (pe.pieceType == KING) {
            std::vector<Move> kingMoves;
            addMovesForKing(kingMoves, b, pe.rank, pe.file);
            for (Move m : kingMoves) {
                b.doMove(m);
                if (!inCheck(b, !b.whiteToMove)) {
                    moves.push_back(m);
                }
                b.undoMove(m);
            }
        } else if (getNthBit(bc.absolutePinned, pinIdx)) {
            continue;
        } else {
            addMovesForPiece(moves, b, pe, getNthBit(bc.pinned, pinIdx));
        }
    }
    return moves;
}

bool comparePieceElement(const PieceElement &p1, const PieceElement &p2) {
    return p1.pieceType < p2.pieceType;
}

void Board::doMove(const Move &m) {
    uint8_t pieceIdx = boardMap[m.startRank][m.startFile];
    boardMap[m.startRank][m.startFile] = EMPTY;
    boardMap[m.destRank][m.destFile] = pieceIdx;
    PieceElement &pe = whiteToMove ? whitePieces[pieceIdx-WHITE_LIST_START] : blackPieces[pieceIdx-BLACK_LIST_START];
    pe.rank = m.destRank;
    pe.file = m.destFile;
    if (m.promoteType) {
        pe.pieceType = QUEEN;
    }

    if (m.captureType != EMPTY) {
        if (whiteToMove) {
            blackPieces[m.captureIdx].pieceType = CAPTURED;
        } else {
            whitePieces[m.captureIdx].pieceType = CAPTURED;
        }
    }
    whiteToMove = !whiteToMove;
}

void Board::undoMove(const Move &m) {
    whiteToMove = !whiteToMove;

    uint8_t pieceIdx = boardMap[m.destRank][m.destFile];
    boardMap[m.startRank][m.startFile] = pieceIdx;
    PieceElement &pe = whiteToMove ? whitePieces[pieceIdx-WHITE_LIST_START] : blackPieces[pieceIdx-BLACK_LIST_START];
    pe.rank = m.startRank;
    pe.file = m.startFile;
    if (m.promoteType) {
        pe.pieceType = PAWN;
    }

    uint8_t boardMapValue;
    if (m.captureType == EMPTY) {
        boardMapValue = EMPTY;
    } else {
        if (whiteToMove) {
            blackPieces[m.captureIdx].pieceType = m.captureType;
            boardMapValue = BLACK_LIST_START + m.captureIdx;

        } else {
            whitePieces[m.captureIdx].pieceType = m.captureType;
            boardMapValue = WHITE_LIST_START + m.captureIdx;
        }
    }
    boardMap[m.destRank][m.destFile] = boardMapValue;
}

char Board::getCharForBoardMapValue(int rank, int file) const {
    uint8_t res = boardMap[rank][file];
    if (res == EMPTY || res == INVALID) {
        return pieceTypeToChar(res);

    } else if (res >= BLACK_LIST_START) {
        int listIdx = res - BLACK_LIST_START;
        uint8_t pt = blackPieces[listIdx].pieceType;
        return pieceTypeToChar(pt);
    } else {
        int listIdx = res - WHITE_LIST_START;
        uint8_t pt = whitePieces[listIdx].pieceType;
        return std::toupper(pieceTypeToChar(pt));
    }
}

Move::Move(uint8_t pieceType, uint8_t startRank, uint8_t startFile, uint8_t destRank, uint8_t destFile,
           uint8_t captureType, uint8_t captureIdx, bool isPromote) : pieceType(pieceType), startRank(startRank),
                                                                      startFile(startFile), destRank(destRank),
                                                                      destFile(destFile), captureType(captureType),
                                                                      captureIdx(captureIdx), promoteType(isPromote) {}

bool Move::operator==(const Move &rhs) const {
    return pieceType == rhs.pieceType &&
           startRank == rhs.startRank &&
           startFile == rhs.startFile &&
           destRank == rhs.destRank &&
           destFile == rhs.destFile &&
           captureType == rhs.captureType &&
           captureIdx == rhs.captureIdx &&
           promoteType == rhs.promoteType;
}

double getPiecesScore(const Board &b) {
    return sumPieceList(b.whitePieces) - sumPieceList(b.blackPieces);
}

double getCheckmateScore(bool whiteCheckmate) {
    return (whiteCheckmate ? 1 : -1) * std::numeric_limits<double>::max();
}

bool isBetterEvaluationResult(const PositionEvaluation &e1, const PositionEvaluation &e2, bool whiteTurn) {
    if (whiteTurn) {
        if (e1.value > e2.value) {
            return true;
        } else if (e1.value == std::numeric_limits<double>::max()) {
            if (e1.bestMovePath.size() < e2.bestMovePath.size()) {
                return true;
            }
        } else if (e1.value == -std::numeric_limits<double>::max()) {
            if (e1.bestMovePath.size() > e2.bestMovePath.size()) {
                return true;
            }
        }
    } else {
        if (e1.value < e2.value) {
            return true;
        } else if (e1.value == -std::numeric_limits<double>::max()) {
            if (e1.bestMovePath.size() < e2.bestMovePath.size()) {
                return true;
            }
        } else if (e1.value == std::numeric_limits<double>::max()) {
            if (e1.bestMovePath.size() > e2.bestMovePath.size()) {
                return true;
            }
        }
    }
    return false;
}

PositionEvaluation evaluateHelper(Board &b, int depth, double pieceScore, Statistics &stats) {
    stats.methodCalls++;
    if (depth == 0) {
        stats.leafNodesReached++;
        return PositionEvaluation(pieceScore, std::vector<Move>());
    }

    BoardContext bc(b);
    std::vector<Move> moves = getMoves(b, bc);
    if (moves.empty()) {
        if (inCheck(b, b.whiteToMove)) {
            stats.checkMateEvaluations++;
            return PositionEvaluation(getCheckmateScore(!b.whiteToMove), std::vector<Move>());
        } else {
            stats.staleMateEvaluations++;
            return PositionEvaluation(0, std::vector<Move>());
        }
    }

    Move bestMove;
    PositionEvaluation best(0, std::vector<Move>());
    bool haveBest = false;

    for (const Move &m : moves) {
        b.doMove(m);
        double newPieceScore = pieceScore + (b.whiteToMove ? 1 : -1) * getPieceScoreChange(m);
        PositionEvaluation res = evaluateHelper(b, depth - 1, newPieceScore, stats);
        b.undoMove(m);

        if (!haveBest || isBetterEvaluationResult(res, best, b.whiteToMove)) {
            best = res;
            bestMove = m;
            haveBest = true;
        }
    }

    best.bestMovePath.insert(best.bestMovePath.begin(), bestMove);

    return best;
}

Evaluation evaluateBoard(Board &b, int maxDepth) {
    Evaluation e;
    auto start = std::chrono::system_clock::now();
    double pieceScore = sumPieceList(b.whitePieces) - sumPieceList(b.blackPieces);

    e.pos = evaluateHelper(b, maxDepth, pieceScore, e.stats);
    auto end = std::chrono::system_clock::now();
    e.stats.evaluationDurationMillis = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    return e;
}


std::ostream &operator<<(std::ostream &os, const PieceElement &pe) {
    return os << '(' << pieceTypeToChar(pe.pieceType) << ',' << (int)pe.rank << ',' << (int)pe.file << ')';
}

std::ostream &operator<<(std::ostream &os, const std::vector<PieceElement> &pList) {
    os << '{';
    auto it = pList.begin();
    if (it != pList.end()) {
        os << *it;
        it++;
    }
    for (; it != pList.end(); it++) {
        os << ',' << *it;
    }
    return os << '}';
}

std::ostream &operator<<(std::ostream &os, const std::vector<Move> &mList) {
    os << '{';
    auto it = mList.begin();
    if (it != mList.end()) {
        os << *it;
        it++;
    }
    for (; it != mList.end(); it++) {
        os << ',' << *it;
    }
    return os << '}';
}

std::ostream &operator<<(std::ostream &os, const Move &m) {
    os << pieceTypeToChar(m.pieceType);
    if (m.captureType != EMPTY) {
        os << 'x';
    }
    return os << unAdjFile(m.startFile)
    << unAdjRank(m.startRank)
    << unAdjFile(m.destFile)
    << unAdjRank(m.destRank);
}

std::ostream &operator<<(std::ostream &os, const Statistics &s) {
    os << "executionTimeMillis: " << s.evaluationDurationMillis << " functionCalls: " << s.methodCalls << " leafNodes: " << s.leafNodesReached;
    return os;
}

PositionEvaluation::PositionEvaluation(double value, const std::vector<Move> &bestMovePath) : value(value),
                                                                                          bestMovePath(bestMovePath) {}
bool PieceElement::operator==(const PieceElement &rhs) const {
    return pieceType == rhs.pieceType &&
           rank == rhs.rank &&
           file == rhs.file;
}

bool Board::operator==(const Board &rhs) const {
    bool boardMapsAreEqual = true;
    for (int r = 0; r < 12; r++) {
        bool shouldBreak = false;
        for (int f = 0; f < 12; f++) {
            if (boardMap[r][f] != rhs.boardMap[r][f]) {
                boardMapsAreEqual = false;
                shouldBreak = true;
                break;
            }
        }
        if (shouldBreak) {
            break;
        }
    }

    return boardMapsAreEqual &&
        whitePieces == rhs.whitePieces &&
        blackPieces == rhs.blackPieces &&
        whiteToMove == rhs.whiteToMove;
}

std::string evaluationValueToString(const PositionEvaluation &res) {
    if (std::fabs(res.value) == std::numeric_limits<double>::max()) {
        return std::string("#") + std::to_string((res.bestMovePath.size()+1)/2);
    } else {
        return std::to_string(res.value);
    }
}

Board::Board(const Board &rhs) : whitePieces(rhs.whitePieces), blackPieces(rhs.blackPieces), whiteToMove(rhs.whiteToMove) {
    for (int r = 0; r < 12; r++) {
        for (int f = 0; f < 12; f++) {
            boardMap[r][f] = rhs.boardMap[r][f];
        }
    }
}

std::string Board::toFen() const {
    std::string fen;
    for (int r = adjRank(8); r >= adjRank(1); r--) {
        int curEmpty = 0;
        for (int f = adjFile('a'); f <= adjFile('h'); f++) {
            uint8_t res = boardMap[r][f];
            if (res == EMPTY) {
                curEmpty++;
            } else {
                if (res < BLACK_LIST_START) {
                    uint8_t pieceType = whitePieces[res-WHITE_LIST_START].pieceType;
                    char c = pieceTypeToChar(pieceType);
                    fen += std::toupper(c);
                } else {
                    uint8_t pieceType = blackPieces[res-BLACK_LIST_START].pieceType;
                    char c = pieceTypeToChar(pieceType);
                    fen += c;
                }

            }
        }
        if (curEmpty > 0) {
            fen += std::to_string(curEmpty);
        }
        if (r != adjRank(1)) {
            fen += "/";
        }
    }
    fen += " ";
    fen += (whiteToMove ? "w" : "b");
    return fen;
}

PieceElement& Board::pieceElementForBoardValue(uint8_t boardRes) {
    if (boardRes < BLACK_LIST_START) {
        return whitePieces[boardRes-WHITE_LIST_START];
    } else {
        return blackPieces[boardRes-BLACK_LIST_START];
    }
}

const PieceElement& Board::pieceElementForBoardValue(uint8_t boardRes) const {
    if (boardRes < BLACK_LIST_START) {
        return whitePieces[boardRes-WHITE_LIST_START];
    } else {
        return blackPieces[boardRes-BLACK_LIST_START];
    }
}

Move moveFromString(const std::string &s, const Board &b) {
    uint8_t pieceType = pieceTypeFromChar(s[0]);
    uint8_t sRank;
    uint8_t sFile;
    uint8_t dRank;
    uint8_t dFile;
    uint8_t captureIdx;
    uint8_t captureType;

    if (s[1] == 'x') {
        sFile = adjFile(s[2]);
        sRank = adjRank(s[3]-'0');
        dFile = adjFile(s[4]);
        dRank = adjRank(s[5]-'0');
        captureIdx = b.boardMap[dRank][dFile] - (b.whiteToMove ? BLACK_LIST_START : WHITE_LIST_START);
        captureType = (b.whiteToMove ? b.blackPieces[captureIdx] : b.whitePieces[captureIdx]).pieceType;
    } else {
        sFile = adjFile(s[1]);
        sRank = adjRank(s[2]-'0');
        dFile = adjFile(s[3]);
        dRank = adjRank(s[4]-'0');
        captureIdx = 0;
        captureType = EMPTY;
    }
    bool isPromote = pieceType == PAWN && dRank == (b.whiteToMove ? adjRank(8) : adjRank(1));
    return {pieceType, sRank, sFile, dRank, dFile, captureType, captureIdx, isPromote};
}

void printBitBoard(uint64_t bitBoard) {
    for (int r = adjRank(8); r >= adjRank(1); r--) {
        for (int f = adjFile('a'); f <= adjFile('h'); f++) {
            std::cout << (getBitBoardBit(bitBoard, r, f) ? "1" : "0");
        }
        std::cout << '\n';
    }
    std::cout << std::endl;
}


Board::Board(std::string fen) {
    {
        for (int r = 0; r < 12; r++) {
            for (int f = 0; f < 12 ; f++) {
                if (r < 2 || r >= 10 || f < 2 || f >= 10) {
                    boardMap[r][f] = INVALID;
                }
            }
        }

        int rank = PADDING+8-1;
        int file = PADDING;

        bool afterSpace = false;

        for (auto it = fen.begin(); it != fen.end(); it++)
        {
            bool shouldContinue = false;
            switch (*it) {
                case ' ':
                    afterSpace = true;
                    shouldContinue = true;
                    break;
                case '/':
                    rank--;
                    file = PADDING;
                    shouldContinue = true;
                    break;
            }
            if (shouldContinue) {
                continue;
            }

            if (afterSpace) {
                whiteToMove = *it == 'w';
                break;
            }


            if (std::isdigit(*it)) {
                int value = *it - '0';
                int endFile = file+value;

                for (; file < endFile; file++) {
                    boardMap[rank][file] = EMPTY;
                }
                continue;
            }

            uint8_t pieceType = pieceTypeFromChar(std::tolower(*it));
            if (std::islower(*it)) {
//                boardMap[rank][file] = BLACK_LIST_START + blackPieces.size();
                blackPieces.emplace_back(pieceType, rank, file);
            } else {
//                boardMap[rank][file] = WHITE_LIST_START + whitePieces.size();
                whitePieces.emplace_back(pieceType, rank, file);
            }
            file++;
        }

        std::sort(whitePieces.begin(), whitePieces.end(), comparePieceElement);
        std::sort(blackPieces.begin(), blackPieces.end(), comparePieceElement);

        for (int i = 0; i < blackPieces.size(); i++) {
            PieceElement pe = blackPieces[i];
            boardMap[pe.rank][pe.file] = BLACK_LIST_START + i;
        }
        for (int i = 0; i < whitePieces.size(); i++) {
            PieceElement pe = whitePieces[i];
            boardMap[pe.rank][pe.file] = WHITE_LIST_START + i;
        }
    }
}

void test() {

    std::cout << getBitIdx(2, 2) << " " <<  getBitIdx(9, 9) << std::endl;

//    Board b("8/8/8/8/5k2/7r/7N/5qRK w - - 0 1");
    Board b("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

//    std::vector<Move> m1 = getMoves(b);
//    std::vector<Move> m2 = getLegalMoves(b);
//
//    std::cout << m1 << std::endl;
//    std::cout << m2 << std::endl;
//
//    std::cout << (m1 == m2);
}

BoardContext::BoardContext(const Board &b) {
    const PieceElement &k(b.whiteToMove ? b.whitePieces[0] : b.blackPieces[0]);
    updatePinnedPiecesForDirection(b, k, 1, 1);
    updatePinnedPiecesForDirection(b, k, 1, 0);
    updatePinnedPiecesForDirection(b, k, 1, -1);
    updatePinnedPiecesForDirection(b, k, 0, 1);
    updatePinnedPiecesForDirection(b, k, 0, -1);
    updatePinnedPiecesForDirection(b, k, -1, 1);
    updatePinnedPiecesForDirection(b, k, -1, 0);
    updatePinnedPiecesForDirection(b, k, -1, 1);
}

void BoardContext::updatePinnedPiecesForDirection(const Board &b, const PieceElement &k, int dRank, int dFile) {
    int cRank = k.rank;
    int cFile = k.file;

    bool isDiag = !(dRank == 0 || dFile == 0);

    const PieceElement* pinnedPiecePtr = nullptr;
    while (true) {
        cRank += dRank;
        cFile += dFile;

        uint8_t res = b.boardMap[cRank][cFile];
        if (res == EMPTY) {
            continue;
        } else if (res == INVALID) {
            return;
        } else {
            bool pieceIsWhite = res < BLACK_LIST_START;
            if (pieceIsWhite == b.whiteToMove) {
                if (pinnedPiecePtr != nullptr) {
                    return;
                }
                pinnedPiecePtr = &b.pieceElementForBoardValue(res);

            } else {

                const PieceElement &pe = b.pieceElementForBoardValue(res);
                if (pe.pieceType == QUEEN || (isDiag ? (pe.pieceType == BISHOP) : (pe.pieceType == ROOK))) {
                    if (pinnedPiecePtr == nullptr) {
                        return;
                    }
                    int pinIdx = getBitIdx(pinnedPiecePtr->rank, pinnedPiecePtr->file);
                    if (pinnedPiecePtr->pieceType == QUEEN || (isDiag ? (pinnedPiecePtr->pieceType == BISHOP) : (pinnedPiecePtr->pieceType == ROOK))) {
                        setNthBit(pinned, pinIdx);
                    } else {
                        setNthBit(absolutePinned, pinIdx);
                    }
                }
                return;
            }

        }
    }
}
