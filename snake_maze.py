# snake_maze.py — Змейка в лабиринте на Python

import random
import os
import sys
import time
from collections import deque

# Настройки
WIDTH = 20
HEIGHT = 15
WALL = '#'
EMPTY = ' '
SNAKE_HEAD = '@'
SNAKE_BODY = 'o'
FOOD = '$'
BONUS = '*'

# Направления
DIRS = {
    'up': (0, -1),
    'down': (0, 1),
    'left': (-1, 0),
    'right': (1, 0)
}
OPPOSITE = {'up': 'down', 'down': 'up', 'left': 'right', 'right': 'left'}

def clear_screen():
    os.system('cls' if os.name == 'nt' else 'clear')

class SnakeMaze:
    def __init__(self):
        self.width = WIDTH
        self.height = HEIGHT
        self.maze = []
        self.snake = deque()
        self.direction = 'right'
        self.next_direction = 'right'
        self.food = None
        self.bonus = None
        self.bonus_timer = 0
        self.score = 0
        self.high_score = self.load_high_score()
        self.game_over = False
        self.paused = False
        self.level = 1
        self.speed = 0.15  # начальная задержка
        self.generate_maze()
        self.spawn_snake()
        self.place_food()
        self.place_bonus()

    def load_high_score(self):
        try:
            with open('highscore.txt', 'r') as f:
                return int(f.read())
        except:
            return 0

    def save_high_score(self):
        with open('highscore.txt', 'w') as f:
            f.write(str(self.high_score))

    def generate_maze(self):
        # Простая генерация стен по периметру и случайные внутренние стены
        self.maze = [[EMPTY for _ in range(self.width)] for _ in range(self.height)]
        for x in range(self.width):
            self.maze[0][x] = WALL
            self.maze[self.height-1][x] = WALL
        for y in range(self.height):
            self.maze[y][0] = WALL
            self.maze[y][self.width-1] = WALL
        # случайные внутренние стены (10% клеток)
        for y in range(2, self.height-2):
            for x in range(2, self.width-2):
                if random.random() < 0.15:
                    self.maze[y][x] = WALL

    def spawn_snake(self):
        start_x = self.width // 2
        start_y = self.height // 2
        self.snake = deque()
        for i in range(3):
            self.snake.append((start_x - i, start_y))
        self.direction = 'right'
        self.next_direction = 'right'

    def place_food(self):
        while True:
            x = random.randint(1, self.width-2)
            y = random.randint(1, self.height-2)
            if (x, y) not in self.snake and self.maze[y][x] != WALL and (self.bonus is None or (x,y) != self.bonus):
                self.food = (x, y)
                break

    def place_bonus(self):
        # бонус появляется с вероятностью 20% при каждой новой еде
        if random.random() < 0.2:
            while True:
                x = random.randint(1, self.width-2)
                y = random.randint(1, self.height-2)
                if (x, y) not in self.snake and self.maze[y][x] != WALL and (x,y) != self.food:
                    self.bonus = (x, y)
                    self.bonus_timer = 10  # количество шагов до исчезновения
                    break
        else:
            self.bonus = None
            self.bonus_timer = 0

    def move(self):
        if self.game_over or self.paused:
            return
        dx, dy = DIRS[self.direction]
        head = self.snake[0]
        new_head = (head[0] + dx, head[1] + dy)
        # проверка столкновений
        if (new_head[0] < 0 or new_head[0] >= self.width or
            new_head[1] < 0 or new_head[1] >= self.height or
            self.maze[new_head[1]][new_head[0]] == WALL or
            new_head in self.snake):
            self.game_over = True
            if self.score > self.high_score:
                self.high_score = self.score
                self.save_high_score()
            return
        # движение
        self.snake.appendleft(new_head)
        # еда
        ate = False
        if new_head == self.food:
            self.score += 1
            ate = True
            self.place_food()
            self.place_bonus()
            if self.score % 5 == 0:
                self.level += 1
                self.speed = max(0.05, self.speed * 0.9)
        elif self.bonus and new_head == self.bonus:
            self.score += 5
            ate = True
            self.bonus = None
            self.bonus_timer = 0
            self.place_food()
            self.place_bonus()
            if self.score % 5 == 0:
                self.level += 1
                self.speed = max(0.05, self.speed * 0.9)
        if not ate:
            self.snake.pop()
        # обновление таймера бонуса
        if self.bonus:
            self.bonus_timer -= 1
            if self.bonus_timer <= 0:
                self.bonus = None

    def draw(self):
        clear_screen()
        print(f"🐍 SnakeMaze  |  Счёт: {self.score}  |  Уровень: {self.level}  |  Рекорд: {self.high_score}")
        if self.paused:
            print("⏸ ПАУЗА")
        # отрисовка поля
        top_border = '+' + '-' * self.width + '+'
        print(top_border)
        for y in range(self.height):
            line = '|'
            for x in range(self.width):
                if (x, y) == self.snake[0]:
                    line += SNAKE_HEAD
                elif (x, y) in self.snake:
                    line += SNAKE_BODY
                elif self.food and (x, y) == self.food:
                    line += FOOD
                elif self.bonus and (x, y) == self.bonus:
                    line += BONUS
                else:
                    line += self.maze[y][x]
            line += '|'
            print(line)
        print(top_border)
        print("Управление: стрелки, P - пауза, Q - выход")

    def get_input(self):
        # Используем библиотеку keyboard для неблокирующего ввода (установка не требуется, но может быть)
        # В этом примере используем простой ввод с ожиданием ввода (для совместимости)
        # В реальном проекте можно установить keyboard: pip install keyboard
        try:
            import keyboard
            if keyboard.is_pressed('up'): return 'up'
            if keyboard.is_pressed('down'): return 'down'
            if keyboard.is_pressed('left'): return 'left'
            if keyboard.is_pressed('right'): return 'right'
            if keyboard.is_pressed('p'): return 'pause'
            if keyboard.is_pressed('q'): return 'quit'
        except ImportError:
            # fallback: использовать msvcrt / termios
            # здесь опускаем для краткости
            pass
        return None

    def run(self):
        last_time = time.time()
        while not self.game_over:
            self.draw()
            # обработка ввода (неблокирующая, если есть keyboard)
            cmd = self.get_input()
            if cmd == 'quit':
                break
            if cmd == 'pause':
                self.paused = not self.paused
            elif cmd in DIRS and cmd != OPPOSITE[self.direction]:
                self.next_direction = cmd
            # обновление направления
            self.direction = self.next_direction
            # движение по таймеру
            if time.time() - last_time > self.speed:
                self.move()
                last_time = time.time()
                if self.game_over:
                    self.draw()
                    print("ИГРА ОКОНЧЕНА! Ваш счёт:", self.score)
                    input("Нажмите Enter для выхода...")
                    break
            time.sleep(0.02)
        self.save_high_score()

if __name__ == "__main__":
    game = SnakeMaze()
    game.run()
