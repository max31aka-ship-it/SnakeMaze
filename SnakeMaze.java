// SnakeMaze.java — Змейка в лабиринте на Java (Swing для графики)

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.util.*;
import java.util.List;
import java.util.Random;

public class SnakeMaze extends JPanel implements ActionListener, KeyListener {
    private static final int WIDTH = 20, HEIGHT = 15;
    private static final int CELL_SIZE = 25;
    private static final char WALL = '#', EMPTY = ' ', HEAD = '@', BODY = 'o', FOOD = '$', BONUS = '*';

    private char[][] maze;
    private List<Point> snake;
    private Point food, bonus;
    private int bonusTimer;
    private int dx, dy; // направление
    private int score, highScore, level;
    private double speed;
    private boolean gameOver, paused;
    private Timer timer;
    private Random rand;

    public SnakeMaze() {
        setPreferredSize(new Dimension(WIDTH*CELL_SIZE, HEIGHT*CELL_SIZE));
        setBackground(Color.BLACK);
        setFocusable(true);
        addKeyListener(this);
        rand = new Random();
        loadHighScore();
        initGame();
        timer = new Timer(100, this);
        timer.start();
    }

    private void loadHighScore() {
        // упрощённо
        highScore = 0;
    }

    private void initGame() {
        maze = new char[HEIGHT][WIDTH];
        for (int i=0; i<HEIGHT; i++) Arrays.fill(maze[i], EMPTY);
        for (int x=0; x<WIDTH; x++) maze[0][x] = maze[HEIGHT-1][x] = WALL;
        for (int y=0; y<HEIGHT; y++) maze[y][0] = maze[y][WIDTH-1] = WALL;
        for (int y=2; y<HEIGHT-2; y++)
            for (int x=2; x<WIDTH-2; x++)
                if (rand.nextInt(100) < 15) maze[y][x] = WALL;

        snake = new ArrayList<>();
        int cx = WIDTH/2, cy = HEIGHT/2;
        for (int i=0; i<3; i++) snake.add(new Point(cx-i, cy));
        dx = 1; dy = 0; // right
        score = 0; level = 1; speed = 0.15; gameOver = false; paused = false;
        placeFood();
        placeBonus();
    }

    private void placeFood() {
        while (true) {
            int x = rand.nextInt(WIDTH-2)+1;
            int y = rand.nextInt(HEIGHT-2)+1;
            Point p = new Point(x,y);
            if (!snake.contains(p) && maze[y][x] != WALL && (bonus==null || !p.equals(bonus))) {
                food = p;
                break;
            }
        }
    }

    private void placeBonus() {
        if (rand.nextInt(100) < 20) {
            while (true) {
                int x = rand.nextInt(WIDTH-2)+1;
                int y = rand.nextInt(HEIGHT-2)+1;
                Point p = new Point(x,y);
                if (!snake.contains(p) && maze[y][x] != WALL && !p.equals(food)) {
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

    private void move() {
        if (gameOver || paused) return;
        Point head = snake.get(0);
        int nx = head.x + dx;
        int ny = head.y + dy;
        if (nx<0 || nx>=WIDTH || ny<0 || ny>=HEIGHT || maze[ny][nx]==WALL) {
            gameOver = true;
            if (score > highScore) highScore = score;
            return;
        }
        Point newHead = new Point(nx, ny);
        if (snake.contains(newHead)) { gameOver = true; return; }
        snake.add(0, newHead);
        boolean ate = false;
        if (newHead.equals(food)) {
            score++; ate = true;
            placeFood(); placeBonus();
            if (score % 5 == 0) { level++; speed = Math.max(0.05, speed * 0.9); timer.setDelay((int)(speed*1000)); }
        } else if (bonus != null && newHead.equals(bonus)) {
            score += 5; ate = true;
            bonus = null; bonusTimer = 0;
            placeFood(); placeBonus();
            if (score % 5 == 0) { level++; speed = Math.max(0.05, speed * 0.9); timer.setDelay((int)(speed*1000)); }
        }
        if (!ate) snake.remove(snake.size()-1);
        if (bonus != null) { bonusTimer--; if (bonusTimer<=0) bonus=null; }
    }

    @Override
    public void paintComponent(Graphics g) {
        super.paintComponent(g);
        Graphics2D g2 = (Graphics2D) g;
        // отрисовка
        for (int y=0; y<HEIGHT; y++) {
            for (int x=0; x<WIDTH; x++) {
                char c = maze[y][x];
                Color col = Color.WHITE;
                if (c == WALL) col = Color.DARK_GRAY;
                else if (c == EMPTY) {
                    // проверяем змейку
                    if (x==snake.get(0).x && y==snake.get(0).y) { c = HEAD; col = Color.GREEN; }
                    else {
                        boolean isBody = false;
                        for (Point p : snake) if (p.x==x && p.y==y) { isBody=true; break; }
                        if (isBody) { c = BODY; col = new Color(0, 180, 0); }
                        else if (food!=null && x==food.x && y==food.y) { c = FOOD; col = Color.RED; }
                        else if (bonus!=null && x==bonus.x && y==bonus.y) { c = BONUS; col = Color.YELLOW; }
                    }
                }
                g2.setColor(col);
                g2.fillRect(x*CELL_SIZE, y*CELL_SIZE, CELL_SIZE, CELL_SIZE);
                if (c == WALL) {
                    g2.setColor(Color.BLACK);
                    g2.drawRect(x*CELL_SIZE, y*CELL_SIZE, CELL_SIZE, CELL_SIZE);
                }
            }
        }
        // информация
        g2.setColor(Color.WHITE);
        g2.drawString("Счёт: "+score+"  Уровень: "+level+"  Рекорд: "+highScore, 10, 20);
        if (paused) g2.drawString("ПАУЗА", WIDTH*CELL_SIZE/2-30, HEIGHT*CELL_SIZE/2);
        if (gameOver) {
            g2.drawString("ИГРА ОКОНЧЕНА! Нажмите R для рестарта", WIDTH*CELL_SIZE/2-80, HEIGHT*CELL_SIZE/2+20);
        }
    }

    @Override
    public void actionPerformed(ActionEvent e) {
        move();
        repaint();
    }

    @Override
    public void keyPressed(KeyEvent e) {
        int key = e.getKeyCode();
        if (key == KeyEvent.VK_UP && dy != 1) { dx=0; dy=-1; }
        else if (key == KeyEvent.VK_DOWN && dy != -1) { dx=0; dy=1; }
        else if (key == KeyEvent.VK_LEFT && dx != 1) { dx=-1; dy=0; }
        else if (key == KeyEvent.VK_RIGHT && dx != -1) { dx=1; dy=0; }
        else if (key == KeyEvent.VK_P) paused = !paused;
        else if (key == KeyEvent.VK_R && gameOver) {
            // рестарт
            initGame();
            timer.setDelay((int)(speed*1000));
        }
        else if (key == KeyEvent.VK_ESCAPE) System.exit(0);
    }

    @Override public void keyReleased(KeyEvent e) {}
    @Override public void keyTyped(KeyEvent e) {}

    public static void main(String[] args) {
        JFrame frame = new JFrame("🐍 SnakeMaze");
        SnakeMaze game = new SnakeMaze();
        frame.add(game);
        frame.pack();
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        frame.setVisible(true);
        frame.setLocationRelativeTo(null);
    }
}
