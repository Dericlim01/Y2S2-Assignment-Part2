#include <iostream>
#include <fstream>
#include <stack>
#include <string>
#include <iomanip>
#include <sstream>
using namespace std;

// Structure to store match details
struct Match {
    string matchID;
    string player1;
    string player2;
    int score1;
    int score2;
    string winner;

    Match(string id, string p1, string p2) {
        matchID = id;
        player1 = p1;
        player2 = p2;
        score1 = 0;
        score2 = 0;
        winner = "";
    }
};

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

// Class to manage match history
class MatchHistory {
    private:
        stack<Match> history;
        int matchCounter = 1;  // Auto-increment match ID

    public:
        // Record match results
        void recordMatch(string p1, string p2) {
            string matchID = generateId("MatchHistory.txt", "H", 3);
            Match newMatch(matchID, p1, p2);

            while (true) {
                cout << newMatch.player1 << " (" << newMatch.score1 << ") vs " 
                    << newMatch.player2 << " (" << newMatch.score2 << ")\n";

                cout << "Player get point (Enter Player ID): ";
                string player;
                cin >> player;

                if (player == newMatch.player1) {
                    newMatch.score1++;
                } else if (player == newMatch.player2) {
                    newMatch.score2++;
                } else {
                    cout << "Invalid player ID. Try again.\n";
                    continue;
                }

                // Check for winner
                if ((newMatch.score1 >= 12 || newMatch.score2 >= 12) && abs(newMatch.score1 - newMatch.score2) >= 2) {
                    newMatch.winner = (newMatch.score1 > newMatch.score2) ? newMatch.player1 : newMatch.player2;
                    break;
                }
            }

            history.push(newMatch);
            cout << "Match recorded: " << newMatch.matchID << ": " << newMatch.player1 
                << " vs " << newMatch.player2 << " - Winner: " << newMatch.winner << endl;
        }

        // Display all match history
        void displayHistory() {
            if (history.empty()) {
                cout << "No match history available.\n";
                return;
            }

            stack<Match> temp = history;
            cout << "\nMatch History:\n";
            while (!temp.empty()) {
                Match m = temp.top();
                temp.pop();
                cout << m.matchID << "," << m.player1 << "," << m.player2 << "," 
                    << m.score1 << "," << m.score2 << "," << m.winner << "\n";
            }
        }

        // Save match history to file (proper format)
        void saveMatchHistoryToFile(const string &filename) {
            ofstream outFile(filename, ios::app);
            if (!outFile) {
                cout << "Error: Unable to open file for saving.\n";
                return;
            }

            stack<Match> temp = history;
            while (!temp.empty()) {
                Match m = temp.top();
                temp.pop();
                outFile << m.matchID << "," << m.player1 << "," << m.player2 << "," 
                        << m.score1 << "," << m.score2 << "," << m.winner << "\n";
            }

            outFile.close();
            cout << "Match history saved successfully!\n";
        }

        // Search matches for a specific player
        void displayMatchesForPlayer() {
            string playerID;
            cout << "Enter Player ID to search for: ";
            cin >> playerID;

            bool found = false;
            stack<Match> temp = history;

            cout << "Matches for Player " << playerID << ":\n";
            while (!temp.empty()) {
                Match m = temp.top();
                temp.pop();

                if (m.player1 == playerID || m.player2 == playerID) {
                    cout << m.matchID << ": " << m.player1 << " (" << m.score1 << ") vs " 
                        << m.player2 << " (" << m.score2 << ") - Winner: " << m.winner << "\n\n";
                    found = true;
                }
            }

            if (!found) {
                cout << "No matches found for Player ID: " << playerID << endl;
            }
        }
};

// Main function with menu
int main() {
    MatchHistory mh;
    int choice;

    do {
        cout << "\n===== Match Management Menu =====\n";
        cout << "1. Record a Match\n";
        cout << "2. Search Match History by Player ID\n";
        cout << "3. Display Match History\n";
        cout << "4. Exit\n";
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {
            case 1: {
                string player1, player2;
                cout << "Enter Player 1 ID: ";
                cin >> player1;
                cout << "Enter Player 2 ID: ";
                cin >> player2;
                mh.recordMatch(player1, player2);
                mh.saveMatchHistoryToFile("MatchHistory.txt");
                break;
            }
            case 2:
                mh.displayMatchesForPlayer();
                break;
            case 3:
                mh.displayHistory();
                break;
            case 4:
                cout << "Exiting program...\n";
                break;
            default:
                cout << "Invalid choice! Please enter a valid option.\n";
        }
    } while (choice != 4);

    return 0;
}