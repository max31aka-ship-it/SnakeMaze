// snake_maze.js — Змейка в лабиринте на JavaScript (Node.js)

const readline = require('readline');
const { EventEmitter } = require('events');

const WIDTH = 20, HEIGHT = 15;
const WALL = '#', EMPTY = ' ', HEAD = '@', BODY = 'o', FOOD = '$', BONUS = '*';

let maze = [];
let snake = [];
let food = {x:0,y:0}, bonus = null;
let bonusTimer = 0;
let dx = 1, dy = 0;
let score = 0, highScore = 0, level = 1;
let speed = 0.15;
let gameOver = false, paused = false;
let lastMove = Date.now();
let inputQueue = [];

// Чтение ввода без ожидания Enter (используем keypress)
readline.emitKeypressEvents(process.stdin);
process.stdin.setRawMode(true);
process.stdin.on('keypress', (str, key) => {
    if (key) {
        if (key.name === 'up') inputQueue.push('up');
        else if (key.name === 'down') inputQueue.push('down');
        else if (key.name === 'left') inputQueue.push('left');
        else if (key.name === 'right') inputQueue.push('right');
        else if (key.name === 'p') inputQueue.push('pause');
        else if (key.name === 'q') inputQueue.push('quit');
    }
});

function clearScreen() { console.clear(); }

function loadHighScore() { highScore = 0; }
function saveHighScore() {}

function generateMaze() {
    maze = Array.from({length: HEIGHT}, () => Array(WIDTH).fill(EMPTY));
    for (let x=0; x<WIDTH; x++) { maze[0][x] = WALL; maze[HEIGHT-1][x] = WALL; }
    for (let y=0; y<HEIGHT; y++) { maze[y][0] = WALL; maze[y][WIDTH-1] = WALL; }
    for (let y=2; y<HEIGHT-2; y++) {
        for (let x=2; x<WIDTH-2; x++) {
            if (Math.random() < 0.15) maze[y][x] = WALL;
        }
    }
}

function spawnSnake() {
    const cx = Math.floor(WIDTH/2), cy = Math.floor(HEIGHT/2);
    snake = [];
    for (let i=0; i<3; i++) snake.push({x: cx-i, y: cy});
    dx = 1; dy = 0;
}

function placeFood() {
    while (true) {
        const x = Math.floor(Math.random() * (WIDTH-2)) + 1;
        const y = Math.floor(Math.random() * (HEIGHT-2)) + 1;
        const p = {x,y};
        const inSnake = snake.some(s => s.x===x && s.y===y);
        if (!inSnake && maze[y][x] !== WALL && (!bonus || bonus.x!==x || bonus.y!==y)) {
            food = p;
            break;
        }
    }
}

function placeBonus() {
    if (Math.random() < 0.2) {
        while (true) {
            const x = Math.floor(Math.random() * (WIDTH-2)) + 1;
            const y = Math.floor(Math.random() * (HEIGHT-2)) + 1;
            const p = {x,y};
            const inSnake = snake.some(s => s.x===x && s.y===y);
            if (!inSnake && maze[y][x] !== WALL && (food.x!==x || food.y!==y)) {
                bonus = p;
                bonusTimer = 10;
                break;
            }
        }
    } else {
        bonus = null;
        bonusTimer = 0;
    }
}

function move() {
    if (gameOver || paused) return;
    const head = snake[0];
    const nx = head.x + dx;
    const ny = head.y + dy;
    if (nx<0 || nx>=WIDTH || ny<0 || ny>=HEIGHT || maze[ny][nx] === WALL) {
        gameOver = true;
        if (score > highScore) highScore = score;
        return;
    }
    const newHead = {x:nx, y:ny};
    if (snake.some(s => s.x===nx && s.y===ny)) {
        gameOver = true;
        return;
    }
    snake.unshift(newHead);
    let ate = false;
    if (nx===food.x && ny===food.y) {
        score++; ate = true;
        placeFood(); placeBonus();
        if (score%5===0) { level++; speed = Math.max(0.05, speed*0.9); }
    } else if (bonus && nx===bonus.x && ny===bonus.y) {
        score += 5; ate = true;
        bonus = null; bonusTimer = 0;
        placeFood(); placeBonus();
        if (score%5===0) { level++; speed = Math.max(0.05, speed*0.9); }
    }
    if (!ate) snake.pop();
    if (bonus) {
        bonusTimer--;
        if (bonusTimer<=0) bonus = null;
    }
}

function draw() {
    clearScreen();
    console.log(`🐍 SnakeMaze  |  Счёт: ${score}  |  Уровень: ${level}  |  Рекорд: ${highScore}`);
    if (paused) console.log("⏸ ПАУЗА");
    const top = '+' + '-'.repeat(WIDTH) + '+';
    console.log(top);
    for (let y=0; y<HEIGHT; y++) {
        let line = '|';
        for (let x=0; x<WIDTH; x++) {
            let c = maze[y][x];
            if (x===snake[0].x && y===snake[0].y) c = HEAD;
            else if (snake.some(s => s.x===x && s.y===y)) c = BODY;
            else if (food.x===x && food.y===y) c = FOOD;
            else if (bonus && bonus.x===x && bonus.y===y) c = BONUS;
            line += c;
        }
        line += '|';
        console.log(line);
    }
    console.log(top);
    console.log("Управление: стрелки, P - пауза, Q - выход");
}

function processInput() {
    while (inputQueue.length > 0) {
        const cmd = inputQueue.shift();
        if (cmd === 'quit') process.exit(0);
        if (cmd === 'pause') { paused = !paused; continue; }
        if (cmd === 'up' && dy !== 1) { dx=0; dy=-1; }
        else if (cmd === 'down' && dy !== -1) { dx=0; dy=1; }
        else if (cmd === 'left' && dx !== 1) { dx=-1; dy=0; }
        else if (cmd === 'right' && dx !== -1) { dx=1; dy=0; }
    }
}

function gameLoop() {
    if (!gameOver) {
        processInput();
        const now = Date.now();
        if ((now - lastMove) / 1000 > speed) {
            move();
            lastMove = now;
            if (gameOver) {
                draw();
                console.log(`ИГРА ОКОНЧЕНА! Счёт: ${score}`);
                saveHighScore();
                process.stdin.setRawMode(false);
                process.exit(0);
            }
        }
        draw();
        setTimeout(gameLoop, 20);
    }
}

// Инициализация
loadHighScore();
generateMaze();
spawnSnake();
placeFood();
placeBonus();
lastMove = Date.now();
gameLoop();
