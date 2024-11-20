# Competitive Typing Game for Operational Systems Class  

This project is an multiplayer competitive typing game, developed as part of the Operational Systems course at the University of São Paulo. The objective of this game is to implement key concepts of concurrent programming, such as semaphores and threads, to manage the ranking of players in real time while the game progresses.  

## Gameplay Overview  

Players compete by typing words or sentences as quickly and accurately as possible. Each player's performance is tracked and scored live. The game calculates the ranking dynamically, updating it as players type.  

## Technical Implementation  

### Threads  
- **Player Threads**: Each player has a dedicated thread handling their input and score calculation. This ensures simultaneous input processing for all players.  
- **Game Manager Thread**: A central thread oversees the gameplay, aggregates scores, and updates the live rankings.  

### Semaphores  
- **Synchronization**: Semaphores are used to coordinate access to shared resources, such as the score table and ranking list, ensuring data integrity.  
- **Concurrency Control**: They manage critical sections of the program, preventing race conditions when multiple threads attempt to update shared resources.  
- **Game Flow**: Semaphores enforce proper synchronization for game phases like starting the game, processing player input, and ending rounds.  

## Features  
1. **Live Ranking**: A scoreboard updates dynamically to reflect players' performances in real time.  
2. **Fairness**: Semaphore-controlled access ensures that the game's core operations are free from errors caused by concurrent updates.  
3. **Scalability**: The use of threads allows the game to support a large number of players with minimal performance impact.  

This project not only demonstrates the practical use of semaphores and threads but also provides an engaging platform for learning and exploring concurrent programming principles.

## Running the game 

Install SDL2 in your computer. I'm using `#include<SDL2/SDL.h>`, so if you have a linux you're probably ok. If not, let me know so we can configure OS specific import. If you use windows os, I don't care about you.

Dependencies:

```bash
sudo apt-get install libsdl2-dev
sudo apt-get install libsdl2-2.0
sudo apt install libsdl2-ttf-dev
```

```bash
    make all
    make run
```

And you should be able to see a little window popping up!
