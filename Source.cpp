#include<iostream>
#include<windows.h>
#include<conio.h>
#include<math.h>
#include <fstream>
#include<mmsystem.h>
#pragma comment(lib,"winmm.lib")
using namespace std;
enum MouseClick
{
    NO_CLICK,
    LEFT_CLICK,
    RIGHT_CLICK
};

MouseClick GetRowColByMouseClick(int& rpos, int& cpos)
{
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    DWORD Events;
    DWORD numEvents;
    INPUT_RECORD InputRecord;

    SetConsoleMode(hInput,
        ENABLE_PROCESSED_INPUT |
        ENABLE_MOUSE_INPUT |
        ENABLE_EXTENDED_FLAGS);

    GetNumberOfConsoleInputEvents(hInput, &numEvents);

    if (numEvents == 0)
        return NO_CLICK;

    ReadConsoleInput(hInput, &InputRecord, 1, &Events);

    if (InputRecord.EventType != MOUSE_EVENT)
        return NO_CLICK;

    cpos = InputRecord.Event.MouseEvent.dwMousePosition.X;
    rpos = InputRecord.Event.MouseEvent.dwMousePosition.Y;

    DWORD button = InputRecord.Event.MouseEvent.dwButtonState;

    if (button & FROM_LEFT_1ST_BUTTON_PRESSED)
        return LEFT_CLICK;

    if (button & RIGHTMOST_BUTTON_PRESSED)
        return RIGHT_CLICK;

    return NO_CLICK;
}
void gotoRowCol(int rpos, int cpos)
{
    COORD scrn;
    HANDLE hOuput = GetStdHandle(STD_OUTPUT_HANDLE);
    scrn.X = cpos;
    scrn.Y = rpos;
    SetConsoleCursorPosition(hOuput, scrn);
}

void color(int k)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, k);
}
void hideConsoleCursor()
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}
struct Cell
{
    bool isMine;      // true if the cell contains a mine
    bool isFlagged;   // true if the player has flagged the cell
    int  nCount;      // number of mines among the 8 neighboring cells
    bool isOpen;      // true if the cell has been revealed
};
int noofflags = 0;
time_t startTime;
int savedTime = 0;
struct position
{
    int r;
    int c;
};
void  createGrid(int rows, int cols, Cell**& grid)
{
    grid = new Cell * [rows];
    for (int i = 0; i < rows;i++)
    {
        grid[i] = new Cell[cols];
        for (int j = 0;j < cols;j++) {
            grid[i][j].isFlagged = false;
            grid[i][j].isMine = false;
            grid[i][j].isOpen = false;
            grid[i][j].nCount = 0;
        }

    }
}
void  destroyGrid(int rows, Cell**& grid)
{
    for (int i = 0;i < rows;i++)
    {
        delete[]grid[i];
    }
    delete[]grid;
    grid = nullptr;
}
void  updateNeighborCounts(int rows, int cols, Cell** grid, int ri, int ci)
{
    for (int i = ri - 1; i <= ri + 1;i++)
    {
        for (int j = ci - 1;j <= ci + 1;j++)
        {
            if (!(i < 0 or i >= rows or j < 0 or j >= cols)) {

                if (!(i == ri and j == ci))
                {
                    if (!grid[i][j].isMine)
                    {
                        grid[i][j].nCount++;

                    }
                }
            }
        }
    }
}
void  placeMines(int rows, int cols, Cell**& grid, int mines)
{
    int placed = 0;
    while (placed < mines)
    {
        int ri = rand() % rows;
        int ci = rand() % cols;
        if (grid[ri][ci].isMine == false)
        {
            grid[ri][ci].isMine = true;
            updateNeighborCounts(rows, cols, grid, ri, ci);
            placed += 1;
        }

    }
}

bool isValidCell(int rows, int cols, int ri, int ci)
{
    if (ri < 0 or ri >= rows or ci < 0 or ci >= cols)
    {
        return false;
    }
    else {
        return true;
    }
}
void  openCell(int ri, int ci, Cell**& grid, int& opencellsno)
{
    if (grid[ri][ci].isFlagged == true)
    {
        return;
    }
    if (grid[ri][ci].isOpen == true)
    {
        return;
    }
    grid[ri][ci].isOpen = true;
    if (grid[ri][ci].isMine == false)
    {
        opencellsno++;
    }
}
void toggleflag(Cell**& grid, int ri, int ci)
{
    if (grid[ri][ci].isOpen == true)
    {
        return;
    }
    if (grid[ri][ci].isFlagged == true)
    {
        grid[ri][ci].isFlagged = false;
        noofflags -= 1;

    }
    else if (grid[ri][ci].isFlagged == false)
    {
        grid[ri][ci].isFlagged = true;
        noofflags += 1;

    }
}

void cascadeOpenCells(Cell**& grid, int rows, int cols, int ri, int ci, int& opencellsno)
{
    int start = 0;
    int end = 0;
    position* pos = new position[rows * cols];
    pos[end].r = ri;
    pos[end].c = ci;
    end += 1;
    while (start != end)
    {
        position temp = pos[start];
        start += 1;
        if (!grid[temp.r][temp.c].isOpen)
        {
            openCell(temp.r, temp.c, grid, opencellsno);
            if (grid[temp.r][temp.c].nCount == 0)
            {
                for (int i = temp.r - 1; i <= temp.r + 1;i++)
                {
                    for (int j = temp.c - 1; j <= temp.c + 1;j++)
                    {
                        if (!(i == temp.r and j == temp.c))
                        {
                            if (isValidCell(rows, cols, i, j))
                            {

                                if (!grid[i][j].isOpen and !grid[i][j].isMine and !grid[i][j].isFlagged)
                                {
                                   
                                    pos[end].r = i;
                                    pos[end].c = j;
                                    end += 1;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    delete[]pos;
}
bool  checkWinCondition(int mines, int opencellsno, int rows, int cols)
{
    if ((rows * cols - mines) == opencellsno)
    {
        return true;
    }
    else return false;
}
void startTimer()
{
    startTime = time(0) - savedTime;
}
int getElapsedTime()
{
    return (int)difftime(time(NULL), startTime);
}


void displayGrid(Cell** grid, int rows, int cols, int mines)
{
    // Title
    color(14);
    gotoRowCol(0, 8);
    cout << "======================================";
    gotoRowCol(1, 15);
    cout << "M I N E S W E E P E R";
    gotoRowCol(2, 8);
    cout << "======================================";

    color(15);
    gotoRowCol(4, cols * 3 + 10);
    color(11);

    cout << "Time : " << getElapsedTime() << " sec      ";

    gotoRowCol(6, cols * 3 + 10);
    color(14);

    cout << "Mines Left : " << mines - noofflags << "     ";
    color(13);

    gotoRowCol(8, cols * 3 + 10);

    cout << "Flags : " << noofflags << "      ";

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            gotoRowCol(i + 4, j * 3 + 2);

            if (grid[i][j].isFlagged)
            {
                color(12);
                cout << "[F]";
            }
            else if (!grid[i][j].isOpen)
            {
                color(8);
                cout << "[ ]";
            }
            else if (grid[i][j].isMine)
            {
                color(4);
                cout << "[*]";
            }
            else if (grid[i][j].nCount == 0)
            {
                color(15);
                cout << "[ ]";
            }
            else
            {
                switch (grid[i][j].nCount)
                {
                case 1: color(9); break;
                case 2: color(10); break;
                case 3: color(12); break;
                case 4: color(1); break;
                case 5: color(6); break;
                case 6: color(11); break;
                case 7: color(13); break;
                case 8: color(7); break;
                }

                cout << "[" << grid[i][j].nCount << "]";
            }

            color(15);
        }
    }
    int buttonrow = 12;
    int buttoncol = cols * 3 + 10;

    gotoRowCol(buttonrow, buttoncol);
    color(10);
    cout << "[ SAVE GAME ]";
    gotoRowCol(buttonrow + 2, buttoncol);
    color(15);
    cout << "                          "; 
}
bool isSaveButtonClicked(int mouserow, int mousecol, int cols)
{
    int buttonrow = 12;
    int buttoncol = cols * 3 + 10;

    if (mouserow == buttonrow and mousecol >= buttoncol and mousecol <= buttoncol + 12)
    {
        return true;
    }

    return false;
}
void loadGame(const char* filename, Cell**& grid, int& rows, int& cols, int& mines, int& opencellsno)
{
    ifstream rdr(filename, ios::binary);
    if (!rdr)
    {
        cout << "file not found!" << endl;;
        return;
    }
    rdr.read((char*)&rows, sizeof(rows));
    rdr.read((char*)&cols, sizeof(cols));
    rdr.read((char*)&mines, sizeof(mines));
    rdr.read((char*)&opencellsno, sizeof(opencellsno));
    rdr.read((char*)&noofflags, sizeof(int));
    rdr.read((char*)&savedTime, sizeof(savedTime));
    createGrid(rows, cols, grid);

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            rdr.read((char*)&grid[i][j], sizeof(Cell));
        }
    }
}
void saveGame(const char* filename, Cell** grid, int rows, int cols, int mines, int opencellsno)
{
    ofstream wrt(filename, ios::binary);
    if (!wrt)
    {
        cout << "file not found!" << endl;;
        return;
    }
    wrt.write((char*)&rows, sizeof(rows));
    wrt.write((char*)&cols, sizeof(cols));
    wrt.write((char*)&mines, sizeof(mines));
    wrt.write((char*)&opencellsno, sizeof(opencellsno));
    wrt.write((char*)&noofflags, sizeof(int));
    int elapsed = getElapsedTime();
    wrt.write((char*)&elapsed, sizeof(elapsed));
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            wrt.write((char*)&grid[i][j], sizeof(Cell));
        }
    }
}
void runGameLoop(Cell** grid, int rows, int cols, int mines, int& opencellsno)


{
    while (true)
    {
        displayGrid(grid, rows, cols, mines);

        int ri, ci;
        MouseClick click = GetRowColByMouseClick(ri, ci);
        if (click == NO_CLICK)
        {
            Sleep(50);
            continue;
        }
        int mouserow = ri;
        int mousecol = ci;
        if (click == LEFT_CLICK && isSaveButtonClicked(mouserow, mousecol, cols))
        {
            saveGame("game.dat", grid, rows, cols, mines, opencellsno);

            gotoRowCol(14, cols * 3 + 10);
            color(10);
            cout << "Game Saved Successfully!";
            Sleep(300);
            PlaySound(TEXT("input-button.wav"), NULL, SND_ASYNC);
            Sleep(100);

            continue;
        }
        ri = ri - 4;       
        ci = (ci - 2) / 3;
        if (!isValidCell(rows, cols, ri, ci))
        {
            continue;
        }
        {
            if (click == RIGHT_CLICK)
            {
                PlaySound(TEXT("toggle.wav"), NULL, SND_ASYNC);
                toggleflag(grid, ri, ci);
            }

            else if (click == LEFT_CLICK)
            {

                if (!grid[ri][ci].isFlagged)
                {
                    if (grid[ri][ci].isMine)
                    {
                        PlaySound(TEXT("explosion.wav"), NULL, SND_ASYNC);

                        grid[ri][ci].isOpen = true;
                      
                        for (int i = 0; i < rows; i++)
                        {
                            for (int j = 0; j < cols; j++)
                            {
                                if (grid[i][j].isMine)
                                {
                                    openCell(i, j, grid, opencellsno);
                                }
                            }
                        }
                        displayGrid(grid, rows, cols, mines);
                        gotoRowCol(rows + 6, 0);   
                        color(12);
                        cout << "Game Over!";
                        Sleep(1500);
                        break;
                    }
                    PlaySound(TEXT("ui-click.wav"), NULL, SND_ASYNC);

                    if (grid[ri][ci].nCount == 0)
                    {
                        cascadeOpenCells(grid, rows, cols, ri, ci, opencellsno);
                    }
                    openCell(ri, ci, grid, opencellsno);
                }
            }
        }

        if (checkWinCondition(mines, opencellsno, rows, cols))
        {
            displayGrid(grid, rows, cols, mines);
            gotoRowCol(rows + 6, 0);
            PlaySound(TEXT("win.wav"), NULL, SND_ASYNC);
            color(10);
            cout << "You Win!";
            Sleep(1500); 
            break;

        }

    }
}


int main()
{

    cout << "\n\n";
    color(14);
    cout << "\t\t\t\t\t\t=================================================================\n";
    cout << " \t\t\t\t\t\t                                                                \n";
    color(15);
    cout << " \t\t\t\t\t\t                       M I N E S W E E P E R                \n";
    cout << "  \t\t\t\t\t\t                                                              \n";
    color(14);
    cout << "\t\t\t\t\t\t=================================================================\n";
    color(11);
    cout << " \t\t\t\t\t\t                  Redemption Edition - C++ Project\n\n";
    color(11);
    cout << "\t\t\t\t\t\t----------------INSTRUCTIONS----------------\n";
    color(10);
    cout << "\t\t\t\t\t\tLeft Click";
    color(15);
    cout << " : Open Cell\n";
    color(14);
    cout << "\t\t\t\t\t\tRight Click";
    color(15);
    cout << " : Flag/Unflag Cell\n";
    color(12);
    cout << "\t\t\t\t\t\tObjective";
    color(15);
    cout << " : Avoid Mines and Open All Safe Cells\n\n";
    srand(time(0));
    color(7);

    int rows = 9;
    int cols = 9;
    int mines = 10;
    int opencells = 0;
    Cell** grid = nullptr;

    cout << "\t\t\t\t\t\t1. New Game" << endl;
    cout << "\t\t\t\t\t\t2. Load Game" << endl;
    color(10);
    cout << "\t\t\t\t\t\tEnter choice: ";
    color(15);
    int choice;
    cin >> choice;
    PlaySound(TEXT("input-button.wav"), NULL, SND_ASYNC);
    if (choice == 1)
    {
        cout << "\t\t\t\t\t\tEnter 1 for Easy level, Enter 2 for Medium level, Enter 3 for Hard level: ";

        int level;
        cin >> level;
        PlaySound(TEXT("input-button.wav"), NULL, SND_ASYNC);

        if (level == 1)
        {
            rows = 9;
            cols = 9;
            mines = 10;
            noofflags = 0;
        }
        else if (level == 2)
        {
            rows = 16;
            cols = 16;
            mines = 40;
            noofflags = 0;

        }
        else if (level == 3)
        {
            rows = 16;
            cols = 30;
            mines = 99;
            noofflags = 0;

        }
        else
        {
            cout << "Invalid choice! Easy level selected by default." << endl;
        }

        createGrid(rows, cols, grid);
        placeMines(rows, cols, grid, mines);
        savedTime = 0;
    }
    else if (choice == 2)
    {
        loadGame("game.dat", grid, rows, cols, mines, opencells);
    }

    system("pause");
    system("cls");
    hideConsoleCursor();

    startTimer();
    runGameLoop(grid, rows, cols, mines, opencells);



    destroyGrid(rows, grid);

    return 0;
}