// SnakeMaze.cs — Змейка в лабиринте на C#

using System;
using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;

class SnakeMaze
{
    const int WIDTH = 20, HEIGHT = 15;
    const char WALL = '#', EMPTY = ' ', HEAD = '@', BODY = 'o', FOOD = '$', BONUS = '*';

    static char[,] maze;
    static List<(int x, int y)> snake;
    static (int x, int y) food, bonus;
    static int bonusTimer;
    static int dx, dy; // направление
    static int score, highScore, level;
    static double speed;
    static bool gameOver, paused;
    static Random rand = new Random();
    static DateTime lastMove;

    static void LoadHighScore() { highScore = 0; }
    static void SaveHighScore() { }

    static void GenerateMaze()
    {
        maze = new char[HEIGHT, WIDTH];
        for (int y=0; y<HEIGHT; y++) for (int x=0; x<WIDTH; x++) maze[y,x] = EMPTY;
        for (int x=0; x<WIDTH; x++) { maze[0,x] = WALL; maze[HEIGHT-1,x] = WALL; }
        for (int y=0; y<HEIGHT; y++) { maze[y,0] = WALL; maze[y,WIDTH-1] = WALL; }
        for (int y=2; y<HEIGHT-2; y++)
            for (int x=2; x<WIDTH-2; x++)
                if (rand.Next(100) < 15) maze[y,x] = WALL;
    }

    static void SpawnSnake()
    {
        int cx = WIDTH/2, cy = HEIGHT/2;
        snake = new List<(int,int)>();
        for (int i=0; i<3; i++) snake.Add((cx-i, cy));
        dx = 1; dy = 0;
    }

    static void PlaceFood()
    {
        while (true)
        {
            int x = rand.Next(1, WIDTH-1);
            int y = rand.Next(1, HEIGHT-1);
            if (!snake.Contains((x,y)) && maze[y,x] != WALL && (bonus.x==0 && bonus.y==0 || (x,y)!=bonus))
            { food = (x,y); break; }
        }
    }

    static void PlaceBonus()
    {
        if (rand.Next(100) < 20)
        {
            while (true)
            {
                int x = rand.Next(1, WIDTH-1);
                int y = rand.Next(1, HEIGHT-1);
                if (!snake.Contains((x,y)) && maze[y,x] != WALL && (x,y)!=food)
                { bonus = (x,y); bonusTimer = 10; break; }
            }
        }
        else { bonus = (0,0); bonusTimer = 0; }
    }

    static void Move()
    {
        if (gameOver || paused) return;
        var head = snake[0];
        int nx = head.x + dx;
        int ny = head.y + dy;
        if (nx<0 || nx>=WIDTH || ny<0 || ny>=HEIGHT || maze[ny,nx]==WALL)
        { gameOver = true; if (score>highScore) highScore=score; return; }
        var newHead = (nx, ny);
        if (snake.Contains(newHead)) { gameOver = true; return; }
        snake.Insert(0, newHead);
        bool ate = false;
        if (newHead == food)
        {
            score++; ate = true;
            PlaceFood(); PlaceBonus();
            if (score%5==0) { level++; speed = Math.Max(0.05, speed*0.9); }
        }
        else if (bonusTimer>0 && newHead == bonus)
        {
            score += 5; ate = true;
            bonusTimer = 0; bonus = (0,0);
            PlaceFood(); PlaceBonus();
            if (score%5==0) { level++; speed = Math.Max(0.05, speed*0.9); }
        }
        if (!ate) snake.RemoveAt(snake.Count-1);
        if (bonusTimer>0) { bonusTimer--; if (bonusTimer==0) bonus=(0,0); }
    }

    static void Draw()
    {
        Console.Clear();
        Console.WriteLine($"🐍 SnakeMaze  |  Счёт: {score}  |  Уровень: {level}  |  Рекорд: {highScore}");
        if (paused) Console.WriteLine("⏸ ПАУЗА");
        string top = new string('-', WIDTH+2);
        Console.WriteLine("+" + top + "+");
        for (int y=0; y<HEIGHT; y++)
        {
            Console.Write("|");
            for (int x=0; x<WIDTH; x++)
            {
                char c = maze[y,x];
                if (x==snake[0].x && y==snake[0].y) c = HEAD;
                else if (snake.Contains((x,y))) c = BODY;
                else if (food.x==x && food.y==y) c = FOOD;
                else if (bonusTimer>0 && bonus.x==x && bonus.y==y) c = BONUS;
                Console.Write(c);
            }
            Console.WriteLine("|");
        }
        Console.WriteLine("+" + top + "+");
        Console.WriteLine("Управление: WASD, P - пауза, Q - выход");
    }

    static int GetInput()
    {
        if (Console.KeyAvailable)
        {
            var key = Console.ReadKey(true).Key;
            if (key == ConsoleKey.W) return 1; // up
            if (key == ConsoleKey.S) return 2; // down
            if (key == ConsoleKey.A) return 3; // left
            if (key == ConsoleKey.D) return 4; // right
            if (key == ConsoleKey.P) return -2;
            if (key == ConsoleKey.Q) return -1;
        }
        return 0;
    }

    public static async Task Main()
    {
        LoadHighScore();
        GenerateMaze();
        SpawnSnake();
        PlaceFood();
        PlaceBonus();
        lastMove = DateTime.Now;
        while (!gameOver)
        {
            Draw();
            int inp = GetInput();
            if (inp == -1) break;
            if (inp == -2) { paused = !paused; continue; }
            if (inp == 1 && dy != 1) { dx=0; dy=-1; }
            else if (inp == 2 && dy != -1) { dx=0; dy=1; }
            else if (inp == 3 && dx != 1) { dx=-1; dy=0; }
            else if (inp == 4 && dx != -1) { dx=1; dy=0; }
            var now = DateTime.Now;
            if ((now - lastMove).TotalSeconds > speed)
            {
                Move();
                lastMove = now;
                if (gameOver)
                {
                    Draw();
                    Console.WriteLine($"ИГРА ОКОНЧЕНА! Счёт: {score}");
                    SaveHighScore();
                    Console.ReadKey();
                    break;
                }
            }
            await Task.Delay(20);
        }
    }
}
