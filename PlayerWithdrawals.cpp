#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <map>
#include <iomanip>
#include <string>

using namespace std;

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
        Player* head;
        int withdrawalCount;

    public:
        // Constructor to initialize the head
        PlayerWithdrawals() : head(nullptr), withdrawalCount(getLastWithdrawalId()) {}

        // Generate a withdrawal ID
        string generateWithdrawalId() {
            withdrawalCount++;
            stringstream ss;
            ss << "W" << setfill('0') << setw(3) << withdrawalCount;
            return ss.str();
        }

        // Get the last withdrawal ID
        int getLastWithdrawalId() {
            ifstream file("Withdrawals.txt");
            if (!file.is_open()) return 0;

            string line, lastId;
            while (getline(file, line)) {
                stringstream ss(line);
                getline(ss, lastId, ',');
            }
            file.close();
            return lastId.empty() ? 0 : stoi(lastId.substr(1));
        }

        /**
         * Withdraw a player
         * @param name the name of the player
         * @param reason the reason for withdrawal
         */
        void withdraw(string playerId, string name, string reason) {
            string withdrawalId = generateWithdrawalId();
            string currentTime = getCurrentTime();
            Player* newPlayer = new Player{withdrawalId, playerId, name, reason, currentTime, head};
            head = newPlayer;

            cout << "Player " << name << " has been withdrawn. Reason: " << reason << endl;
            saveToFile(withdrawalId, playerId, name, reason, currentTime);
        }

        // Display the withdrawn players
        void displayWithdrawals() {
            cout << "Withdrawn Players:\n";
            Player* temp = head;
            while (temp) {
                cout << "Player ID: " << temp -> playerId << ", Name: " << temp -> name 
                    << ", Reason: " << temp -> reason << ", Time: " << temp -> time << endl;
                temp = temp -> next;
            }
            cout << endl;
        }

        // Destructor to free the memory
        ~PlayerWithdrawals() {
            while (head) {
                Player* temp = head;
                head = head -> next;
                delete temp;
            }
        }

        /**
         * Save the player withdrawal to a file
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
 * Check for player upcoming matches
 * @param playerId the ID of the player
 * @param filename the name of the file
 */
bool checkUpcomingMatch(const string& playerId, const string& filename) {
    ifstream file(filename);
    if (!file) return false;

    string line, matchId, stage, round, p1Id, p2Id, scheduledTime, matchStatus, courtId;
    bool found = false;

    // Read headers
    if (getline(file, line)) {
        while (getline(file, line)) {
            stringstream ss(line);
            getline(ss, matchId, '\t');
            getline(ss, stage, '\t');
            getline(ss, round, '\t');
            getline(ss, p1Id, '\t');
            getline(ss, p2Id, '\t');
            getline(ss, scheduledTime, '\t');
            getline(ss, matchStatus, '\t');
            getline(ss, courtId, '\t');

            // Check if the player is in a waiting match
            if ((p1Id == playerId || p2Id == playerId) && matchStatus == "waiting") {
                cout << "Player " << playerId << " has an upcoming match (" << matchId 
                    << ") at " << scheduledTime << ".\n";
                file.close();
                return true;
            }
        }
    }
    file.close();
    return false;
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
    map<string, string> players = readPlayersFromFile(playersFile);
    if (players.empty()) {
        cout << "Error reading player list.\n";
        return;
    }

    string line, matchId, stage, round, p1Id, p2Id, scheduledTime, matchStatus, courtId;
    streampos lastPos = file.tellg(); // Store position for overwriting

    // Read the header
    if (getline(file, line)) {
        cout << line << endl; // Optional: Display the header
    }

    // Read and update match records
    while (file.tellg() != -1) {
        lastPos = file.tellg(); // Store position before reading the line

        if (!getline(file, line)) break;
        stringstream ss(line);
        getline(ss, matchId, '\t');
        getline(ss, stage, '\t');
        getline(ss, round, '\t');
        getline(ss, p1Id, '\t');
        getline(ss, p2Id, '\t');
        getline(ss, scheduledTime, '\t');
        getline(ss, matchStatus, '\t');
        getline(ss, courtId, '\t');

        // Check if the player needs substitution
        if ((p1Id == playerId || p2Id == playerId) && matchStatus == "waiting") {
            cout << "Please enter a substitute player name for match " << matchId << ": ";
            string substituteName;
            getline(cin, substituteName);

            // Find the substitute's player ID
            auto it = players.find(substituteName);
            if (it == players.end()) {
                cout << "Player not found in Players.txt. Substitution failed.\n";
                continue;
            }
            string substituteId = it->second;

            // Replace player ID
            if (p1Id == playerId) p1Id = substituteId;
            else p2Id = substituteId;

            // Move back to the beginning of the line
            file.seekp(lastPos);

            // Overwrite the line with the updated player ID
            file << matchId << '\t' << stage << '\t' << round << '\t' 
                << p1Id << '\t' << p2Id << '\t' << scheduledTime << '\t' 
                << matchStatus << '\t' << courtId << '\n';

            file.flush(); // Ensure the changes are written
            cout << "Substituted " << substituteName << " (ID: " << substituteId << ") in match " << matchId << ".\n";
        }
    }

    file.close();
    cout << "Substitution process completed.\n";
}

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
    cout << "Available Players:\n";
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
 * Print available players
 * @param players the map of players
 */
void printAvailablePlayers(const map<string, string>& players) {
    cout << "Available Players:\n";
    for (const auo& [name, id] :players) {
        cout << "Player ID: " << id << ", Name: " << name << endl;
    }
}

/**
 * Handle user input for player withdrawals
 * @param withdrawals the player withdrawals object
 */
void handleUserInput(PlayerWithdrawals &withdrawals) {
    // Read players from file
    map<string, string> players = readPlayersFromFile("Players.txt");
    printAvailablePlayers(players);

    string name;
    // Enter player name to withdraw
    cout << "Enter player name to withdraw: ";
    cin >> ws; // Remove leading whitespace
    getline(cin, name);

    // Check if player exists
    if (players.find(name) == players.end()) {
        cout << "Player not found" << endl;
        return;
    }

    string playerId = players[name], reason;
    // Enter reason for withdrawal
    cout << "Enter reason for withdrawal: ";
    getline(cin, reason);

    withdrawals.withdraw(playerId, name, reason);
    withdrawals.displayWithdrawals();
    checkAndSubstitutePlayer(playerId, "Matches.txt");
}

int main() {
    PlayerWithdrawals playerWithdrawals;
    handleUserInput(playerWithdrawals);
    return 0;
}