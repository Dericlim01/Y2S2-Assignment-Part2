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
    // Linked List Structures
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
    Node* newNode = new Node{spectator, nullptr}; // Make a newline to add new person
    int spectatorPriority = spectator->priority; // Check priority of spectators
    // If the queue is empty or new spectator has higher priority
    // Line is empty 
    // OR
    // Someone has the higher priority
    if (ticketQueueFront == nullptr || spectatorPriority > (ticketQueueFront->spectator->priority)) {
        newNode->next = ticketQueueFront; // Insert to the Front
        ticketQueueFront = newNode; 
    }
    else {
        Node* current = ticketQueueFront; // Start from the Front
        // Traverse the queue to find correct position based on priority
        while (current->next != nullptr && (current->next->spectator->priority) >= spectatorPriority) {
            current = current->next;
        }
        newNode->next = current->next; // Insert a New Node
        current->next = newNode;
    }
}

// Function to remove and return the highest priority spectator from the Priority Queue
Spectator* dequeuePriorityQueue() {
    if (ticketQueueFront == nullptr) return nullptr; // Do nothing if there is no one in line
    Node* temp = ticketQueueFront; 
    Spectator* spectator = temp->spectator; // Store temp when the first person leaves
    ticketQueueFront = ticketQueueFront->next; // Move the line to next person
    delete temp; // Delete the old first person
    return spectator; // Return to the spectators
}

bool isPriorityQueueEmpty() {
    return ticketQueueFront == nullptr; // Return if the queue is empty
}

// Spectator List for storing all Spectators
Node* spectatorList = nullptr; // Head of the spectator list

void addToSpectatorList(Spectator* spectator) {
    Node* newNode = new Node{spectator, nullptr}; // Create a New Node for the Spectator
    newNode->next = spectatorList; // Insert at the head of the list
    spectatorList = newNode; 
}

// Function to search for a Spectator by TicketID
Spectator* searchByTicketID(const string& ticketID) {
    Node* current = spectatorList; // Start from the head of the list
    // Start to traverse the list
    while (current != nullptr) {
        // Check if the TicketID matches 
        if (current->spectator->ticketID == ticketID) {
            return current->spectator; // Return the spectator
        }
        current = current->next; // Move to the Next Node
    }
    return nullptr; // Return nullptr if not found
}

// Structure for Court Capacity Management
struct Court {
    string courtID; // CourtID 
    int capacity; // Current capacity of the court
};

// Using Array to store the Courts (in fixed size)
Court courts[] = {
    {"C001", 1500}, // Center Court
    {"C002", 1000}, // Championship Court
    {"C003", 750}   // Progression Court
};

// Function to get the current capacity of a court
int getCourtCapacity(const string& courtID) {
    // Loop through the courts array list
    for (int i = 0; i < 3; i++) {
        if (courts[i].courtID == courtID) { // If the courtID matches
            return courts[i].capacity;
        }
    }
    cout << "\nCourtID not found.\n";
    return 0; // Return 0 if the courtID is not found
}

// Function to update the court capacity 
// Decrease for entry
// Increase for exit
void updateCourtCapacity(const string& courtID, int seats, bool isEntry) {
    // Loop through the courts array
    for (int i = 0; i < 3; i++) {
        // If the courtID matches
        if (courts[i].courtID == courtID) {
            if (isEntry) {
                courts[i].capacity -= seats; // Decrease court capacity
            }
            else {
                courts[i].capacity += seats; // Increase court capacity
            }
            cout << "\nUpdated capacity for court " << courtID << ": " << courts[i].capacity << "\n";
            break;
        }
    }
}

// Function to validate if a date is in April 2025
bool isDateInApril2025(const string& dateTime) {
    // Extract the date format DD-MM-YYYY HH:MM:SS
    string datePart = dateTime.substr(0, 10); // DatePart E.g.: 25-04-2025
    // Extract the month and year
    string month = datePart.substr(3, 2); // Start from index 3, extract 2 characters
    string year = datePart.substr(6, 4);  // Start from index 6, extract 4 characters
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
    // Error handling for file not found
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
        // Read separated by commas
        getline(ss, matchID, ',');
        getline(ss, stageID, ',');
        getline(ss, roundID, ',');
        getline(ss, p1ID, ',');
        getline(ss, p2ID, ',');
        getline(ss, dateTime, ',');
        getline(ss, matchStatus, ',');
        getline(ss, courtID, ',');

        // Trimming whitespace
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

        // Check the matches date
        if (!isDateInApril2025(dateTime)) {
            cout << "The matched " << matchID << " is not in April 2025.\n";
            continue;
        }

        // Only process include matches with status "waiting"
        if (matchStatus != "waiting") {
            cout << "The matched " << matchID << " status is " << matchStatus << ". Only waiting matches are available for ticket purchase.\n";
            continue;
        }

        // Create a new Match and add to the linked list
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

    // Error handling for matchCount is 0
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
    string purchasedDateTime;
    string status;
    SalesRecord* next; // Pointer to the next node
};

// Sales Record Management
SalesRecord* salesRecordList = nullptr; // Create a Sales Record List

// Function to add a sales record to the list and write into Sales.txt
void addToSalesRecord(Spectator* spectator, const string& status) {
    SalesRecord* newRecord = new SalesRecord; // Create a new sales record
    static int salesCounter = 1; // For generating salesID

    // Generate salesID
    // Ensure the ID always has three digits e.g. TKS001 TKS002
    newRecord->salesID = "TKS" + string(3 - to_string(salesCounter).length(), '0') + to_string(salesCounter);
    newRecord->spectatorName = spectator->name; // The spectator's name
    newRecord->ticketsQuantity = spectator->seatsQuantity; // The number of the tickets
    newRecord->ticketType = spectator->ticketType; // The purchased ticket type

    // Get the date and time
    time_t now = time(0); // Return the current system time
    // Convert to local time
    struct tm* timeinfo = localtime(&now);
    // Time character array buffer
    char buffer[20];
    // Formatting the datetime
    strftime(buffer, sizeof(buffer), "%d-%m-%Y %H:%M:%S", timeinfo);
    newRecord->purchasedDateTime = string(buffer);
    newRecord->status = status; // Set the status (Purchased or Rejected)
    newRecord->next = salesRecordList; // Insert at the head of the list
    salesRecordList = newRecord;
    salesCounter++;

    // Rewrite the entire Sales.txt file with all records
    ofstream outFile("Sales.txt");  // Open the text file
    if (!outFile) {
        cout << "Error: Could not open Sales.txt for writing.\n";
        return;
    }
    SalesRecord* current = salesRecordList; // Record List head
    // Traverse the record list
    while (current != nullptr) {
        outFile << current->salesID << "," << current->spectatorName << "," << current->ticketsQuantity << "," << current->ticketType << "," << current->purchasedDateTime << "," << current->status << "\n";
        current = current->next; // Move to add the next record
    }
    outFile.close(); // Close the file
}

// Function to view the sales records from text file
void viewSalesRecord() {
    ifstream inFile("Sales.txt"); // Open Sales.txt
    if (!inFile) {
        cout << "No sales records found.\n";
        return;
    }
    cout << "\n==========Sales Records:==========\n";
    cout << "SalesID, SpectatorName, TicketsQuantity, TicketType, PurchasedDateTime, Status\n";
    string line;
    // Read the lines from the file
    while (getline(inFile, line)) {
        // Display each line
        cout << line << "\n";
    }
    inFile.close(); // Close the file
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
    // Read matches from Matches.txt
    int matchCount;
    Match* matches = readMatchesFromFile(matchCount);

    if (matchCount == 0) {
        cout << "No matches available for ticket purchase at this time.\n";
        return;
    }

    string name, ticketType, matchID, dateTime;
    int seatsQuantity;
    string courtID;

    // Get spectator details
    cout << "Enter spectator name: ";
    cin.ignore();
    getline(cin, name);

    // Get ticket type with validation
    cout << "Enter ticket type (VIP, Early-bird, General): ";
    getline(cin, ticketType);
    while (ticketType != "VIP" && ticketType != "Early-bird" && ticketType != "General") {
        cout << "\nInvalid ticket type! Please enter again (VIP, Early-bird, General): ";
        getline(cin, ticketType);
    }

    // Display and select a match to purchase tickets
    cout << "===================================Select match===================================\n";
    Match* current = matches;
    int index = 1;
    while (current != nullptr) {
        cout << index << ". MatchID: " << current->matchID << ", StageID: " << current->stageID << ", Round: " << current->roundID << ", " << current->p1ID << " vs " << current->p2ID << ", Scheduled: " << current->dateTime << ", Status: " << current->matchStatus << ", Court: " << current->courtID << "\n";
        current = current->next;
        index++;
    }

    // Get the user's match selection
    int matchChoice;
    cout << "Enter your choice (1-" << matchCount << "): ";
    cin >> matchChoice;
    while (matchChoice < 1 || matchChoice > matchCount) {
        cout << "Invalid choice! Enter again (1-" << matchCount << "): ";
        cin >> matchChoice;
    }

    // Get the selected match details
    current = matches;
    for (int i = 1; i < matchChoice; i++) {
        current = current->next;
    }
    matchID = current->matchID;
    courtID = current->courtID;
    dateTime = current->dateTime;

    // Get the number of tickets to purchase
    cout << "Enter number of tickets to purchase: ";
    cin >> seatsQuantity;
    // Validate the quantity
    while (seatsQuantity <= 0) {
        cout << "Invalid quantity! Enter a positive number: ";
        cin >> seatsQuantity;
    }

    // Generate a unique TicketID e.g. T001, T002
    string ticketID = "T" + string(3 - to_string(ticketCounter).length(), '0') + to_string(ticketCounter);
    ticketCounter++;

    // Create a new spectator and add to the Priority Queue
    Spectator* spectator = new Spectator{name, ticketType, 0, ticketID, courtID, seatsQuantity, matchID, dateTime, nullptr};
    spectator->priority = getPriority(spectator->ticketType);
    enqueuePriorityQueue(spectator);

    // Process ticket sales by Priority Queue
    while (!isPriorityQueueEmpty()) {
        // To get the highest priority spectator
        Spectator* s = dequeuePriorityQueue();
        // Now check the court capacity
        int courtCapacity = getCourtCapacity(s->courtID); 
        // If enough capacity
        if (courtCapacity >= s->seatsQuantity) {
            // Display the ticket purchase details
            cout << "Ticket purchased: " << s->ticketID << ", " << s->name << ", " << s->ticketType << ", Court: " << s->courtID << ", Match: " << s->matchID << ", DateTime: " << s->dateTime << ", Seats: " << s->seatsQuantity << "\n";
            // Update court capacity
            updateCourtCapacity(s->courtID, s->seatsQuantity, true);
            // Add the spectator list for searching
            addToSpectatorList(s);
            // Record once done purchasing
            addToSalesRecord(s, "Purchased");
        }
        // Prompt if the capacity is exceeded
        else {
            cout << "Court capacity exceeded. Cannot sell ticket to " << s->name << " on court " << s->courtID << "\n";
            // Record when rejected to sell the ticket
            addToSalesRecord(s, "Rejected");
            // Free the spectator memory
            delete s;
        }
    }

    // Clean up the matches linked list
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
    // Check if the queue is empty
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
    // Check if the gate is empty
    if (gateRequestFront == nullptr) {
        return nullptr;
    }
    GateRequest* temp = gateRequestFront;
    gateRequestFront = gateRequestFront->next;
    // If the queue becomes empty
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
    Node* top; // Top of the stack
    int size; // Current number of spectators in the stack

    GateStack() : top(nullptr), size(0) {} // Constructor to initialise an empty stack

    // Function to push a spectator onto the stack
    void push(Spectator* spectator) {
        // Check if the gate capacity is reached
        if (size >= MAX_GATE_CAPACITY) {
            cout << "\nGate capacity reached. Please choose the next gate.\n";
            return;
        }
        Node* newNode = new Node{spectator, nullptr}; // Create a New Node
        newNode->next = top; // Push onto the stack
        top = newNode;
        size++; // Increment the size
    }

    // Pop the spectators from the stack
    Spectator* pop() {
        if (top == nullptr) return nullptr; // Return nullptr if the stack is empty
        Node* temp = top;
        Spectator* spectator = temp->spectator; // Store the top node
        top = top->next; // Get the spectator
        size--; // Decrement the size
        delete temp; // Free the node memory
        return spectator; // Return the spectator
    }

    bool isEmpty() {
        return top == nullptr; // Return true if the stack is empty
    }
};

// Function to handle court gates requests through different gates
void processGateRequests(GateStack* gateStacks, char* gateNames, int& currentGateIndex) {
    while (!isGateRequestQueueEmpty()) {
        GateRequest* request = dequeueGateRequest();
        if (request == nullptr) {
            continue;
        }

        // Search for the spectator by ticketID
        Spectator* spectator = searchByTicketID(request->ticketID);
        // If the ticketID is not found
        if (spectator == nullptr) {
            cout << "TicketID " << request->ticketID << " is not found.\n";
            delete request;
            continue;
        }

        // Handle the spectators' seats, splitting across gates
        // Total seats to process
        int remainingSeats = spectator->seatsQuantity;
        // Maximum seats per gate
        int seatsPerGate = MAX_GATE_CAPACITY;
        // Start with the current gate index for current spectator
        int tempGateIndex = currentGateIndex;

        // Continue until all seats are assigned
        while (remainingSeats > 0) {
            // Use the current gate index
            char gate = gateNames[tempGateIndex];
            int seatsToAssign = min(remainingSeats, seatsPerGate);

            // Check if the gate can accommodate the seats
            if (gateStacks[tempGateIndex].size + seatsToAssign <= MAX_GATE_CAPACITY) {
                // Push the spectator into the gate stack
                gateStacks[tempGateIndex].push(spectator);

                if (request->isEntry) {
                    cout << "\nTicket buyer " << spectator->name << " enters through gate " << gate << " with " << seatsToAssign << " seats.\n";
                }
                else {
                    cout << "\nTicket buyer " << spectator->name << " exits through gate " << gate << " with " << seatsToAssign << " seats.\n";
                    // Update the court capacity
                    updateCourtCapacity(spectator->courtID, seatsToAssign, false);
                    // Pop the spectators from the stack
                    gateStacks[tempGateIndex].pop();
                }
                // Reduce the remaining seats
                remainingSeats -= seatsToAssign;
                // Move to the next gate
                tempGateIndex++;
                if (tempGateIndex >= NUM_GATES) {
                    cout << "\nReached the last gate (Gate " << gateNames[NUM_GATES - 1] << "). Could not process remaining " << remainingSeats << " seats for " << spectator->name << ".\n";
                    break;
                }
            }
            // If the gate cannot accommodate the seats
            else {
                cout << "\nGate " << gate << " cannot accommodate " << seatsToAssign << " seats. Trying the next gate...\n";
                // Move to the next gate
                tempGateIndex++;
                if (tempGateIndex >= NUM_GATES) {
                    cout << "\nReached the last gate (Gate " << gateNames[NUM_GATES - 1] << "). Could not process remaining " << remainingSeats << " seats for " << spectator->name << ".\n";
                    break;
                }
            }
        }
        // Update the current gate index for the next spectator
        currentGateIndex = tempGateIndex;
        // Reset to the first gate
        currentGateIndex = 0;
        // Free the request memory
        delete request;
    }
}

// Function to display and handle the Ticket Sales Menu
void ticketSalesMenu(int& ticketCounter) {
    int choice;
    do {
        // Display the Ticket Sales Menu
        cout << "\n==============================Ticket Sales Menu==============================\n";
        cout << "1. Purchase Tickets\n";
        cout << "2. View Sales Record\n";
        cout << "3. Back to Main Menu\n";
        cout << "Enter choice: ";
        cin >> choice;

        // Handle the user choice
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
            // Error handling invalid choice
            default:
                cout << "Invalid choice! Please try again.\n";
        }
    } while (choice != 3); // Continue until the user chooses to go back to main menu
}

// Function to display and handle the Spectator Management Menu
void spectatorManagementMenu(GateStack* gateStacks, char* gateNames, int& currentGateIndex) {
    int choice;
    do {
        // Display the Spectator Management Menu
        cout << "\n==============================Spectator Management Menu==============================\n";
        cout << "1. Add Entry Request\n";
        cout << "2. Add Exit Request\n";
        cout << "3. View All Gate Requests Process\n";
        cout << "4. Back to Main Menu\n";
        cout << "Enter choice: ";
        cin >> choice;

        // Handle the user choice
        switch (choice) {
            case 1: {
                string ticketID;
                cout << "\nPlease insert your ticketID to enter: ";
                cin.ignore();
                getline(cin, ticketID);
                // Add entry request
                enqueueGateRequest(ticketID, true);
                cout << "\nEntry request for ticketID " << ticketID << " added to the queue.\n";
                break;
            }
            case 2: {
                string ticketID;
                cout << "\nPlease insert your ticketID to exit: ";
                cin.ignore();
                getline(cin, ticketID);
                // Add exit request
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
    } while (choice != 4); // Continue until the user chooses to go back to the main menu
}

// Main function to run the program
int main() {
    // Clear Sales.txt at the start of the program
    ofstream outFile("Sales.txt");
    outFile.close();

    // Initialise gate stacks and gate names
    // Array of stacks for each gate
    GateStack gateStacks[NUM_GATES];
    char gateNames[] = {'A', 'B', 'C', 'D', 'E', 'F'};
    int currentGateIndex = 0;
    int ticketCounter = 1;

    int choice;
    do {
        // Display the Main Menu
        cout << "\nTicket Sales and Spectator Management Main Menu\n";
        cout << "1. Ticket Sales Menu Page\n";
        cout << "2. Spectator Management Menu Page\n";
        cout << "3. Exit\n";
        cout << "Enter choice: ";
        cin >> choice;

        // Handle the user choice
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

    // Clean up memory before exiting
    // Free the spectator list
    while (spectatorList != nullptr) {
        Node* temp = spectatorList;
        spectatorList = spectatorList->next;
        // Delete the spectator object
        delete temp->spectator;
        // Delete the node
        delete temp;
    }
    // Free the sales record list
    while (salesRecordList != nullptr) {
        SalesRecord* temp = salesRecordList;
        salesRecordList = salesRecordList->next;
        delete temp;
    }
    // Free the gate request queue
    while (gateRequestFront != nullptr) {
        GateRequest* temp = gateRequestFront;
        gateRequestFront = gateRequestFront->next;
        delete temp;
    }

    // Exit the program
    return 0;
}