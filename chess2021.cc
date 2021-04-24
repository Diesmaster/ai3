
//
// C++ program to play simple chess:
// white king (WK) and white queen/rook (WQ) against black king (BK)
// Compile: g++ -Wall -Wextra -O2 -o chess chess2021.cc
// Run:     ./chess <thesize> <simulations> <maxgamelength>
//                  <depth> <gametypeW> <gametypeB> [Q|R] <print> <seed>
//          thesize: number of rows = columns of the game board
//          simulations: number of games played
//          maxgamelength: game is tied after this number of moves
//          depth: for gametree in Minimax
//          gametypeW: 0 white random; 1 pure Monte Carlo;
//                     2 Minimax; 3 alpha-beta;
//          gametypeB: 0 black random; 1 try to capture queen;
//                     2 seek king mobility
//          [Q|R]: Q/q for white queen, everything else for white rook
//          print: if 1, print game(s)
//          seed: seeds random number generator
// Walter Kosters, March 17, 2021; w.a.kosters@liacs.leidenuniv.nl
//

#include <iostream>
#include <cstdlib>
#include <ctime>
using namespace std;

const int MAX = 30; // maximum board size

class Chess {
  public:
    // top left is (1,1) = (row,column)
    int thesize;         // height = width of game board
    int xBK, yBK;        // position of black king
    int xWK, yWK;        // position of white king
    int xWQ, yWQ;        // position of white queen (or rook)
    bool queencaptured;  // is white queen captured?
    bool whoistomove;    // who is to move? (true: white)
    int countmoves;      // number of moves so far
    bool queenorrook;    // true: white queen; false: white rook
    int maxgamelength2;
    int gametypeB2;
    static int thecalls; // calls to evaluate
    // member functions, see below for comments
    Chess ( );
    void reset (int somesize, bool qor);
    void printboard ( );
    bool legalforblackking (int i, int j);
    bool legalforwhiteking (int i, int j);
    bool canwhitequeenreach (int i, int j);
    bool canwhiterookreach (int i, int j);
    bool legalforwhitequeen (int i, int j);
    bool incheck (int i, int j);
    bool checkmate ( );
    int numberofwhitequeenmoves ( );
    int numberofwhitekingmoves ( );
    int numberofwhitemoves ( );
    int numberofblackmoves ( );
    int blackmove (int gametypeB);
    void humanwhitemove ( );
    void humanblackmove ( );
    void dowhitekingmove (int k);
    void dowhitequeenmove (int k);
    void dowhitemove (int k);
    void doblackkingmove (int k);
    int playthegame (int maxgamelength, int depth, bool print,
		     int & nrmoves, int gametypeW, int gametypeB);
    void randomwhitemove ( );
    void copyBord(Chess & neperd);
    //=============================================================
    void MCwhitemove (int maxgamelength, int playouts);
    int MCwhitefakemoves (int maxgamelength, int playouts, int bestmove);
    int Minimaxvalue (int depth, int maxgamelength, int & bestmove);
    int MinimaxvalueAlphaBeta (int depth, int maxgamelength,
		               int & bestmove, int alpha, int beta);
    int evaluate ( );
};//Chess

//=================================================================

// get first non-enter from keyboard
char getfirstchar ( ) {
  char c = cin.get ( );
  while ( c == '\n' ) {
    c = cin.get ( );
  }//while
  return c;
}//getfirstchar

// initialize static class variable
int Chess::thecalls = 0;

// default constructor
Chess::Chess ( ) {
  // nothing
}//Chess::Chess

// reset the whole game
void Chess::reset (int somesize, bool qor) {
  thesize = somesize;
  queenorrook = qor;
  xWK = rand ( ) % thesize + 1;
  yWK = rand ( ) % thesize + 1;
  do {
    xBK = rand ( ) % thesize + 1;
    yBK = rand ( ) % thesize + 1;
  } while ( xBK - xWK <= 1 && xWK - xBK <= 1
            && yBK - yWK <= 1 && yWK - yBK <= 1 );
  do {
    xWQ = rand ( ) % thesize + 1;
    yWQ = rand ( ) % thesize + 1;
  } while ( ( xWQ == xWK && yWQ == yWK ) || ( xWQ == xWK && yWQ == yWK )
            || incheck (xBK,yBK) || numberofblackmoves ( ) == 0 );
  queencaptured = false;
  whoistomove = true;
  countmoves = 0;
}//Chess::reset

// can BK move from (xBK,yBK) to (i,j)?
bool Chess::legalforblackking (int i, int j) {
  if ( i <= 0 || j <= 0 || i > thesize || j > thesize )
    return false;
  if ( i == xBK && j == yBK )
    return false;
  if ( i > xBK+1 || i < xBK-1 || j > yBK+1 || j < yBK-1 )
    return false;
  if ( i - xWK <= 1 && xWK - i <= 1 && j - yWK <= 1 && yWK - j <= 1 )
    return false;
  if ( i == xWQ && j == yWQ )
    return true;  // king captures queen
  if ( incheck (i,j) )
    return false;
  return true;
}//legalforblackking

// can WK move from (xWK,yWK) to (i,j)?
bool Chess::legalforwhiteking (int i, int j) {
  if ( i <= 0 || j <= 0 || i > thesize || j > thesize )
    return false;
  if ( i == xWK && j == yWK )
    return false;
  if ( i > xWK+1 || i < xWK-1 || j > yWK+1 || j < yWK-1 )
    return false;
  if ( i - xBK <= 1 && xBK - i <= 1 && j - yBK <= 1 && yBK - j <= 1 )
    return false;
  if ( i == xWQ && j == yWQ )
    return false;
  return true;
}//legalforwhiteking

// can WQ reach (i,j) from (xWQ,yWQ)?
bool Chess::canwhitequeenreach (int i, int j) {
  if ( i <= 0 || j <= 0 || i > thesize || j > thesize )
    return false;
  if ( i != xWQ && j != yWQ && i + j != xWQ + yWQ && i - j != xWQ - yWQ )
    return false;
  if ( i == xWQ && i == xWK
       && ( ( yWQ < yWK && yWK < j ) || ( j < yWK && yWK < yWQ ) ) )
    return false;
  if ( j == yWQ && j == yWK
       && ( ( xWQ < xWK && xWK < i ) || ( i < xWK && xWK < xWQ ) ) )
    return false;
  if ( i + j == xWQ + yWQ && i + j == xWK + yWK
       && ( ( xWQ - yWQ < xWK - yWK && xWK - yWK < i - j )
            || ( i - j < xWK - yWK && xWK - yWK < xWQ - yWQ ) ) )
    return false;
  if ( i - j == xWQ - yWQ && i - j == xWK - yWK
       && ( ( xWQ + yWQ < xWK + yWK && xWK + yWK < i + j )
            || ( i + j < xWK + yWK && xWK + yWK < xWQ + yWQ ) ) )
    return false;
  return true;
}//canwhitequeenreach

// can WQ reach (i,j) from (xWQ,yWQ), when being a ROOK?
bool Chess::canwhiterookreach (int i, int j) {
  if ( i <= 0 || j <= 0 || i > thesize || j > thesize )
    return false;
  if ( i != xWQ && j != yWQ )
    return false;
  if ( i == xWQ && i == xWK
       && ( ( yWQ < yWK && yWK < j ) || ( j < yWK && yWK < yWQ ) ) )
    return false;
  if ( j == yWQ && j == yWK
       && ( ( xWQ < xWK && xWK < i ) || ( i < xWK && xWK < xWQ ) ) )
    return false;
  return true;
}//canwhiterookreach

// is BK at (i,j) in check?
bool Chess::incheck (int i, int j) {
  if ( queenorrook )
    return canwhitequeenreach (i,j);
  else
    return canwhiterookreach (i,j);
}//Chess::incheck

// can WQ move from (xWQ,yWQ) to (i,j)?
bool Chess::legalforwhitequeen (int i, int j) {
  if ( i <= 0 || j <= 0 || i > thesize || j > thesize )
    return false;
  if ( i == xWQ && j == yWQ )
    return false;
  if ( i == xWK && j == yWK )
    return false;
  return incheck (i,j);
}//legalforwhitequeen

// black cannot move and is in check
bool Chess::checkmate ( ) {
  return ( numberofblackmoves ( ) == 0 && incheck (xBK,yBK) );
}//checkmate

// how many moves are available for WK?
int Chess::numberofwhitekingmoves ( ) {
  int howmany = 0;
  int i, j;
  for ( i = -1; i <= 1; i++ )
    for ( j = -1; j <= 1; j++ )
      howmany += legalforwhiteking (xWK+i,yWK+j);
  return howmany;
}//Chess::numberofwhitekingmoves

// how many moves are available for WQ?
int Chess::numberofwhitequeenmoves ( ) {
  int howmany = 0;
  int i, j;
  for ( i = 1; i <= thesize; i++ )
    for ( j = 1; j <= thesize; j++ )
      howmany += legalforwhitequeen (i,j);
  return howmany;
}//Chess::numberofwhitequeenmoves

// how many moves are available for white?
int Chess::numberofwhitemoves ( ) {
  return ( numberofwhitequeenmoves ( ) + numberofwhitekingmoves ( ) );
}//Chess:numberofwhitemoves

// how many moves are available for black?
int Chess::numberofblackmoves ( ) {
  int howmany = 0;
  int i, j;
  for ( i = -1; i <= 1; i++ )
    for ( j = -1; j <= 1; j++ )
      howmany += legalforblackking (xBK+i,yBK+j);
  return howmany;
}//Chess::numberofblackmoves

// do k-th move for WK, 0 <= k < numberofwhitekingmoves ( )
void Chess::dowhitekingmove (int k) {
  int i, j;
  for ( i = -1; i <= 1; i++ )
    for ( j = -1; j <= 1; j++ )
      if ( legalforwhiteking (xWK+i,yWK+j) ) {
        if ( k == 0 ) {
          xWK = xWK+i; yWK = yWK+j;
	  whoistomove = ! whoistomove;
          countmoves++;
	  return;
	}//if
	k--;
      }//if
}//Chess::dowhitekingmove

// do k-th move for WQ, 0 <= k < numberofwhitequeenmoves ( )
void Chess::dowhitequeenmove (int k) {
  int i, j;
  for ( i = 1; i <= thesize; i++ )
    for ( j = 1; j <= thesize; j++ )
      if ( legalforwhitequeen (i,j) ) {
        if ( k == 0 ) {
          xWQ = i; yWQ = j;
	  whoistomove = ! whoistomove;
	  countmoves++;
	  return;
	}//if
	k--;
      }//if
}//Chess::dowhitequeenmove

// do k-th move for white, 0 <= k < numberofwhitemoves ( )
void Chess::dowhitemove (int k) {
  int wqms = numberofwhitequeenmoves ( );
  if ( k < wqms ) {
    dowhitequeenmove (k);
  }//if
  else {
    dowhitekingmove (k - wqms);
  }//else
}//Chess::dowhitemove

// do k-th move for BK, 0 <= k < numberofblackkingmoves ( )
void Chess::doblackkingmove (int k) {
  int i, j;
  for ( i = -1; i <= 1; i++ )
    for ( j = -1; j <= 1; j++ )
      if ( legalforblackking (xBK+i,yBK+j) ) {
        if ( k == 0 ) {
          xBK = xBK+i; yBK = yBK+j;
	  if ( xBK == xWQ && yBK == yWQ ) {
	    queencaptured = true;
	  }//if
	  whoistomove = ! whoistomove;
          countmoves++;
	  return;
	}//if
	k--;
      }//if
}//Chess::doblackkingmove

// do a random move for White
void Chess::randomwhitemove ( ) {
  dowhitemove (rand ( ) % numberofwhitemoves ( ));
}//Chess::randomwhitemove

// do a random move for BK
// return 0 if checkmate, 1 if stalemate (in both cases: no move),
// 2 if queen captured, 3 otherwise
// gametypeB = 0: totally random
//           = 1: capture WQ if possible
//           = 2: AND try for largest freedom
int Chess::blackmove (int gametypeB) {
  int move, i, j;
  int number = numberofblackmoves ( );
  int freedom[8];
  int nr = 0;
  int equals = 1;
  int largest = -1;
  if ( number == 0 ) {
    if ( incheck (xBK,yBK) )
      return 0;
    else
      return 1;
  }//if
  if ( gametypeB > 0 ) { // try to capture WQ
    for ( i = -1; i <= 1; i++ )
      for ( j = -1; j <= 1; j++ )
        if ( legalforblackking (xBK+i,yBK+j) ) {
          if ( xBK+i == xWQ && yBK+j == yWQ ) {
            xBK = xBK+i; yBK = yBK+j;
	    queencaptured = true;
	    whoistomove = ! whoistomove;
	    countmoves++;
	    return 2;
	  }//if
        }//if
  }//if
  if ( gametypeB < 2 ) { // random
    move = rand ( ) % number;
    for ( i = -1; i <= 1; i++ )
      for ( j = -1; j <= 1; j++ )
        if ( legalforblackking (xBK+i,yBK+j) ) {
          if ( move == 0 ) {
            xBK = xBK+i; yBK = yBK+j;
	    if ( xBK == xWQ && yBK == yWQ ) {
	      queencaptured = true;
	      whoistomove = ! whoistomove;
	      countmoves++;
	      return 2;
	    }//if
	    whoistomove = ! whoistomove;
	    countmoves++;
	    return 3;
	  }//if
	  move--;
        }//if
  }//if
  for ( i = -1; i <= 1; i++ )
    for ( j = -1; j <= 1; j++ )
      if ( legalforblackking (xBK+i,yBK+j) ) {
        xBK = xBK+i; yBK = yBK+j;
	freedom[nr] = numberofblackmoves ( );
	if ( freedom[nr] > largest ) {
	  largest = freedom[nr];
	  equals = 1;
	}//if
	else if ( freedom[nr] == largest ) {
	  equals++;
	}//if
	nr++;
        xBK = xBK-i; yBK = yBK-j; // undo partial move!
      }//if
  move = rand ( ) % equals;
  for ( i = 0; i < number; i++ ) {
    if ( freedom[i] == largest ) {
      if ( move == 0 ) {
	doblackkingmove (i);
	return 3;
      }//if
      move--;
    }//if
  }//for
  return 999;
}//Chess::blackmove

// do a move for White: human player
void Chess::humanwhitemove ( ) {
  char choice;
  bool OK = false;
  int i, j;
  do {
    printboard ( );
    cout << "A (white) move please (Q or K) .. ";
    choice = getfirstchar ( );
    if ( choice == 'q' || choice == 'Q' ) {
      cout << "Row number for queen (1..)     .. ";
      i = getfirstchar ( ) - '0';
      cout << "Column number for queen (1..)  .. ";
      j = getfirstchar ( ) - '0';
      if ( legalforwhitequeen (i,j) ) {
        xWQ = i; yWQ = j;
	OK = true;
      }//if
      else {
        cout << "Illegal queen move ..." << endl;
      }//else
    }//if
    else {
      cout << "Row number for king (1..)      .. ";
      i = getfirstchar ( ) - '0';
      cout << "Column number for king (1..)   .. ";
      j = getfirstchar ( ) - '0';
      if ( legalforwhiteking (i,j) ) {
        xWK = i; yWK = j;
	OK = true;
      }//if
      else {
        cout << "Illegal king move ..." << endl;
      }//else
    }//if
  } while ( ! OK );
  whoistomove = ! whoistomove;
  countmoves++;
}//Chess::humanwhitemove

// do a move for Black: human player
void Chess::humanblackmove ( ) {
  bool OK = false;
  int i, j;
  do {
    printboard ( );
    cout << "A (black) move please .. " << endl;
    cout << "Row number for king (1..)      .. ";
    i = getfirstchar ( ) - '0';
    cout << "Column number for king (1..)   .. ";
    j = getfirstchar ( ) - '0';
    if ( legalforblackking (i,j) ) {
      xBK = i; yBK = j;
      OK = true;
    }//if
    else {
      cout << "Illegal king move ..." << endl;
    }//else
  } while ( ! OK );
  whoistomove = ! whoistomove;
  countmoves++;
}//Chess::humanblackmove

// print board
void Chess::printboard ( ) {
  int i, j;
  char A[MAX][MAX];
  for ( i = 1; i <= thesize; i++ ) {
    for ( j = 1; j <= thesize; j++ ) {
      A[i][j] = '.';
    }//for
  }//for
  A[xBK][yBK] = 'z';
  A[xWK][yWK] = 'K';
  if ( ! queencaptured ) {
    A[xWQ][yWQ] = ( queenorrook ? 'Q' : 'R' );
  }//if
  cout << "  ";
  for ( j = 1; j <= thesize; j++ ) {
    cout << " " << j % 10;
  }//if
  cout << "   " << countmoves;
  if ( whoistomove ) {
    cout << "  WTM" << endl;
  }//if
  else {
    cout << "  BTM" << endl;
  }//else
  for ( i = 1; i <= thesize; i++ ) {
    cout << ( i < 10 ? " " : "" ) << i << " ";
    for ( j = 1; j <= thesize; j++ ) {
      cout << A[i][j] << " ";
    }//for
    cout << endl;
  }//for
  cout << endl;
}//Chess::printboard

// play the game, print if asked to do so
// maxgamelength is the maximum length of the entire game
// depth is used in the gametree for Minimax
// return 0 if checkmate, 1 if stalemate, 2 if simple tie,
// 3 if stopped
// nrmoves is the final number of moves
// gametypeW 0: random; 1: pure Monte Carlo; 2: Minimax; 3: alpha-beta
// gametypeB 0: random; 1: capture queen if possible; 2: largest freedom
int Chess::playthegame (int maxgamelength, int depth, bool print,
		        int & nrmoves, int gametypeW, int gametypeB) {
  int themove = 3;
  int bestmove;
  this->maxgamelength2 = maxgamelength;
  this->gametypeB2 = gametypeB;
  if ( print ) {
    printboard ( );
  }//if
  while ( themove == 3 && countmoves < maxgamelength ) {
    if ( whoistomove ) {
      switch ( gametypeW ) {
	case 0: // random
	  randomwhitemove ( );
	  break;
        case 1: // pure Monte Carlo
          MCwhitemove (maxgamelength,100);
	  break;
        case 2: // Minimax
          Minimaxvalue (depth,maxgamelength,bestmove); // ignore return value!
	  dowhitemove (bestmove);
	  break;
        case 3: // alpha-beta
	  MinimaxvalueAlphaBeta (depth,maxgamelength,bestmove,-2000000000,2000000000);
          // ignore return value!
	  dowhitemove (bestmove);
	  break;
      }//switch
      if ( print ) {
        printboard ( );
      }//if
    }//if
    else {
      themove = blackmove (gametypeB);
      if ( themove > 1 && print ) {
        printboard ( );
      }//if
    }//else
  }//while
  nrmoves = countmoves;
  return themove;
}//Chess::playthegame

//=================================================================

// do one pure Monte Carlo (MC) move for white
// playouts random games (of maxgamelength) per candidate move
void Chess::MCwhitemove (int maxgamelength, int playouts) {
  //queen daarna king
  int scores[numberofwhitemoves()];

  for(int x = 0; x < numberofwhitemoves(); x++){
    scores[x] = MCwhitefakemoves(maxgamelength, playouts, x);
  }

  int counter = 0;
  int highscore = scores[0];
  for(int x = 1; x < numberofwhitemoves(); x++){
    if(highscore < scores[x]){
      highscore = scores[x];
      counter = x;
    }
  }

  dowhitemove(counter);
}//Chess::MCwhitemove

void Chess::copyBord(Chess & neperd){
  neperd.thesize = this->thesize;
  neperd.xBK = this->xBK;
  neperd.yBK = this->yBK;
  neperd.xWK = this->xWK;
  neperd.yWK = this->yWK;        // position of white king
  neperd.xWQ = this->xWQ;
  neperd.yWQ = this->yWK;        // position of white queen (or rook)
  neperd.queencaptured = this->queencaptured;  // is white queen captured?
  neperd.maxgamelength2 = this->maxgamelength2;
  neperd.whoistomove = this->whoistomove;    // who is to move? (true: white)
  neperd.countmoves = this->countmoves;      // number of moves so far
  neperd.queenorrook = this->queenorrook;    // true: white queen; false: white rook
}

int Chess::MCwhitefakemoves(int maxgamelength, int playouts, int bestmove){
  Chess neperd;
  float score = 0;
  this->copyBord(neperd);
  neperd.dowhitemove(bestmove);
  int themove = 3;
  for(int x = 0; x < playouts; x++){
    while (themove == 3 && neperd.countmoves < neperd.maxgamelength2 ) {
      if ( neperd.whoistomove ) {
  	     neperd.randomwhitemove( );
      }//if
      else {
        themove = neperd.blackmove(neperd.gametypeB2);
      }//else
      neperd.countmoves++;
    }//while
    if(themove == 3){
      score = score + (10000/playouts);
      //score = score - (neperd.countmoves/playouts);
      //std::cout << "winning" << endl;
    }else{
      score = score - ((100*neperd.countmoves)/playouts);
    }
  }

  return score;
}

// compute and return Minimax value, including best move
// games of maxgamelength; look depth moves ahead
int Chess::Minimaxvalue (int depth, int maxgamelength, int & bestmove) {
  // TODO
  return 0;
}//Chess::Minimaxvalue

// compute and return Minimax value, alpha-beta pruning, including best move
// games of maxgamelength; look depth moves ahead
int Chess::MinimaxvalueAlphaBeta (int depth, int maxgamelength, int & bestmove,
		                  int alpha, int beta) {
  // TODO
  return 0;
}//Chess::MinimaxvalueAlphaBeta

// evaluate quality of position, large if good for white
// not a leaf
int Chess::evaluate ( ) {
  // TODO
  thecalls++;
  return 0;
}//Chess::evaluate

// main program
int main (int argc, char* argv[ ]) {
  int i;
  int stats[5];
  int somesize;
  int simulations;
  int maxgamelength;
  int depth;
  int gametypeW, gametypeB;
  int nrmoves;
  bool print;
  bool queenorrook;
  Chess theboard;
  if ( argc == 10 ) {
    somesize = atoi (argv[1]);
    if ( somesize > MAX - 2 ) {
      cout << "Size too large!" << endl;
      somesize = MAX - 2;
    }//if
    if ( somesize <= 3 ) {
      cout << "Size too small!" << endl;
      somesize = 4;
    }//if
    simulations = atoi (argv[2]);
    maxgamelength = atoi (argv[3]);
    depth = atoi (argv[4]);
    gametypeW = atoi (argv[5]);
    gametypeB = atoi (argv[6]);
    queenorrook = ( argv[7][0] == 'Q' || argv[7][0] == 'q' );
    print = ( atoi (argv[8]) == 1 );
    srand (atoi (argv[9]));
  }//if
  else {
    cout << "Usage " << argv[0] << " <thesize> <simulations> "
         << "<maxgamelength> <depth> <gametypeW> <gametypeB> "
	 << "[Q/R] <print> <seed>" << endl;
    return 1;
  }//else
  stats[0] = stats[1] = stats[2] = stats[3] = stats[4] = 0;
  for ( i = 0; i < simulations; i++ ) {
    theboard.reset (somesize,queenorrook);
    stats[theboard.playthegame (maxgamelength,depth,print,nrmoves,
		                gametypeW,gametypeB)]++;
    stats[4] += nrmoves;
  }//for
  cout << "Board size:          " << somesize
       << ( queenorrook ? " with Queen" : " with Rook" ) << endl
       << "Game type white:     " << gametypeW << endl
       << "Game type black:     " << gametypeB << endl
       << "Max game length:     " << maxgamelength << endl
       << "Depth:               " << depth << endl
       << "Wins:                " << stats[0] << endl
       << "Stalemates:          " << stats[1] << endl
       << "Simple ties:         " << stats[2] << endl
       << "Stopped:             " << stats[3] << endl
       << "Average game length: " << stats[4] / (double)simulations << endl
       << "Total:               " << simulations << endl << endl;
  cout << "And " << Chess::thecalls << " calls to evaluate." << endl << endl;
  return 0;
}//main
