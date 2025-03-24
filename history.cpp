#include <iostream>
#include <fstream>
#include <stack>
#include <string>
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

class MatchHistory {
private:
    stack<Match> history; // Stack to store match history

public:
    // Record match results
    void recordMatch(string matchID, string p1, string p2) {
        Match newMatch(matchID, p1, p2);
        
        while (true) {
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
            
            cout << newMatch.player1 << " (" << newMatch.score1 << ") vs " << newMatch.player2 << " (" << newMatch.score2 << ")\n";
            
            // Check for winner
            if ((newMatch.score1 >= 12 || newMatch.score2 >= 12) && abs(newMatch.score1 - newMatch.score2) >= 2) {
                newMatch.winner = (newMatch.score1 > newMatch.score2) ? newMatch.player1 : newMatch.player2;
                break;
            }
        }
        
        history.push(newMatch);
        cout << "Match recorded: " << newMatch.player1 << " vs " << newMatch.player2 << " - Winner: " << newMatch.winner << endl;
    }

    // Display match history
    void displayHistory() {
        if (history.empty()) {
            cout << "No match history available." << endl;
            return;
        }
        
        stack<Match> temp = history;
        cout << "Match History:\n";
        while (!temp.empty()) {
            Match m = temp.top();
            temp.pop();
            cout << m.matchID << ": " << m.player1 << " (" << m.score1 << ") vs " << m.player2 << " (" << m.score2 << ") - Winner: " << m.winner << endl;
        }
    }
};

int main() {
    MatchHistory mh;
    
    // Example: Read from file (mocked here)
    string matchID = "M001", player1 = "A/1", player2 = "B/2";
    mh.recordMatch(matchID, player1, player2);
    
    cout << "\nDisplaying Match History:\n";
    mh.displayHistory();
    
    return 0;
}
