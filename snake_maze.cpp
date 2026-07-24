// snake_maze.cpp — Змейка в лабиринте на C++

#include <iostream>
#include <vector>
#include <deque>
#include <random>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <ctime>
#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#endif

using namespace std;

const int WIDTH = 20, HEIGHT = 15;
const char WALL = '#', EMPTY = ' ', HEAD = '@', BODY = 'o', FOOD = '$', BONUS = '*';

struct Point { int x, y; };
enum Dir { UP, DOWN, LEFT, RIGHT };

class SnakeMaze {
private:
    vector<string> maze;
    deque<Point> snake;
    Dir dir, nextDir;
    Point food, bonus;
    int bonusTimer;
    int score, highScore, level;
    double speed;
    bool gameOver, paused;

    void loadHighScore() {
        // простой загрузчик (для демонстрации)
        highScore = 0;
    }
    void saveHighScore() {}

    void generateMaze() {
        maze.assign(HEIGHT, string(WIDTH, EMPTY));
        for (int x=0; x<WIDTH; x++) maze[0][x] = maze[HEIGHT-1][x] = WALL;
        for (int y=0; y<HEIGHT; y++) maze[y][0] = maze[y][WIDTH-1] = WALL;
        // случайные внутренние стены
        random_device rd; mt19937 gen(rd()); uniform_int_distribution<> dis(0, 100);
        for (int y=2; y<HEIGHT-2; y++)
            for (int x=2; x<WIDTH-2; x++)
                if (dis(gen) < 15) maze[y][x] = WALL;
    }

    void spawnSnake() {
        int cx = WIDTH/2, cy = HEIGHT/2;
        snake.clear();
        for (int i=0; i<3; i++) snake.push_back({cx-i, cy});
        dir = RIGHT; nextDir = RIGHT;
    }

    void placeFood() {
        random_device rd; mt19937 gen(rd()); uniform_int_distribution<> disX(1, WIDTH-2), disY(1, HEIGHT-2);
        while (true) {
            int x = disX(gen), y = disY(gen);
            bool ok = true;
            for (auto p : snake) if (p.x==x && p.y==y) ok=false;
            if (maze[y][x] == WALL) ok=false;
            if (bonusTimer>0 && x==bonus.x && y==bonus.y) ok=false;
            if (ok) { food = {x,y}; break; }
        }
    }

    void placeBonus() {
        random_device rd; mt19937 gen(rd()); uniform_int_distribution<> dis(0,100);
        if (dis(gen) < 20) {
            uniform_int_distribution<> disX(1, WIDTH-2), disY(1, HEIGHT-2);
            while (true) {
                int x = disX(gen), y = disY(gen);
                bool ok = true;
                for (auto p : snake) if (p.x==x && p.y==y) ok=false;
                if (maze[y][x] == WALL) ok=false;
                if (x==food.x && y==food.y) ok=false;
                if (ok) { bonus = {x,y}; bonusTimer = 10; break; }
            }
        } else {
            bonusTimer = 0;
        }
    }

    void move() {
        if (gameOver || paused) return;
        int dx=0, dy=0;
        if (dir==UP) dy=-1; else if (dir==DOWN) dy=1; else if (dir==LEFT) dx=-1; else dx=1;
        Point head = snake.front();
        Point newHead = {head.x+dx, head.y+dy};
        // столкновения
        if (newHead.x<0 || newHead.x>=WIDTH || newHead.y<0 || newHead.y>=HEIGHT ||
            maze[newHead.y][newHead.x] == WALL) {
            gameOver = true; return;
        }
        for (auto p : snake) if (p.x==newHead.x && p.y==newHead.y) { gameOver=true; return; }
        snake.push_front(newHead);
        bool ate = false;
        if (newHead.x==food.x && newHead.y==food.y) {
            score++; ate = true;
            placeFood(); placeBonus();
            if (score % 5 == 0) { level++; speed = max(0.05, speed * 0.9); }
        } else if (bonusTimer>0 && newHead.x==bonus.x && newHead.y==bonus.y) {
            score += 5; ate = true;
            bonusTimer = 0;
            placeFood(); placeBonus();
            if (score % 5 == 0) { level++; speed = max(0.05, speed * 0.9); }
        }
        if (!ate) snake.pop_back();
        if (bonusTimer>0) {
            bonusTimer--;
            if (bonusTimer==0) bonus = {0,0};
        }
        if (score > highScore) highScore = score;
    }

    void draw() {
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif
        cout << "🐍 SnakeMaze  |  Счёт: " << score << "  |  Уровень: " << level << "  |  Рекорд: " << highScore << endl;
        if (paused) cout << "⏸ ПАУЗА" << endl;
        string top(WIDTH+2, '-');
        cout << "+" << top << "+" << endl;
        for (int y=0; y<HEIGHT; y++) {
            cout << "|";
            for (int x=0; x<WIDTH; x++) {
                char c = maze[y][x];
                if (x==snake.front().x && y==snake.front().y) c = HEAD;
                else {
                    bool isBody = false;
                    for (auto p : snake) if (p.x==x && p.y==y) { isBody=true; break; }
                    if (isBody) c = BODY;
                    else if (food.x==x && food.y==y) c = FOOD;
                    else if (bonusTimer>0 && bonus.x==x && bonus.y==y) c = BONUS;
                }
                cout << c;
            }
            cout << "|" << endl;
        }
        cout << "+" << top << "+" << endl;
        cout << "Управление: WASD, P - пауза, Q - выход" << endl;
    }

    int getInput() {
#ifdef _WIN32
        if (_kbhit()) {
            char ch = _getch();
            if (ch == 'w') return UP;
            if (ch == 's') return DOWN;
            if (ch == 'a') return LEFT;
            if (ch == 'd') return RIGHT;
            if (ch == 'p') return -2; // пауза
            if (ch == 'q') return -1; // выход
        }
#else
        // неблокирующий ввод для Linux
        struct termios oldt, newt;
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        int oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
        char ch = 0;
        if (read(STDIN_FILENO, &ch, 1) > 0) {
            if (ch == 'w') return UP;
            if (ch == 's') return DOWN;
            if (ch == 'a') return LEFT;
            if (ch == 'd') return RIGHT;
            if (ch == 'p') return -2;
            if (ch == 'q') return -1;
        }
        fcntl(STDIN_FILENO, F_SETFL, oldf);
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif
        return 0;
    }

public:
    SnakeMaze() : score(0), highScore(0), level(1), speed(0.15), gameOver(false), paused(false), bonusTimer(0) {
        loadHighScore();
        generateMaze();
        spawnSnake();
        placeFood();
        placeBonus();
    }

    void run() {
        auto lastTime = chrono::steady_clock::now();
        while (!gameOver) {
            draw();
            int inp = getInput();
            if (inp == -1) break;
            if (inp == -2) { paused = !paused; continue; }
            if (inp == UP || inp == DOWN || inp == LEFT || inp == RIGHT) {
                if ((inp == UP && dir != DOWN) || (inp == DOWN && dir != UP) ||
                    (inp == LEFT && dir != RIGHT) || (inp == RIGHT && dir != LEFT))
                    nextDir = (Dir)inp;
            }
            dir = nextDir;
            auto now = chrono::steady_clock::now();
            if (chrono::duration<double>(now-lastTime).count() > speed) {
                move();
                lastTime = now;
                if (gameOver) {
                    draw();
                    cout << "ИГРА ОКОНЧЕНА! Счёт: " << score << endl;
                    saveHighScore();
                    cin.ignore();
                    break;
                }
            }
            this_thread::sleep_for(chrono::milliseconds(20));
        }
    }
};

int main() {
    SnakeMaze game;
    game.run();
    return 0;
}
