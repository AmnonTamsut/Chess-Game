
#include "chess.h"


typedef struct {
	char srcPiece,originalSrcPiece , srcRow, srcCol, destPiece, destRow, destCol;
	int iSrc, jSrc, iDest, jDest, tmpIsrc, tmpJsrc, iOriginalSrc, jOriginalSrc, iOriginalDest, jOriginalDest;
	int isWhite, isCapture, isPromotion, isCheck, isMate, firstCharMark, firstNumMark;
	int iWhiteKing, jWhiteKing, iBlackKing, jBlackKing, wKingThreat, bKingThreat;
	int isLegal, promTestRes, capTestRes, genTestRes, checkTestRes;
} Move;

void resetVars(Move* movePtr);
int toLowCaseConv(char upperCase);
int charConv(char k);
int canMove(Move* movePtr);
void genTest(Move* movePtr, char board[][SIZE]);
int captTest(Move* movePtr, char board[][SIZE]);
void checkTest(Move* movePtr, char board[][SIZE]);
int findSrc(Move* movePtr, char board[][SIZE]);
void isCheckOccured(Move* movePtr, char board[][SIZE]);
int isPathClear(Move* movePtr, char board[][SIZE]);
void pgnEval(Move* movePtr, char pgn[]);
void promTest(Move* movePtr, char board[][SIZE]);



///////////////////////////////////////////////////////////////////////////////////////////////////

							/*makeMove main functions section*/

///////////////////////////////////////////////////////////////////////////////////////////////////

/***********************************************************
* Function Name: isValid
* Input: char x, char board[][SIZE], int i, int j, int isWhite
* Output: int
* Function Operation: Checks if a given move is a valid move.
***********************************************************/
int isValid(char board[][SIZE], char pgn[], int isWhiteTurn)
{
	Move move;
	char tempPiece, tempPiece2;
	resetVars(&move);
	move.isWhite = isWhiteTurn;

	pgnEval(&move, pgn);
	findSrc(&move, board);

	move.iOriginalDest = move.iDest;
	move.jOriginalDest = move.jDest;


	promTest(&move, board);
	captTest(&move, board);
	genTest(&move, board);

	
	if (move.isPromotion && !move.promTestRes)
		return 0;
	if (move.isCapture && !move.capTestRes)
		return 0;
	if (!move.genTestRes && !move.isCapture && !move.isPromotion && !move.isCheck)
		return 0;

	tempPiece = board[move.iOriginalSrc][move.jOriginalSrc];
	tempPiece2 = board[move.iOriginalDest][move.jOriginalDest];
	board[move.iOriginalSrc][move.jOriginalSrc] = ' ';
	board[move.iOriginalDest][move.jOriginalDest] = move.srcPiece;
	if (!move.isWhite)
		move.destPiece = toLowCaseConv(move.destPiece);
	if (move.promTestRes)
		board[move.iOriginalDest][move.jOriginalDest] = move.destPiece;

	//Last check cause it runs every turn.
	isCheckOccured(&move, board);
	checkTest(&move, board);

	if (move.isCheck && !move.checkTestRes)
	{
		board[move.iOriginalSrc][move.jOriginalSrc] = tempPiece;
		board[move.iOriginalDest][move.jOriginalDest] = tempPiece2;
		return 0;
	}
		
	return 1;
}

/***********************************************************
* Function Name: pgnEval
* Input: char arr[]
* Output: int
* Function Operation: Evaluates pgn & turns on the cases' flags.
***********************************************************/
void pgnEval(Move* movePtr, char pgn[])
{
	int i;

	//Saves the relevant piece that is currently being used in srcPiece
	if (pgn[0] >= 'A' && pgn[0] <= 'Z')
	{
		movePtr->srcPiece = pgn[0];
		movePtr->originalSrcPiece = movePtr->srcPiece;
	}
	else
	{
		movePtr->srcPiece = 'P';
		movePtr->originalSrcPiece = 'P';
	}

	//PGN analizing while reading it backwards
	for (i = strlen(pgn)-1; i >= 0; i--)
	{

		//Check or Mate is being claimed
		if (pgn[i] == '+' || pgn[i] == '#')
		{
			movePtr->isCheck = 1;
		}

		//Promotion is being claimed
		if (pgn[i] == '=')
		{
			movePtr->destPiece = pgn[i + 1];
			movePtr->isPromotion = 1;
		}

		//Capture is being claimed
		if (pgn[i] == 'x')
		{
			movePtr->isCapture = 1;
		}

		//Saving row data from PGN 
		if (pgn[i] >= '1' && pgn[i] <= '9')
		{
			if (!movePtr->firstNumMark)
			{
				movePtr->iDest = charConv(pgn[i]);
				movePtr->firstNumMark = 1;
			}
			else if (movePtr->firstCharMark && movePtr->firstNumMark)
			{
				movePtr->iSrc = charConv(pgn[i]);
			}
		}

		//Saving column data from PGN 
		if (pgn[i] >= 'a' && pgn[i] <= 'i')
		{
			if (!movePtr->firstCharMark)
			{
				movePtr->jDest = charConv(pgn[i]);
				movePtr->firstCharMark = 1;
			}
			else if (movePtr->firstCharMark && movePtr->firstNumMark)
			{
				movePtr->jSrc = charConv(pgn[i]);
			}
		}
	}
}

/***********************************************************
* Function Name: isVacant
* Input: char arr[]
* Output: int
* Function Operation: This functions' purpose is to check if a destination is vacant.
***********************************************************/
int isDestVacant(Move* movePtr, char board[][SIZE])
{
	if (board[movePtr->iDest][movePtr->jDest] == ' ' && !movePtr->isCapture)
		return 1;
	else
		return 0;
}

/***********************************************************
* Function Name: findSrc
* Input: char arr[], Move*
* Output: int
* Function Operation: This functions purpose is to find the source of a piece.
***********************************************************/
int findSrc(Move* movePtr, char board[][SIZE])
{
	int i, j;

	//Converts to lowercase when it's a Black turn
	if (!movePtr->isWhite)
		movePtr->srcPiece = toLowCaseConv(movePtr->srcPiece);

	//When both row & column is mentioned in PGN
	if (movePtr->iSrc != -1 && movePtr->jSrc != -1 && canMove(movePtr) && isPathClear(movePtr, board))
		return 1;


	//When the source row is mentioned in PGN
	if (movePtr->iSrc != -1)
	{
		//Saving iSrc for further use
		movePtr->iOriginalSrc = movePtr->iSrc;
		for (j = 0;j < SIZE;j++)
		{
			movePtr->jSrc = j;
			if (board[movePtr->iSrc][j] == movePtr->srcPiece)
			{
				movePtr->jSrc = j;
				if (canMove(movePtr) && isPathClear(movePtr, board))
				{
					movePtr->jOriginalSrc = movePtr->jSrc;
					return 1;
				}
			}
		}
	}

	//When the source column is mentioned in PGN
	if (movePtr->jSrc != -1)
	{
		//Saving jSrc for further use
		movePtr->jOriginalSrc = movePtr->jSrc;

		for (i = 0;i < SIZE;i++)
		{
			movePtr->iSrc = i;
			if (board[i][movePtr->jSrc] == movePtr->srcPiece)
			{
				//movePtr->jSrc = i;

				if (canMove(movePtr) && isPathClear(movePtr, board))
				{
					movePtr->iOriginalSrc = movePtr->iSrc;
					return 1;
				}
			}
		}
	}

	//When source row & column are not mentioned in PGN 
	else if (movePtr->jSrc == -1 && movePtr->iSrc == -1)
	{
		for (i = 0;i < SIZE;i++)
		{
			for (j = 0; j < SIZE; j++)
			{

				//Saving iSrc & jSrc for further use if needed
				movePtr->iSrc = i;
				movePtr->iOriginalSrc = movePtr->iSrc;
				movePtr->jSrc = j;
				movePtr->jOriginalSrc = movePtr->jSrc;

				if (board[i][j] == movePtr->srcPiece)
				{

					if (canMove(movePtr) && isPathClear(movePtr, board))
						return 1;
				}
			}
		}

	}

	//Re-initializing iSrc & jSrc for when no match is found
	movePtr->iSrc = -1;
	movePtr->jSrc = -1;
	return 0;
}

/***********************************************************
* Function Name: findSrc
* Input: Move*, char
* Output: int
* Function Operation: Determines if a capture claimed is valid.
***********************************************************/
int isCaptureValid(Move* movePtr, char board[][SIZE])
{
	int isCapital = -1;

	if (board[movePtr->iDest][movePtr->jDest] > 'A' && board[movePtr->iDest][movePtr->jDest] < 'Z')
		isCapital = 1;
	else if (board[movePtr->iDest][movePtr->jDest] > 'a' && board[movePtr->iDest][movePtr->jDest] < 'z')
		isCapital = 0;

	//Destination filled with a matching case piece - means that capturing can't be done
	if ((movePtr->isWhite && isCapital))
		return 0;
	if (canMove(movePtr))
		return 1;
	else
		return 0;
}

/***********************************************************
* Function Name: isPromotionValid
* Input: Move*, char
* Output: int
* Function Operation: Determines if a capture claimed is valid.
***********************************************************/
int isPromotionValid(Move* movePtr, char board[][SIZE])
{
	int iPromSrc;

	//Assigning the correct row source as per isWhite
	if (movePtr->isWhite)
		iPromSrc = 1;
	else
		iPromSrc = SIZE - 2;

	//Promotion should be claimed only Pawn
	if (movePtr->srcPiece != 'P' && movePtr->srcPiece != 'p')
		return 0;

	//Making sure that iSrc is a valid src for promotion
	if (movePtr->iSrc != iPromSrc)
		return 0;

	//Seperating cases when isCapture claimed
	if (!movePtr->isCapture && (!canMove(movePtr) || !isDestVacant(movePtr, board)))
		return 0;
	else if (!canMove(movePtr))
		return 0;
	else
		return 1;
}

/***********************************************************
* Function Name: findKing
* Input: Move*, char
* Output: int
* Function Operation: Finds the King location.
***********************************************************/
int findKing(Move* movePtr, char board[][SIZE])
{
	int i, j, wFlag = 0, bFlag = 0;


	for (i = 0; i < SIZE; i++)
	{
		for (j = 0; j < SIZE; j++)
		{

			if (board[i][j] == 'K' && !wFlag)
			{
				movePtr->iWhiteKing = i;
				movePtr->jWhiteKing = j;
				wFlag = 1;
			}
			if (board[i][j] == 'k' && !bFlag)
			{
				movePtr->iBlackKing = i;
				movePtr->jBlackKing = j;
				bFlag = 1;
			}
		}
	}

	if (!wFlag || !bFlag)
		return 0;
	else
		return 1;
}

/*****************************************************************************************************
* Function Name: isCheckOccured
* Input: Move*, char
* Output: int
* Function Operation: Determines if a check has occured. Whether its self-made or pro-active Check.
*****************************************************************************************************/
void isCheckOccured(Move* movePtr, char board[][SIZE])
{
	int i, j;

	findKing(movePtr, board);


	//Active Check/Mate

	for (i = 0; i < SIZE; i++)
	{
		for (j = 0; j < SIZE; j++)
		{
			//White king threat check
			if (board[i][j] > 'a' && board[i][j] < 'z')
			{
				movePtr->iDest = movePtr->iWhiteKing;
				movePtr->jDest = movePtr->jWhiteKing;
				movePtr->iSrc = i;
				movePtr->jSrc = j;
				movePtr->srcPiece = board[i][j];
				if (canMove(movePtr) && isPathClear(movePtr, board))
					movePtr->wKingThreat = 1;
			}

			//Black king threat check
			if (board[i][j] > 'A' && board[i][j] < 'Z')
			{
				movePtr->iDest = movePtr->iBlackKing;
				movePtr->jDest = movePtr->jBlackKing;
				movePtr->iSrc = i;
				movePtr->jSrc = j;
				movePtr->srcPiece = board[i][j];
				if (canMove(movePtr) && isPathClear(movePtr, board))
					movePtr->bKingThreat = 1;
			}
		}
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////

							/*Board creation & printing section*/

///////////////////////////////////////////////////////////////////////////////////////////////////

/*********************************************************************************************************
* Function Name: createBoard
* Input: char[][], char[]
* Output: void
* Function Operation: Copy the charachters in 'fen' into a 2d array called 'board'.
				few details regarding the proccess:
						* board[][] is initialized to contain only spaces (i.e. the char ' ').
						* When the char is '\' it moves to a new row.
						* If the char is a valid char it copies to its appropriate spot in 'board'.
						* If the char is a number it skips these slots so they'll be represented as ' '.
**********************************************************************************************************/
void createBoard(char board[][SIZE], char fen[])
{
	int i, j, boardRow = 0, boardCol = 0;

	// board array initialization,
	for (i = 0; i < SIZE;i++)
	{
		for (j = 0; j < SIZE; j++)
		{
			board[i][j] = ' ';
		}
	}

	//i,j reset for further use in fen[].
	i = 0;
	j = 0;

	// loop designed to allocate every char its proper location in the 'board' array
	while (fen[i] != '\0')
	{
		if (fen[i] == '/')
		{
			boardRow++;
			boardCol = 0;
		}

		// Game pieces implementaion on 'board' array
		else if (fen[i] >= 'a' && fen[i] <= 'z' || fen[i] >= 'A' && fen[i] <= 'Z')
		{
			board[boardRow][boardCol] = fen[i];
			boardCol++;
		}

		// Row vacant spots calculation 
		else if (fen[i] >= '1' && fen[i] < ('0' + SIZE))
		{
			boardCol = boardCol + (fen[i] - '0');
		}
		i++;
	}
}

/***********************************************************
* Function Name: printBoardColIndex
* Input: void
* Output: void
* Function Operation: Print columns Indexs alphabetically.
							Example: * |A B C D E F G H| *
***********************************************************/
void printBoardColIndex()
{
	int i;
	char startChar = 'A';

	printf("* |");
	for (i = 0; i < SIZE; i++)
	{
		printf("%c", startChar);
		if (startChar != 'A' + (SIZE - 1))
		{
			printf(" ");
		}
		startChar++;
	}
	printf("| *\n");

}

/***********************************************************
* Function Name: printBoarddashes
* Input: void
* Output: void
* Function Operation: Print dashes.
	 Example: * ----------------- *
***********************************************************/
void printBoardDashes()
{
	int i;
	printf("* -");
	for (i = 0; i < SIZE; i++)
	{
		printf("--");
	}
	printf(" *\n");

}

/***********************************************************
* Function Name: printBoard
* Input: char
* Output: void
* Function Operation: Print a visualization of the board.
	Example:		* |A B C D E F G H| *
					* ----------------- *
					8 |r|n|b|q|k|b|n|r| 8
					7 | | | | | | | | | 7
					6 | | | | | | | | | 6
					5 | | | | | | | | | 5
					4 | | | | | | | | | 4
					3 | | | | | | | | | 3
					2 | | | | | | | | | 2
					1 |R|N|B|Q|K|B|N|R| 1
					* ----------------- *
					* |A B C D E F G H| *
***********************************************************/
void printBoard(char board[][SIZE])
{
	int i, j, boardRow = SIZE;
	printBoardColIndex();
	printBoardDashes();

	// Print body of the board 
	for (i = 0; i < SIZE; i++)
	{
		printf("%d ", boardRow);

		printf("|");

		for (j = 0;j < SIZE;j++)
		{
			printf("%c", board[i][j]);
			printf("|");
		}
		printf(" %d", boardRow);

		printf("\n");
		boardRow--;

	}

	printBoardDashes();
	printBoardColIndex();
}



///////////////////////////////////////////////////////////////////////////////////////////////////

							/*Side functions section*/

///////////////////////////////////////////////////////////////////////////////////////////////////

/***********************************************************
* Function Name: charConv
* Input: char
* Output: int
* Function Operation: Converts a char into an index row number.
***********************************************************/
int charConv(char k)
{
	int numValue = 0;

	if (k >= 'a' && k <= ('a' + SIZE- 1))
	{
		numValue = k - 'a';
	}
	else if (k >= '1' && k <= SIZE + '0')
	{
		numValue = SIZE - (k - '0');
	}

	return numValue;
}

/***********************************************************
* Function Name: toLowCaseConv
* Input: char
* Output: char
* Function Operation: Converts an upper case letter into lower case.
***********************************************************/ 
int toLowCaseConv(char upperCase)
{
		return (upperCase + ('a'-'A')) ;
}

/***********************************************************
* Function Name: resetVars
* Input: char
* Output: void
* Function Operation: Initial Resets all the vars in Move.
***********************************************************/

void resetVars(Move* movePtr)
{
	movePtr->iSrc = -1, movePtr->jSrc = -1, movePtr->iDest = -1, movePtr->jDest = -1, movePtr->tmpIsrc = -1, movePtr->tmpJsrc = -1;
	movePtr->iOriginalSrc = -1, movePtr->jOriginalSrc = -1, movePtr->iOriginalDest = -1, movePtr->jOriginalDest = -1;
	movePtr->isWhite = 1; movePtr->isCapture = 0; movePtr->isPromotion = 0; movePtr->isCheck = 0;
	movePtr->isMate = 0; movePtr->isLegal = 0; movePtr->firstCharMark=0,movePtr->firstNumMark=0;
	movePtr->iWhiteKing = -1, movePtr->jWhiteKing = -1, movePtr->iBlackKing = -1, movePtr->jBlackKing = -1;
	movePtr->wKingThreat = -1, movePtr->bKingThreat = -1;
	movePtr->promTestRes=0, movePtr->capTestRes=0,movePtr->genTestRes=0, movePtr->genTestRes = 0, movePtr->checkTestRes=0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

						/*Pieces movement rules section*/

///////////////////////////////////////////////////////////////////////////////////////////////////

/************************************************************************************************
* Function Name: canRookMove
* Input: Move*
* Output: int
* Function Operation: Determines whether can a Rook move or not according to the movement rules.
					  A Rook can move in 2 different ways:	- Moves in row & Stays in the same column.
					  										- Stays in the same row & Moves in column.
************************************************************************************************/
int canRookMove(Move* movePtr)
{
	if ((movePtr->iSrc == movePtr->iDest) && (movePtr->jSrc != movePtr->jDest) 
		|| (movePtr->iSrc != movePtr->iDest) && (movePtr->jSrc == movePtr->jDest))
		return 1;
	else
		return 0;
}

/************************************************************************************************
* Function Name: canKnightMove
* Input: Move*
* Output: int
* Function Operation: Determines wether can a Knight move or not according to the movement rules.
					  A Knight can move in 2 different ways: - Moves 1 in row & 2 in column.
															 - Moves 2 in row & 1 in column.
************************************************************************************************/
int canKnightMove(Move* movePtr)
{
	
	if ((abs(movePtr->iDest - movePtr->iSrc) == 1 && abs(movePtr->jDest - movePtr->jSrc) == 2) 
		|| (abs(movePtr->iDest - movePtr->iSrc) == 2 && abs(movePtr->jDest - movePtr->jSrc) == 1) 
		&& (movePtr->jDest != movePtr->jSrc)&& (movePtr->iDest != movePtr->iSrc))
		return 1;
	else
		return 0;
}

/************************************************************************************************
* Function Name: canBishopMove
* Input: Move*
* Output: int
* Function Operation: Determines wether can a Bishop move or not according to the movement rules.
************************************************************************************************/
int canBishopMove(Move* movePtr)
{
	if ((abs(movePtr->iDest - movePtr->iSrc) == abs(movePtr->jDest - movePtr->jSrc)) 
		&& (movePtr->iDest != movePtr->iSrc) && (movePtr->jDest != movePtr->jSrc))
		return 1;
	else
		return 0;
}

/************************************************************************************************
* Function Name: canKingMove
* Input: Move*
* Output: int
* Function Operation: Determines wether can a King move or not according to the movement rules.
************************************************************************************************/
int canKingMove(Move* movePtr)
{
	if ((abs(movePtr->iDest - movePtr->iSrc) == abs(movePtr->jDest - movePtr->jSrc))
		|| ((movePtr->iDest == movePtr->iSrc && abs(movePtr->jDest-movePtr->jSrc) < 2) || 
		(abs(movePtr->iDest - movePtr->iSrc) < 2  && movePtr->jDest == movePtr->jSrc)))
		return 1;
	else
		return 0;
}

/************************************************************************************************
* Function Name: canQueenMove
* Input: Move*
* Output: int
* Function Operation: Determines wether can a Queen move or not according to the movement rules.
************************************************************************************************/
int canQueenMove(Move* movePtr)
{
	//Queen can move according to Rook rules or Bishop rules
	if (canRookMove(movePtr)|| canBishopMove(movePtr))
		return 1;
	else
		return 0;

}

/*****************************************************************************************************
* Function Name: canWhitePawnMove
* Input: Move*
* Output: int
* Function Operation: Determines wether can a white Pawn move or not according to the movement rules.
					  special case is mentioned for when capturing is being claimed.
******************************************************************************************************/
int canWhitePawnMove(Move* movePtr)
{
	//White pawn must be on the 2nd row from the top in order to perform a double-step
	int doubleStepSrc = SIZE - 2;

	switch (movePtr->isCapture)
	{
		//Capture is claimed
	case 1:
		if ((movePtr->iDest - movePtr->iSrc == -1 && abs(movePtr->jDest - movePtr->jSrc) == 1))
			return 1;
		else
			return 0;

	default:
		
		//Special case for when the white Pawn can move 2 slots.
		if (movePtr->iSrc == (doubleStepSrc) && (movePtr->iDest - movePtr->iSrc == -1 || movePtr->iDest - movePtr->iSrc == -2) && movePtr->jDest == movePtr->jSrc)
				return 1;
		else if ((movePtr->iDest - movePtr->iSrc == -1) && movePtr->jDest == movePtr->jSrc)
			return 1;
		else
			return 0;
	}

}

/************************************************************************************************
* Function Name: canBlackPawnMove
* Input: Move*
* Output: int
* Function Operation: Determines wether can a black Pawn move or not according to the movement rules.
					  special case is mentioned for when capturing is being claimed.
************************************************************************************************/
int canBlackPawnMove(Move* movePtr)
{
	//Black pawn must be on the 2nd row from the top in order to perform a double-step
	int doubleStepSrc=1;

	switch (movePtr->isCapture)
	{
		//Capture is claimed
	case 1:
		if ( ((movePtr->iDest - movePtr->iSrc == 1) && (abs(movePtr->jDest - movePtr->jSrc) == 1)) )
			return 1;
		else
			return 0;

	default:
		//Special case for when the black Pawn can move 2 slots.
		if (movePtr->iSrc == doubleStepSrc && (movePtr->iDest - movePtr->iSrc == 1 || movePtr->iDest - movePtr->iSrc== 2) && movePtr->jDest == movePtr->jSrc)
				return 1;
		else if ( (movePtr->iDest - movePtr->iSrc == 1) && (movePtr->jDest == movePtr->jSrc) )
			return 1;
		else
			return 0;
	}
}

/*****************************************************************************
* Function Name: pieceMenu
* Input: Move*
* Output: int
* Function Operation: Menu designed to determine if a piece can move to 
					  its destination as if it was moving on an empty board.
*****************************************************************************/
int canMove(Move* movePtr)
{
	switch (movePtr->srcPiece)
	{
	case 'k':
	case 'K':
		if (canKingMove(movePtr))
			return 1;
		else
			return 0;

	case 'r':
	case 'R':
		if (canRookMove(movePtr))
			return 1;
		else
			return 0;

	case 'q':
	case 'Q':
		if (canQueenMove(movePtr))
			return 1;
		else
			return 0;
	case 'b':
	case 'B':
		if (canBishopMove(movePtr))
			return 1;
		else
			return 0;
	
	case 'n':
	case 'N':
		if (canKnightMove(movePtr))
			return 1;
		else
			return 0;
	
	case 'p':
	case 'P':

		// Separates between white and black Pawn movment analizing 
		switch (movePtr->isWhite)
		{
		case 1:
			if (canWhitePawnMove(movePtr))
			{
				return 1;
			}
			else
			{
				return 0;
			}
		case 0:
			if (canBlackPawnMove(movePtr))
			{
				return 1;
			}
			else
			{
				return 0;
			}
		}
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

							/*Clean Path Section*/

///////////////////////////////////////////////////////////////////////////////////////////////////

/***********************************************************
* Function Name: isRookPathClear
* Input: Move*, char*
* Output: int
* Function Operation: Checks if path is clear according to Rooks' movement
***********************************************************/
int isRookPathClear(Move* movePtr, char board[][SIZE])
{
	int i, j;

	//Locating path when Rook is moving downwards along the rows in the chess board
	if (movePtr->iDest - movePtr->iSrc > 1)
	{
		for (i = movePtr->iSrc+1; i < movePtr->iDest; i++)
		{
			if (board[i][movePtr->jDest] != ' ')
				return 0;
		}
	}

	//Locating path when Rook is moving upwards along the rows in the chess board
	if (movePtr->iDest - movePtr->iSrc < -1)
	{
		for (i = movePtr->iSrc-1; i > movePtr->iDest; i--)
		{
			if (board[i][movePtr->jDest] != ' ')
				return 0;
		}
	}

	//Locating path when Rook is moving downwards along the columns in the chess board
	if (movePtr->jDest - movePtr->jSrc > 1)
	{
		for (j = movePtr->jSrc+1; j < movePtr->jDest; j++)
		{
			if (board[movePtr->iDest][j] != ' ')
				return 0;
		}
	}

	//Locating path when Rook is moving upwards along the columns in the chess board
	if (movePtr->jDest - movePtr->jSrc < 1)
	{
		for (j = movePtr->jSrc-1; j > movePtr->jDest; j--)
		{
			if (board[movePtr->iDest][j] != ' ')
				return 0;
		}
	}

	//Everything is vacant - means that the path is clear
		return 1;
}

/***********************************************************
* Function Name: isBishopPathClear
* Input: Move*, char*
* Output: int
* Function Operation: Checks if path is clear according to Bishops' movement
***********************************************************/
int isBishopPathClear(Move* movePtr, char board[][SIZE])
{
	int iCalc = 0, jCalc = 0, k = 0, ik = 0, jk = 0, slotsDelta = 0;

	//Calculation for how many vacant slots should we look for
	slotsDelta = abs((movePtr->iDest - movePtr->iSrc));

	//Indicator when the Bishop is moving up the chess board rows
	if (movePtr->iDest < movePtr->iSrc)
	{
		ik = -1;
	}

	//Indicator when the Bishop is moving up the chess board columns
	if (movePtr->jDest > movePtr->jSrc)
	{
		jk = 1;
	}

	//Indicator when the Bishop is moving down the chess board rows
	if (movePtr->iDest > movePtr->iSrc)
	{
		ik = 1;
	}

	//Indicator when the Bishop is moving down the chess board coloumns
	if (movePtr->jDest < movePtr->jSrc)
	{
		jk = -1;
	}

	iCalc = movePtr->iSrc + ik;
	jCalc = movePtr->jSrc + jk;

	//Runs on the number of slots the piece supposed to run on and check if they're not vacant.
	for (k = 1; k < slotsDelta; k++)
	{

		if (board[iCalc][jCalc] != ' ')
			return 0;

		//iClac progress according to movement direction
		iCalc = iCalc + ik;
		jCalc = jCalc + jk;

	}

	//No interruption found - means Bihops' path is clear.
	return 1;
}

/***********************************************************
* Function Name: isQueenPathClear
* Input: Move*, char
* Output: int
* Function Operation: Checks if path is clear according to Queens' movement
***********************************************************/
int isQueenPathClear(Move* movePtr, char board[][SIZE])
{
	if (isRookPathClear(movePtr, board) && canRookMove(movePtr) 
		|| isBishopPathClear(movePtr, board) && canBishopMove(movePtr))
		return 1;
	else
		return 0;
}

/***********************************************************
* Function Name: isPawnPathClear
* Input: Move*, char*
* Output: int
* Function Operation: Checks if path is clear according to Pawns' movement
***********************************************************/
int isPawnPathClear(Move* movePtr, char board[][SIZE])
{
	int doubleStep;

	//Check if double-step is being claimed
	if (abs(movePtr->iDest - movePtr->iSrc) == 2)
		doubleStep = 1;
	else
		doubleStep = 0;

	//If the Pawn makes a double-step its 'path' is only the next row Else there's no 'path' so it's always true
	switch (movePtr->isWhite)

	{
	case 0:
		if (doubleStep && !(board[movePtr->iSrc+1][movePtr->jSrc] == ' '))
			return 0;
		else
			return 1;

	case 1:
		if (doubleStep && !board[movePtr->iSrc-1][movePtr->jSrc] == ' ')
			return 0;
		else
			return 1;
	default:
		return 1;
	}
}

/**********************************************************************
* Function Name: isPathClear
* Input: Move*, char
* Output: int
* Function Operation: Determines if path of an alleged move is clear
					  via calling srcPieces' respective function.
**********************************************************************/
int isPathClear(Move* movePtr, char board[][SIZE])
{
	switch (movePtr->srcPiece)
	{

		//Checks if Rooks' path is clear
	case 'r':
	case 'R':
	{
		if (isRookPathClear(movePtr, board))
			return 1;
		else
			return 0;
	}

	//Checks if Bishops' path is clear
	case 'b':
	case 'B':
	{
		if (isBishopPathClear(movePtr, board))
			return 1;
		else
			return 0;
	}

	//Checks if Queens' path is clear
	case 'q':
	case 'Q':
	{
		if (isQueenPathClear(movePtr, board))
			return 1;
		else
			return 0;
	}

	//King moves one spot each move so there's no 'path' to check
	case 'k':
	case 'K':
		return 1;

		//Knight can skip on other pieces
	case 'n':
	case 'N':
		return 1;
		//Checks if Pawns' path is clear
	case 'p':
	case 'P':
	{
		if (isPawnPathClear(movePtr, board))
			return 1;
		else
			return 0;
	}

	default:
		return 1;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////

							/*Final Test Section*/

///////////////////////////////////////////////////////////////////////////////////////////////////

/**********************************************************************
* Function Name: promTest
* Input: Move*, char
* Output: int
* Function Operation: Runs a test to determine if Promotion valid or not.
**********************************************************************/
void promTest(Move* movePtr, char board[][SIZE])
{
	if (movePtr->isPromotion && isPromotionValid(movePtr, board))
		movePtr->promTestRes = 1;
	else if (movePtr->isPromotion && !isPromotionValid(movePtr, board))
		movePtr->promTestRes = 0;
}

/**********************************************************************
* Function Name: captTest
* Input: Move*, char
* Output: int
* Function Operation: Runs a test to determine if Capture valid or not.
**********************************************************************/
int captTest(Move* movePtr, char board[][SIZE])
{
	if (movePtr->isCapture && isCaptureValid(movePtr, board))
	{
		movePtr->capTestRes = 1;
		return 1;
	}
	else if (movePtr->isCapture && !isCaptureValid(movePtr, board))
	{
		movePtr->capTestRes = 0;
		return 0;
	}
	else
	{
		movePtr->capTestRes = 0;
		return	0;
	}
}

/**********************************************************************
* Function Name: genTest
* Input: Move*, char
* Output: int
* Function Operation: Runs a test to determine if a general move valid or not.
**********************************************************************/
void genTest(Move* movePtr, char board[][SIZE])
{
	if (!movePtr->isCapture && !movePtr->isPromotion && !movePtr->isCheck)
	{
		if (canMove(movePtr) && isDestVacant(movePtr, board) && isPathClear(movePtr, board))
			movePtr->genTestRes = 1;
		else
			movePtr->genTestRes = 0;
	}
}

/**********************************************************************
* Function Name: checkTest
* Input: Move*, char
* Output: int
* Function Operation: Runs a test to determine if a Check or Mate has occured.
**********************************************************************/
void checkTest(Move* movePtr, char board[][SIZE])
{
	if (movePtr->isWhite && movePtr->wKingThreat || !movePtr->isWhite && movePtr->bKingThreat)
		movePtr->checkTestRes = 0;
	if (movePtr->isCheck && movePtr->isWhite && movePtr->bKingThreat)
		movePtr->checkTestRes = 1;
	if (movePtr->isCheck && !movePtr->isWhite && movePtr->wKingThreat)
		movePtr->checkTestRes = 1;

}


///////////////////////////////////////////////////////////////////////////////////////////////////

int makeMove(char board[][SIZE], char pgn[], int isWhiteTurn)
	{
		//Checks if a move is valid or not
		if (isValid (board, pgn, isWhiteTurn))
			return 1;
		else
			return 0;
	}
