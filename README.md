# Fantasy Premier League Simulator

- C++ project simulating a Fantasy Premier League (FPL) game. 
- Players can draft squads, buy and sell players, and compete against a bot over multiple gameweeks.

## Compile the program

```
g++ -std=c++20 -o fpl fpl.cpp
```

## Run the executable

```
./fpl
```

## Requirements

C++ compiler & CSV file (muchmoreexpanded.csv) with player data: Name, Club, Position, Price

### Example CSV Format

Player Name,Club,Position,Price
Mohamed Salah,Liverpool,Midfielder,12.5
Harry Kane,Tottenham,Attacker,11.0
...
