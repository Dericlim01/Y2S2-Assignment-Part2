#include <iostream>

using namespace std;

/**
 * ----------------------------------------
 * Schedule a match
 * ----------------------------------------
 */
void scheduleMatch() {
    struct Match {
        string stage;
        string round;
        string p1Id;
        string p2Id;
        string scheduledTime;
        string matchStatus;
        string courtId;
    };

    Match match;
    cin.ignore();
    cout << "Schedule Match" << endl;
    cout << "Enter stage: ";
    cin >> match.stage;
    cout << "Enter round: ";
    cin >> match.round;
    cout << "Enter player 1 ID: ";
    cin >> match.p1Id;
    cout << "Enter player 2 ID: ";
    cin >> match.p2Id;
    cout << "Enter scheduled time: ";
    cin >> match.scheduledTime;
    cout << "Enter match status: ";
    cin >> match.matchStatus;
    cout << "Enter court ID: ";
    cin >> match.courtId;

    string matchId = generateId("Matches.txt", "M", 3);

    ofstream file("Matches.txt", ios::app);
    if (!file.is_open()) return;
    else {
        file << matchId << ","
            << match.stage << ","
            << match.round << ","
            << match.p1Id << ","
            << match.p2Id << ","
            << match.scheduledTime << ","
            << match.matchStatus << ","
            << match.courtId << endl;
        file.close();
        cout << "Match scheduled successfully." << endl;
    }
}

int main() {
    return 0;
}