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
#include <stdexcept>
#include <regex>
#include <queue>
#include <cmath>

using namespace std;

/**
 * ----------------------------------------------------------------------------------------------------------------
 * -------------------------------- Tournament Scheduling and Player Progression ----------------------------------
 * ----------------------------------------------------------------------------------------------------------------
 */
// Custom exception class for validation errors
class ValidationException : public runtime_error {
    public:
        ValidationException(const string& message) 
            : runtime_error(message) {}
};

// Structure for a Court
struct Courts {
    string courtID;
    string courtType;
    int capacity;
    int maxConcurrentMatches;
};

// Structure for a Player
class Players {
    public:
        string playerID;
        string name;
        string nationality;
        int ranking;
        char gender;
        string stageID;
};

// Structure for a Match
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

// Structure for a Time Slot
struct TimeSlot {
    string date;
    int startHour;
    string formattedTime;
    string courtID;
    int concurrentCount;
};

// Main TournamentScheduler class
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

        /**
         * Initialize the schedules for each court
         */
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

        /**
         * Resize the matches array when it reaches capacity
         */
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

        /**
         * Resize the players array when it reaches capacity
         */
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

        /**
         * Validate the player ID format
         * @param playerID the player ID to validate
         */
        void validatePlayerID(const string& playerID) {
            if (playerID.empty()) {
                throw ValidationException("Player ID cannot be empty");
            }
            if (!regex_match(playerID, regex("^APUTCP\\d{3}$"))) {
                throw ValidationException("Invalid Player ID format. Must be APUTCP followed by 3 digits");
            }
        }

        /**
         * Validate the stage ID format
         * @param stageID the stage ID to validate
         */
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

        /**
         * Load the players from the Players.txt file
         */
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

        /**
         * Load the matches from the Matches.txt file
         */
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

        /**
         * Update the court schedules based on the matches
         */
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

        /**
         * Get the court index from the court ID
         * @param courtID the court ID to search for
         * @return the index of the court in the array
         */
        int getCourtIndexFromID(const string& courtID) {
            for (int i = 0; i < courtsCount; i++) {
                if (courts[i].courtID == courtID) {
                    return i;
                }
            }
            return -1;
        }

        /**
         * Get the court index from the stage ID
         * @param stageID the stage ID to search for
         * @return the index of the court in the array
         */
        int getCourtIndexFromStage(const string& stageID) {
            if (stageID == "S001") return 0;  // C001
            if (stageID == "S002") return 1;  // C002
            if (stageID == "S003") return 2;  // C003
            return -1;
        }

        /**
         * Get the next available time slot for a match in a stage
         * @param stageID the stage ID to search for
         * @return the next available time slot
         */
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

        /**
         * Save the matches to the Matches.txt file
         */
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
        /**
         * Constructor to initialize the tournament scheduler
         */
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

        /**
         * Destructor to clean up memory
         */
        ~TournamentScheduler() {
            delete[] courts;
            delete[] matches;
            delete[] players;
            delete[] courtSchedules;
        }

        /**
         * Display the available players in a stage
         * @param stageID the stage ID to search for
         */
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

        /**
         * Schedule a match between two players
         * @param p1ID the ID of player 1
         * @return true if the match was scheduled successfully
         */
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

        /**
         * Advance a player to the next stage
         * @param playerID the ID of the player to advance
         * @return true if the player was advanced successfully
         */
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

        /**
         * Save the players to the Players.txt file
         */
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

        /**
         * Display the scheduled matches
         */
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

        /**
         * Display all available players by stage
         */
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

struct matchHistory {
    string matchId;
    string stage;
    string p1Id;
    string p2Id;
    string setScores;
    string matchedTime;
    string duration;
    matchHistory* next;
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
void insertMatch(matchHistory*& head, const matchHistory& newMatch) {
    matchHistory* newNode = new matchHistory(newMatch);
    newNode -> next = nullptr;

    if (!head) {
        head = newNode;
        return;
    }

    matchHistory* temp = head;
    while (temp -> next)
        temp = temp -> next;

    temp -> next = newNode;
}

/**
 * Function to read matches from file
 * @param filename the name of the file
 * @param head the head of the linked list
 */
void readMatches(const string& filename, matchHistory*& head) {
    ifstream file(filename);
    if (!file) {
        cerr << "Error opening file!" << endl;
        return;
    }

    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string historyId; // Skipping the first column
        matchHistory match;

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
void freeMatches(matchHistory*& head) {
    while (head) {
        matchHistory* temp = head;
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

    matchHistory* head = nullptr;
    readMatches("MatchHistory.txt", head);

    cout << "Player ID: " << playerId << endl;
    cout << "Player Name: " << players[playerId] << endl;
    cout << "Performance: " << endl;

    int matchesPlayed = 0, wins = 0, losses = 0;
    matchHistory* temp = head;

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

void tournamentScheduleAndPlayer() {
    TournamentScheduler scheduler;
    int choice;
    string input;
    string p1ID;
    bool running = true;
    while (running) {
        cout << "\n--- Tennis Tournament Management System ---" << endl;
        cout << "1. Schedule a Match" << endl;
        cout << "2. Display Scheduled Matches" << endl;
        cout << "3. Display Available Players" << endl;
        cout << "4. Advance Player Stage" << endl;
        cout << "5. Track Player Performance" << endl;
        cout << "6. Exit" << endl;
        cout << "Enter your choice: ";
        cin >> choice;
        
        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Discard input
            cout << "Invalid input. Please try again." << endl;
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
                cout << "Track Player Performance" << endl;
                trackPlayerPerformance();
                break;
            case 6:
                cout << "Exiting..." << endl;
                running = false;
                break;
            default:
                cout << "Invalid choice. Please try again." << endl;
                break;
        }
    }
}

/**
 * ----------------------------------------------------------------------------------------------------------------
 * ----------------------------------- Ticket Sales and Spectator Management --------------------------------------
 * ----------------------------------------------------------------------------------------------------------------
 */

// Constants for Priority (used to integer value comparison)
const int VIP_PRIORITY = 3;         // Highest Priority
const int EARLY_BIRD_PRIORITY = 2;  // Medium Priority
const int GENERAL_PRIORITY = 1;     // Lowest Priority

// Constants for Gates
const int MAX_GATE_CAPACITY = 20;   // Maximum capacity of per gate
const int NUM_GATES = 6;            // For Gates A, B, C, D, E, F

// Structure for a Spectator
struct Spectator {
    string name;
    string ticketType;
    int priority;
    string ticketID;
    string courtID;
    int seatsQuantity;
    string matchID;
    string dateTime;
    Spectator* next;
    int gateSeats[NUM_GATES]; // Array to store seats assigned to each gate (A to F)
};

// Structure Node for a Priority and Queue (making a queue line)
struct Node {
    Spectator* spectator; // Person waiting
    Node* next; // Pointer to the next node in line
};

// Priority Queue for Ticket Sales
Node* ticketQueueFront = nullptr; // Front of the Priority Queue

/**
 * Insert new spectator value based on priority (adding people in order)
 * @param spectator The spectator to add
 */
void enqueuePriorityQueue(Spectator* spectator) {
    // Create a new node for the spectator
    Node* newNode = new Node{spectator, nullptr};
    int spectatorPriority = spectator->priority;  // Get the priority of the spectator
    // Check if the queue is empty or highest priority spectator than the front (Vip> General> Early-bird)
    if (ticketQueueFront == nullptr || spectatorPriority > (ticketQueueFront -> spectator -> priority)) {
        // Insert at front
        newNode -> next = ticketQueueFront;
        ticketQueueFront = newNode;
    } else {
        // Start from the front of the queue
        Node* current = ticketQueueFront;
        while (current -> next != nullptr && (current -> next -> spectator -> priority) >= spectatorPriority) {
            current = current -> next;
        }
        // Insert the new node in the correct position
        newNode -> next = current -> next;
        current -> next = newNode;
    }
}

/**
 * Function to remove and return the highest priority spectator from the Priority Queue
 * @return The spectator with the highest priority
 */
Spectator* dequeuePriorityQueue() {
    if (ticketQueueFront == nullptr) {
        return nullptr;
    }
    Node* temp = ticketQueueFront;
    Spectator* spectator = temp -> spectator;
    ticketQueueFront = ticketQueueFront -> next;
    delete temp;
    return spectator;
}

/**
 * Check if the priority queue is empty
 * @return True if the queue is empty, else False
 */
bool isPriorityQueueEmpty() {
    // Return true if the queue is empty, else return false
    return ticketQueueFront == nullptr;
}

// Spectator List for storing all Spectators
Node* spectatorList = nullptr; // Head of the spectator list

/**
 * Function to add the spectator to spectator list
 * @param spectator The spectator to add
 */
void addToSpectatorList(Spectator* spectator) {
    // Create a new node for the spectator
    Node* newNode = new Node{spectator, nullptr};
    // Insert at the head of the list
    newNode -> next = spectatorList;
    // Update the head to the new node
    spectatorList = newNode;
}

/**
 * Function to search for a Spectator by TicketID
 * @param ticketID The TicketID to search
 * @return The Spectator if found, else nullptr
 */
Spectator* searchByTicketID(const string& ticketID) {
    // Start from the head of the spectator list
    Node* current = spectatorList;
    // Traverse the spectator list
    while (current != nullptr) {
        // Check if the ticketID match
        if (current -> spectator -> ticketID == ticketID) {
            // Return the spectator if found
            return current -> spectator;
        }
        // Move to the next node
        current = current -> next;
    }
    // Return nullptr if the ticketID is not found
    return nullptr;
}

// Structure for Court Capacity Management
struct Court {
    string courtID;
    int capacity;
};

// Using Array to store the Courts (in fixed size)
Court courts[] = {
    {"C001", 1500}, // Center Court
    {"C002", 1000}, // Championship Court
    {"C003", 750}   // Progression Court
};

/**
 * Function to get the current capacity of a court
 * @param courtID The ID of the court
 * @return The capacity of the court
 */
int getCourtCapacity(const string& courtID) {
    // Loop through the array of courts
    for (int i = 0; i < 3; i++) {
        if (courts[i].courtID == courtID) {
            return courts[i].capacity; // Return the capacity of the court
        }
    }
    // Error handling when the court is not found
    cout << "\nCourtID not found.\n";
    return 0;
}

/**
 * Function to update the court capacity (during entry and exit)
 * @param courtID The ID of the court
 * @param seats The number of seats to update
 * @param isEntry True: Entry, False: Exit
 * @param showOutput True: Display the updated capacity, False: Do not display
 */
void updateCourtCapacity(const string& courtID, int seats, bool isEntry, bool showOutput = true) {
    // Loop the court array
    for (int i = 0; i < 3; i++) {
        // Check if the courtID match
        if (courts[i].courtID == courtID) {
            // If the spectators enter
            if (isEntry) {
                // Decrease the capacity
                courts[i].capacity -= seats;
            } 
            // If the spectators exit
            else {
                // Increase the capacity
                courts[i].capacity += seats;
            }
            // If the output is enabled to diaplay about the capacity, show the results
            if (showOutput) {
                cout << "\nUpdated capacity for court " << courtID << ": " << courts[i].capacity << "\n";
            }
            // Exit for loop
            break;
        }
    }
}

/**
 * Function to validate if a date is in April 2025
 * @param dateTime The date and time to validate
 * @return True if the date is in April 2025, else False
 */
bool isDateInApril2025(const string& dateTime) {
    // Extract the date part in format DD-MM-YYYY
    string datePart = dateTime.substr(0, 10); 
    // Start extract the month from index 3 and 2 characters
    string month = datePart.substr(3, 2); 
    // Start extract the year from index 6 and 4 characters
    string year = datePart.substr(6, 4);
    return (month == "04" && year == "2025");
}

// Structure for Matches
struct Match {
    string matchID;
    string stageID;
    string roundID;
    string p1ID;
    string p2ID;
    string dateTime;
    string matchStatus;
    string courtID;
    Match* next;
};

/**
 * Function to read match data from Matches.txt into a linked list
 * @param matchCount The number of valid matches
 * @return The head of the linked list of matches
 */
Match* readMatchesFromFile(int& matchCount) {
    // Head of the linked list of matches
    Match* head = nullptr;
    // Tail of the linked list for insertion
    Match* tail = nullptr;
    // Counter for the number of valid matches
    matchCount = 0;

    ifstream inFile("Matches.txt"); // Open Matches.txt file
    // Error handling if the file cannot be opened
    if (!inFile) {
        cout << "Error: could not open Matches.txt. Using default matches.\n";
        return nullptr; // Return nullptr
    }

    string line;
    // Read each line from the file
    while (getline(inFile, line)) {
        // Use stringstream to parse the line
        stringstream ss(line);
        string matchID, stageID, roundID, p1ID, p2ID, dateTime, matchStatus, courtID;
        // Extract each entity from the line and separated with commas
        getline(ss, matchID, ',');
        getline(ss, stageID, ',');
        getline(ss, roundID, ',');
        getline(ss, p1ID, ',');
        getline(ss, p2ID, ',');
        getline(ss, dateTime, ',');
        getline(ss, matchStatus, ',');
        getline(ss, courtID, ',');

        // Trimming whitespace if necessary
        matchID.erase(0, matchID.find_first_not_of(" \t"));
        matchID.erase(matchID.find_last_not_of(" \t") + 1);

        stageID.erase(0, stageID.find_first_not_of(" \t"));
        stageID.erase(stageID.find_last_not_of(" \t") + 1);

        roundID.erase(0, roundID.find_first_not_of(" \t"));
        roundID.erase(roundID.find_last_not_of(" \t") + 1);

        p1ID.erase(0, p1ID.find_first_not_of(" \t"));
        p1ID.erase(p1ID.find_last_not_of(" \t") + 1);

        p2ID.erase(0, p2ID.find_first_not_of(" \t"));
        p2ID.erase(p2ID.find_last_not_of(" \t") + 1);

        dateTime.erase(0, dateTime.find_first_not_of(" \t"));
        dateTime.erase(dateTime.find_last_not_of(" \t") + 1);

        matchStatus.erase(0, matchStatus.find_first_not_of(" \t"));
        matchStatus.erase(matchStatus.find_last_not_of(" \t") + 1);

        courtID.erase(0, courtID.find_first_not_of(" \t"));
        courtID.erase(courtID.find_last_not_of(" \t") + 1);

        // Validate the match date
        if (!isDateInApril2025(dateTime)) {
            cout << "The matched " << matchID << " is not in April 2025.\n";
            continue; // Skip the matches not in April 2025
        }

        // Validate the match status
        if (matchStatus != "waiting") {
            cout << "The matched " << matchID << " status is " << matchStatus << ". Only waiting matches are available for ticket purchase.\n";
            continue; // Skip the matches that are not in waiting
        }

        // Create a new match node
        Match* newMatch = new Match{matchID, stageID, roundID, p1ID, p2ID, dateTime, matchStatus, courtID, nullptr};
        // Check if the list is empty
        if (head == nullptr) { 
            head = newMatch; // Set the head and tail to the new match
            tail = newMatch;
        } 
        // Append to the tail
        else {
            tail->next = newMatch;
            tail = newMatch;
        }
        matchCount++; // Increment the match count
    }
    inFile.close();

    return nullptr; // Return nullptr if no valid matches found
}

// Structure for Sales Records
struct SalesRecord {
    string salesID;
    string spectatorName;
    int ticketsQuantity;
    string ticketType;
    string ticketID;
    string purchasedDateTime;
    string status;
    SalesRecord* next;
};

// Sales Record Management
SalesRecord* salesRecordList = nullptr; // Head of the sales records linked list

/**
 * Function to add a sales record to the list and write into Sales.txt
 * @param spectator The spectator to add
 * @param status The status of the sales record (Purchased/Rejected)
 */
void addToSalesRecord(Spectator* spectator, const string& status) {
    SalesRecord* newRecord = new SalesRecord; // Create a new sales record
    static int salesCounter = 1; // Static counter for generating unique salesID

    // Generating a unique salesID e.g. TKS001 TKS002
    newRecord -> salesID = "TKS" + string(3 - to_string(salesCounter).length(), '0') + to_string(salesCounter);
    newRecord -> spectatorName = spectator -> name; // Set the spectator name
    newRecord -> ticketsQuantity = spectator -> seatsQuantity; // Set the number of tickets
    newRecord -> ticketType = spectator -> ticketType; // Set the ticket type
    newRecord -> ticketID = spectator -> ticketID; // Set the ticketID
    // Get current date and time when purchasing
    time_t now = time(0);
    struct tm* timeinfo = localtime(&now);
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%d-%m-%Y %H:%M:%S", timeinfo);
    newRecord -> purchasedDateTime = string(buffer); // Set the purchase date and time
    newRecord -> status = status; // Set the status Purchased/Rejected
    newRecord -> next = nullptr; // Move to the next pointer

    // Insert into salesRecordList in sorted order by salesID
    if (salesRecordList == nullptr || salesRecordList -> salesID > newRecord -> salesID) {
        newRecord -> next = salesRecordList;
        salesRecordList = newRecord;
    } else {
        SalesRecord* current = salesRecordList;
        // Traverse to find the correct position for insertion
        while (current -> next != nullptr && current -> next -> salesID < newRecord -> salesID) {
            current = current -> next;
        }
        newRecord -> next = current -> next; // Inserting new record
        current -> next = newRecord;
    }
    salesCounter++; // Increment the sales counter

    // Write the sorted list to Sales.txt
    ofstream outFile("Sales.txt"); // Open the Sales.txt file and write
    // Error handling if the file cannot open
    if (!outFile) {
        cout << "Error: Could not open Sales.txt for writing.\n";
        return;
    }
    SalesRecord* current = salesRecordList; // Start from the head of the list
    // Write each record into the Sales.txt
    while (current != nullptr) {
        outFile << current->salesID << "," << current->spectatorName << "," << current->ticketsQuantity << "," << current->ticketType << "," << current->ticketID << "," << current->purchasedDateTime << "," << current->status << "\n";
        current = current->next; // Move to the next sales record
    }
    outFile.close();
}

// Function to view the sales records from text file
void viewSalesRecord() {
    ifstream inFile("Sales.txt"); // Open the Sales.txt file and read
    // If the file cannot open
    if (!inFile) {
        cout << "No sales records found.\n";
        return;
    }
    cout << "\n=====================================Sales Records:=========================================\n"; // Display the header
    cout << "SalesID | SpectatorName | TicketsQuantity | TicketType | TicketID | PurchasedDateTime | Status\n";
    cout << "==============================================================================================\n";
    string line;
    // Read and display each line
    while (getline(inFile, line)) {
        cout << line << "\n";
    }
    inFile.close(); // Close Sales.txt
}

/**
 * Function to set the priority based on the ticket type
 * @param ticketType The type of the ticket
 * @return The priority value
 */
int getPriority(const string& ticketType) {
    if (ticketType == "VIP") {
        return VIP_PRIORITY;
    } 
    else if (ticketType == "Early-bird") {
        return EARLY_BIRD_PRIORITY;
    } 
    else {
        return GENERAL_PRIORITY;
    }
}

/**
 * Function to add a spectator to the priority queue without assigning ticketID
 * @param ticketCounter The counter for ticketID
 */
void addSpectatorToQueue(int& ticketCounter) {
    int matchCount; // Variable to store the number of matches
    Match* matches = readMatchesFromFile(matchCount); // Read the matches from the file

    // Check if there is no match available
    if (matchCount == 0) {
        cout << "No matches available for ticket purchase at this time.\n";
        return;
    }

    // Variables to store spectator details
    string name, ticketType, matchID, dateTime;
    int seatsQuantity;
    string courtID;

    // Get the spectator name
    cout << "Enter spectator name: ";
    cin.ignore(); // Clearing input buffer (like newline)
    getline(cin, name); // Read the spectator's name

    // Get the ticket type
    cout << "Enter ticket type (VIP, Early-bird, General): ";
    getline(cin, ticketType);
    // Validate the ticket type
    while (ticketType != "VIP" && ticketType != "Early-bird" && ticketType != "General") {
        cout << "\nInvalid ticket type! Please enter again (VIP, Early-bird, General): ";
        getline(cin, ticketType);
    }

    // Display available matches for selection
    cout << "===================================Select match===================================\n";
    Match* current = matches; // Start from the head of the matches list
    int index = 1; // Index for displaying match options
    // Loop through the matches
    while (current != nullptr) {
        cout << index << ". MatchID: " << current -> matchID 
            << ", StageID: " << current -> stageID
            << ", RoundID: " << current -> roundID << ", "
            << current -> p1ID << " vs " << current -> p2ID
            << ", Scheduled: " << current -> dateTime
            << ", Status: " << current -> matchStatus
            << ", Court: " << current -> courtID << "\n";
        current = current->next; // Move to the next match
        index++;
    }

    // Variable to store the user match selection
    int matchChoice;
    cout << "Enter your choice (1-" << matchCount << "): ";
    cin >> matchChoice;
    // Validate the match choice
    while (matchChoice < 1 || matchChoice > matchCount) {
        cout << "Invalid choice! Enter again (1-" << matchCount << "): ";
        cin >> matchChoice;
    }

    // Reset to the head of the match list
    current = matches;
    // Move to the selected match
    for (int i = 1; i < matchChoice; i++) {
        current = current->next;
    }
    matchID = current -> matchID; // Set the matchID
    courtID = current -> courtID; // Set the courtID
    dateTime = current -> dateTime; // Set the date and time

    cout << "Enter number of tickets to purchase: ";
    cin >> seatsQuantity;
    // Validate the number of tickets
    while (seatsQuantity <= 0) {
        cout << "Invalid quantity! Enter a positive number: ";
        cin >> seatsQuantity;
    }

    // Create a new spectator with the provided details
    Spectator* spectator = new Spectator{name, ticketType, 0, "", courtID, seatsQuantity, matchID, dateTime, nullptr, {0}};
    spectator -> priority = getPriority(spectator -> ticketType); // Set the priority
    enqueuePriorityQueue(spectator); // Add the spectator to priority queue
    cout << "\nSpectator " << name << " (Type: " << ticketType << ") added to the queue.\n";

    // Free the memory allocated for the matches list
    while (matches != nullptr) {
        Match* temp = matches;
        matches = matches -> next;
        delete temp;
    }
}

/**
 * Function to process the entire ticket queue in priority order and assign ticketID
 * @param ticketCounter The counter for ticketID
 */
void processTicketQueue(int& ticketCounter) {
    // Check if the queue is empty
    if (isPriorityQueueEmpty()) {
        cout << "\nNo spectators in the queue to process.\n";
        return;
    }

    cout << "\nProcessing ticket queue...\n";
    // Process each spectator in the queue
    while (!isPriorityQueueEmpty()) {
        Spectator* s = dequeuePriorityQueue(); // Get the highest priority spectator
        int courtCapacity = getCourtCapacity(s -> courtID); // Get the current capacity of the court
        // Check if the court has enough capacity
        if (courtCapacity >= s -> seatsQuantity) {
            // Generate a unique ticketID e.g. T001 T002
            s -> ticketID = "T" + string(3 - to_string(ticketCounter).length(), '0') + to_string(ticketCounter);
            ticketCounter++; // Increment the ticket counter

            // Display the ticket purchase details
            cout << "Ticket purchased: TicketID: " << s -> ticketID
                << ", Name: " << s -> name
                << ", Type: " << s -> ticketType
                << ", Court: " << s -> courtID
                << ", Match: " << s -> matchID
                << ", DateTime: " << s -> dateTime
                << ", Seats: " << s -> seatsQuantity << "\n";
            // Update the court capacity
            updateCourtCapacity(s -> courtID, s -> seatsQuantity, true);
            // Add the spectator to the spectator list
            addToSpectatorList(s);
            // Record the sales status as Purchased
            addToSalesRecord(s, "Purchased");
        } 
        // If the court capacity is exceeded
        else {
            cout << "Court capacity exceeded. Cannot sell ticket to " << s -> name << " on court " << s -> courtID << "\n";
            // ticketID for rejected record
            s -> ticketID = "T" + string(3 - to_string(ticketCounter).length(), '0') + to_string(ticketCounter);
            ticketCounter++;
            // Record the sales status as Rejected
            addToSalesRecord(s, "Rejected");
            // Free the memory of the rejected spectator
            delete s;
        }
    }
}

// Structure for an Entry/Exit Process
struct GateRequest {
    string ticketID;
    bool isEntry; // True: Entry, False: Exit
    GateRequest* next;
};

// Queue for Gate Requests (Entry or Exit)
GateRequest* gateRequestFront = nullptr; // Front of the gate request queue
GateRequest* gateRequestRear = nullptr; // Rear of the gate request queue

/**
 * Function to enqueue a gate request
 * @param ticketID The ticketID of the spectator
 * @param isEntry True: Entry, False: Exit
 */
void enqueueGateRequest(const string& ticketID, bool isEntry) {
    // Create a new gate request
    GateRequest* newRequest = new GateRequest{ticketID, isEntry, nullptr};
    // Check if the queue is empty
    if (gateRequestRear == nullptr) {
        // Set both front and rear to the new request
        gateRequestFront = gateRequestRear = newRequest;
    } 
    // Append to the rear
    else {
        gateRequestRear -> next = newRequest;
        gateRequestRear = newRequest;
    }
}

/**
 * Function to dequeue a gate request
 * @return The gate request
 */
GateRequest* dequeueGateRequest() {
    // Check if the queue is empty
    if (gateRequestFront == nullptr) {
        return nullptr; // Return nullptr
    }
    GateRequest* temp = gateRequestFront; // To store the front request temporarily
    gateRequestFront = gateRequestFront -> next; // Move the front to the next request
    // If the queue becomes empty
    if (gateRequestFront == nullptr) {
        gateRequestRear = nullptr; // Reset the rear
    }
    temp -> next = nullptr;
    return temp;
}

// Function to check if the gate request is empty
bool isGateRequestQueueEmpty() {
    return gateRequestFront == nullptr; // True: Queue is empty
}

// Stack for Entry/Exit at Each Gate
struct GateStack {
    Node* top;  // Top of the stack for the gate
    int size;   // Represents total seats, not number of spectators

    GateStack() : top(nullptr), size(0) {} // Constructor to initialize an empty stack

    /**
     * Push a spectator into the gate stack
     * @param spectator The spectator to push
     * @param seatsToAssign The number of seats to assign
     */
    void push(Spectator* spectator, int seatsToAssign) {
        // Check if the gate capacity would be exceeded
        if (size + seatsToAssign > MAX_GATE_CAPACITY) {
            cout << "\nGate capacity reached. Please choose the next gate.\n";
            return;
        }
        Node* newNode = new Node{ spectator, nullptr }; // Create a new node for the spectator
        newNode -> next = top; // Push the node into the stack
        top = newNode; 
        size += seatsToAssign; // Increment the size by number of the seats
    }

    /**
     * Pop spectator from the gate stack and remove the number of seats with he or she
     * @param seatsToRemove The number of seats to remove
     */
    void pop(int seatsToRemove) {
        // If the stack is empty then do nothing
        if (top == nullptr) {
            return;
        }
        // Store the top node temporarily
        Node* temp = top;
        top = top -> next;      // Pop the top node
        size -= seatsToRemove;  // Decrement the size by the number of seats
        delete temp;            // Free the memory of the pop node
    }

    // Check if the gate stack is empty
    bool isEmpty() {
        return top == nullptr; // True: is empty
    }
};

/**
 * Function to handle entry or exit court gates requests through different gates
 * @param gateStacks The array of gate stacks
 * @param gateNames The names of the gates
 */
void processGateRequests(GateStack* gateStacks, char* gateNames) {
    // Process each gate request in the queue
    while (!isGateRequestQueueEmpty()) {
        GateRequest* request = dequeueGateRequest(); // Dequeue the next request
        // Check if the request is empty and skip to the next
        if (request == nullptr) {
            continue;
        }

        // Find the ticketID 
        Spectator* spectator = searchByTicketID(request -> ticketID);
        // Check if the spectator is not found
        if (spectator == nullptr) {
            cout << "TicketID " << request -> ticketID << " is not found.\n";
            delete request; // Free the memory of the request
            continue;
        }

        // Check if the request is for entry
        if (request -> isEntry) {
            int remainingSeats = spectator -> seatsQuantity; // Total seats to assign
            int seatsPerGate = MAX_GATE_CAPACITY; // Maximum seats per gate
            int totalSeatsAssigned = 0; // Track teh total seats assigned
            int gateIndex = 0; // Start from the first gate

            // Initialize the gate seats array for the spectators
            for (int i = 0; i < NUM_GATES; i++) {
                spectator -> gateSeats[i] = 0;
            }

            // Cycle progress tracking
            bool madeProgress = true;
            // Track the number pf full cycles through gates
            int cyclesCompleted = 0;

            // Assign the capacity to gates until all spectators are assigned
            while (remainingSeats > 0 && cyclesCompleted < 2) {
                // Get the current gate from array
                char gate = gateNames[gateIndex]; 
                // Determine capacity ans assign to corresponding gate
                int seatsToAssign = min(remainingSeats, seatsPerGate); 

                // Check if the gate can be accommodated 
                if (gateStacks[gateIndex].size + seatsToAssign <= MAX_GATE_CAPACITY) {
                    // Push the spectator into the gate stack
                    gateStacks[gateIndex].push(spectator, seatsToAssign);
                    cout << "\nTicket buyer " << spectator->name << " enters through gate " << gate << " with " << seatsToAssign << " seats.\n";
                    // Record the spectators assigned to this gate
                    spectator -> gateSeats[gateIndex] = seatsToAssign;
                    // Update the total capacity
                    totalSeatsAssigned += seatsToAssign;
                    // Decrease the remaining seats
                    remainingSeats -= seatsToAssign;
                    // Progress tracking
                    madeProgress = true;
                } 
                // Check if the gate cannot be accommodated
                else {
                    cout << "\nGate " << gate << " cannot accommodate " << seatsToAssign << " seats. Trying the next gate...\n";
                }
                gateIndex = (gateIndex + 1) % NUM_GATES; // Move to the next gate

                // Check if a full cycle through gates is completed
                if (gateIndex == 0) {
                    cyclesCompleted++; // Increment by the cycle count
                    // Check if no progress was made in the cycle
                    if (!madeProgress) {
                        cout << "\nAll gates are full. Could not process remaining " << remainingSeats << " seats for " << spectator->name << ".\n";
                        break;
                    }
                    madeProgress = false; // Reset the progress tracking flag for the next cycle
                }
            }
            // Check if any seats are assigned
            if (totalSeatsAssigned > 0) {
                // Update the court capacity
                updateCourtCapacity(spectator -> courtID, totalSeatsAssigned, true, false);
            }
        } 
        // If the request is for exit
        else {
            bool hasEntered = false; // Flag to check the spectator entered
            // Check all gates
            for (int i = 0; i < NUM_GATES; i++) {
                if (spectator -> gateSeats[i] > 0) { // If the spectator has seats in any gate
                    hasEntered = true;
                    break;
                }
            }

            // If the spectator has not entered
            if (!hasEntered) {
                cout << "\nNo entry record found for " << spectator -> name << ". Cannot process exit request.\n";
                // Free the memory of the exit request
                delete request;
                continue;
            }

            // Process the exit for each gate 
            for (int gateIndex = 0; gateIndex < NUM_GATES; gateIndex++) {
                int seatsAssigned = spectator -> gateSeats[gateIndex];
                // Check if the spectator has seats in this gate
                if (seatsAssigned > 0) {
                    char gate = gateNames[gateIndex]; // Get the gate character
                    gateStacks[gateIndex].pop(seatsAssigned); // Pop the spectator from the gate stack
                    cout << "\nTicket buyer " << spectator -> name << " exits through gate " << gate << " with " << seatsAssigned << " seats.\n";
                }
            }
            // Update the court capacity
            updateCourtCapacity(spectator -> courtID, spectator -> seatsQuantity, false, false);
            // Reset the gate seats array for the spectator
            for (int i = 0; i < NUM_GATES; i++) {
                spectator -> gateSeats[i] = 0;
            }
        }
        // Free the memory of the processed request
        delete request;
    }
}

/**
 * Function to display and handle the Ticket Sales Menu
 * @param ticketCounter The counter for ticketID
 */
void ticketSalesMenu(int& ticketCounter) {
    int choice;
    do {
        cout << "\n==============================Ticket Sales Menu==============================\n";
        cout << "1. Add Spectator to Queue\n";
        cout << "2. Process Ticket Queue\n";
        cout << "3. View Sales Record\n";
        cout << "4. Back to Main Menu\n";
        cout << "Enter choice: ";
        cin >> choice;

        // Handle user choice
        switch (choice) {
            case 1:
                addSpectatorToQueue(ticketCounter);
                break;
            case 2:
                processTicketQueue(ticketCounter);
                break;
            case 3:
                viewSalesRecord();
                break;
            case 4:
                cout << "Returning to the Main Menu...\n";
                break;
            default:
                cout << "Invalid choice! Please try again.\n";
        }
    } while (choice != 4); // Continue until the user choose to exit
}

/**
 * Function to display and handle the Spectator Management Menu
 * @param gateStacks The array of gate stacks
 * @param gateNames The names of the gates
 */
void spectatorManagementMenu(GateStack* gateStacks, char* gateNames) {
    int choice;
    do {
        cout << "\n==============================Spectator Management Menu==============================\n";
        cout << "1. Add Entry Request\n";
        cout << "2. Add Exit Request\n";
        cout << "3. View All Gate Requests Process\n";
        cout << "4. Back to Main Menu\n";
        cout << "Enter choice: ";
        cin >> choice;

        // Handle user choice
        switch (choice) {
            case 1: {
                string ticketID;
                cout << "\nPlease insert your ticketID to enter: ";
                cin.ignore();
                getline(cin, ticketID);
                enqueueGateRequest(ticketID, true);
                cout << "\nEntry request for ticketID " << ticketID << " added to the queue.\n";
                break;
            }
            case 2: {
                string ticketID;
                cout << "\nPlease insert your ticketID to exit: ";
                cin.ignore();
                getline(cin, ticketID);
                enqueueGateRequest(ticketID, false);
                cout << "\nExit request for ticketID " << ticketID << " added to the queue.\n";
                break;
            }
            case 3:
                if (isGateRequestQueueEmpty()) {
                    cout << "\nNo gate requests to process.\n";
                } else {
                    cout << "\nProcessing all gate requests...\n";
                    processGateRequests(gateStacks, gateNames);
                }
                break;
            case 4:
                cout << "\nReturning to Main Menu...\n";
                break;
            default:
                cout << "Invalid choice! Try again.\n";
        }
    } while (choice != 4); // Continue until the user choose to exit
}

/**
 * Ticket Sales and Spectator Management
 */
void ticketSales() {
    ofstream outFile("Sales.txt");
    outFile.close();

    GateStack gateStacks[NUM_GATES];
    char gateNames[] = {'A', 'B', 'C', 'D', 'E', 'F'};
    int ticketCounter = 1;

    // Handle user choice
    int choice;
    do {
        cout << "\n========================Ticket Sales and Spectator Management Main Menu========================\n";
        cout << "1. Ticket Sales Menu Page\n";
        cout << "2. Spectator Management Menu Page\n";
        cout << "3. Exit\n";
        cout << "Enter choice: ";
        cin >> choice;

        switch (choice) {
            case 1:
                ticketSalesMenu(ticketCounter);
                break;
            case 2:
                spectatorManagementMenu(gateStacks, gateNames);
                break;
            case 3:
                cout << "\nExiting Ticket Sales and Spectator Management Main Menu...\n";
                break;
            default:
                cout << "\nInvalid choice! Please try again.\n";
        }
    } while (choice != 3); // Continue until the user choose to exit

    // Free the memory allocated for the spectator list
    while (spectatorList != nullptr) {
        Node* temp = spectatorList;
        spectatorList = spectatorList -> next;
        delete temp -> spectator;
        delete temp;
    }

    // Free the memory allocated for the sales record list
    while (salesRecordList != nullptr) {
        SalesRecord* temp = salesRecordList;
        salesRecordList = salesRecordList -> next;
        delete temp;
    }
    // Free the memory allocated for gate request queue
    while (gateRequestFront != nullptr) {
        GateRequest* temp = gateRequestFront;
        gateRequestFront = gateRequestFront -> next;
        delete temp;
    }
}

/**
 * ----------------------------------------------------------------------------------------------------------------
 * ----------------------------------------- Handling Player Withdrawal -------------------------------------------
 * ----------------------------------------------------------------------------------------------------------------
 */
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
            cout << "Withdrawn Players:\n";
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
            string substituteName;
            string substituteId;

            while (true) {
                cout << "Please enter a substitute player name for match " << matchId << ": ";
                getline(cin, substituteName);

                // Find the substitute's player ID
                auto it = players.find(substituteName);
                if (it == players.end()) {
                    cout << "Player not found in Players.txt. Substitution failed.\n";
                    continue;
                }

                substituteId = it -> second;
                if (substituteId == p1Id || substituteId == p2Id) {
                    cout << "Invalid Player, Pls choose another player.\n";
                    continue;
                }
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
            cout << "Substituted " << substituteName << " (ID: " << substituteId << ") in match " << matchId << ".\n";
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
 * Create a new player
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

    string playerId = generateId("Players.txt", "APUTCP", 3);

    ofstream file("Players.txt", ios::app);
    if (!file.is_open()) return;
    else {
        file << playerId << ","
            << Player.name << ","
            << Player.nationality << ","
            << Player.ranking << ","
            << Player.gender << ","
            << "S001" << endl;
        file.close();
        cout << "Player added successfully." << endl;
    }
}

void HandlePlayer() {
    PlayerWithdrawals playerWithdrawals;
    bool running = true;
    while (running) {
        int choice;
        cout << "==================== Player Menu ====================" << endl;
        cout << "1. Create a new player" << endl;
        cout << "2. Withdraw a player" << endl;
        cout << "3. Exit to main menu" << endl;
        cout << "===================================================" << endl;
        cout << "Enter your choice: ";
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
                createNewPlayer();
                break;
            case 2:
                withdrawPlayer(playerWithdrawals);
                break;
            case 3:
                cout << "Exiting Player Menu..." << endl;
                running = false;
                break;
            default:
                cout << "Invalid choice. Please try again." << endl;
        }
    }
}

/**
 * ----------------------------------------------------------------------------------------------------------------
 * ------------------------------------------ Match History Tracking ----------------------------------------------
 * ----------------------------------------------------------------------------------------------------------------
 */

// Match structure aligned with database schema
struct MatchScores {
    string matchID;
    string stageID;
    string roundID;
    string p1ID;
    string p2ID;
    int score1;
    int score2;
    string winner;
    string scheduledTime;
    string matchStatus; // "waiting", "ongoing", "completed"
    string courtID;
    string matchTime;  // Match time when recorded
    string matchDuration; // Duration in minutes:seconds

    MatchScores() {} // Default constructor

    MatchScores(string id, string stage, string round, string player1, string player2, 
        string court, string scheduled) {
        matchID = id;
        stageID = stage;
        roundID = round;
        p1ID = player1;
        p2ID = player2;
        score1 = 0;
        score2 = 0;
        winner = "";
        scheduledTime = scheduled;
        matchStatus = "waiting";
        courtID = court;

        // Get current time for matchTime
        time_t now = time(0);
        char* dt = ctime(&now);
        matchTime = string(dt);

        // Remove the trailing newline from matchTime
        if (!matchTime.empty() && matchTime[matchTime.size() - 1] == '\n') {
            matchTime.pop_back();
        }

        // Assuming match duration to be zero initially
        matchDuration = "00:00"; 
    }
};

// Match History structure to store in the custom stack
struct MatchHistory {
    string historyID;
    string matchID;
    string stageID;
    string p1ID;
    string p2ID;
    string score;
    string matchTime;
    string matchDuration;
};

// Custom Stack Implementation for MatchHistory
class MatchHistoryStack {
    private:
        struct Node {
            MatchHistory data;
            Node* next;
            
            Node(MatchHistory mh) : data(mh), next(nullptr) {}
        };
        
        Node* topNode;
        int stackSize;
    
    public:
        MatchHistoryStack() : topNode(nullptr), stackSize(0) {}
        
        ~MatchHistoryStack() {
            while (!isEmpty()) {
                pop();
            }
        }
        
        void push(MatchHistory mh) {
            Node* newNode = new Node(mh);
            newNode->next = topNode;
            topNode = newNode;
            stackSize++;
        }
        
        MatchHistory pop() {
            if (isEmpty()) {
                throw runtime_error("Stack is empty");
            }
            
            Node* temp = topNode;
            MatchHistory mh = temp->data;
            topNode = topNode->next;
            delete temp;
            stackSize--;
            return mh;
        }
        
        MatchHistory peek() {
            if (isEmpty()) {
                throw runtime_error("Stack is empty");
            }
            return topNode->data;
        }
        
        bool isEmpty() {
            return topNode == nullptr;
        }
        
        int size() {
            return stackSize;
        }
        
        // Additional method to iterate through stack without popping elements
        void forEach(void (*func)(MatchHistory&)) {
            Node* current = topNode;
            while (current != nullptr) {
                func(current->data);
                current = current->next;
            }
        }
};

// Class to manage match history
class MatchHistoryManager {
    private:
        MatchHistoryStack history;
        int matchCounter;
        int historyCounter;
        map<string, int> playerWins;

        // Helper function to format duration string
        string formatDuration(int minutes, int seconds) {
            stringstream ss;
            ss << setw(2) << setfill('0') << minutes << ":"
            << setw(2) << setfill('0') << seconds;
            return ss.str();
        }
        
        // Convert MatchScores to MatchHistory for storage
        MatchHistory createHistoryFromMatch(MatchScores& match) {
            MatchHistory mh;
            mh.historyID = generateHistoryID();
            mh.matchID = match.matchID;
            mh.stageID = match.stageID;
            mh.p1ID = match.p1ID;
            mh.p2ID = match.p2ID;
            
            // Format score
            stringstream ss;
            ss << match.score1 << "-" << match.score2;
            mh.score = ss.str();
            
            mh.matchTime = match.matchTime;
            mh.matchDuration = match.matchDuration;
            
            return mh;
        }

        // Helper function to update Matches.txt
        void updateMatchesFile(MatchScores& updatedMatch) {
            ifstream inFile("Matches.txt");
            ofstream tempFile("Matches_temp.txt");
            
            if (!inFile || !tempFile) {
                cout << "Error: Unable to update Matches.txt" << endl;
                return;
            }
            
            string line;
            while (getline(inFile, line)) {
                stringstream ss(line);
                string id;
                getline(ss, id, ',');
                
                if (id == updatedMatch.matchID) {
                    // Write the updated match
                    tempFile << updatedMatch.matchID << ","
                            << updatedMatch.stageID << ","
                            << updatedMatch.roundID << ","
                            << updatedMatch.p1ID << ","
                            << updatedMatch.p2ID << ","
                            << updatedMatch.scheduledTime << ","
                            << updatedMatch.matchStatus << ","
                            << updatedMatch.courtID << "\n";
                } else {
                    // Write the original line
                    tempFile << line << "\n";
                }
            }
            
            inFile.close();
            tempFile.close();
            
            // Replace the old file with the new one
            remove("Matches.txt");
            rename("Matches_temp.txt", "Matches.txt");
        }

        // Helper function to load current match history from file
        void loadCurrentMatchHistory(MatchHistoryStack& dest) {
            ifstream inFile("MatchHistory.txt");
            if (!inFile) {
                cout << "No match history file found." << endl;
                return;
            }

            string line;
            while (getline(inFile, line)) {
                stringstream ss(line);
                MatchHistory mh;
                
                getline(ss, mh.historyID, ',');
                getline(ss, mh.matchID, ',');
                getline(ss, mh.stageID, ',');
                getline(ss, mh.p1ID, ',');
                getline(ss, mh.p2ID, ',');
                getline(ss, mh.score, ',');
                
                // The rest of the line might contain commas within date strings
                string restOfLine;
                getline(ss, restOfLine);
                
                // Find the last comma
                size_t lastCommaPos = restOfLine.find_last_of(',');
                if (lastCommaPos != string::npos) {
                    mh.matchTime = restOfLine.substr(0, lastCommaPos);
                    mh.matchDuration = restOfLine.substr(lastCommaPos + 1);
                } else {
                    mh.matchTime = restOfLine;
                    mh.matchDuration = "00:00";
                }
                
                dest.push(mh);
            }

            inFile.close();
        }

    public:
        MatchHistoryManager() : matchCounter(1), historyCounter(1) {
            // Try to load existing history if available
            loadMatchHistoryFromFile("MatchHistory.txt");
        }

        // Generate a new match ID
        string generateMatchID() {
            ostringstream ss;
            ss << "M" << setw(3) << setfill('0') << matchCounter++;
            return ss.str();
        }

        // Generate a unique history ID
        string generateHistoryID() {
            ostringstream ss;
            ss << "H" << setw(3) << setfill('0') << historyCounter++;
            return ss.str();
        }

        // Record match results with minimal user input
        void recordMatch() {
            string matchID;
            int score1, score2;
            string duration;
            
            // Get minimal input from the user
            cout << "Enter Match ID: ";
            cin >> matchID;
            cout << "Enter Player 1 Score: ";
            cin >> score1;
            cout << "Enter Player 2 Score: ";
            cin >> score2;
            cout << "Enter Match Duration (MM:SS): ";
            cin >> duration;

            // Auto-generate or use default values for other fields
            string stageID = "S001";  // Default to qualifier stage
            string roundID = "R001";  // Default to first round
            string p1ID = "APUTCP001"; // Default player 1
            string p2ID = "APUTCP002"; // Default player 2
            string courtID = "C001";   // Default to center court
            
            // Format current time in the correct format: DD-MM-YYYY HH:MM:SS
            time_t now = time(0);
            struct tm* timeinfo = localtime(&now);
            char buffer[80];
            strftime(buffer, sizeof(buffer), "%d-%m-%Y %H:%M:%S", timeinfo);
            string formattedTime = string(buffer);
            
            // Create new match record
            MatchScores newMatch;
            newMatch.matchID = matchID;
            newMatch.stageID = stageID;
            newMatch.roundID = roundID;
            newMatch.p1ID = p1ID;
            newMatch.p2ID = p2ID;
            newMatch.score1 = score1;
            newMatch.score2 = score2;
            newMatch.scheduledTime = formattedTime;
            newMatch.matchTime = formattedTime;
            newMatch.matchStatus = "completed";  // Since we're recording final scores
            newMatch.courtID = courtID;
            newMatch.matchDuration = duration;   // Use user-provided duration
            
            // Determine winner
            newMatch.winner = (score1 > score2) ? p1ID : p2ID;
            
            // Create history entry and save to stack
            MatchHistory mh = createHistoryFromMatch(newMatch);
            history.push(mh);
            
            cout << "Match recorded and added to history successfully!" << endl;
            cout << "Match details: " << matchID << ", " 
                << p1ID << " vs " << p2ID << ", "
                << "Score: " << score1 << "-" << score2 << ", "
                << "Winner: " << newMatch.winner << endl;
            cout << "Time: " << formattedTime << endl;
            cout << "Duration: " << duration << endl;
            
            // Only save to the history file, not to Matches.txt
            saveMatchHistoryToFile("MatchHistory.txt");
        }

        // Update match status - Fixed version
        void updateMatchStatus() {
            string matchID;
            cout << "Enter Match ID to update: ";
            cin >> matchID;
            
            // First check if this match exists in our files
            ifstream matchFile("Matches.txt");
            bool matchFound = false;
            string line;
            MatchScores match;
            
            if (matchFile.is_open()) {
                while (getline(matchFile, line)) {
                    stringstream ss(line);
                    string id, stageID, roundID, p1ID, p2ID, scheduledTime, status, courtID;
                    
                    getline(ss, id, ',');
                    getline(ss, stageID, ',');
                    getline(ss, roundID, ',');
                    getline(ss, p1ID, ',');
                    getline(ss, p2ID, ',');
                    getline(ss, scheduledTime, ',');
                    getline(ss, status, ',');
                    getline(ss, courtID);
                    
                    if (id == matchID) {
                        match.matchID = id;
                        match.stageID = stageID;
                        match.roundID = roundID;
                        match.p1ID = p1ID;
                        match.p2ID = p2ID;
                        match.scheduledTime = scheduledTime;
                        match.matchStatus = status;
                        match.courtID = courtID;
                        matchFound = true;
                        break;
                    }
                }
                matchFile.close();
            }
            
            if (!matchFound) {
                cout << "Match ID " << matchID << " not found in database." << endl;
                return;
            }
            
            cout << "Current match details:" << endl;
            cout << "Match ID: " << match.matchID << endl;
            cout << "Stage ID: " << match.stageID << endl;
            cout << "Player 1: " << match.p1ID << endl;
            cout << "Player 2: " << match.p2ID << endl;
            cout << "Current status: " << match.matchStatus << endl;
            
            cout << "\nEnter new status (waiting/ongoing/completed): ";
            string newStatus;
            cin >> newStatus;
            
            if (newStatus != "waiting" && newStatus != "ongoing" && newStatus != "completed") {
                cout << "Invalid status. Must be waiting, ongoing, or completed." << endl;
                return;
            }
            
            match.matchStatus = newStatus;
            
            // If status is completed, ask for scores
            if (match.matchStatus == "completed") {
                cout << "Enter Player 1 score: ";
                cin >> match.score1;
                cout << "Enter Player 2 score: ";
                cin >> match.score2;
                cout << "Enter match duration (MM:SS): ";
                cin >> match.matchDuration;
                
                // Format current time for the match time
                time_t now = time(0);
                struct tm* timeinfo = localtime(&now);
                char buffer[80];
                strftime(buffer, sizeof(buffer), "%d-%m-%Y %H:%M:%S", timeinfo);
                match.matchTime = string(buffer);
                
                // Determine winner
                match.winner = (match.score1 > match.score2) ? match.p1ID : match.p2ID;
                
                // Create history entry and save to stack
                MatchHistory mh = createHistoryFromMatch(match);
                history.push(mh);
                
                // Save to history file only
                saveMatchHistoryToFile("MatchHistory.txt");
                
                cout << "Match completed and added to history!" << endl;
            }
            
            // Update the matches file
            updateMatchesFile(match);
            
            cout << "Match status updated to: " << match.matchStatus << endl;
        }

        // Simulate match play
        void simulateMatch(MatchScores& match) {
            cout << "\n--- Match Simulation ---" << endl;
            match.matchStatus = "ongoing";
            
            time_t startTime = time(0); // Record start time for duration calculation
            
            // Simulate play until a winner is determined
            while (true) {
                cout << match.p1ID << " (" << match.score1 << ") vs " 
                    << match.p2ID << " (" << match.score2 << ")\n";

                cout << "Player get point (Enter Player ID): ";
                string player;
                cin >> player;

                if (player == match.p1ID) {
                    match.score1++;
                } else if (player == match.p2ID) {
                    match.score2++;
                } else {
                    cout << "Invalid player ID. Try again.\n";
                    continue;
                }

                // Check for winner - tennis rules typically require winning by 2 points
                if ((match.score1 >= 11 || match.score2 >= 11) && abs(match.score1 - match.score2) >= 2) {
                    match.winner = (match.score1 > match.score2) ? match.p1ID : match.p2ID;
                    match.matchStatus = "completed";
                    break;
                }
            }
            
            // Calculate match duration
            time_t endTime = time(0);
            int durationSeconds = difftime(endTime, startTime);
            int minutes = durationSeconds / 60;
            int seconds = durationSeconds % 60;
            match.matchDuration = formatDuration(minutes, seconds);
            
            cout << "Match completed! Winner: " << match.winner << endl;
            cout << "Final score: " << match.score1 << "-" << match.score2 << endl;
            cout << "Match duration: " << match.matchDuration << endl;
        }

        // Save match to Matches.txt file
        void saveMatchToFile(MatchScores& match) {
            ofstream outFile("Matches.txt", ios::app);
            if (!outFile) {
                cout << "Error: Unable to open Matches.txt for saving.\n";
                return;
            }

            outFile << match.matchID << ","
                    << match.stageID << ","
                    << match.roundID << ","
                    << match.p1ID << ","
                    << match.p2ID << ","
                    << match.scheduledTime << ","
                    << match.matchStatus << ","
                    << match.courtID << "\n";

            outFile.close();
        }

        // Save match history to file
        void saveMatchHistoryToFile(const string &filename) {
            ofstream outFile(filename);
            if (!outFile) {
                cout << "Error: Unable to open " << filename << " for saving.\n";
                return;
            }

            // Create a temporary stack and copy all elements
            MatchHistoryStack tempStack;
            MatchHistoryStack backupStack;
            
            while (!history.isEmpty()) {
                MatchHistory mh = history.pop();
                tempStack.push(mh);
                backupStack.push(mh);
            }
            
            // Restore original stack
            while (!backupStack.isEmpty()) {
                history.push(backupStack.pop());
            }
            
            // Write from temp stack to file (this reverses the order back to chronological)
            while (!tempStack.isEmpty()) {
                MatchHistory mh = tempStack.pop();
                outFile << mh.historyID << ","
                        << mh.matchID << ","
                        << mh.stageID << ","
                        << mh.p1ID << ","
                        << mh.p2ID << ","
                        << mh.score << ","
                        << mh.matchTime << ","
                        << mh.matchDuration << "\n";
            }

            outFile.close();
            cout << "Match history saved successfully to " << filename << "!\n";
        }

        // Load match history from file
        void loadMatchHistoryFromFile(const string &filename) {
            ifstream inFile(filename);
            if (!inFile) {
                cout << "No existing history file found. Starting fresh.\n";
                return;
            }

            string line;
            while (getline(inFile, line)) {
                stringstream ss(line);
                MatchHistory mh;
                
                getline(ss, mh.historyID, ',');
                getline(ss, mh.matchID, ',');
                getline(ss, mh.stageID, ',');
                getline(ss, mh.p1ID, ',');
                getline(ss, mh.p2ID, ',');
                getline(ss, mh.score, ',');
                
                // The rest of the line might contain commas within date strings
                string restOfLine;
                getline(ss, restOfLine);
                
                // Find the last comma
                size_t lastCommaPos = restOfLine.find_last_of(',');
                if (lastCommaPos != string::npos) {
                    mh.matchTime = restOfLine.substr(0, lastCommaPos);
                    mh.matchDuration = restOfLine.substr(lastCommaPos + 1);
                } else {
                    mh.matchTime = restOfLine;
                    mh.matchDuration = "00:00";
                }
                
                history.push(mh);
                
                // Update counters based on loaded data
                int historyNum = stoi(mh.historyID.substr(1));
                int matchNum = stoi(mh.matchID.substr(1));
                
                historyCounter = max(historyCounter, historyNum + 1);
                matchCounter = max(matchCounter, matchNum + 1);
            }

            inFile.close();
            cout << "Loaded " << history.size() << " match history records.\n";
        }

        // Display all match history
        void displayHistory() {
            if (history.isEmpty()) {
                cout << "No match history available.\n";
                return;
            }

            cout << "\n--- Match History ---\n";
            cout << left
                << setw(10) << "HistoryID" 
                << setw(10) << "MatchID" 
                << setw(10) << "StageID" 
                << setw(12) << "Player1" 
                << setw(12) << "Player2" 
                << setw(10) << "Score" 
                << setw(26) << "Match Time" 
                << "Duration\n";
            cout << string(95, '-') << endl;
            
            // Create a temporary stack to avoid destroying the original
            MatchHistoryStack tempStack;
            MatchHistoryStack backupStack;
            
            while (!history.isEmpty()) {
                MatchHistory mh = history.pop();
                tempStack.push(mh);
                backupStack.push(mh);
            }
            
            // Restore original stack
            while (!backupStack.isEmpty()) {
                history.push(backupStack.pop());
            }
            
            // Display from temp stack
            while (!tempStack.isEmpty()) {
                MatchHistory mh = tempStack.pop();
                cout << left
                    << setw(10) << mh.historyID 
                    << setw(10) << mh.matchID 
                    << setw(10) << mh.stageID 
                    << setw(12) << mh.p1ID 
                    << setw(12) << mh.p2ID 
                    << setw(10) << mh.score 
                    << setw(26) << mh.matchTime 
                    << mh.matchDuration << endl;
            }
        }

        // Search matches for a specific player - Fixed version
        void searchMatchesByPlayer() {
            string playerID;
            cout << "Enter Player ID to search for: ";
            cin >> playerID;

            // First load match history from file to ensure we have the latest data
            MatchHistoryStack freshHistory;
            loadCurrentMatchHistory(freshHistory);
            
            bool found = false;
            cout << "\nMatches for Player " << playerID << ":\n";
            cout << string(80, '-') << endl;
            
            // Create a temporary stack to search through
            MatchHistoryStack tempStack;
            
            while (!freshHistory.isEmpty()) {
                MatchHistory mh = freshHistory.pop();
                tempStack.push(mh);
                
                if (mh.p1ID == playerID || mh.p2ID == playerID) {
                    cout << "Match ID: " << mh.matchID << ", Stage: " << mh.stageID << endl;
                    cout << "Players: " << mh.p1ID << " vs " << mh.p2ID << endl;
                    cout << "Score: " << mh.score << endl;
                    cout << "Time: " << mh.matchTime << endl;
                    cout << "Duration: " << mh.matchDuration << endl;
                    cout << string(80, '-') << endl;
                    found = true;
                }
            }

            if (!found) {
                cout << "No matches found for Player ID: " << playerID << endl;
            }
        }
        
        // Search matches by stage - Fixed version
        void searchMatchesByStage() {
            string stageID;
            cout << "Enter Stage ID to search for (e.g., S001): ";
            cin >> stageID;

            // First load match history from file to ensure we have the latest data
            MatchHistoryStack freshHistory;
            loadCurrentMatchHistory(freshHistory);
            
            bool found = false;
            cout << "\nMatches in Stage " << stageID << ":\n";
            cout << string(80, '-') << endl;
            
            // Create a temporary stack to search through
            MatchHistoryStack tempStack;
            
            while (!freshHistory.isEmpty()) {
                MatchHistory mh = freshHistory.pop();
                tempStack.push(mh);
                
                if (mh.stageID == stageID) {
                    cout << "Match ID: " << mh.matchID << endl;
                    cout << "Players: " << mh.p1ID << " vs " << mh.p2ID << endl;
                    cout << "Score: " << mh.score << endl;
                    cout << "Time: " << mh.matchTime << endl;
                    cout << "Duration: " << mh.matchDuration << endl;
                    cout << string(80, '-') << endl;
                    found = true;
                }
            }

            if (!found) {
                cout << "No matches found for Stage ID: " << stageID << endl;
            }
        }
        
        // Generate statistics report - Fixed version
        void generateStatsReport() {
            cout << "\n--- Match Statistics Report ---\n";
            
            // First load match history from file to ensure we have the latest data
            MatchHistoryStack freshHistory;
            loadCurrentMatchHistory(freshHistory);
            
            // Variables for statistics
            int totalMatches = 0;
            map<string, int> playerMatches;
            map<string, int> playerWins;
            map<string, int> stageMatches;
            
            // Create a temporary stack to avoid destroying the original
            MatchHistoryStack tempStack;
            
            while (!freshHistory.isEmpty()) {
                MatchHistory mh = freshHistory.pop();
                tempStack.push(mh);
                
                // Count statistics
                totalMatches++;
                playerMatches[mh.p1ID]++;
                playerMatches[mh.p2ID]++;
                stageMatches[mh.stageID]++;
                
                // Determine winner from score
                stringstream ss(mh.score);
                int score1, score2;
                char dash;
                ss >> score1 >> dash >> score2;
                
                string winner = (score1 > score2) ? mh.p1ID : mh.p2ID;
                playerWins[winner]++;
            }
            
            // Display statistics
            cout << "Total matches recorded: " << totalMatches << endl << endl;
            
            cout << "Matches by stage:" << endl;
            for (auto& pair : stageMatches) {
                cout << "Stage " << pair.first << ": " << pair.second << " matches" << endl;
            }
            cout << endl;
            
            cout << "Player statistics:" << endl;
            cout << left << setw(12) << "Player ID" << setw(12) << "Matches" << setw(12) << "Wins" << "Win Rate" << endl;
            cout << string(50, '-') << endl;
            
            for (auto& pair : playerMatches) {
                string playerID = pair.first;
                int matches = pair.second;
                int wins = playerWins[playerID];
                double winRate = (matches > 0) ? (double)wins / matches * 100.0 : 0.0;
                
                cout << left << setw(12) << playerID
                    << setw(12) << matches
                    << setw(12) << wins
                    << fixed << setprecision(1) << winRate << "%" << endl;
            }
        }
};

void matchHistoryTrack() {
    MatchHistoryManager manager;
    int choice;
    bool running = true;
    while (running) {
        cout << "\n===== Match History Management =====\n";
        cout << "1. Record a New Match\n";
        cout << "2. Update Match Status\n";
        cout << "3. Display All Match History\n";
        cout << "4. Search Matches by Player\n";
        cout << "5. Search Matches by Stage\n";
        cout << "6. Generate Statistics Report\n";
        cout << "7. Exit\n";
        cout << "Enter your choice: ";
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
                manager.recordMatch();
                break;
            case 2:
                manager.updateMatchStatus();
                break;
            case 3:
                manager.displayHistory();
                break;
            case 4:
                manager.searchMatchesByPlayer();
                break;
            case 5:
                manager.searchMatchesByStage();
                break;
            case 6:
                manager.generateStatsReport();
                break;
            case 7:
                cout << "Exiting program...\n";
                running = false;
                break;
            default:
                cout << "Invalid choice! Please enter a valid option.\n";
        }
    }
}

/**
 * Display User Menu
 */
void displayMenu() {
    cout << "==================== User Menu ====================" << endl;
    cout << "1. Tournament Scheduling and Player Progression" << endl;
    cout << "2. Ticket Sales and Spectator Management" << endl;
    cout << "3. Handling Player Withdrawal" << endl;
    cout << "4. Match History Tracking" << endl;
    cout << "===================================================" << endl;
    cout << "Enter your choice: ";
}

/**
 * Main Function
 */
int main() {
    do {
        displayMenu();
        int choice;
        cin >> choice;
        // Input validation
        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Discard input
            cout << "Invalid input. Please try again." << endl;
            continue;
        }
        switch (choice)
        {
        case 1:
            // Tournament Scheduling and Player Progression
            tournamentScheduleAndPlayer();
            break;

        case 2:
            // Ticket Sales and Spectator Management
            ticketSales();
            break;

        case 3:
            // Handling Player Withdrawal
            HandlePlayer();
            break;

        case 4:
            // Match History Tracking
            matchHistoryTrack();
            break;

        default:
            cout << "Invalid choice. Please try again." << endl;
        }
    } while (true);

    return 0;
}