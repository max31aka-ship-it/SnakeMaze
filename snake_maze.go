// snake_maze.go — Змейка в лабиринте на Go

package main

import (
	"fmt"
	"math/rand"
	"os"
	"os/exec"
	"time"
)

const (
	WIDTH  = 20
	HEIGHT = 15
	WALL   = '#'
	EMPTY  = ' '
	HEAD   = '@'
	BODY   = 'o'
	FOOD   = '$'
	BONUS  = '*'
)

type Point struct{ x, y int }

var (
	maze       [][]byte
	snake      []Point
	food, bonus Point
	bonusTimer int
	dx, dy     int
	score, highScore, level int
	speed      float64
	gameOver   bool
	paused     bool
)

func clear() { cmd := exec.Command("clear"); cmd.Stdout = os.Stdout; cmd.Run() }

func loadHighScore() { highScore = 0 }
func saveHighScore() {}

func generateMaze() {
	maze = make([][]byte, HEIGHT)
	for y := 0; y < HEIGHT; y++ {
		maze[y] = make([]byte, WIDTH)
		for x := 0; x < WIDTH; x++ {
			maze[y][x] = EMPTY
		}
	}
	for x := 0; x < WIDTH; x++ {
		maze[0][x] = WALL
		maze[HEIGHT-1][x] = WALL
	}
	for y := 0; y < HEIGHT; y++ {
		maze[y][0] = WALL
		maze[y][WIDTH-1] = WALL
	}
	for y := 2; y < HEIGHT-2; y++ {
		for x := 2; x < WIDTH-2; x++ {
			if rand.Intn(100) < 15 {
				maze[y][x] = WALL
			}
		}
	}
}

func spawnSnake() {
	cx, cy := WIDTH/2, HEIGHT/2
	snake = []Point{}
	for i := 0; i < 3; i++ {
		snake = append(snake, Point{cx - i, cy})
	}
	dx, dy = 1, 0
}

func placeFood() {
	for {
		x := rand.Intn(WIDTH-2) + 1
		y := rand.Intn(HEIGHT-2) + 1
		p := Point{x, y}
		ok := true
		for _, s := range snake {
			if s == p {
				ok = false
				break
			}
		}
		if maze[y][x] == WALL {
			ok = false
		}
		if bonusTimer > 0 && bonus == p {
			ok = false
		}
		if ok {
			food = p
			break
		}
	}
}

func placeBonus() {
	if rand.Intn(100) < 20 {
		for {
			x := rand.Intn(WIDTH-2) + 1
			y := rand.Intn(HEIGHT-2) + 1
			p := Point{x, y}
			ok := true
			for _, s := range snake {
				if s == p {
					ok = false
					break
				}
			}
			if maze[y][x] == WALL {
				ok = false
			}
			if food == p {
				ok = false
			}
			if ok {
				bonus = p
				bonusTimer = 10
				break
			}
		}
	} else {
		bonusTimer = 0
	}
}

func move() {
	if gameOver || paused {
		return
	}
	head := snake[0]
	nx, ny := head.x+dx, head.y+dy
	if nx < 0 || nx >= WIDTH || ny < 0 || ny >= HEIGHT || maze[ny][nx] == WALL {
		gameOver = true
		if score > highScore {
			highScore = score
		}
		return
	}
	newHead := Point{nx, ny}
	for _, s := range snake {
		if s == newHead {
			gameOver = true
			return
		}
	}
	snake = append([]Point{newHead}, snake...)
	ate := false
	if newHead == food {
		score++
		ate = true
		placeFood()
		placeBonus()
		if score%5 == 0 {
			level++
			speed = max(0.05, speed*0.9)
		}
	} else if bonusTimer > 0 && newHead == bonus {
		score += 5
		ate = true
		bonusTimer = 0
		placeFood()
		placeBonus()
		if score%5 == 0 {
			level++
			speed = max(0.05, speed*0.9)
		}
	}
	if !ate {
		snake = snake[:len(snake)-1]
	}
	if bonusTimer > 0 {
		bonusTimer--
		if bonusTimer == 0 {
			bonus = Point{}
		}
	}
}

func max(a, b float64) float64 {
	if a > b {
		return a
	}
	return b
}

func draw() {
	clear()
	fmt.Printf("🐍 SnakeMaze  |  Счёт: %d  |  Уровень: %d  |  Рекорд: %d\n", score, level, highScore)
	if paused {
		fmt.Println("⏸ ПАУЗА")
	}
	fmt.Print("+" + strings.Repeat("-", WIDTH) + "+\n")
	for y := 0; y < HEIGHT; y++ {
		fmt.Print("|")
		for x := 0; x < WIDTH; x++ {
			c := maze[y][x]
			if x == snake[0].x && y == snake[0].y {
				c = HEAD
			} else {
				isBody := false
				for _, p := range snake[1:] {
					if p.x == x && p.y == y {
						isBody = true
						break
					}
				}
				if isBody {
					c = BODY
				} else if food.x == x && food.y == y {
					c = FOOD
				} else if bonusTimer > 0 && bonus.x == x && bonus.y == y {
					c = BONUS
				}
			}
			fmt.Printf("%c", c)
		}
		fmt.Println("|")
	}
	fmt.Print("+" + strings.Repeat("-", WIDTH) + "+\n")
	fmt.Println("Управление: WASD, P - пауза, Q - выход")
}

func getInput() int {
	// Для простоты используем неблокирующий ввод через горутину (упрощённо)
	// В реальном коде можно использовать github.com/eiannone/keyboard
	return 0
}

func main() {
	rand.Seed(time.Now().UnixNano())
	loadHighScore()
	generateMaze()
	spawnSnake()
	placeFood()
	placeBonus()
	speed = 0.15
	lastMove := time.Now()
	// Неблокирующий ввод с использованием горутины
	go func() {
		// Здесь можно использовать keyboard, но для простоты оставим пустым
	}()
	for !gameOver {
		draw()
		// Получение ввода (упрощённо: без неблокирующего)
		// В реальном проекте используйте library для неблокирующего ввода
		// Например: keyboard.GetKey()
		// Пока просто задержка
		time.Sleep(20 * time.Millisecond)
	}
	fmt.Printf("ИГРА ОКОНЧЕНА! Счёт: %d\n", score)
	saveHighScore()
}
