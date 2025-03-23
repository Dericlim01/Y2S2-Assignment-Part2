#include <iostream>
#include <string>
#include <fstream>
#include <ctime>
#include <sstream>
using namespace std;

// Constants for Priority (used to integer value comparison)
const int VIP_PRIORITY = 3; // Highest Priority
const int EARLY_BIRD_PRIORITY = 2; // Medium Priority
const int GENERAL_PRIORITY = 1; // Lowest Priority

// Constants for Gates
const int MAX_GATE_CAPACITY = 20;
const int NUM_GATES = 6; // For Gates A,B,C,D,E,F

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
    Spectator* next; // Pointer to the next node
};

// Structure Node for a Priority and Queue (making a queue line)
struct Node {
    Spectator* spectator; // Person waiting
    Node* next; // Pointer to the next node in line
};

// Priority Queue for Ticket Sales
Node* ticketQueueFront = nullptr; // Front of the Priority Queue

// Insert new spectator value based on priority (adding people in order)
void enqueuePriorityQueue(Spectator* spectator) {
    Node* newNode = new Node{spectator, nullptr};
    int spectatorPriority = spectator->priority;
    if (ticketQueueFront == nullptr || spectatorPriority > (ticketQueueFront->spectator->priority)) {
        newNode->next = ticketQueueFront;
        ticketQueueFront = newNode;
    }
    else {
        Node* current = ticketQueueFront;
        while (current->next != nullptr && (current->next->spectator->priority) >= spectatorPriority) {
            current = current->next;
        }
        newNode->next = current->next;
        current->next = newNode;
    }
}

// Function to remove and return the highest priority spectator from the Priority Queue
Spectator* dequeuePriorityQueue() {
    if (ticketQueueFront == nullptr) return nullptr;
    Node* temp = ticketQueueFront;
    Spectator* spectator = temp->spectator;
    ticketQueueFront = ticketQueueFront->next;
    delete temp;
    return spectator;
}

bool isPriorityQueueEmpty() {
    return ticketQueueFront == nullptr;
}

// Spectator List for storing all Spectators
Node* spectatorList = nullptr; // Head of the spectator list

void addToSpectatorList(Spectator* spectator) {
    Node* newNode = new Node{spectator, nullptr};
    newNode->next = spectatorList;
    spectatorList = newNode;
}

// Function to search for a Spectator by TicketID
Spectator* searchByTicketID(const string& ticketID) {
    Node* current = spectatorList;
    while (current != nullptr) {
        if (current->spectator->ticketID == ticketID) {
            return current->spectator;
        }
        current = current->next;
    }
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

// Function to get the current capacity of a court
int getCourtCapacity(const string& courtID) {
    for (int i = 0; i < 3; i++) {
        if (courts[i].courtID == courtID) {
            return courts[i].capacity;
        }
    }
    cout << "\nCourtID not found.\n";
    return 0;
}

// Function to update the court capacity
void updateCourtCapacity(const string& courtID, int seats, bool isEntry) {
    for (int i = 0; i < 3; i++) {
        if (courts[i].courtID == courtID) {
            if (isEntry) {
                courts[i].capacity -= seats;
            }
            else {
                courts[i].capacity += seats;
            }
            cout << "\nUpdated capacity for court " << courtID << ": " << courts[i].capacity << "\n";
            break;
        }
    }
}

// Function to validate if a date is in April 2025
bool isDateInApril2025(const string& dateTime) {
    string datePart = dateTime.substr(0, 10);
    string month = datePart.substr(3, 2);
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

// Function to read match data from Matches.txt into a linked list
Match* readMatchesFromFile(int& matchCount) {
    Match* head = nullptr;
    Match* tail = nullptr;
    matchCount = 0;

    ifstream inFile("Matches.txt");
    if (!inFile) {
        cout << "Error: could not open Matches.txt. Using default matches.\n";
        Match* match1 = new Match{"M001", "S001", "R001", "APUTCP001", "APUTCP002", "28-04-2025 14:00:00", "waiting", "C001", nullptr};
        Match* match2 = new Match{"M002", "S002", "R002", "APUTCP003", "APUTCP004", "28-04-2025 16:00:00", "waiting", "C002", nullptr};
        Match* match3 = new Match{"M003", "S003", "R003", "APUTCP005", "APUTCP006", "29-04-2025 10:00:00", "waiting", "C003", nullptr};
        match1->next = match2;
        match2->next = match3;
        head = match1;
        matchCount = 3;
        return head;
    }

    string line;
    while (getline(inFile, line)) {
        stringstream ss(line);
        string matchID, stageID, roundID, p1ID, p2ID, dateTime, matchStatus, courtID;
        getline(ss, matchID, ',');
        getline(ss, stageID, ',');
        getline(ss, roundID, ',');
        getline(ss, p1ID, ',');
        getline(ss, p2ID, ',');
        getline(ss, dateTime, ',');
        getline(ss, matchStatus, ',');
        getline(ss, courtID, ',');

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

        if (!isDateInApril2025(dateTime)) {
            cout << "The matched " << matchID << " is not in April 2025.\n";
            continue;
        }

        if (matchStatus != "waiting") {
            cout << "The matched " << matchID << " status is " << matchStatus << ". Only waiting matches are available for ticket purchase.\n";
            continue;
        }

        Match* newMatch = new Match{matchID, stageID, roundID, p1ID, p2ID, dateTime, matchStatus, courtID, nullptr};
        if (head == nullptr) {
            head = newMatch;
            tail = newMatch;
        }
        else {
            tail->next = newMatch;
            tail = newMatch;
        }
        matchCount++;
    }
    inFile.close();

    if (matchCount == 0) {
        cout << "Error: could not open Matches.txt. Using default matches.\n";
        Match* match1 = new Match{"M001", "S001", "R001", "APUTCP001", "APUTCP002", "28-04-2025 14:00:00", "waiting", "C001", nullptr};
        Match* match2 = new Match{"M002", "S002", "R002", "APUTCP003", "APUTCP004", "28-04-2025 16:00:00", "waiting", "C002", nullptr};
        Match* match3 = new Match{"M003", "S003", "R003", "APUTCP005", "APUTCP006", "29-04-2025 10:00:00", "waiting", "C003", nullptr};
        match1->next = match2;
        match2->next = match3;
        head = match1;
        matchCount = 3;
    }
    return head;
}

// Structure for Sales Records
struct SalesRecord {
    string salesID;
    string spectatorName;
    int ticketsQuantity;
    string ticketType;
    string ticketID; // Added to store the ticketID
    string purchasedDateTime;
    string status;
    SalesRecord* next;
};

// Sales Record Management
SalesRecord* salesRecordList = nullptr;

// Function to add a sales record to the list and write into Sales.txt
void addToSalesRecord(Spectator* spectator, const string& status) {
    SalesRecord* newRecord = new SalesRecord;
    static int salesCounter = 1;

    newRecord->salesID = "TKS" + string(3 - to_string(salesCounter).length(), '0') + to_string(salesCounter);
    newRecord->spectatorName = spectator->name;
    newRecord->ticketsQuantity = spectator->seatsQuantity;
    newRecord->ticketType = spectator->ticketType;
    newRecord->ticketID = spectator->ticketID; // Store the ticketID
    time_t now = time(0);
    struct tm* timeinfo = localtime(&now);
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%d-%m-%Y %H:%M:%S", timeinfo);
    newRecord->purchasedDateTime = string(buffer);
    newRecord->status = status;
    newRecord->next = salesRecordList;
    salesRecordList = newRecord;
    salesCounter++;

    ofstream outFile("Sales.txt");
    if (!outFile) {
        cout << "Error: Could not open Sales.txt for writing.\n";
        return;
    }
    SalesRecord* current = salesRecordList;
    while (current != nullptr) {
        outFile << current->salesID << "," << current->spectatorName << "," << current->ticketsQuantity << "," << current->ticketType << "," << current->ticketID << "," << current->purchasedDateTime << "," << current->status << "\n";
        current = current->next;
    }
    outFile.close();
}

// Function to view the sales records from text file
void viewSalesRecord() {
    ifstream inFile("Sales.txt");
    if (!inFile) {
        cout << "No sales records found.\n";
        return;
    }
    cout << "\n==========Sales Records:==========\n";
    cout << "SalesID | SpectatorName | TicketsQuantity | TicketType | TicketID | PurchasedDateTime | Status\n";
    cout << "===============================================================================================\n";
    string line;
    while (getline(inFile, line)) {
        cout << line << "\n";
    }
    inFile.close();
}

// Function to set the priority based on the ticket type
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

// Function to handle ticket purchasing
void purchaseTickets(int& ticketCounter) {
    int matchCount;
    Match* matches = readMatchesFromFile(matchCount);

    if (matchCount == 0) {
        cout << "No matches available for ticket purchase at this time.\n";
        return;
    }

    string name, ticketType, matchID, dateTime;
    int seatsQuantity;
    string courtID;

    cout << "Enter spectator name: ";
    cin.ignore();
    getline(cin, name);

    cout << "Enter ticket type (VIP, Early-bird, General): ";
    getline(cin, ticketType);
    while (ticketType != "VIP" && ticketType != "Early-bird" && ticketType != "General") {
        cout << "\nInvalid ticket type! Please enter again (VIP, Early-bird, General): ";
        getline(cin, ticketType);
    }

    cout << "===================================Select match===================================\n";
    Match* current = matches;
    int index = 1;
    while (current != nullptr) {
        cout << index << ". MatchID: " << current->matchID << ", StageID: " << current->stageID << ", Round: " << current->roundID << ", " << current->p1ID << " vs " << current->p2ID << ", Scheduled: " << current->dateTime << ", Status: " << current->matchStatus << ", Court: " << current->courtID << "\n";
        current = current->next;
        index++;
    }

    int matchChoice;
    cout << "Enter your choice (1-" << matchCount << "): ";
    cin >> matchChoice;
    while (matchChoice < 1 || matchChoice > matchCount) {
        cout << "Invalid choice! Enter again (1-" << matchCount << "): ";
        cin >> matchChoice;
    }

    current = matches;
    for (int i = 1; i < matchChoice; i++) {
        current = current->next;
    }
    matchID = current->matchID;
    courtID = current->courtID;
    dateTime = current->dateTime;

    cout << "Enter number of tickets to purchase: ";
    cin >> seatsQuantity;
    while (seatsQuantity <= 0) {
        cout << "Invalid quantity! Enter a positive number: ";
        cin >> seatsQuantity;
    }

    string ticketID = "T" + string(3 - to_string(ticketCounter).length(), '0') + to_string(ticketCounter);
    ticketCounter++;

    Spectator* spectator = new Spectator{name, ticketType, 0, ticketID, courtID, seatsQuantity, matchID, dateTime, nullptr};
    spectator->priority = getPriority(spectator->ticketType);
    enqueuePriorityQueue(spectator);

    while (!isPriorityQueueEmpty()) {
        Spectator* s = dequeuePriorityQueue();
        int courtCapacity = getCourtCapacity(s->courtID);
        if (courtCapacity >= s->seatsQuantity) {
            cout << "Ticket purchased: TicketID: " << s->ticketID << ", Name: " << s->name << ", Type: " << s->ticketType << ", Court: " << s->courtID << ", Match: " << s->matchID << ", DateTime: " << s->dateTime << ", Seats: " << s->seatsQuantity << "\n";
            updateCourtCapacity(s->courtID, s->seatsQuantity, true);
            addToSpectatorList(s);
            addToSalesRecord(s, "Purchased");
        }
        else {
            cout << "Court capacity exceeded. Cannot sell ticket to " << s->name << " on court " << s->courtID << "\n";
            addToSalesRecord(s, "Rejected");
            delete s;
        }
    }

    while (matches != nullptr) {
        Match* temp = matches;
        matches = matches->next;
        delete temp;
    }
}

// Structure for an Entry/Exit Process
struct GateRequest {
    string ticketID;
    bool isEntry;
    GateRequest* next;
};

// Queue for Gate Requests (Entry or Exit)
GateRequest* gateRequestFront = nullptr;
GateRequest* gateRequestRear = nullptr;

// Function to enqueue a gate request
void enqueueGateRequest(const string& ticketID, bool isEntry) {
    GateRequest* newRequest = new GateRequest{ticketID, isEntry, nullptr};
    if (gateRequestRear == nullptr) {
        gateRequestFront = gateRequestRear = newRequest;
    }
    else {
        gateRequestRear->next = newRequest;
        gateRequestRear = newRequest;
    }
}

// Function to dequeue a gate request
GateRequest* dequeueGateRequest() {
    if (gateRequestFront == nullptr) {
        return nullptr;
    }
    GateRequest* temp = gateRequestFront;
    gateRequestFront = gateRequestFront->next;
    if (gateRequestFront == nullptr) {
        gateRequestRear = nullptr;
    }
    temp->next = nullptr;
    return temp;
}

// Function to check if the gate request is empty
bool isGateRequestQueueEmpty() {
    return gateRequestFront == nullptr;
}

// Stack for Entry/Exit at Each Gate
struct GateStack {
    Node* top;
    int size;

    GateStack() : top(nullptr), size(0) {}

    void push(Spectator* spectator) {
        if (size >= MAX_GATE_CAPACITY) {
            cout << "\nGate capacity reached. Please choose the next gate.\n";
            return;
        }
        Node* newNode = new Node{spectator, nullptr};
        newNode->next = top;
        top = newNode;
        size++;
    }

    Spectator* pop() {
        if (top == nullptr) return nullptr;
        Node* temp = top;
        Spectator* spectator = temp->spectator;
        top = top->next;
        size--;
        delete temp;
        return spectator;
    }

    bool isEmpty() {
        return top == nullptr;
    }
};

// Function to handle court gates requests through different gates
void processGateRequests(GateStack* gateStacks, char* gateNames, int& currentGateIndex) {
    while (!isGateRequestQueueEmpty()) {
        GateRequest* request = dequeueGateRequest();
        if (request == nullptr) {
            continue;
        }

        Spectator* spectator = searchByTicketID(request->ticketID);
        if (spectator == nullptr) {
            cout << "TicketID " << request->ticketID << " is not found.\n";
            delete request;
            continue;
        }

        int remainingSeats = spectator->seatsQuantity;
        int seatsPerGate = MAX_GATE_CAPACITY;
        int tempGateIndex = currentGateIndex;

        while (remainingSeats > 0) {
            char gate = gateNames[tempGateIndex];
            int seatsToAssign = min(remainingSeats, seatsPerGate);

            if (gateStacks[tempGateIndex].size + seatsToAssign <= MAX_GATE_CAPACITY) {
                gateStacks[tempGateIndex].push(spectator);
                if (request->isEntry) {
                    cout << "\nTicket buyer " << spectator->name << " enters through gate " << gate << " with " << seatsToAssign << " seats.\n";
                }
                else {
                    cout << "\nTicket buyer " << spectator->name << " exits through gate " << gate << " with " << seatsToAssign << " seats.\n";
                    updateCourtCapacity(spectator->courtID, seatsToAssign, false);
                    gateStacks[tempGateIndex].pop();
                }
                remainingSeats -= seatsToAssign;
                tempGateIndex++;
                if (tempGateIndex >= NUM_GATES) {
                    cout << "\nReached the last gate (Gate " << gateNames[NUM_GATES - 1] << "). Could not process remaining " << remainingSeats << " seats for " << spectator->name << ".\n";
                    break;
                }
            }
            else {
                cout << "\nGate " << gate << " cannot accommodate " << seatsToAssign << " seats. Trying the next gate...\n";
                tempGateIndex++;
                if (tempGateIndex >= NUM_GATES) {
                    cout << "\nReached the last gate (Gate " << gateNames[NUM_GATES - 1] << "). Could not process remaining " << remainingSeats << " seats for " << spectator->name << ".\n";
                    break;
                }
            }
        }
        currentGateIndex = tempGateIndex;
        currentGateIndex = 0;
        delete request;
    }
}

// Function to display and handle the Ticket Sales Menu
void ticketSalesMenu(int& ticketCounter) {
    int choice;
    do {
        cout << "\n==============================Ticket Sales Menu==============================\n";
        cout << "1. Purchase Tickets\n";
        cout << "2. View Sales Record\n";
        cout << "3. Back to Main Menu\n";
        cout << "Enter choice: ";
        cin >> choice;

        switch (choice) {
            case 1:
                purchaseTickets(ticketCounter);
                break;
            case 2:
                viewSalesRecord();
                break;
            case 3:
                cout << "Returning to the Main Menu...\n";
                break;
            default:
                cout << "Invalid choice! Please try again.\n";
        }
    } while (choice != 3);
}

// Function to display and handle the Spectator Management Menu
void spectatorManagementMenu(GateStack* gateStacks, char* gateNames, int& currentGateIndex) {
    int choice;
    do {
        cout << "\n==============================Spectator Management Menu==============================\n";
        cout << "1. Add Entry Request\n";
        cout << "2. Add Exit Request\n";
        cout << "3. View All Gate Requests Process\n";
        cout << "4. Back to Main Menu\n";
        cout << "Enter choice: ";
        cin >> choice;

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
                }
                else {
                    cout << "\nProcessing all gate requests...\n";
                    processGateRequests(gateStacks, gateNames, currentGateIndex);
                }
                break;
            case 4:
                cout << "\nReturning to Main Menu...\n";
                break;
            default:
                cout << "Invalid choice! Try again.\n";
        }
    } while (choice != 4);
}

// Main function to run the program
int main() {
    ofstream outFile("Sales.txt");
    outFile.close();

    GateStack gateStacks[NUM_GATES];
    char gateNames[] = {'A', 'B', 'C', 'D', 'E', 'F'};
    int currentGateIndex = 0;
    int ticketCounter = 1;

    int choice;
    do {
        cout << "\nTicket Sales and Spectator Management Main Menu\n";
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
                spectatorManagementMenu(gateStacks, gateNames, currentGateIndex);
                break;
            case 3:
                cout << "\nExiting Ticket Sales and Spectator Management Main Menu...\n";
                break;
            default:
                cout << "\nInvalid choice! Please try again.\n";
        }
    } while (choice != 3);

    while (spectatorList != nullptr) {
        Node* temp = spectatorList;
        spectatorList = spectatorList->next;
        delete temp->spectator;
        delete temp;
    }
    while (salesRecordList != nullptr) {
        SalesRecord* temp = salesRecordList;
        salesRecordList = salesRecordList->next;
        delete temp;
    }
    while (gateRequestFront != nullptr) {
        GateRequest* temp = gateRequestFront;
        gateRequestFront = gateRequestFront->next;
        delete temp;
    }

    return 0;
}