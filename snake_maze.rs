// snake_maze.rs — Змейка в лабиринте на Rust

use std::io::{self, Write, stdout};
use std::thread;
use std::time::{Duration, Instant};
use rand::Rng;
use termion::{clear, cursor, color, style};
use termion::input::TermRead;

const WIDTH: usize = 20;
const HEIGHT: usize = 15;
const WALL: char = '#';
const EMPTY: char = ' ';
const HEAD: char = '@';
const BODY: char = 'o';
const FOOD: char = '$';
const BONUS: char = '*';

type Point = (usize, usize);

struct SnakeMaze {
    maze: Vec<Vec<char>>,
    snake: Vec<Point>,
    food: Point,
    bonus: Option<Point>,
    bonus_timer: i32,
    dir: (i32, i32),
    next_dir: (i32, i32),
    score: i32,
    high_score: i32,
    level: i32,
    speed: f64,
    game_over: bool,
    paused: bool,
}

impl SnakeMaze {
    fn new() -> Self {
        let mut game = SnakeMaze {
            maze: vec![vec![EMPTY; WIDTH]; HEIGHT],
            snake: Vec::new(),
            food: (0,0),
            bonus: None,
            bonus_timer: 0,
            dir: (1,0),
            next_dir: (1,0),
            score: 0,
            high_score: 0,
            level: 1,
            speed: 0.15,
            game_over: false,
            paused: false,
        };
        game.load_high_score();
        game.generate_maze();
        game.spawn_snake();
        game.place_food();
        game.place_bonus();
        game
    }

    fn load_high_score(&mut self) {
        self.high_score = 0;
    }

    fn generate_maze(&mut self) {
        for y in 0..HEIGHT {
            for x in 0..WIDTH {
                self.maze[y][x] = EMPTY;
            }
        }
        for x in 0..WIDTH {
            self.maze[0][x] = WALL;
            self.maze[HEIGHT-1][x] = WALL;
        }
        for y in 0..HEIGHT {
            self.maze[y][0] = WALL;
            self.maze[y][WIDTH-1] = WALL;
        }
        let mut rng = rand::thread_rng();
        for y in 2..HEIGHT-2 {
            for x in 2..WIDTH-2 {
                if rng.gen_range(0..100) < 15 {
                    self.maze[y][x] = WALL;
                }
            }
        }
    }

    fn spawn_snake(&mut self) {
        let cx = WIDTH/2;
        let cy = HEIGHT/2;
        self.snake.clear();
        for i in 0..3 {
            self.snake.push((cx - i, cy));
        }
        self.dir = (1,0);
        self.next_dir = (1,0);
    }

    fn place_food(&mut self) {
        let mut rng = rand::thread_rng();
        loop {
            let x = rng.gen_range(1..WIDTH-1);
            let y = rng.gen_range(1..HEIGHT-1);
            let p = (x,y);
            if !self.snake.contains(&p) && self.maze[y][x] != WALL {
                if let Some(b) = self.bonus {
                    if b == p { continue; }
                }
                self.food = p;
                break;
            }
        }
    }

    fn place_bonus(&mut self) {
        let mut rng = rand::thread_rng();
        if rng.gen_range(0..100) < 20 {
            loop {
                let x = rng.gen_range(1..WIDTH-1);
                let y = rng.gen_range(1..HEIGHT-1);
                let p = (x,y);
                if !self.snake.contains(&p) && self.maze[y][x] != WALL && self.food != p {
                    self.bonus = Some(p);
                    self.bonus_timer = 10;
                    break;
                }
            }
        } else {
            self.bonus = None;
            self.bonus_timer = 0;
        }
    }

    fn move_snake(&mut self) {
        if self.game_over || self.paused { return; }
        let head = self.snake[0];
        let nx = head.0 as i32 + self.dir.0;
        let ny = head.1 as i32 + self.dir.1;
        if nx < 0 || nx >= WIDTH as i32 || ny < 0 || ny >= HEIGHT as i32 {
            self.game_over = true;
            if self.score > self.high_score { self.high_score = self.score; }
            return;
        }
        let new_head = (nx as usize, ny as usize);
        if self.maze[new_head.1][new_head.0] == WALL {
            self.game_over = true;
            return;
        }
        if self.snake.contains(&new_head) {
            self.game_over = true;
            return;
        }
        self.snake.insert(0, new_head);
        let mut ate = false;
        if new_head == self.food {
            self.score += 1;
            ate = true;
            self.place_food();
            self.place_bonus();
            if self.score % 5 == 0 {
                self.level += 1;
                self.speed = (self.speed * 0.9).max(0.05);
            }
        } else if let Some(b) = self.bonus {
            if new_head == b {
                self.score += 5;
                ate = true;
                self.bonus = None;
                self.bonus_timer = 0;
                self.place_food();
                self.place_bonus();
                if self.score % 5 == 0 {
                    self.level += 1;
                    self.speed = (self.speed * 0.9).max(0.05);
                }
            }
        }
        if !ate {
            self.snake.pop();
        }
        if let Some(_) = self.bonus {
            self.bonus_timer -= 1;
            if self.bonus_timer <= 0 {
                self.bonus = None;
            }
        }
    }

    fn draw(&self) {
        print!("{}{}", clear::All, cursor::Goto(1,1));
        println!("🐍 SnakeMaze  |  Счёт: {}  |  Уровень: {}  |  Рекорд: {}", self.score, self.level, self.high_score);
        if self.paused {
            println!("⏸ ПАУЗА");
        }
        let top = format!("+{}+", "-".repeat(WIDTH));
        println!("{}", top);
        for y in 0..HEIGHT {
            print!("|");
            for x in 0..WIDTH {
                let mut c = self.maze[y][x];
                if x == self.snake[0].0 && y == self.snake[0].1 {
                    c = HEAD;
                } else if self.snake.iter().any(|p| p.0==x && p.1==y) {
                    c = BODY;
                } else if self.food.0==x && self.food.1==y {
                    c = FOOD;
                } else if let Some(b) = self.bonus {
                    if b.0==x && b.1==y { c = BONUS; }
                }
                print!("{}", c);
            }
            println!("|");
        }
        println!("{}", top);
        println!("Управление: стрелки, P - пауза, Q - выход");
        stdout().flush().unwrap();
    }

    fn run(&mut self) {
        let stdin = io::stdin();
        let mut keys = stdin.keys();
        let mut last_time = Instant::now();
        while !self.game_over {
            self.draw();
            // обработка ввода
            if let Some(Ok(key)) = keys.next() {
                match key {
                    termion::event::Key::Up => if self.dir.1 != 1 { self.next_dir = (0,-1); },
                    termion::event::Key::Down => if self.dir.1 != -1 { self.next_dir = (0,1); },
                    termion::event::Key::Left => if self.dir.0 != 1 { self.next_dir = (-1,0); },
                    termion::event::Key::Right => if self.dir.0 != -1 { self.next_dir = (1,0); },
                    termion::event::Key::Char('p') => self.paused = !self.paused,
                    termion::event::Key::Char('q') => return,
                    _ => {}
                }
            }
            self.dir = self.next_dir;
            if last_time.elapsed().as_secs_f64() > self.speed {
                self.move_snake();
                last_time = Instant::now();
                if self.game_over {
                    self.draw();
                    println!("ИГРА ОКОНЧЕНА! Счёт: {}", self.score);
                    break;
                }
            }
            thread::sleep(Duration::from_millis(20));
        }
    }
}

fn main() {
    let mut game = SnakeMaze::new();
    game.run();
}
