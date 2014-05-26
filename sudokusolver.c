/*=========================*
	Sudoku Solver
	AUTHOR:	Tracy Parsons
*=========================*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define N 9//9	size of each dimension of the sudoku board

struct { unsigned int ask : N; } M;
int n, Nsq, depth;

// OPERATION COUNTS:
int UPDATE, UPDONE, CELLCHECK, SQUARENUM, HIGHLANDER, SOLVEIT, GUESSIT, DISPLAY, READ;

int SquareNum () {	// computes the total number of cells on the board
	SQUARENUM++;
	int a = 0;
	while (a*a < N) {a++;}
	if (a*a == N) { return a; }
	return 0;
}

unsigned int CellCheck (unsigned int in) {
	CELLCHECK++;
	unsigned int out = 1;
	unsigned int test = 1;
	while (test < (M.ask & ~in) && out < N) {
		out++;
		test = test << 1;
	}
	if (test == (M.ask & ~in)) { return out; }
	else if (!(~in & M.ask)) { return -1; }
	return 0;
}

struct Cell {
	unsigned int nums	: N;
	//unsigned int done	: 1;
	unsigned int done;
};

struct MaskSet {
	unsigned int Row	: N;
	unsigned int Column	: N;
	unsigned int Square	: N;
};

struct Sudoku {
	struct Cell Board[N][N];
	struct MaskSet Masks[N];
	unsigned int paradox	: 1;
	unsigned int numdone;
};

struct Sudoku InitializeSudoku () {
	struct Sudoku out;
	out.numdone = 0;
	out.paradox = 0;
	int a = Nsq;
	while (a-- > 0) {
		int b = a%N, c = a/N;
		out.Board[b][c].nums = 0;
		out.Board[b][c].done = 0;
		if (!(b)) {
			out.Masks[c].Row = 0;
			out.Masks[c].Column = 0;
			out.Masks[c].Square = 0;
		}
	}
	return out;
}

void Display (struct Sudoku in) {
	DISPLAY++;
	int a = -1;
	printf ("Number done: %i\n", in.numdone);
	int b = 3*N+n;
	while (b-- >= 0) { printf ("-"); }
	printf ("\n");
	while (++a < Nsq) {
		if ((a%n) == 0) {printf ("|");}
		//int ass = CellCheck (in.Board[a%N][a/N].nums);
		if (in.Board[a%N][a/N].done > 9) { printf (" %c ", 'A'+in.Board[a%N][a/N].done-10); }
		else if (in.Board[a%N][a/N].done > 0) { printf (" %i ", in.Board[a%N][a/N].done); }
		else if (in.Board[a%N][a/N].done < 0) { printf (" P "); }
		else if (in.Board[a%N][a/N].nums) { printf (" x "); }
		else { printf (" . "); }
		if (a%N == N-1) {
			printf ("|  %i : ROW: x%X ; COLUMN: x%X ; SQUARE: x%X\n", a/N, in.Masks[a/N].Row, in.Masks[a/N].Column, in.Masks[a/N].Square);
			if (((a/(N))%n == n-1)) {
				b = 3*N+n;
				while (b-- >= 0) { printf ("-"); }
				printf ("\n");
			}
		}
	}
	printf ("\n");
	fflush(0);
	//sleep (1);
	return;
}

struct Sudoku UpDone (int x, int y, struct Sudoku in);		// prototype, so Update and Highlander isn't left hanging

struct Sudoku Highlander (int x, int y, struct Sudoku in) {
#ifdef DEBUG
	printf ("Highlander (%i, %i) in;\n", x, y);
	Display (in);
	sleep (1);
#endif

	// Find cells that are able to hold values unique to their row/column/square.

	if (in.Board[x][y].done) { return in; }
	HIGHLANDER++;
	struct Sudoku out = in;
	int SN = n*(y/n) + x/n, T = 0;
	unsigned int tmp = 1;
	while (T++ < N) {
		if (!(out.Masks[y].Row & out.Masks[x].Column & out.Masks[SN].Square & out.Board[x][y].nums & tmp)) {	// SOMEBODY NEEDS THIS NUMBER, and this cell could help!
			int K = N;
			unsigned int rass, cass, sass;
			rass = tmp; cass = tmp; sass = tmp;
			while (K-- > 0) {
				// we know the target cell CAN be T; check if ANY other cells can be T
				if (K != x) { rass &= out.Board[K][y].nums; }
				if (K != y) { cass &= out.Board[x][K].nums; }
				int a = (SN%n)*n+K%n, b = (SN/n)*n+K/n; 
				if ((a != x) || (b != y)) { sass &= out.Board[a][b].nums; }
			}
			if (rass | cass | sass) {	// only the target cell can be the one!
				out.Board[x][y].nums = ~tmp;
#ifdef DEBUG
				printf ("Highlander (%i, %i) out;\n", x, y);
#endif
				return UpDone (x, y, out);
			}
		}
		tmp = tmp << 1;
	}

#ifdef DEBUG
	printf ("Highlander (%i, %i) out;\n", x, y);
#endif
	return in;
}

struct Sudoku Update (int x, int y, struct Sudoku in) {	// update cell mask with row/column/square masks
#ifdef DEBUG
	printf ("Update (%i, %i) in;\n", x, y);
	Display (in);
	sleep (1);
#endif
	
	if (!in.Board[x][y].done && !in.paradox && in.numdone < Nsq) {	// if not already done and no errors
		UPDATE++;
		struct Sudoku out = in;
		int SN = n*(y/n) + x/n;

		// update cell
		out.Board[x][y].nums |= (in.Masks[y].Row | in.Masks[x].Column | in.Masks[SN].Square);
		in.Board[x][y].done = CellCheck (out.Board[x][y].nums);

#ifdef DEBUG
		printf ("Update (%i, %i) out;\n", x, y);
#endif
		if (in.Board[x][y].done > 0) { return UpDone (x, y, out); }
		else if (in.Board[x][y].done < 0) {
			out.paradox = 1;
			printf ("PARADOX IN UPDATE AT (%i, %i): %i\n", x, y, in.Board[x][y].done);
		}
		return out;
	}
	//else: skip cell, issue warnings if necessary

#ifdef DEBUG
	printf ("Update (%i, %i) out;\n", x, y);
#endif
	return in;
}

struct Sudoku UpDone (int x, int y, struct Sudoku in) { // cell completed; update row/column/square masks
#ifdef DEBUG
	printf ("UpDone (%i, %i) in;\n", x, y);
	Display (in);
	sleep (1);
#endif

	UPDONE++;
	struct Sudoku out = in;
	if (!out.Board[x][y].done) { out.Board[x][y].done = CellCheck (out.Board[x][y].nums); }	// update cell done bit
	out.numdone++;				// update number of completed cells

	int SN = n*(y/n) + x/n;

	// update row/column/square
	if (!((in.Masks[y].Row | in.Masks[x].Column | in.Masks[SN].Square) & ~in.Board[x][y].nums) && out.Board[x][y].done != (unsigned int)(-1)) {
	//	if row, column, or square masks already include the number on the board, then one of these already has the number in them, so we've got a paradox
#ifdef DEBUG
		printf ("UpDone picking %i for (%i, %i);\n", out.Board[x][y].done, x, y);
#endif
		out.Masks[y].Row |= ~out.Board[x][y].nums;
		out.Masks[x].Column |= ~out.Board[x][y].nums;
		out.Masks[SN].Square |= ~out.Board[x][y].nums;
	}
	else {
		out.paradox = 1;
		printf ("PARADOX IN UPDONE (%i) AT (%i, %i)\n", out.Board[x][y].done, x, y);
		return out;
	}
	// Now update all of the cells in same row and column
	int a = N;
	while (a-- > 0 && !out.paradox) {
		out = Update ((SN/n)*n+a/n, (SN%n)*n+a%n, Update (x, a, Update (a, y, out)));	// Square ( Column ( Row )))
	}
	
#ifdef DEBUG
	printf ("UpDone (%i, %i) out;\n", x, y);
#endif
	return out;
}

struct Sudoku SolveIt (struct Sudoku in) {
#ifdef DEBUG
	printf ("SolveIt in;\n");
	Display (in);
	sleep (1);
#endif
	SOLVEIT++;
	int c = 0, old = 0;
	struct Sudoku out = in;
	while (out.numdone < Nsq && old < out.numdone) {
	//	printf ("Highlander - round %i...", ++c);
		int a = Nsq;
		old = out.numdone;
		while (a-- > 0 && out.numdone < Nsq && !out.paradox) { out = Highlander (a%N, a/N, out); }	// Highlander must be run on every cell with up-to-date masks
	//	printf ("Gain of %i\n", out.numdone - old);
	}
	
#ifdef DEBUG
	printf ("SolveIt out;\n");
#endif
	return out;
}

struct Sudoku GuessIt (struct Sudoku in) {
	int a = -1;
	while (++a < Nsq && in.Board[a%N][a/N].done);	// find next unresolved cell
	if (a == Nsq) { return in; }		// if all cells done, return
	
	GUESSIT++;
	depth++;
	struct Sudoku out = in;
	int gg = 0;
	unsigned int tmp = 1;
	while (gg++ < N) {	// && !out.paradox) {
		if (!(out.Board[a%N][a/N].nums & tmp)) {	// try each number allowed by cell mask
			out.Board[a%N][a/N].nums = ~tmp & M.ask;
			int x = a%N, y = a/N;
			int SN = N*(y/N) + x/N;
			out = UpDone (a%N, a/N, out);
			printf ("Guessing %i at %i, %i; depth %i\n", CellCheck (~tmp & M.ask), a%N, a/N, depth);
			out = SolveIt (out);
			Display (out);
			sleep (1);
			if (!out.paradox) {	// no paradox, so guess value of next cell
				out = GuessIt (out);
				if (out.numdone == Nsq && !out.paradox) {
//					printf ("Success on %i at %i, %i; depth %i\n", CellCheck (~tmp & M.ask), a%N, a/N, depth);
					depth--;
					return out;
				}	// all cells done with no paradoxes, so we're finished
//				if (out.paradox) { printf ("Paradox on %i at %i, %i; depth %i\n", CellCheck (~tmp & M.ask), a%N, a/N, depth); } 
			}
			out = in;			// otherwise, undo guess and continue
		}
		tmp = tmp << 1;
	}
//	printf ("Failure on %i at %i, %i; depth %i\n", CellCheck (~tmp & M.ask), a%N, a/N, depth);
	depth--;
	out.paradox = 1;
	return out;	// we're done whether paradox or not; if so, 
}

struct Sudoku Read (const char *name) { // read in from command line
	READ++;
	FILE *fp; fp = fopen (name, "r");
	//char (*func)(FILE *);
	struct Sudoku out = InitializeSudoku ();
	int i = -1;
	if (!fp) { return out; } // something wrong or no name given
	//else { func = &fgetc; }
	printf ("Initial board:\n");
	while (i++ < (Nsq-1)) {
		if (!(i%N)) { printf ("\n"); }
		int t = fgetc (fp);
		if (t >= 'A' && t <= 'Z') {t -= 'A'-'0';}
		else if (t >= 'a' && t <= 'z') {t -= 'a'-'0';}
		else if (t == EOF) { t = 0; }
		while (t > '0'+N || t < '0') { t = fgetc (fp); }
		if (t > '0' && !out.Board[i%N][i/N].done) {
			out.Board[i%N][i/N].nums = ~(1 << (t-'1'));
			out.Board[i%N][i/N].done = t-'0'; //CellCheck (out.Board[i%N][i/N].nums);
			out = UpDone (i%N, i/N, out);
			printf (" %i ", out.Board[i%N][i/N].done);
		}
		else { printf (" . "); }
	}
	printf ("\n\n");
	fflush (0);
	fclose (fp);
	//free (func = 0);
	free (fp = 0);
	if (out.paradox) { printf ("INITIAL BOARD INVALID!\n"); }
	return out;
}

int main (int argc, char *argv[]) {
	UPDATE=0; UPDONE=0; CELLCHECK=0; SQUARENUM=0; HIGHLANDER=0; SOLVEIT=0; GUESSIT=0; DISPLAY=0; READ=0;
	M.ask = ~0;
	n = SquareNum ();
	Nsq = N*N;
	depth = 0;
	
	if (argc > 1) { printf ("Filename supplied!\n"); }
	else {
		printf ("ERROR: No file specified!\n");
		return 0;
	}
	printf ("Reading in and analyzing board;\n");
	struct Sudoku duh = Read (argv[1]);	// don't analyze during reading!
	Display (duh);
	if (duh.paradox) { return 0; }
	
	printf ("Inferring cell values with Highlander subroutine;\n");
	duh = SolveIt (duh);
	Display (duh);
	//printf ("OPERATION COUNT:\n\tCELLCHECK : %i\n\tDISPLAY : %i\n\tGUESSIT : %i\n\tHIGHLANDER : %i\n\tREAD : %i\n\tSOLVEIT : %i\n\tSQUARENUM : %i\n\tUPDATE : %i\n\tUPDONE : %i\n\t", CELLCHECK, DISPLAY, GUESSIT, HIGHLANDER, READ, SOLVEIT, SQUARENUM, UPDATE, UPDONE);

	if (duh.numdone == Nsq) {
		printf ("SUCCESS!!\n");
		printf ("OPERATION COUNT:\n\tCELLCHECK : %i\n\tDISPLAY : %i\n\tGUESSIT : %i\n\tHIGHLANDER : %i\n\tREAD : %i\n\tSOLVEIT : %i\n\tSQUARENUM : %i\n\tUPDATE : %i\n\tUPDONE : %i\n", CELLCHECK, DISPLAY, GUESSIT, HIGHLANDER, READ, SOLVEIT, SQUARENUM, UPDATE, UPDONE);
		return 1;
	}
	else if (duh.paradox || duh.numdone > Nsq) { printf ("PARADOX OR ERROR; INVALID SUDOKU BOARD\n"); return 0; }
	else {
		printf ("BOARD INCOMPLETE; %i OF %i SQUARES FILLED. FULLEST POSSIBLE SOLUTION PRESENTED\n\nRESORTING TO GUESSING...\n", duh.numdone, Nsq);
	}
	// TIME TO GUESS
	
	duh = GuessIt (duh);
	Display (duh);
	if (!duh.paradox && duh.numdone == Nsq) { printf ("Guessed right!\n"); }
	else { printf ("Guessed wrong...\n"); }
	printf ("OPERATION COUNT:\n\tCELLCHECK : %i\n\tDISPLAY : %i\n\tGUESSIT : %i\n\tHIGHLANDER : %i\n\tREAD : %i\n\tSOLVEIT : %i\n\tSQUARENUM : %i\n\tUPDATE : %i\n\tUPDONE : %i\n", CELLCHECK, DISPLAY, GUESSIT, HIGHLANDER, READ, SOLVEIT, SQUARENUM, UPDATE, UPDONE);

	/*if (duh.numdone < Nsq) {
		printf ("INTERACTIVE MODE!!\n");
		int Ix = 0, Iy = 0;
		char Ic = 's';
		while (Ic != 'q') {
			printf ("s/u/h/q x y: "); fflush (0);
			scanf ("%c %i %i", &Ic, &Ix, &Iy); 
			if (Ic == 's') { printf ("%i,%i: %X; %i\n", Ix, Iy, duh.Board[Ix][Iy].nums, duh.Board[Ix][Iy].done); }
			else if (Ic == 'u') {
				duh = Update (Ix, Iy, duh);
				Display (duh);
			}
			else if (Ic == 'h') {
				duh = Highlander (Ix, Iy, duh);
				Display (duh);
			}
		}
	}*/
	return 0;
}
