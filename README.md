Project Description: Frogger Game using ATmega64 Microcontroller
This project is a microcontroller-based implementation of the classic arcade game Frogger. Using an ATmega64 microcontroller, the game features a 4x20 character LCD for display, a 4x4 matrix keypad for user input, and a seven-segment display to show the remaining lives of the player.

Game Objective
The goal of the game is to navigate a frog (represented by an asterisk *) from the start line on the left side of the screen to the finish line on the right side while avoiding moving obstacles represented by ^ (upward moving) and v (downward moving) characters. The player controls the frog using the matrix keypad.

Key Features
LCD Display: The 4x20 character LCD shows the game name, the author's name, game instructions, and the game map.
Keypad Control: The 4x4 matrix keypad allows the player to move the frog in four directions: up, down, left, and right.
Seven-segment Display: Displays the number of remaining lives for the player.
Obstacle Movement: The obstacles move vertically, adding a challenge to the game.
Collision Detection: The game checks for collisions between the frog and obstacles, reducing the frog's lives upon collision.
Hardware Components
ATmega64 Microcontroller: The brain of the project, responsible for running the game logic.
4x20 Character LCD (Port A): Used to display the game environment and messages.
4x4 Matrix Keypad (Port D): Used by the player to control the frog.
Seven-segment Display (Port C): Used to show the remaining lives of the frog.
Passive Components: Resistors, capacitors, and other components for supporting the microcontroller and displays.
Gameplay
Start the Game: The player presses any key on the matrix keypad to start the game.
Control the Frog: The player uses the keypad to move the frog:
1: Move up
2: Move down
3: Move left
4: Move right
Avoid Obstacles: The frog must avoid the moving obstacles to reach the finish line.
Winning and Losing:
The player wins by reaching the rightmost column.
The frog loses a life if it collides with an obstacle. The game ends when all lives are lost.
![image](https://github.com/AliElectricy/frogger/assets/74547659/14e120e1-cecb-4c21-a905-76e571fb8733)

