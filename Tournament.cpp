#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>
#include <algorithm>
#include <stdexcept>
#include <regex>
#include <limits>
#include <queue>

using namespace std;

class ValidationException : public runtime_error {
public:
    ValidationException(const string& message) 
        : runtime_error(message) {}
};

struct Courts {
    string courtID;
    string courtType;
    int capacity;
    int maxConcurrentMatches;
};

class Players {
public:
    string playerID;
    string name;
    string nationality;
    int ranking;
    char gender;
    string stageID;
};

class Matches {
public:
    string matchID;
    string stageID;
    string roundID;
    string p1ID;
    string p2ID;
    string scheduledTime;
    string matchStatus;
    string courtID;
};

struct TimeSlot {
    string date;
    int startHour;
    string formattedTime;
    string courtID;
    int concurrentCount;
};

class TournamentScheduler {
private:
    Courts* courts;
    int courtsCount;
    Matches* matches;
    int matchesCount;
    int matchesCapacity;
    Players* players;
    int playersCount;
    int playersCapacity;
    queue<TimeSlot>* courtSchedules;

    void initializeSchedules() {
        courtSchedules = new queue<TimeSlot>[courtsCount];
        
        string dates[] = {"28-04-2025", "29-04-2025", "30-04-2025"};
        
        for (int courtIndex = 0; courtIndex < courtsCount; courtIndex++) {
            string date = dates[courtIndex];
            string courtID = courts[courtIndex].courtID;
            int maxConcurrent = (courtID == "C001") ? 2 : 1;
            
            for (int hour = 7; hour < 19; hour++) {
                stringstream timeSS;
                timeSS << date << " " << setfill('0') << setw(2) << hour << ":00";
                
                TimeSlot slot;
                slot.date = date;
                slot.startHour = hour;
                slot.formattedTime = timeSS.str();
                slot.courtID = courtID;
                slot.concurrentCount = 0;
                
                courtSchedules[courtIndex].push(slot);
            }
        }
    }

    void resizeMatchesArray() {
        int newCapacity = matchesCapacity == 0 ? 10 : matchesCapacity * 2;
        Matches* newMatches = new Matches[newCapacity];
        
        for (int i = 0; i < matchesCount; ++i) {
            newMatches[i] = matches[i];
        }
        
        delete[] matches;
        matches = newMatches;
        matchesCapacity = newCapacity;
    }

    void resizePlayersArray() {
        int newCapacity = playersCapacity == 0 ? 50 : playersCapacity * 2;
        Players* newPlayers = new Players[newCapacity];
        
        for (int i = 0; i < playersCount; ++i) {
            newPlayers[i] = players[i];
        }
        
        delete[] players;
        players = newPlayers;
        playersCapacity = newCapacity;
    }

    void validatePlayerID(const string& playerID) {
        if (playerID.empty()) {
            throw ValidationException("Player ID cannot be empty");
        }
        if (!regex_match(playerID, regex("^APUTCP\\d{3}$"))) {
            throw ValidationException("Invalid Player ID format. Must be APUTCP followed by 3 digits");
        }
    }

    void validateStageID(const string& stageID) {
        const string validStages[] = {"S001", "S002", "S003"};
        bool valid = false;
        for (const auto& stage : validStages) {
            if (stageID == stage) {
                valid = true;
                break;
            }
        }
        if (!valid) {
            throw ValidationException("Invalid Stage ID. Must be S001, S002, or S003");
        }
    }
    
    void loadPlayers() {
        ifstream playerFile("Players.txt");
        if (!playerFile) {
            throw ValidationException("Cannot open Players.txt");
        }

        playersCount = 0;
        playersCapacity = 50;
        players = new Players[playersCapacity];

        string line;
        while (getline(playerFile, line)) {
            if (playersCount >= playersCapacity) {
                resizePlayersArray();
            }

            istringstream iss(line);
            Players& player = players[playersCount];
            
            getline(iss, player.playerID, ',');
            getline(iss, player.name, ',');
            getline(iss, player.nationality, ',');
            
            string temp;
            getline(iss, temp, ',');
            player.ranking = stoi(temp);
            
            getline(iss, temp, ',');
            player.gender = temp[0];
            
            getline(iss, player.stageID);

            playersCount++;
        }
        playerFile.close();
    }
    
    void loadMatches() {
        ifstream matchFile("Matches.txt");
        if (!matchFile) {
            ofstream createFile("Matches.txt");
            createFile.close();
            
            matchesCount = 0;
            matchesCapacity = 10;
            matches = new Matches[matchesCapacity];
            
            initializeSchedules();
            
            return;
        }

        matchesCount = 0;
        matchesCapacity = 10;
        matches = new Matches[matchesCapacity];

        string line;
        while (getline(matchFile, line)) {
            if (matchesCount >= matchesCapacity) {
                resizeMatchesArray();
            }

            istringstream iss(line);
            Matches& match = matches[matchesCount];
            
            getline(iss, match.matchID, ',');
            getline(iss, match.stageID, ',');
            getline(iss, match.roundID, ',');
            getline(iss, match.p1ID, ',');
            getline(iss, match.p2ID, ',');
            getline(iss, match.scheduledTime, ',');
            getline(iss, match.matchStatus, ',');
            getline(iss, match.courtID);

            matchesCount++;
        }
        matchFile.close();
        
        initializeSchedules();
        updateSchedulesFromMatches();
    }
    
    void updateSchedulesFromMatches() {
        for (int i = 0; i < matchesCount; i++) {
            Matches& match = matches[i];
            
            if (match.scheduledTime == "TBD") {
                continue;
            }
            
            int courtIndex = getCourtIndexFromID(match.courtID);
            if (courtIndex == -1) continue;
            
            queue<TimeSlot> tempQueue;
            while (!courtSchedules[courtIndex].empty()) {
                TimeSlot slot = courtSchedules[courtIndex].front();
                courtSchedules[courtIndex].pop();
                
                if (slot.formattedTime == match.scheduledTime) {
                    slot.concurrentCount++;
                }
                
                tempQueue.push(slot);
            }
            
            courtSchedules[courtIndex] = tempQueue;
        }
    }
    
    int getCourtIndexFromID(const string& courtID) {
        for (int i = 0; i < courtsCount; i++) {
            if (courts[i].courtID == courtID) {
                return i;
            }
        }
        return -1;
    }
    
    int getCourtIndexFromStage(const string& stageID) {
        if (stageID == "S001") return 0;  // C001
        if (stageID == "S002") return 1;  // C002
        if (stageID == "S003") return 2;  // C003
        return -1;
    }
    
    string getNextAvailableTimeSlot(const string& stageID) {
        int courtIndex = getCourtIndexFromStage(stageID);
        if (courtIndex == -1) {
            return "TBD";
        }
        
        int maxConcurrent = (courtIndex == 0) ? 2 : 1;
        queue<TimeSlot> tempQueue;
        string availableTime = "TBD";
        
        while (!courtSchedules[courtIndex].empty()) {
            TimeSlot slot = courtSchedules[courtIndex].front();
            courtSchedules[courtIndex].pop();
            
            if (slot.concurrentCount < maxConcurrent) {
                slot.concurrentCount++;
                availableTime = slot.formattedTime;
                tempQueue.push(slot);
                
                while (!courtSchedules[courtIndex].empty()) {
                    tempQueue.push(courtSchedules[courtIndex].front());
                    courtSchedules[courtIndex].pop();
                }
                
                courtSchedules[courtIndex] = tempQueue;
                return availableTime;
            }
            
            tempQueue.push(slot);
        }
        
        courtSchedules[courtIndex] = tempQueue;
        return "TBD - No available slots";
    }

    void saveMatchesToFile() {
        ofstream matchFile("Matches.txt");
        if (!matchFile) {
            throw ValidationException("Cannot open Matches.txt for writing");
        }

        for (int i = 0; i < matchesCount; ++i) {
            matchFile << matches[i].matchID << ","
                     << matches[i].stageID << ","
                     << matches[i].roundID << ","
                     << matches[i].p1ID << ","
                     << matches[i].p2ID << ","
                     << matches[i].scheduledTime << ","
                     << matches[i].matchStatus << ","
                     << matches[i].courtID << endl;
        }

        matchFile.close();
    }

public:
    TournamentScheduler() {
        courts = new Courts[3]{
            {"C001", "Center", 1500, 2},
            {"C002", "Championship", 1000, 1},
            {"C003", "Progression", 750, 1}
        };
        courtsCount = 3;

        matches = nullptr;
        matchesCount = 0;
        matchesCapacity = 0;

        players = nullptr;
        playersCount = 0;
        playersCapacity = 0;
        
        courtSchedules = nullptr;

        try {
            loadPlayers();
            loadMatches();
        } catch (const exception& e) {
            cerr << "Error loading data: " << e.what() << endl;
            playersCount = 0;
            playersCapacity = 10;
            players = new Players[playersCapacity];
            
            matchesCount = 0;
            matchesCapacity = 10;
            matches = new Matches[matchesCapacity];
            
            initializeSchedules();
        }
    }

    ~TournamentScheduler() {
        delete[] courts;
        delete[] matches;
        delete[] players;
        delete[] courtSchedules;
    }

    void displayAvailablePlayers(const string& stageID) {
        cout << "Available Players in Stage " << stageID << ":" << endl;
        bool foundPlayers = false;

        for (int i = 0; i < playersCount; ++i) {
            if (players[i].stageID == stageID) {
                bool inMatch = false;
                for (int j = 0; j < matchesCount; ++j) {
                    if (matches[j].p1ID == players[i].playerID || 
                        matches[j].p2ID == players[i].playerID) {
                        inMatch = true;
                        break;
                    }
                }

                if (!inMatch) {
                    cout << "Player ID: " << players[i].playerID 
                         << ", Name: " << players[i].name 
                         << ", Nationality: " << players[i].nationality 
                         << endl;
                    foundPlayers = true;
                }
            }
        }

        if (!foundPlayers) {
            cout << "No available players in this stage." << endl;
        }
    }

    bool scheduleMatch(const string& p1ID) {
        try {
            validatePlayerID(p1ID);
            
            for (int i = 0; i < matchesCount; ++i) {
                if (matches[i].p1ID == p1ID || matches[i].p2ID == p1ID) {
                    throw ValidationException("Player 1 is already scheduled in a match");
                }
            }
            
            Players player1;
            bool player1Found = false;
            
            for (int i = 0; i < playersCount; ++i) {
                if (players[i].playerID == p1ID) {
                    player1 = players[i];
                    player1Found = true;
                    break;
                }
            }

            if (!player1Found) {
                throw ValidationException("Player 1 not found");
            }
            
            string stageID = player1.stageID;
            
            cout << "\nAvailable players for match in Stage " << stageID << ":" << endl;
            
            int availableCount = 0;
            string* availablePlayerIDs = new string[playersCount];
            
            for (int i = 0; i < playersCount; ++i) {
                Players& potentialOpponent = players[i];
                
                if (potentialOpponent.playerID == p1ID || potentialOpponent.stageID != stageID) {
                    continue;
                }
                
                bool inMatch = false;
                for (int j = 0; j < matchesCount; ++j) {
                    if (matches[j].p1ID == potentialOpponent.playerID || 
                        matches[j].p2ID == potentialOpponent.playerID) {
                        inMatch = true;
                        break;
                    }
                }
                
                if (!inMatch) {
                    cout << (availableCount + 1) << ". Player ID: " << potentialOpponent.playerID 
                         << ", Name: " << potentialOpponent.name 
                         << ", Nationality: " << potentialOpponent.nationality << endl;
                    
                    availablePlayerIDs[availableCount] = potentialOpponent.playerID;
                    availableCount++;
                }
            }
            
            if (availableCount == 0) {
                delete[] availablePlayerIDs;
                throw ValidationException("No available opponents in the same stage");
            }
            
            string selection;
            int selectedIndex;
            
            cout << "\nSelect opponent by number (1-" << availableCount << "): ";
            getline(cin, selection);
            
            try {
                selectedIndex = stoi(selection) - 1;
                if (selectedIndex < 0 || selectedIndex >= availableCount) {
                    throw out_of_range("Index out of range");
                }
            } catch (const exception&) {
                delete[] availablePlayerIDs;
                throw ValidationException("Invalid selection");
            }
            
            string p2ID = availablePlayerIDs[selectedIndex];
            delete[] availablePlayerIDs;
            
            string courtID;
            if (stageID == "S001") courtID = "C001";
            else if (stageID == "S002") courtID = "C002";
            else if (stageID == "S003") courtID = "C003";
            else throw ValidationException("Invalid stage ID");
            
            string scheduledTime = getNextAvailableTimeSlot(stageID);
            
            if (matchesCount >= matchesCapacity) {
                resizeMatchesArray();
            }

            Matches& newMatch = matches[matchesCount];
            
            stringstream ss;
            ss << "M" << setw(3) << setfill('0') << (matchesCount + 1);
            newMatch.matchID = ss.str();
            
            newMatch.stageID = stageID;
            newMatch.roundID = "R001";
            newMatch.p1ID = p1ID;
            newMatch.p2ID = p2ID;
            newMatch.scheduledTime = scheduledTime;
            newMatch.matchStatus = "waiting";
            newMatch.courtID = courtID;

            matchesCount++;

            saveMatchesToFile();

            cout << "Match scheduled successfully!" << endl;
            cout << "Match details: " << newMatch.matchID << ", " 
                 << newMatch.stageID << ", " 
                 << "Players: " << p1ID << " vs " << p2ID << ", "
                 << "Time: " << scheduledTime << ", "
                 << "Court: " << courtID << endl;
            
            return true;
        } catch (const ValidationException& e) {
            cerr << "Match scheduling error: " << e.what() << endl;
            return false;
        }
    }

    bool advancePlayerStage(const string& playerID) {
        try {
            validatePlayerID(playerID);
            
            int playerIndex = -1;
            for (int i = 0; i < playersCount; ++i) {
                if (players[i].playerID == playerID) {
                    playerIndex = i;
                    break;
                }
            }
            
            if (playerIndex == -1) {
                throw ValidationException("Player not found");
            }
            
            Players& player = players[playerIndex];
            
            if (player.stageID == "S003") {
                throw ValidationException("Player is already at the highest stage (Knockout)");
            }
            
            bool hasCompletedMatch = false;
            for (int i = 0; i < matchesCount; ++i) {
                if ((matches[i].p1ID == playerID || matches[i].p2ID == playerID) && 
                    matches[i].matchStatus == "completed") {
                    hasCompletedMatch = true;
                    break;
                }
            }
            
            if (!hasCompletedMatch) {
                throw ValidationException("Player must complete a match in current stage before advancing");
            }
            
            string currentStage = player.stageID;
            string nextStage;
            
            if (currentStage == "S001") {
                nextStage = "S002";
                cout << "Advancing player from Qualifier to Round Robin stage." << endl;
            } else if (currentStage == "S002") {
                nextStage = "S003";
                cout << "Advancing player from Round Robin to Knockout stage." << endl;
            }
            
            player.stageID = nextStage;
            
            savePlayersToFile();
            
            cout << "Player " << player.name << " (ID: " << playerID << ") "
                 << "has been advanced to stage " << nextStage << endl;
                 
            return true;
        } catch (const ValidationException& e) {
            cerr << "Player advancement error: " << e.what() << endl;
            return false;
        }
    }
    
    void savePlayersToFile() {
        ofstream playerFile("Players.txt");
        if (!playerFile) {
            throw ValidationException("Cannot open Players.txt for writing");
        }

        for (int i = 0; i < playersCount; ++i) {
            playerFile << players[i].playerID << ","
                      << players[i].name << ","
                      << players[i].nationality << ","
                      << players[i].ranking << ","
                      << players[i].gender << ","
                      << players[i].stageID << endl;
        }

        playerFile.close();
    }
    
    void displayScheduledMatches() {
        if (matchesCount == 0) {
            cout << "No matches scheduled yet." << endl;
            return;
        }
        
        cout << "\n--- Scheduled Matches ---" << endl;
        cout << left << setw(8) << "Match ID" 
             << setw(10) << "Stage" 
             << setw(10) << "Round" 
             << setw(12) << "Player 1" 
             << setw(12) << "Player 2" 
             << setw(20) << "Time" 
             << setw(12) << "Status" 
             << setw(10) << "Court" << endl;
        cout << string(84, '-') << endl;
        
        for (int i = 0; i < matchesCount; ++i) {
            Matches& match = matches[i];
            cout << left << setw(8) << match.matchID 
                 << setw(10) << match.stageID 
                 << setw(10) << match.roundID 
                 << setw(12) << match.p1ID 
                 << setw(12) << match.p2ID 
                 << setw(20) << match.scheduledTime 
                 << setw(12) << match.matchStatus 
                 << setw(10) << match.courtID << endl;
        }
    }
    
    void displayAllAvailablePlayers() {
        cout << "\n--- Available Players by Stage ---" << endl;
        
        cout << "\nStage S001 (Qualifier):" << endl;
        displayAvailablePlayers("S001");
        
        cout << "\nStage S002 (Round Robin):" << endl;
        displayAvailablePlayers("S002");
        
        cout << "\nStage S003 (Knockout):" << endl;
        displayAvailablePlayers("S003");
    }
};

int main() {
    try {
        TournamentScheduler scheduler;
        int choice;
        string input;
        string p1ID;

        do {
            cout << "\n--- Tennis Tournament Management System ---" << endl;
            cout << "1. Schedule a Match" << endl;
            cout << "2. Display Scheduled Matches" << endl;
            cout << "3. Display Available Players" << endl;
            cout << "4. Advance Player Stage" << endl;
            cout << "5. Exit" << endl;
            cout << "Enter your choice: ";
            
            getline(cin, input);
            
            try {
                choice = stoi(input);
            } catch (const exception&) {
                cout << "Invalid input. Please enter a number." << endl;
                continue;
            }

            switch(choice) {
                case 1: {
                    cout << "Enter Player 1 ID: ";
                    getline(cin, p1ID);
                    scheduler.scheduleMatch(p1ID);
                    break;
                }
                case 2:
                    scheduler.displayScheduledMatches();
                    break;
                case 3:
                    scheduler.displayAllAvailablePlayers();
                    break;
                case 4: {
                    cout << "Enter Player ID to advance: ";
                    getline(cin, p1ID);
                    scheduler.advancePlayerStage(p1ID);
                    break;
                }
                case 5:
                    cout << "Exiting..." << endl;
                    break;
                default:
                    cout << "Invalid choice. Please try again." << endl;
            }
        } while (choice != 5);  // Fixed exit condition
    } catch (const exception& e) {
        cerr << "Critical error: " << e.what() << endl;
        return 1;
    }
    return 0;
}