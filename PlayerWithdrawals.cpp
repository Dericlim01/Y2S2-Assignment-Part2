#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <map>
#include <iomanip>
#include <string>
#include <cstring>
#include <stack>
#include <limits>
#include <algorithm>

using namespace std;

/**
 * Generic function to generate a new ID.
 * @param filename The file to read existing IDs from.
 * @param prefix The prefix for the ID ("APUTCP" or "W").
 * @param width The numeric part width (3 for "001").
 * @param defaultStart The starting number if no ID is found.
 * @return A new auto-incremented ID.
 */
string generateId(const string& filename, const string& prefix, int width, int defaultStart = 1) {
    ifstream file(filename);
    string lastId = "";
    string line;
    while(getline(file, line)) {
        if(line.empty()) continue;
        // Assume the ID is the first field (separated by a comma)
        stringstream ss(line);
        getline(ss, lastId, ',');
    }
    file.close();
    
    int num = defaultStart;
    if(!lastId.empty()){
        // Extract numeric part from lastId assuming it starts with prefix.
        string numStr = lastId.substr(prefix.size());
        num = stoi(numStr) + 1;
    }
    
    stringstream ss;
    ss << prefix << setfill('0') << setw(width) << num;
    return ss.str();
}

// Structure for a Player
struct Player {
    string withdrawalId;
    string playerId;
    string name;
    string reason;
    string time;
    Player* next;
};

class PlayerWithdrawals {
    private:
        stack<Player> withdrawalStack;

    public:
        /**
         * Withdraw a player
         * @param name the name of the player
         * @param reason the reason for withdrawal
         */
        void withdraw(string playerId, string name, string reason) {
            string withdrawalId = generateId("Withdrawals.txt", "W", 3);
            string currentTime = getCurrentTime();
            Player newPlayer { withdrawalId, playerId, name, reason, currentTime, nullptr };
            withdrawalStack.push(newPlayer);

            cout << "Player " << name << " has been withdrawn. Reason: " << reason << endl;
            cout << "========================================" << endl;
            saveToFile(withdrawalId, playerId, name, reason, currentTime);
        }

        // Display the withdrawn players
        void displayWithdrawals() {
            cout << "Withdrawn Players Summary:\n";
            stack<Player> temp = withdrawalStack;
            while (!temp.empty()) {
                Player top = temp.top();
                temp.pop();
                cout << "Player ID : " << top.playerId
                    << "\nName      : " << top.name
                    << "\nReason    : " << top.reason
                    << "\nTime      : " << top.time << endl;
            }
            cout << "========================================" << endl;
        }

        /**
         * Save the player withdrawal to a file
         * @param withdrawalId the ID of the withdrawal
         * @param playerId the ID of the player
         * @param name the name of the player
         * @param reason the reason for withdrawal
         * @param time the time of withdrawal
         */
        void saveToFile(string withdrawalId, string playerId, string name, string reason, string time) {
            ofstream file("Withdrawals.txt", ios::app);
            if (!file.is_open()) return;

            file << withdrawalId << "," << playerId << "," << name << "," << reason << "," << time << endl;
            file.close();
        }

        // Get the current time
        string getCurrentTime() {
            time_t now = time(0);
            char buffer[80];
            strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&now));
            return string(buffer);
        }
};

/**
 * Read players from a file
 * @param filename the name of the file
 * @return a map of players
 */
map<string, string> readPlayersFromFile(const string& filename) {
    ifstream file(filename);
    map<string, string> players;
    if (!file) return players;

    // Read players from the file
    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string playerId, name;
        getline(ss, playerId, ',');
        getline(ss, name, ',');
        players[name] = playerId;
    }
    file.close();
    return players;
}

/**
 * Check for player upcoming matches
 * @param playerId the ID of the player
 * @param filename the name of the file
 */
bool checkUpcomingMatch(const string& playerId, const string& filename) {
    ifstream file(filename);
    if (!file) {
        cout << "Error: Could not open Matches.txt\n";
        return false;
    }

    string line, matchId, stage, round, p1Id, p2Id, scheduledTime, matchStatus, courtId;

    while (getline(file, line)) {
        stringstream ss(line);
        getline(ss, matchId, ',');
        getline(ss, stage, ',');
        getline(ss, round, ',');
        getline(ss, p1Id, ',');
        getline(ss, p2Id, ',');
        getline(ss, scheduledTime, ',');
        getline(ss, matchStatus, ',');
        getline(ss, courtId, ',');

        if ((p1Id == playerId || p2Id == playerId) && matchStatus.compare("waiting") == 0) {
            cout << "\n Match Found! Player needs substitution.\n";
            cout << "================================================================================\n";
            cout << "Match ID       : " << matchId << "\n";
            cout << "Player 1 (P1)  : " << p1Id << "\n";
            cout << "Player 2 (P2)  : " << p2Id << "\n";
            cout << "Scheduled Time : " << scheduledTime << "\n";
            cout << "Status         : " << matchStatus << "\n";
            cout << "================================================================================\n";

            file.close();
            return true;
        }
    }
    cout << "No upcoming match found for player.\n";
    file.close();
    return false;
}

/**
 * Read players from a file along with their stage.
 * The Players.txt file is assumed to have six comma-separated fields:
 * playerId, name, nationality, ranking, gender, stage.
 * @param filename the name of the file
 * @return a map from player name to a pair {playerId, stage}
 */
map<string, pair<string, string>> readPlayersWithStage(const string& filename) {
    ifstream file(filename);
    map<string, pair<string, string>> players;
    if (!file) return players;
    
    string line;
    while(getline(file, line)) {
        stringstream ss(line);
        string playerId, name, nationality, ranking, gender, stage;
        getline(ss, playerId, ',');
        getline(ss, name, ',');
        getline(ss, nationality, ',');  // skip nationality
        getline(ss, ranking, ',');      // skip ranking
        getline(ss, gender, ',');       // skip gender
        getline(ss, stage, ',');        // read stage
        players[name] = make_pair(playerId, stage);
    }
    file.close();
    return players;
}

/**
 * Substitute a player using a player name lookup from Players.txt
 * @param playerId The ID of the withdrawn player.
 * @param matchesFile The matches file.
 * @param playersFile The players file.
 */
void substitutePlayer(const string& playerId, const string& matchesFile, const string& playersFile) {
    fstream file(matchesFile, ios::in | ios::out);
    if (!file) {
        cout << "Error opening matches file.\n";
        return;
    }

    // Load players from Players.txt
    auto playersWithStage = readPlayersWithStage(playersFile);
    if (playersWithStage.empty()) {
        cout << "Error reading player list.\n";
        return;
    }

    string line, matchId, stage, round, p1Id, p2Id, scheduledTime, matchStatus, courtId;
    streampos lastPos = file.tellg(); // Store position for overwriting

    // Read and update match records
    while (file.tellg() != -1) {
        lastPos = file.tellg(); // Store position before reading the line

        if (!getline(file, line)) break;
        stringstream ss(line);
        getline(ss, matchId, ',');
        getline(ss, stage, ',');
        getline(ss, round, ',');
        getline(ss, p1Id, ',');
        getline(ss, p2Id, ',');
        getline(ss, scheduledTime, ',');
        getline(ss, matchStatus, ',');
        getline(ss, courtId, ',');

        // Check if the player needs substitution
        if ((p1Id == playerId || p2Id == playerId) && matchStatus == "waiting") {
            cout << "\nMatch Found: " << matchId << endl;
            cout << "Stage" << stage << endl;

            map<string, string> availablePlayers;
            for (const auto& p : playersWithStage) {
                // p.first is player name, p.second.first is playerId, p.second.second is stage.
                if (p.second.second == stage && p.second.first != p1Id && p.second.first != p2Id)
                    availablePlayers[p.first] = p.second.first;
            }

            if (availablePlayers.empty()) {
                cout << "No available substitute players in stage " << stage << ".\n";
                continue;
            }

            // Display available substitute players.
            cout << "Available substitute players in stage " << stage << ":\n";
            cout << "----------------------------------------\n";
            for (const auto& p : availablePlayers) {
                cout << "Name: " << p.first << ", Player ID: " << p.second << "\n";
            }
            cout << "----------------------------------------\n";

            string substituteName, substituteId;
            while (true) {
                cout << "Please enter a substitute player name for match " << matchId << ": ";
                getline(cin, substituteName);

                // Find the substitute's player ID
                auto it = availablePlayers.find(substituteName);
                if (it == availablePlayers.end()) {
                    cout << "Player not found among available substitutes. Try again.\n";
                    continue;
                }

                substituteId = it -> second;
                break;
            }

            // Replace player ID
            if (p1Id == playerId) p1Id = substituteId;
            else p2Id = substituteId;

            // Move back to the beginning of the line
            file.seekp(lastPos);

            // Overwrite the line with the updated player ID
            file << matchId << ',' << stage << ',' << round << ','
                << p1Id << ',' << p2Id << ',' << scheduledTime << ','
                << matchStatus << ',' << courtId << '\n';

            file.flush(); // Ensure the changes are written
            cout << "Substituted with " << substituteName << " (ID: " << substituteId << ") in match " << matchId << ".\n";
        }
    }

    file.close();
    cout << "Substitution process completed.\n";
}

/**
 * Print available players
 * @param players the map of players
 */
void printAvailablePlayers(const map<string, string>& players) {
    cout << "Available Players:\n";
    cout << "========================================\n";
    cout << left << setw(15) << "Player ID" << "Name\n";
    cout << "========================================\n";
    for (const auto& [name, id] : players) {
        cout << left << setw(15) << id << name << endl;
    }
    cout << "========================================\n";
}

/**
 * Handle user input for player withdrawals
 * @param withdrawals the player withdrawals object
 */
void withdrawPlayer(PlayerWithdrawals &withdrawals) {
    // Read players from file
    map<string, string> players = readPlayersFromFile("Players.txt");
    printAvailablePlayers(players);

    string name;
    // Enter player name to withdraw
    cout << "Enter player name to withdraw: ";
    cin >> ws; // Remove leading whitespace
    getline(cin, name);
    cout << "----------------------------------------" << endl;

    // Check if player exists
    if (players.find(name) == players.end()) {
        cout << "Player not found" << endl;
        return;
    }

    string playerId = players[name], reason;
    // Enter reason for withdrawal
    cout << "Enter reason for withdrawal: ";
    getline(cin, reason);
    cout << "----------------------------------------" << endl;

    withdrawals.withdraw(playerId, name, reason);
    withdrawals.displayWithdrawals();
    if (checkUpcomingMatch(playerId, "Matches.txt"))
    {
        substitutePlayer(playerId, "Matches.txt", "Players.txt");
    }
}

/**
 * ----------------------------------------
 * Create a new player
 * ----------------------------------------
 */
void createNewPlayer() {
    struct newPlayer {
        string name;
        string nationality;
        string ranking;
        string gender;
        string stage;
    };

    newPlayer Player;
    cin.ignore();
    cout << "Add Player" << endl;
    cout << "Enter player name: ";
    cin >> Player.name;
    cout << "Enter player nationality: ";
    cin >> Player.nationality;
    cout << "Enter player ranking: ";
    cin >> Player.ranking;
    cout << "Enter player gender: ";
    cin >> Player.gender;
    cout << "Enter player stage: ";
    cin >> Player.stage;

    string playerId = generateId("Players.txt", "APUTCP", 3);

    ofstream file("Players.txt", ios::app);
    if (!file.is_open()) return;
    else {
        file << playerId << ","
            << Player.name << ","
            << Player.nationality << ","
            << Player.ranking << ","
            << Player.gender << ","
            << Player.stage << endl;
        file.close();
        cout << "Player added successfully." << endl;
    }
}

/**
 * ----------------------------------------
 * Track Player performance
 * ----------------------------------------
 */
struct MatchHistory {
    string matchId;
    string stage;
    string p1Id;
    string p2Id;
    string setScores;
    string matchedTime;
    string duration;
    MatchHistory* next;
};

/**
 * Function to read player names as keys (Name -> ID)
 * @param filename the name of the file
 */
map<string, string> readPlayersByName(const string& filename) {
    ifstream file(filename);
    map<string, string> players;
    if (!file) return players;

    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string playerId, name;
        getline(ss, playerId, ',');
        getline(ss, name, ',');
        players[name] = playerId;  // Name as key, ID as value
    }
    file.close();
    return players;
}

/**
 * Function to read player IDs as keys (ID -> Name)
 * @param filename the name of the file
 */
map<string, string> readPlayersByID(const string& filename) {
    ifstream file(filename);
    map<string, string> players;
    if (!file) return players;

    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string playerId, name;
        getline(ss, playerId, ',');
        getline(ss, name, ',');
        players[playerId] = name;  // ID as key, Name as value
    }
    file.close();
    return players;
}

/**
 * Function to insert a match into a linked list
 * @param head the head of the linked list
 * @param newMatch the new match to insert
 */
void insertMatch(MatchHistory*& head, const MatchHistory& newMatch) {
    MatchHistory* newNode = new MatchHistory(newMatch);
    newNode -> next = nullptr;

    if (!head) {
        head = newNode;
        return;
    }

    MatchHistory* temp = head;
    while (temp -> next)
        temp = temp -> next;

    temp -> next = newNode;
}

/**
 * Function to read matches from file
 * @param filename the name of the file
 * @param head the head of the linked list
 */
void readMatches(const string& filename, MatchHistory*& head) {
    ifstream file(filename);
    if (!file) {
        cerr << "Error opening file!" << endl;
        return;
    }

    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string historyId; // Skipping the first column
        MatchHistory match;

        getline(ss, historyId, ',');
        getline(ss, match.matchId, ',');
        getline(ss, match.stage, ',');
        getline(ss, match.p1Id, ',');
        getline(ss, match.p2Id, ',');
        getline(ss, match.setScores, ',');
        getline(ss, match.matchedTime, ',');
        getline(ss, match.duration, ',');

        insertMatch(head, match);
    }
    file.close();
}

/**
 * Function to free allocated memory
 * @param head the head of the linked list
 */
void freeMatches(MatchHistory*& head) {
    while (head) {
        MatchHistory* temp = head;
        head = head -> next;
        delete temp;
    }
}

/**
 * Function to track a player's performance
 */
void trackPlayerPerformance() {
    string playerId;
    cout << "Enter player ID: ";
    cin >> playerId;

    map<string, string> players = readPlayersByID("Players.txt");

    if (players.find(playerId) == players.end()) {
        cout << "Player not found" << endl;
        return;
    }

    MatchHistory* head = nullptr;
    readMatches("MatchHistory.txt", head);

    cout << "Player ID: " << playerId << endl;
    cout << "Player Name: " << players[playerId] << endl;
    cout << "Performance: " << endl;

    int matchesPlayed = 0, wins = 0, losses = 0;
    MatchHistory* temp = head;

    while (temp) {
        if (temp -> p1Id == playerId || temp -> p2Id == playerId) {
            matchesPlayed++;
            cout << "MatchID: " << temp->matchId << endl;
            cout << "Scores: " << temp->setScores << endl;

            // Determine win/loss from SetScores
            int p1Score = temp -> setScores[0] - '0'; // First digit
            int p2Score = temp -> setScores[2] - '0'; // Third digit

            if ((temp -> p1Id == playerId && p1Score > p2Score) ||
                (temp -> p2Id == playerId && p2Score > p1Score)) {
                wins++;
            } else {
                losses++;
            }
        }
        temp = temp -> next;
    }

    cout << "Matches Played: " << matchesPlayed << endl;
    cout << "Wins: " << wins << endl;
    cout << "Losses: " << losses << endl;

    freeMatches(head);
}

/**
 * --------------
 * Main Function
 * --------------
 */
int main() {
    PlayerWithdrawals playerWithdrawals;

    do {
        int choice;
        cout << "Menu" << endl;
        cout << "1. Withdraw a player" << endl;
        cout << "2. Add a new player" << endl;
        cout << "3. Track player performance" << endl;
        cout << "Enter Choice: ";

        cin >> choice;

        // Input validation
        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Discard input
            cout << "Invalid input. Please try again." << endl;
            continue;
        }

        switch (choice) {
            case 1:
                withdrawPlayer(playerWithdrawals);
                break;
            case 2:
                createNewPlayer();
                break;
            case 3:
                // Track Player performance
                trackPlayerPerformance();
                break;
            default:
                cout << "Invalid choice. Please try again." << endl;
                break;
        }
    } while (true);

    return 0;
}