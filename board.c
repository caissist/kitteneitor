#include <stdio.h>
#include "defs.h"
#include "data.h"
#include "protos.h"

//#define NDEBUG
//#include <assert.h>



/*
 ****************************************************************************
 * Move generator *
 ****************************************************************************
 */
void Gen_Push(int from, int dest, int type, MOVE *pBuf, int *pMCount)
{
    MOVE move;
    move.from = from;
    move.dest = dest;
    move.type_of_move = type;
    pBuf[*pMCount] = move;
    *pMCount = *pMCount + 1;
}

void Gen_PushNormal(int from, int dest, MOVE *pBuf, int *pMCount)
{
    Gen_Push(from, dest, MOVE_TYPE_NORMAL, pBuf, pMCount);
}

/* Especial cases for Pawn */

/* Pawn can promote */
void Gen_PushPawn(int from, int dest, MOVE *pBuf, int *pMCount)
{
    if (piece[dest] == EPS_SQUARE)
    {
        Gen_Push(from, dest, MOVE_TYPE_EPS, pBuf, pMCount);
    }
    /* The 7 and 56 are to limit pawns to the 2nd through 7th ranks, which
     * means this isn't a promotion, i.e., a normal pawn move */
    else if (dest > 7 && dest < 56)
    {
        Gen_Push(from, dest, MOVE_TYPE_NORMAL, pBuf, pMCount);
    }
    else /* otherwise it's a promotion */
    {
        Gen_Push(from, dest, MOVE_TYPE_PROMOTION_TO_QUEEN, pBuf, pMCount);
        Gen_Push(from, dest, MOVE_TYPE_PROMOTION_TO_ROOK, pBuf, pMCount);
        Gen_Push(from, dest, MOVE_TYPE_PROMOTION_TO_BISHOP, pBuf, pMCount);
        Gen_Push(from, dest, MOVE_TYPE_PROMOTION_TO_KNIGHT, pBuf, pMCount);
    }
}

/* When a pawn moves two squares then appears the possibility of the en passant capture*/
void Gen_PushPawnTwo(int from, int dest, MOVE *pBuf, int *pMCount)
{
    Gen_Push(from, dest, MOVE_TYPE_PAWN_TWO, pBuf, pMCount);
}

/* Especial cases for King */
void Gen_PushKing(int from, int dest, MOVE *pBuf, int *pMCount)
{
    /* Is it a castle?*/
    if (from == E1 && (dest == G1 || dest == C1)) /* this is a white castle */
    {
        Gen_Push(from, dest, MOVE_TYPE_CASTLE, pBuf, pMCount);
    }
    else if (from == E8 && (dest == G8 || dest == C8)) /* this is a black castle */
    {
        Gen_Push(from, dest, MOVE_TYPE_CASTLE, pBuf, pMCount);
    }
    else /* otherwise it's a normal king's move */
    {
        Gen_Push(from, dest, MOVE_TYPE_NORMAL, pBuf, pMCount);
    }
}

/* Gen all moves of current_side to move and push them to pBuf, and return number of moves */
int GenMoves(int current_side, MOVE *pBuf)
{
    int i; /* Counter for the board squares */
    int k; /* Counter for cols */
    int y;
    int row;
    int col;
    int movecount;
    movecount = 0;

    for (i = 0; i < 64; i++) /* Scan all board */
        if (color[i] == current_side)
        {
            switch (piece[i])
            {

            case PAWN:
                col = COL(i);
                row = ROW(i);
                if (current_side == BLACK)
                {
                    if (color[i + 8] == EMPTY)
                        /* Pawn advances one square.
                         * We use Gen_PushPawn because it can be a promotion */
                        Gen_PushPawn(i, i + 8, pBuf, &movecount);
                    if (row == 1 && color[i + 8] == EMPTY && color[i + 16] == EMPTY)
                        /* Pawn advances two squares */
                        Gen_PushPawnTwo(i, i + 16, pBuf, &movecount);
                    if (col && color[i + 7] == WHITE)
                        /* Pawn captures and it can be a promotion*/
                        Gen_PushPawn(i, i + 7, pBuf, &movecount);
                    if (col < 7 && color[i + 9] == WHITE)
                        /* Pawn captures and can be a promotion*/
                        Gen_PushPawn(i, i + 9, pBuf, &movecount);
                        /* For en passant capture */
                    if (col && piece[i + 7] == EPS_SQUARE)
                        /* Pawn captures and it can be a promotion*/
                        Gen_PushPawn(i, i + 7, pBuf, &movecount);
                    if (col < 7 && piece[i + 9] == EPS_SQUARE)
                        /* Pawn captures and can be a promotion*/
                        Gen_PushPawn(i, i + 9, pBuf, &movecount);
                }
                else
                {
                    if (color[i - 8] == EMPTY)
                        Gen_PushPawn(i, i - 8, pBuf, &movecount);
                    /* Pawn moves 2 squares */
                    if (row == 6 && color[i - 8] == EMPTY && color[i - 16] == EMPTY)
                        Gen_PushPawnTwo(i, i - 16, pBuf, &movecount);
                    /* For captures */
                    if (col && color[i - 9] == BLACK)
                        Gen_PushPawn(i, i - 9, pBuf, &movecount);
                    if (col < 7 && color[i - 7] == BLACK)
                        Gen_PushPawn(i, i - 7, pBuf, &movecount);
                    /* For en passant capture */
                    if (col && piece[i - 9] == EPS_SQUARE)
                        Gen_PushPawn(i, i - 9, pBuf, &movecount);
                    if (col < 7 && piece[i - 7] == EPS_SQUARE)
                        Gen_PushPawn(i, i - 7, pBuf, &movecount);
                }
                break;

            case QUEEN: /* == BISHOP+ROOK */

            case BISHOP:
                for (y = i - 9; y >= 0 && COL(y) != 7; y -= 9)
                { /* go left up */
                    if (color[y] != current_side)
                        Gen_PushNormal(i, y, pBuf, &movecount);
                    if (color[y] != EMPTY)
                        break;
                }
                for (y = i - 7; y >= 0 && COL(y) != 0; y -= 7)
                { /* go right up */
                    if (color[y] != current_side)
                        Gen_PushNormal(i, y, pBuf, &movecount);
                    if (color[y] != EMPTY)
                        break;
                }
                for (y = i + 9; y < 64 && COL(y) != 0; y += 9)
                { /* go right down */
                    if (color[y] != current_side)
                        Gen_PushNormal(i, y, pBuf, &movecount);
                    if (color[y] != EMPTY)
                        break;
                }
                for (y = i + 7; y < 64 && COL(y) != 7; y += 7)
                { /* go left down */
                    if (color[y] != current_side)
                        Gen_PushNormal(i, y, pBuf, &movecount);
                    if (color[y] != EMPTY)
                        break;
                }
                if (piece[i] == BISHOP) /* In the case of the bishop we're done */
                    break;

                /* FALL THROUGH FOR QUEEN {I meant to do that!} ;-) */
            case ROOK:
                col = COL(i);
                for (k = i - col, y = i - 1; y >= k; y--)
                { /* go left */
                    if (color[y] != current_side)
                        Gen_PushNormal(i, y, pBuf, &movecount);
                    if (color[y] != EMPTY)
                        break;
                }
                for (k = i - col + 7, y = i + 1; y <= k; y++)
                { /* go right */
                    if (color[y] != current_side)
                        Gen_PushNormal(i, y, pBuf, &movecount);
                    if (color[y] != EMPTY)
                        break;
                }
                for (y = i - 8; y >= 0; y -= 8)
                { /* go up */
                    if (color[y] != current_side)
                        Gen_PushNormal(i, y, pBuf, &movecount);
                    if (color[y] != EMPTY)
                        break;
                }
                for (y = i + 8; y < 64; y += 8)
                { /* go down */
                    if (color[y] != current_side)
                        Gen_PushNormal(i, y, pBuf, &movecount);
                    if (color[y] != EMPTY)
                        break;
                }
                break;

            case KNIGHT:
                col = COL(i);
                y = i - 6;
                if (y >= 0 && col < 6 && color[y] != current_side)
                    Gen_PushNormal(i, y, pBuf, &movecount);
                y = i - 10;
                if (y >= 0 && col > 1 && color[y] != current_side)
                    Gen_PushNormal(i, y, pBuf, &movecount);
                y = i - 15;
                if (y >= 0 && col < 7 && color[y] != current_side)
                    Gen_PushNormal(i, y, pBuf, &movecount);
                y = i - 17;
                if (y >= 0 && col > 0 && color[y] != current_side)
                    Gen_PushNormal(i, y, pBuf, &movecount);
                y = i + 6;
                if (y < 64 && col > 1 && color[y] != current_side)
                    Gen_PushNormal(i, y, pBuf, &movecount);
                y = i + 10;
                if (y < 64 && col < 6 && color[y] != current_side)
                    Gen_PushNormal(i, y, pBuf, &movecount);
                y = i + 15;
                if (y < 64 && col > 0 && color[y] != current_side)
                    Gen_PushNormal(i, y, pBuf, &movecount);
                y = i + 17;
                if (y < 64 && col < 7 && color[y] != current_side)
                    Gen_PushNormal(i, y, pBuf, &movecount);
                break;

            case KING:
                /* the column and rank checks are to make sure it is on the board*/
                /* The 'normal' moves*/
                col = COL(i);
                if (col && color[i - 1] != current_side)
                    Gen_PushKing(i, i - 1, pBuf, &movecount); /* left */
                if (col < 7 && color[i + 1] != current_side)
                    Gen_PushKing(i, i + 1, pBuf, &movecount); /* right */
                if (i > 7 && color[i - 8] != current_side)
                    Gen_PushKing(i, i - 8, pBuf, &movecount); /* up */
                if (i < 56 && color[i + 8] != current_side)
                    Gen_PushKing(i, i + 8, pBuf, &movecount); /* down */
                if (col && i > 7 && color[i - 9] != current_side)
                    Gen_PushKing(i, i - 9, pBuf, &movecount); /* left up */
                if (col < 7 && i > 7 && color[i - 7] != current_side)
                    Gen_PushKing(i, i - 7, pBuf, &movecount); /* right up */
                if (col && i < 56 && color[i + 7] != current_side)
                    Gen_PushKing(i, i + 7, pBuf, &movecount); /* left down */
                if (col < 7 && i < 56 && color[i + 9] != current_side)
                    Gen_PushKing(i, i + 9, pBuf, &movecount); /* right down */

                /* The castle moves*/
                if (current_side == WHITE)
                {
                    /* Can white short castle? */
                    if (castle & 1)
                    {
                        /* If white can castle the white king has to be in square 60 */
                        if (col &&
                            color[i + 1] == EMPTY &&
                            color[i + 2] == EMPTY &&
                            !IsInCheck(current_side) &&
                            !IsAttacked(current_side, i + 1))
                        {
                            /* The king goes 2 sq to the left */
                            Gen_PushKing(i, i + 2, pBuf, &movecount);
                        }
                    }

                    /* Can white long castle? */
                    if (castle & 2)
                    {
                        if (col &&
                            color[i - 1] == EMPTY &&
                            color[i - 2] == EMPTY &&
                            color[i - 3] == EMPTY &&
                            !IsInCheck(current_side) &&
                            !IsAttacked(current_side, i - 1))
                        {
                            /* The king goes 2 sq to the left */
                            Gen_PushKing(i, i - 2, pBuf, &movecount);
                        }
                    }
                }
                else if (current_side == BLACK)
                {
                    /* Can black short castle? */
                    if (castle & 4)
                    {
                        /* If white can castle the white king has to be in square 60 */
                        if (col &&
                                color[i + 1] == EMPTY &&
                                color[i + 2] == EMPTY &&
                                piece[i + 3] == ROOK &&
                                !IsInCheck(current_side) &&
                                !IsAttacked(current_side, i + 1))
                        {
                            /* The king goes 2 sq to the left */
                            Gen_PushKing(i, i + 2, pBuf, &movecount);
                        }
                    }
                    /* Can black long castle? */
                    if (castle & 8)
                    {
                        if (col &&
                                color[i - 1] == EMPTY &&
                                color[i - 2] == EMPTY &&
                                color[i - 3] == EMPTY &&
                                piece[i - 4] == ROOK &&
                                !IsInCheck(current_side) &&
                                !IsAttacked(current_side, i - 1))
                        {
                            /* The king goes 2 sq to the left */
                            Gen_PushKing(i, i - 2, pBuf, &movecount);
                        }
                    }
                }

                break;
//                default:
//                printf("Piece type unknown, %d", piece[i]);
                // assert(false);
            }
        }
    return movecount;
}

/* Gen all captures of current_side to move and push them to pBuf, return number of moves
 * It's necesary at least ir order to use quiescent in the search */
int GenCaps(int current_side, MOVE *pBuf)
{
    int i; /* Counter for the board squares */
    int k; /* Counter for cols */
    int y;
    int row;
    int col;
    int capscount; /* Counter for the posible captures */
    int xside;
    xside = (WHITE + BLACK) - current_side;
    capscount = 0;

    for (i = 0; i < 64; i++) /* Scan all board */
        if (color[i] == current_side)
        {
            switch (piece[i])
            {

            case PAWN:
                col = COL(i);
                row = ROW(i);
                if (current_side == BLACK)
                {
                    /* This isn't a capture, but it's necesary in order to
                     * not oversee promotions */
                    if (row > 7 && color[i + 8] == EMPTY)
                        /* Pawn advances one square.
                         * We use Gen_PushPawn because it can be a promotion */
                        Gen_PushPawn(i, i + 8, pBuf, &capscount);
                    if (col && color[i + 7] == WHITE)
                        /* Pawn captures and it can be a promotion*/
                        Gen_PushPawn(i, i + 7, pBuf, &capscount);
                    if (col < 7 && color[i + 9] == WHITE)
                        /* Pawn captures and can be a promotion*/
                        Gen_PushPawn(i, i + 9, pBuf, &capscount);
                    /* For en passant capture */
                    if (col && piece[i + 7] == EPS_SQUARE)
                        /* Pawn captures and it can be a promotion*/
                        Gen_PushPawn(i, i + 7, pBuf, &capscount);
                    if (col < 7 && piece[i + 9] == EPS_SQUARE)
                        /* Pawn captures and can be a promotion*/
                        Gen_PushPawn(i, i + 9, pBuf, &capscount);
                }
                else if (current_side == WHITE)
                {
                    if (row < 2 && color[i - 8] == EMPTY)
                    /* This isn't a capture, but it's necesary in order to
                     * not oversee promotions */
                        Gen_PushPawn(i, i - 8, pBuf, &capscount);
                    /* For captures */
                    if (col && color[i - 9] == BLACK)
                        Gen_PushPawn(i, i - 9, pBuf, &capscount);
                    if (col < 7 && color[i - 7] == BLACK)
                        Gen_PushPawn(i, i - 7, pBuf, &capscount);
                    /* For en passant capture */
                    if (col && piece[i - 9] == EPS_SQUARE)
                        Gen_PushPawn(i, i - 9, pBuf, &capscount);
                    if (col < 7 && piece[i - 7] == EPS_SQUARE)
                        Gen_PushPawn(i, i - 7, pBuf, &capscount);
                }
                break;

            case KNIGHT:
                col = COL(i);
                y = i - 6;
                if (y >= 0 && col < 6 && color[y] == xside)
                    Gen_PushNormal(i, y, pBuf, &capscount);
                y = i - 10;
                if (y >= 0 && col > 1 && color[y] == xside)
                    Gen_PushNormal(i, y, pBuf, &capscount);
                y = i - 15;
                if (y >= 0 && col < 7 && color[y] == xside)
                    Gen_PushNormal(i, y, pBuf, &capscount);
                y = i - 17;
                if (y >= 0 && col > 0 && color[y] == xside)
                    Gen_PushNormal(i, y, pBuf, &capscount);
                y = i + 6;
                if (y < 64 && col > 1 && color[y] == xside)
                    Gen_PushNormal(i, y, pBuf, &capscount);
                y = i + 10;
                if (y < 64 && col < 6 && color[y] == xside)
                    Gen_PushNormal(i, y, pBuf, &capscount);
                y = i + 15;
                if (y < 64 && col > 0 && color[y] == xside)
                    Gen_PushNormal(i, y, pBuf, &capscount);
                y = i + 17;
                if (y < 64 && col < 7 && color[y] == xside)
                    Gen_PushNormal(i, y, pBuf, &capscount);
                break;

            case QUEEN: /* == BISHOP+ROOK */

            case BISHOP:
                for (y = i - 9; y >= 0 && COL(y) != 7; y -= 9)
                { /* go left up */
                    if (color[y] != EMPTY)
                    {
                        if (color[y] != current_side)
                            Gen_PushNormal(i, y, pBuf, &capscount);
                        break;
                    }
                }
                for (y = i - 7; y >= 0 && COL(y) != 0; y -= 7)
                { /* go right up */
                    if (color[y] != EMPTY)
                    {
                        if (color[y] != current_side)
                            Gen_PushNormal(i, y, pBuf, &capscount);
                        break;
                    }
                }
                for (y = i + 9; y < 64 && COL(y) != 0; y += 9)
                { /* go right down */
                    if (color[y] != EMPTY)
                    {
                        if (color[y] != current_side)
                            Gen_PushNormal(i, y, pBuf, &capscount);
                        break;
                    }
                }
                for (y = i + 7; y < 64 && COL(y) != 7; y += 7)
                { /* go left down */
                    if (color[y] != EMPTY)
                    {
                        if (color[y] != current_side)
                            Gen_PushNormal(i, y, pBuf, &capscount);
                        break;
                    }
                }
                if (piece[i] == BISHOP) /* In the case of the bishop we're done */
                    break;

                /* FALL THROUGH FOR QUEEN {I meant to do that!} ;-) */
            case ROOK:
                col = COL(i);
                for (k = i - col, y = i - 1; y >= k; y--)
                { /* go left */
                    if (color[y] != EMPTY)
                    {
                        if (color[y] != current_side)
                            Gen_PushNormal(i, y, pBuf, &capscount);
                        break;
                    }
                }
                for (k = i - col + 7, y = i + 1; y <= k; y++)
                { /* go right */
                    if (color[y] != EMPTY)
                    {
                        if (color[y] != current_side)
                            Gen_PushNormal(i, y, pBuf, &capscount);
                        break;
                    }
                }
                for (y = i - 8; y >= 0; y -= 8)
                { /* go up */
                    if (color[y] != EMPTY)
                    {
                        if (color[y] != current_side)
                            Gen_PushNormal(i, y, pBuf, &capscount);
                        break;
                    }
                }
                for (y = i + 8; y < 64; y += 8)
                { /* go down */
                    if (color[y] != EMPTY)
                    {
                        if (color[y] != current_side)
                            Gen_PushNormal(i, y, pBuf, &capscount);
                        break;
                    }
                }
                break;

            case KING:
                /* the column and rank checks are to make sure it is on the board*/
                col = COL(i);
                if (col && color[i - 1] == xside)
                    Gen_PushKing(i, i - 1, pBuf, &capscount); /* left */
                if (col < 7 && color[i + 1] == xside)
                    Gen_PushKing(i, i + 1, pBuf, &capscount); /* right */
                if (i > 7 && color[i - 8] == xside)
                    Gen_PushKing(i, i - 8, pBuf, &capscount); /* up */
                if (i < 56 && color[i + 8] == xside)
                    Gen_PushKing(i, i + 8, pBuf, &capscount); /* down */
                if (col && i > 7 && color[i - 9] == xside)
                    Gen_PushKing(i, i - 9, pBuf, &capscount); /* left up */
                if (col < 7 && i > 7 && color[i - 7] == xside)
                    Gen_PushKing(i, i - 7, pBuf, &capscount); /* right up */
                if (col && i < 56 && color[i + 7] == xside)
                    Gen_PushKing(i, i + 7, pBuf, &capscount); /* left down */
                if (col < 7 && i < 56 && color[i + 9] == xside)
                    Gen_PushKing(i, i + 9, pBuf, &capscount); /* right down */
                break;
//				 default:
//				 printf("Piece type unknown");
                // assert(false);
            }
        }
    return capscount;
}


/* Check if current side is in check. Necesary in order to check legality of moves
 and check if castle is allowed */
int IsInCheck(int current_side)
{
    int k; /* The square where the king is placed */

    /* Find the King of the side to move */
    for (k = 0; k < 64; k++)
        if ((piece[k] == KING) && color[k] == current_side)
            break;

    /* Use IsAttacked in order to know if current_side is under check */
    return IsAttacked(current_side, k);
}

/* Check and return 1 if square k is attacked by current_side, 0 otherwise. Necesary, vg, to check
 * castle rules (if king goes from e1 to g1, f1 can't be attacked by an enemy piece) */
int IsAttacked(int current_side, int k)
{
    int h;
    int y;
    int row; /* Row where the square is placed */
    int col; /* Col where the square is placed */
    int xside;
    xside = (WHITE + BLACK) - current_side; /* opposite current_side, who may be attacking */

    /* Situation of the square*/
    row = ROW(k);
    col = COL(k);

    /* Check Knight attack */
    if (col > 0 && row > 1 && color[k - 17] == xside && piece[k - 17] == KNIGHT)
        return 1;
    if (col < 7 && row > 1 && color[k - 15] == xside && piece[k - 15] == KNIGHT)
        return 1;
    if (col > 1 && row > 0 && color[k - 10] == xside && piece[k - 10] == KNIGHT)
        return 1;
    if (col < 6 && row > 0 && color[k - 6] == xside && piece[k - 6] == KNIGHT)
        return 1;
    if (col > 1 && row < 7 && color[k + 6] == xside && piece[k + 6] == KNIGHT)
        return 1;
    if (col < 6 && row < 7 && color[k + 10] == xside && piece[k + 10] == KNIGHT)
        return 1;
    if (col > 0 && row < 6 && color[k + 15] == xside && piece[k + 15] == KNIGHT)
        return 1;
    if (col < 7 && row < 6 && color[k + 17] == xside && piece[k + 17] == KNIGHT)
        return 1;

    /* Check horizontal and vertical lines for attacking of Queen, Rook, King */
    /* go down */
    y = k + 8;
    if (y < 64)
    {
        if (color[y] == xside && (piece[y] == KING || piece[y] == QUEEN
                || piece[y] == ROOK))
            return 1;
        if (piece[y] == EMPTY || piece[y] == EPS_SQUARE)
            for (y += 8; y < 64; y += 8)
            {
                if (color[y] == xside
                        && (piece[y] == QUEEN || piece[y] == ROOK))
                    return 1;
                if (piece[y] != EMPTY && piece[y] != EPS_SQUARE)
                    break;
            }
    }
    /* go left */
    y = k - 1;
    h = k - col;
    if (y >= h)
    {
        if (color[y] == xside && (piece[y] == KING || piece[y] == QUEEN
                || piece[y] == ROOK))
            return 1;
        if (piece[y] == EMPTY || piece[y] == EPS_SQUARE)
            for (y--; y >= h; y--)
            {
                if (color[y] == xside
                        && (piece[y] == QUEEN || piece[y] == ROOK))
                    return 1;
                if (piece[y] != EMPTY && piece[y] != EPS_SQUARE)
                    break;
            }
    }
    /* go right */
    y = k + 1;
    h = k - col + 7;
    if (y <= h)
    {
        if (color[y] == xside && (piece[y] == KING || piece[y] == QUEEN
                || piece[y] == ROOK))
            return 1;
        if (piece[y] == EMPTY || piece[y] == EPS_SQUARE)
            for (y++; y <= h; y++)
            {
                if (color[y] == xside
                        && (piece[y] == QUEEN || piece[y] == ROOK))
                    return 1;
                if (piece[y] != EMPTY && piece[y] != EPS_SQUARE)
                    break;
            }
    }
    /* go up */
    y = k - 8;
    if (y >= 0)
    {
        if (color[y] == xside && (piece[y] == KING || piece[y] == QUEEN
                || piece[y] == ROOK))
            return 1;
        if (piece[y] == EMPTY || piece[y] == EPS_SQUARE)
            for (y -= 8; y >= 0; y -= 8)
            {
                if (color[y] == xside
                        && (piece[y] == QUEEN || piece[y] == ROOK))
                    return 1;
                if (piece[y] != EMPTY && piece[y] != EPS_SQUARE)
                    break;
            }
    }
    /* Check diagonal lines for attacking of Queen, Bishop, King, Pawn */
    /* go right down */
    y = k + 9;
    if (y < 64 && COL(y) != 0)
    {
        if (color[y] == xside)
        {
            if (piece[y] == KING || piece[y] == QUEEN || piece[y] == BISHOP)
                return 1;
            if (current_side == BLACK && piece[y] == PAWN)
                return 1;
        }
        if (piece[y] == EMPTY || piece[y] == EPS_SQUARE)
            for (y += 9; y < 64 && COL(y) != 0; y += 9)
            {
                if (color[y] == xside && (piece[y] == QUEEN || piece[y]
                                                                     == BISHOP))
                    return 1;
                if (piece[y] != EMPTY && piece[y] != EPS_SQUARE)
                    break;
            }
    }
    /* go left down */
    y = k + 7;
    if (y < 64 && COL(y) != 7)
    {
        if (color[y] == xside)
        {
            if (piece[y] == KING || piece[y] == QUEEN || piece[y] == BISHOP)
                return 1;
            if (current_side == BLACK && piece[y] == PAWN)
                return 1;
        }
        if (piece[y] == EMPTY || piece[y] == EPS_SQUARE)
            for (y += 7; y < 64 && COL(y) != 7; y += 7)
            {
                if (color[y] == xside && (piece[y] == QUEEN || piece[y]
                                                                     == BISHOP))
                    return 1;
                if (piece[y] != EMPTY && piece[y] != EPS_SQUARE)
                    break;

            }
    }
    /* go left up */
    y = k - 9;
    if (y >= 0 && COL(y) != 7)
    {
        if (color[y] == xside)
        {
            if (piece[y] == KING || piece[y] == QUEEN || piece[y] == BISHOP)
                return 1;
            if (current_side == WHITE && piece[y] == PAWN)
                return 1;
        }
        if (piece[y] == EMPTY || piece[y] == EPS_SQUARE)
            for (y -= 9; y >= 0 && COL(y) != 7; y -= 9)
            {
                if (color[y] == xside && (piece[y] == QUEEN || piece[y]
                                                                     == BISHOP))
                    return 1;
                if (piece[y] != EMPTY && piece[y] != EPS_SQUARE)
                    break;

            }
    }
    /* go right up */
    y = k - 7;
    if (y >= 0 && COL(y) != 0)
    {
        if (color[y] == xside)
        {
            if (piece[y] == KING || piece[y] == QUEEN || piece[y] == BISHOP)
                return 1;
            if (current_side == WHITE && piece[y] == PAWN)
                return 1;
        }
        if (piece[y] == EMPTY || piece[y] == EPS_SQUARE)
            for (y -= 7; y >= 0 && COL(y) != 0; y -= 7)
            {
                if (color[y] == xside && (piece[y] == QUEEN || piece[y]
                                                                     == BISHOP))
                    return 1;
                if (piece[y] != EMPTY && piece[y] != EPS_SQUARE)
                    break;
            }
    }
    return 0;
}

int MakeMove(MOVE m)
{
    int r;
    int i;

    count_MakeMove ++;

    hist[hdp].m = m;
    hist[hdp].cap = piece[m.dest]; /* store in history the piece of the dest square */
    hist[hdp].castle = castle ;

    piece[m.dest] = piece[m.from]; /* dest piece is the one in the original square */
    color[m.dest] = color[m.from]; /* The dest square color is the one of the origin piece */
    piece[m.from] = EMPTY;/* The original square becomes empty */
    color[m.from] = EMPTY; /* The original color becomes empty */

    /* en pasant capture */
    if (m.type_of_move == MOVE_TYPE_EPS)
    {
        if (side == WHITE)
        {
            piece[m.dest + 8] = EMPTY;
            color[m.dest + 8] = EMPTY;
        }
        else
        {
            piece[m.dest - 8] = EMPTY;
            color[m.dest - 8] = EMPTY;
        }
    }

    /* Remove possible eps piece, remaining from former move */
    if (hist[hdp-1].m.type_of_move == MOVE_TYPE_PAWN_TWO)
    {
        for (i = 16; i <= 23; i++)
        {
            if (piece[i] == EPS_SQUARE)
            {
                piece[i] = EMPTY;
                break;
            }
        }
        for (i = 40; i <= 47; i++)
        {
            if (piece[i] == EPS_SQUARE)
            {
                piece[i] = EMPTY;
                break;
            }
        }
    }

    /* Add the eps square when a pawn moves two squares */
    if (m.type_of_move == MOVE_TYPE_PAWN_TWO)
    {
        if (side == BLACK)
        {
            piece[m.from + 8] = EPS_SQUARE;
            color[m.from + 8] = EMPTY;
        }
        else if (side == WHITE)
        {
            piece[m.from - 8] = EPS_SQUARE;
            color[m.from - 8] = EMPTY;
        }
    }

    /* Once the move is done we check either this is a promotion */
    if (m.type_of_move >= MOVE_TYPE_PROMOTION_TO_QUEEN)
    {
        /* In this case we put in the destiny sq the chosen piece */
        switch (m.type_of_move)
        {
        case MOVE_TYPE_PROMOTION_TO_QUEEN:
            piece[m.dest] = QUEEN;
            break;

        case MOVE_TYPE_PROMOTION_TO_ROOK:
            piece[m.dest] = ROOK;
            break;

        case MOVE_TYPE_PROMOTION_TO_BISHOP:
            piece[m.dest] = BISHOP;
            break;

        case MOVE_TYPE_PROMOTION_TO_KNIGHT:
            piece[m.dest] = KNIGHT;
            break;

        default:
            puts("Impossible to get here...");
//            assert(false);
        }
    }

    if (m.type_of_move == MOVE_TYPE_CASTLE)
    {
        if (m.dest == G1)
        {
            /* h1-h8 becomes empty */
            piece[m.from + 3] = EMPTY;
            color[m.from + 3] = EMPTY;
            /* rook to f1-f8 */
            piece[m.from + 1] = ROOK;
            color[m.from + 1] = WHITE;
        }
        else if (m.dest == C1)
        {
            /* h1-h8 becomes empty */
            piece[m.from - 4] = EMPTY;
            color[m.from - 4] = EMPTY;
            /* rook to f1-f8 */
            piece[m.from - 1] = ROOK;
            color[m.from - 1] = WHITE;
        }
        else if (m.dest == G8)
        {
            /* h1-h8 becomes empty */
            piece[m.from + 3] = EMPTY;
            color[m.from + 3] = EMPTY;
            /* rook to f1-f8 */
            piece[m.from + 1] = ROOK;
            color[m.from + 1] = BLACK;
        }
        else if (m.dest == C8)
        {
            /* h1-h8 becomes empty */
            piece[m.from - 4] = EMPTY;
            color[m.from - 4] = EMPTY;
            /* rook to f1-f8 */
            piece[m.from - 1] = ROOK;
            color[m.from - 1] = BLACK;
        }
    }

    /* Update ply and hdp */
    ply++;
    hdp++;

    /* Update the castle rights */
    castle &= castle_mask[m.from] & castle_mask[m.dest];


    /* Checking if after making the move we're in check*/
    r = !IsInCheck(side);

    /* After making move, give turn to opponent */
    side = (WHITE + BLACK) - side;

    return r;
}

/* Undo what MakeMove did */
void TakeBack()
{

    side = (WHITE + BLACK) - side;
    hdp--;
    ply--;

    /* Update piece moved and side to move */
    piece[hist[hdp].m.from] = piece[hist[hdp].m.dest];
    piece[hist[hdp].m.dest] = hist[hdp].cap;
    color[hist[hdp].m.from] = side;

    /* Update castle rights */
    castle = hist[hdp].castle;

    /* Return the captured material */
    if (hist[hdp].cap != EMPTY && hist[hdp].cap != EPS_SQUARE)
    {
        color[hist[hdp].m.dest] = (WHITE + BLACK) - side;
    }
    else
    {
        color[hist[hdp].m.dest] = EMPTY;
    }

    /* Promotion */
    if (hist[hdp].m.type_of_move >= MOVE_TYPE_PROMOTION_TO_QUEEN)
    {
        piece[hist[hdp].m.from] = PAWN;
    }

    /* If pawn moved two squares in the former move, we have to restore
     * the eps square */
    if (hist[hdp-1].m.type_of_move == MOVE_TYPE_PAWN_TWO)
    {
        if (side == WHITE)
        {
            piece[hist[hdp-1].m.dest - 8] = EPS_SQUARE;
        }
        else if (side == BLACK)
        {
            piece[hist[hdp-1].m.dest + 8] = EPS_SQUARE;
        }
    }

    /* To remove the eps square after unmaking a pawn
     * moving two squares*/
    if (hist[hdp].m.type_of_move == MOVE_TYPE_PAWN_TWO)
    {
        if (side == WHITE)
        {
            piece[hist[hdp].m.from - 8] = EMPTY;
            color[hist[hdp].m.from - 8] = EMPTY;
        }
        else
        {
            piece[hist[hdp].m.from + 8] = EMPTY;
            color[hist[hdp].m.from + 8] = EMPTY;
        }
    }

    /* Unmaking an en pasant capture */
    if (hist[hdp].m.type_of_move == MOVE_TYPE_EPS)
    {
        if (side == WHITE)
        {
            /* The pawn */
            piece[hist[hdp].m.dest + 8] = PAWN;
            color[hist[hdp].m.dest + 8] = BLACK;
            /* The eps square */
            piece[hist[hdp].m.dest] = EPS_SQUARE;
        }
        else
        {
            /* The pawn */
            piece[hist[hdp].m.dest - 8] = PAWN;
            color[hist[hdp].m.dest - 8] = WHITE;
            /* The eps square */
            piece[hist[hdp].m.dest] = EPS_SQUARE;
        }
    }

    /* Undo Castle: return rook to its original square */
    if (hist[hdp].m.type_of_move == MOVE_TYPE_CASTLE)
    {
        /* Take the tower to its poriginal place */
        if (hist[hdp].m.dest == G1 && side == WHITE)
        {
            piece[H1] = ROOK;
            color[H1] = WHITE;
            piece[F1] = EMPTY;
            color[F1] = EMPTY;
        }
        else if (hist[hdp].m.dest == C1 && side == WHITE)
        {
            piece[A1] = ROOK;
            color[A1] = WHITE;
            piece[D1] = EMPTY;
            color[D1] = EMPTY;
        }
        else if (hist[hdp].m.dest == G8 && side == BLACK)
        {
            piece[H8] = ROOK;
            color[H8] = BLACK;
            piece[F8] = EMPTY;
            color[F8] = EMPTY;
        }
        else if (hist[hdp].m.dest == C8 && side == BLACK)
        {
            piece[A8] = ROOK;
            color[A8] = BLACK;
            piece[D8] = EMPTY;
            color[D8] = EMPTY;
        }
    }
}
