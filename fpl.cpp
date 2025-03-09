#include <iostream>
#include <vector>
#include <unordered_map>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <fstream>  //for std::ifstream (file reading)
#include <sstream>  //for std::stringstream (parsing CSV lines)
#define TOTALBUD 100
#define TOTALWEEKS 10


using namespace std;

//define player structure
struct Player {
    string name;
    string club;
    string position;
    double price;
    int points;

    Player(string n, string c, string p, double pr) : name(n), club(c), position(p), price(pr), points(0) {}
};

//helper functions
bool compareByPrice(const Player& a, const Player& b) {
    return a.price < b.price;
}

//define FPL class
class FantasyPremierLeague {
private:
    vector<Player> database;
    vector<Player> squad;
    double budget;
    double botBudget;
    unordered_map<string, int> positionLimits; //name and price
    vector<Player> botSquad;
    int currentWeek = 0;
    int totalScore = 0;
    int botScore = 0;
    
public:
    FantasyPremierLeague() : budget(TOTALBUD), botBudget(TOTALBUD) {
        //player limits for squat in each position
        positionLimits.insert(make_pair("Attacker", 3));
        positionLimits.insert(make_pair("Midfielder", 5));
        positionLimits.insert(make_pair("Defender", 5));
        positionLimits.insert(make_pair("Goalkeeper", 2));
        loadPlayers();
    }

    void draftSquad(vector<Player>& squad, double& remainingBudget, bool isBot = false) {
        vector<Player> available;

        // Get remaining players that haven't been picked yet for this squad
        for (auto& player : database) {
            available.push_back(player);
        }
        
        sort(available.begin(), available.end(), compareByPrice); //low to high price

        unordered_map<string, int> positionCount;
        for (auto& p : squad) {
            positionCount[p.position]++; //players in each position
        }

        for (auto i = available.begin(); i != available.end();) {
            if (positionCount[i->position] < positionLimits[i->position] && remainingBudget >= i->price) { //position limit and budget check
                squad.push_back(*i);
                positionCount[i->position]++;
                remainingBudget -= i->price;
                // Find and remove the player from database
                for (auto dbIter = database.begin(); dbIter != database.end(); ++dbIter) {
                    if (dbIter->name == i->name) {
                        database.erase(dbIter);
                        break;
                    }
                }
                i = available.erase(i);  //remove from available players
                if (squad.size() == 15) break;   //stop when full
            } else {
                ++i; //skip ineligible players
            }
        }

        if (squad.size() < 15) {
            cout << (isBot ? "Bot" : "User") << " could not autofill squad with 15 players within the budget! Please clear squad and redraft\n";
        } else {
            cout << (isBot ? "Bot" : "User") << " has drafted a squad!\n";
        }
    }

    void loadPlayers() {
            ifstream file("muchmoreexpanded.csv");
            if (!file.is_open()) {
                cout << "Error opening players.csv\n";
                return;
            }

            string line, name, club, position;
            double price;
            
            while (getline(file, line)) {
                stringstream ss(line);
                getline(ss, name, ',');
                getline(ss, club, ',');
                getline(ss, position, ',');
                ss >> price;
                database.push_back(Player(name, club, position, price));
            }

            file.close();
            cout << "players successfully loaded\n";
        }
    
    void botAutofillSquad() {
        draftSquad(botSquad, botBudget, true);
    }

    void displayPlayers(const string& position, const string& club) {
        cout << "\nAvailable " << position << "s from " << club << ":\n";
        for (auto& player : database) {
            if (player.position == position && player.club == club) {
                cout << fixed << setprecision(1);
                cout << player.name << " - $" << player.price << endl;
            }
        }
    }

    // User can pick to add player to their team from the list of players that are in the csv file
    // Checks if the user has enough money and enough space in the position to buy the player and add them to the team
    bool addPlayer(const string& name) {
        for (auto it = database.begin(); it != database.end(); ++it) {
            if (it->name == name) {
                if (budget < it->price) {
                    cout << "Not enough budget!\n";
                    return false;
                }

                int count = 0;
                for (auto& p : squad) {
                    if (p.position == it->position) count++;
                }

                if (count >= positionLimits[it->position]) {
                    cout << "Position limit reached for " << it->position << "!\n";
                    return false;
                }

                // Player successfully added to the squad
                budget -= it->price;
                squad.push_back(*it);
                database.erase(it);  // Remove from database
                cout << name << " added to your squad!\n";
                return true;
            }
        }
        cout << "Player not found!\n";
        return false;
    }

    // User has the option to remove a player from their squad by typing their player number and pressing enter
    // Iterates through squad vector to find the player that matches the index entered by the user
    void removePlayer() {
        if (squad.empty()) { // In the case of 0 squad members
            cout << "Your squad is empty!\n";
            return;
        }

        cout << "\nYour current squad:\n";
        for (size_t i = 0; i < squad.size(); ++i) {
            cout << i + 1 << ". " << squad[i].name << " - $" << squad[i].price << endl;
        }
        int choice;
        cout << "Enter the number of the player to sell: ";
        if (cin >> choice) { 
            if (choice > 0 && choice <= squad.size()) { // Adds budget back to team and erases the player
                budget += squad[choice - 1].price;
                cout << squad[choice - 1].name << " sold!\n";
                squad.erase(squad.begin() + choice - 1);
            } else { // Case where input does not match a player number
                std::cout << "Invalid choice!\n";
            }
        } else { // Case where input is not an integer
            std::cout << "Invalid input! Please enter an integer.\n";
            std::cin.clear(); //clear error flag
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }

    // Function to simulate a match between the player and the bot
    void simulateGameweek(){

        if (squad.size() < 15) {
            cout << "You need a full squad (15 players) to start a gameweek!\n";
            return;
        }

        srand(time(0));
        cout << "\nGameweek " << (currentWeek + 1) << " Simulation:\n";
        
        int weeklyPoints = 0;
        int botWeeklyPoints = 0;

        // Calculates the points for the user
        for (auto& player : squad) {
            int baseScore = rand() % 5;
            int bonus = static_cast<int>(player.price / 3);
            player.points = baseScore + bonus;
            // Simulation for red and yellow cards using random number
            int cardChance = rand() % 100;
            if (cardChance < 4) {
                player.points -= 3;
                cout << player.name << " received a RED CARD! -3 points.\n";
            }
            else if (cardChance < 10) {
                player.points -= 1;
                cout << player.name << " received a YELLOW CARD! -1 point.\n";
            }
            else if (50 < cardChance && cardChance < 53) {
                player.points -= 4;
                cout << player.name << " Own Goal! -4 points.\n";
            }
            weeklyPoints += player.points;
            cout << player.name << " scored " << player.points << " points!\n";
        }

        // Calculates the points for the bot using a random number generator and a bonus for having a highly valued player
        // This is done for every member of the bots team
        for (auto& player : botSquad) {
            int baseScore = rand() % 5;
            int bonus = static_cast<int>(player.price / 3);
            player.points = baseScore + bonus;
            botWeeklyPoints += player.points;
        }

        totalScore += weeklyPoints;
        botScore += botWeeklyPoints;
        currentWeek++;

        cout << "\nYour Total Gameweek Points: " << weeklyPoints << "\n";
        cout << "Bot's Total Gameweek Points: " << botWeeklyPoints << "\n";
        cout << "Your Cumulative Score: " << totalScore << " points.\n";
        cout << "Bot's Cumulative Score: " << botScore << " points.\n";

        if (currentWeek == TOTALWEEKS) { // Prints final messages for the end of the in-game season
            cout << endl << endl << TOTALWEEKS << " weeks completed! Your final score: " << totalScore << " points.\n";
            cout << "Bot's final score: " << botScore << " points.\n";
            if (totalScore > botScore)
                cout << "Congratulations! You defeated the bot!\n";
            else if (totalScore < botScore)
                cout << "The bot defeated you! Try again next time.\n";
            else
                cout << "It's a tie!\n";
        }
    }


    void autofillSquad() {
        if (squad.size() == 15) {
            cout << "Your squad is already full!\n";
            return;
        }
        draftSquad(squad, budget, false);
    }

    void clearSquad() {
        if (squad.empty()) {
            cout << "Your squad is already empty!\n";
            return;
        }
        for (auto& player : squad) {
            budget += player.price;
        }
        squad.clear();
        cout << "Squad cleared! Your budget has been refunded.\n";
    }

    void showSquad() {
        if (squad.empty()) {
            cout << "Your squad is empty!\n";
            return;
        }
        cout << "\nYour current squad:\n";
        for (size_t i = 0; i < squad.size(); ++i) {
            cout << i + 1 << ". " << squad[i].name << " (" << squad[i].position << ") - $" << fixed << setprecision(1) << squad[i].price << endl;
        }
    }

    void showBotSquad(){
        if (botSquad.empty()) {
            cout << "Bot's squad is empty!\n";
            return;
        }
        cout << "\nBot's current squad:\n";
        for (size_t i = 0; i < botSquad.size(); ++i) {
            cout << i + 1 << ". " << botSquad[i].name << " (" << botSquad[i].position << ") - $" << fixed << setprecision(1) << botSquad[i].price << endl;
        }
    }

    void play() {
        cout << "Autofilling bot's squad...\n";
        botAutofillSquad(); //make sure the bot drafts before starting

        while (true) {
            cout << "\nBudget: $" << fixed << setprecision(1) << budget;
            cout << "\nGameweek: " << currentWeek << "/" << TOTALWEEKS;
            cout << "\nYour Total Score: " << totalScore << " points.";
            cout << "\nBot's Total Score: " << botScore << " points.";
            cout << "\n1. Buy Player\n2. Sell Player\n3. Simulate Gameweek\n4. Autofill Squad\n5. Clear Squad\n6. Show Squad\n7. Show Bot Squad\n8. Exit\nChoose an option: ";
            
            int choice;
            if (!(cin >> choice)) {
                cout << "Invalid input! Please enter a number.\n";
                cin.clear();  // Clear the error flag
                cin.ignore(numeric_limits<streamsize>::max(), '\n');  // Discard the invalid input
                continue;  // Restart the loop
            }
            cin.ignore();

            if (choice == 1) {
                string position, club, name;
                cout << "Enter position (Attacker/Midfielder/Defender/Goalkeeper): ";
                getline(cin, position);
                cout << "Enter club name: ";
                getline(cin, club);
                displayPlayers(position, club);
                cout << "Enter player name to buy: ";
                getline(cin, name);
                addPlayer(name);
            }
            else if (choice == 2) {
                removePlayer();
            }
            else if (choice == 3) {
                simulateGameweek();
                if (currentWeek >= TOTALWEEKS) {
                    break;
                }
            }
            else if (choice == 4) {
                autofillSquad();
            }
            else if (choice == 5) {
                clearSquad();
            }
            else if (choice == 6) {
                showSquad();
            }
            else if (choice == 7) {
                showBotSquad();
            }
            else if (choice == 8) {
                cout << "Exiting game...\n";
                break;
            }
            else {
                cout << "Invalid choice!\n";
            }
        }
    }
};

int main() {
    FantasyPremierLeague fpl;
    fpl.play();
    return 0;
}
