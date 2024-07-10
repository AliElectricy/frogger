#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <avr/interrupt.h>

#define BOARD_WIDTH 20
#define BOARD_HEIGHT 4
#define FROG_CHAR '*'
#define OBSTACLE_UP '^'
#define OBSTACLE_DOWN 'V'
#define NUM_OBSTACLE_COLUMNS 17
#define NUM_UP_OBSTACLES 10
#define NUM_DOWN_OBSTACLES 7

#define KEYPAD_PORT PORTD
#define KEYPAD_PIN PIND
#define KEYPAD_DDR DDRD

// Function prototypes
void timer_init();
uint32_t millis();
void lcd_init();
void lcd_command(unsigned char cmd);
void lcd_data(unsigned char data);
void lcd_clear();
void lcd_print(const char *str);
void lcd_set_cursor(uint8_t row, uint8_t col);
void keypad_init();
char keypad_scan();
void seven_seg_init();
void seven_seg_display(uint8_t num);
void init_game();
void update_obstacles();
void move_frog(uint8_t direction);
void update_display();

// Global variables
uint8_t frog_x = 0;
uint8_t frog_y = 1;
uint8_t lives = 3;
char board[BOARD_HEIGHT][BOARD_WIDTH];

const char keys[4][4] = {
    {'7', '8', '9', '/'},
    {'4', '5', '6', '*'},
    {'1', '2', '3', '-'},
    {'C', '0', '=', '+'}
};

volatile uint32_t timer_millis = 0;

ISR(TIMER0_COMP_vect) {
    timer_millis++;
}

void timer_init() {
    TCCR0 |= (1 << WGM01); // CTC mode
    TCCR0 |= (1 << CS01) | (1 << CS00); // Prescaler 64
    OCR0 = 249; // Compare value for 1ms
    TIMSK |= (1 << OCIE0); // Enable compare interrupt
    sei();
}

uint32_t millis() {
    uint32_t m;
    cli();
    m = timer_millis;
    sei();
    return m;
}

void lcd_init() {
    DDRA |= 0xF0; // Set PA4-PA7 as outputs for data
    DDRA |= (1 << PA0) | (1 << PA1); // Set PA0 (RS) and PA1 (E) as outputs
    _delay_ms(50);
    
    lcd_command(0x02); // Return home
    lcd_command(0x28); // 4-bit mode, 2 lines, 5x8 font
    lcd_command(0x0C); // Display on, cursor off
    lcd_command(0x06); // Increment cursor, no shift
    lcd_command(0x01); // Clear display
    _delay_ms(2);
}

void lcd_command(unsigned char cmd) {
    PORTA &= ~(1 << PA0); // RS = 0 for command
    PORTA = (PORTA & 0x0F) | (cmd & 0xF0);
    PORTA |= (1 << PA1);
    _delay_us(1);
    PORTA &= ~(1 << PA1);
    _delay_us(200);
    PORTA = (PORTA & 0x0F) | ((cmd << 4) & 0xF0);
    PORTA |= (1 << PA1);
    _delay_us(1);
    PORTA &= ~(1 << PA1);
    _delay_ms(2);
}

void lcd_data(unsigned char data) {
    PORTA |= (1 << PA0); // RS = 1 for data
    PORTA = (PORTA & 0x0F) | (data & 0xF0);
    PORTA |= (1 << PA1);
    _delay_us(1);
    PORTA &= ~(1 << PA1);
    _delay_us(200);
    PORTA = (PORTA & 0x0F) | ((data << 4) & 0xF0);
    PORTA |= (1 << PA1);
    _delay_us(1);
    PORTA &= ~(1 << PA1);
    _delay_ms(2);
}

void lcd_clear() {
    lcd_command(0x01);
    _delay_ms(2);
}

void lcd_print(const char *str) {
    while (*str) {
        lcd_data(*str++);
    }
}

void lcd_set_cursor(uint8_t row, uint8_t col) {
    uint8_t address[] = {0x00, 0x40, 0x14, 0x54};
    lcd_command(0x80 | (address[row] + col));
}

void keypad_init() {
    KEYPAD_DDR = 0x0F;  // Set PD0-PD3 as outputs for columns, PD4-PD7 as inputs for rows
    KEYPAD_PORT = 0xFF; // Enable pull-ups on PD4-PD7
}

char keypad_scan() {
    for (uint8_t col = 0; col < 4; col++) {
        KEYPAD_PORT = ~(1 << col);
        _delay_us(10);
        for (uint8_t row = 0; row < 4; row++) {
            if (!(KEYPAD_PIN & (1 << (row + 4)))) {
                while (!(KEYPAD_PIN & (1 << (row + 4))));
                return keys[row][col];
            }
        }
    }
    return 0;
}

void seven_seg_init() {
    DDRB = 0xFF; // Set all pins of PORTB as output
}

void seven_seg_display(uint8_t num) {
    static const uint8_t segments[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};
    if (num < 10) {
        PORTB = segments[num];
    } else {
        PORTB = 0x00; // Turn off all segments if number is out of range
    }
}

void init_game() {
    lcd_clear();
    lcd_print("Press any key");
    lcd_set_cursor(1, 0);
    lcd_print("to start");

    while (keypad_scan() == 0);

    for (int i = 0; i < BOARD_HEIGHT; i++) {
        for (int j = 0; j < BOARD_WIDTH; j++) {
            board[i][j] = ' ';
        }
    }

    uint8_t up_obstacles = NUM_UP_OBSTACLES;
    uint8_t down_obstacles = NUM_DOWN_OBSTACLES;
    for (int i = 1; i < BOARD_WIDTH - 1; i++) {
        if (up_obstacles > 0) {
            board[rand() % BOARD_HEIGHT][i] = OBSTACLE_UP;
            up_obstacles--;
        } else if (down_obstacles > 0) {
            board[rand() % BOARD_HEIGHT][i] = OBSTACLE_DOWN;
            down_obstacles--;
        }
    }

    frog_x = 0;
    frog_y = 1;
    lives = 3;
    board[frog_y][frog_x] = FROG_CHAR;
}

void update_obstacles() {
    for (int i = 1; i < BOARD_WIDTH - 1; i++) {
        for (int j = 0; j < BOARD_HEIGHT; j++) {
            if (board[j][i] == OBSTACLE_UP) {
                board[j][i] = ' ';
                board[(j + BOARD_HEIGHT - 1) % BOARD_HEIGHT][i] = OBSTACLE_UP;
                break;
            } else if (board[j][i] == OBSTACLE_DOWN) {
                board[j][i] = ' ';
                board[(j + 1) % BOARD_HEIGHT][i] = OBSTACLE_DOWN;
                break;
            }
        }
    }
}

void move_frog(uint8_t direction) {
    board[frog_y][frog_x] = ' ';
    switch(direction) {
        case 0: if (frog_y > 0) frog_y--; break;
        case 1: if (frog_y < BOARD_HEIGHT - 1) frog_y++; break;
        case 2: if (frog_x > 0) frog_x--; break;
        case 3: if (frog_x < BOARD_WIDTH - 1) frog_x++; break;
    }
    if (board[frog_y][frog_x] == OBSTACLE_UP || board[frog_y][frog_x] == OBSTACLE_DOWN) {
        lives--;
        if (lives == 0) return;
        frog_x = 0;
        frog_y = 1;
    }
    board[frog_y][frog_x] = FROG_CHAR;
}

void update_display() {
    lcd_clear();
    for (int i = 0; i < BOARD_HEIGHT; i++) {
        lcd_set_cursor(i, 0);
        for (int j = 0; j < BOARD_WIDTH; j++) {
            lcd_data(board[i][j]);
        }
    }
    seven_seg_display(lives);
}

int main() {
    lcd_init();
    seven_seg_init();
    keypad_init();
    timer_init();

    while (1) {
        init_game();
        uint32_t last_update = 0;
        uint32_t last_obstacle_update = 0;

        while (lives > 0 && frog_x < BOARD_WIDTH - 1) {
            uint32_t current_time = millis();

            if (current_time - last_obstacle_update >= 2000) {
                update_obstacles();
                last_obstacle_update = current_time;
            }

            char key = keypad_scan();
            if (key != 0) {
                switch(key) {
                    case '8': move_frog(0); break;
                    case '2': move_frog(1); break;
                    case '4': move_frog(2); break;
                    case '6': move_frog(3); break;
                }
            }

            if (current_time - last_update >= 100) {
                update_display();
                last_update = current_time;
            }
        }

        lcd_clear();
        if (frog_x >= BOARD_WIDTH - 1) {
            lcd_print("You Win!");
        } else {
            lcd_print("You Lose!");
        }
        _delay_ms(2000);
    }

    return 0;
}