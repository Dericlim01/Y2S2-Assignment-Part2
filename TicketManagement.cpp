#include <iostream>
#include <string>
#include <fstream>
#include <ctime>

using namespace std;

// Constants for Priority (used to integer value comparison)
const int VIP_PRIORITY = 3; // Highest Priority
const int EARLY_BIRD_PRIORITY = 2; // Medium Priority
const int GENERAL_PRIORITY = 1; // Lowest Priority

// Constants for Gates
const int MAX_GATE_CAPACITY = 20;
const int NUM_GATES = 6; // For Gates A,B,C,D,E,F,G

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
void enqueuePriorityQueue (Spectator* spectator) {
    Node* newNode = new Node {spectator, nullptr}; // Make a newline to add new person
    int spectatorPriority = spectator->priority; //Check priority of spectators
    // If the queue is empty or new spectator has higher priority
    // Line is empty 
    // OR
    // Someone have the higher priority
    if (ticketQueueFront == nullptr || spectatorPriority > (ticketQueueFront->spectator->priority)) {
        newNode->next = ticketQueueFront; // Insert to the Front
        ticketQueueFront = newNode; 
    }
    else {
        Node* current = ticketQueueFront; // Start from the Front
        // Traverse the queue to find correct position based on priority
        while (current->next != nullptr && (current->next->spectator->priority) >= (spectatorPriority)) {
            current = current->next;
        }
        newNode->next = current->next; // Insert a New Node
        current->next = newNode;
    }
}

// Function to remove and return the highest priority spectator from the Priority Queue
Spectator* dequeuePriorityQueue () {
    if (ticketQueueFront == nullptr) return nullptr; // Do nothing if there is no one in line
    Node* temp = ticketQueueFront; 
    Spectator* spectator = temp->spectator; // Store temp when the first person leave
    ticketQueueFront = ticketQueueFront->next; // Move the line to next person
    delete temp; // Delete the old first person
    return spectator; // Return to the spectators
}

bool isPriorityQueueEmpty() {
    return ticketQueueFront == nullptr; // Return if the queue is empty
}

// Spectator List for storing the all Spectators
Node* spectatorList = nullptr; // Head of the spectator list

void addToSpectatorList (Spectator* spectator) {
    Node* newNode = new Node {spectator, nullptr}; // Create a New Node for the Spectator
    newNode->next = spectatorList; // Insert at the head of the list
    spectatorList = newNode; 
}

// Function to search for a Spectator by TicketID
Spectator* searchByTicketID (const string& ticketID) {
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

// Stack for Entry/Exit at Each Gate
struct GateStack {
    Node* top; // Top of the stack
    int size; // Current number of spectators in the stack

    GateStack () : top(nullptr), size(0) {} // Constructor to initialise an empty stack

    // Function to push a spectator onto the stack
    void push (Spectator* spectator) {
        // Check if the gate capacity is reached
        if (size >= MAX_GATE_CAPACITY) {
            cout << "\nGate capacity reached. Please choose the next gate.";
            return;
        }
        Node* newNode = new Node {spectator, nullptr}; // Create a New Node
        newNode->next = top; // Push onto the stack
        top = newNode;
        size++; // Increment the size
    }

    // Pop the spectators from the stack
    Spectator* pop () {
        if (top == nullptr) return nullptr; // Return nullptr if the stack is empty
        Node* temp = top;
        Spectator* spectator = temp->spectator; // Store the top node
        top = top->next; // Get the spectator
        size--; // Decrement the size
        delete temp; // Free the node memory
        return spectator // Return the spectator
    }

    bool isEmpty () {
        return top == nullptr; // Return true if the stack is empty
    }
};

// Structure for Court Capacity Management
struct Court {
    string courtID; // CourtID 
    int capacity; // Current capacity of the court
};

// Using Array to store the Courts (in fixed size)
Court courts[] = {
    {"C001", 1500}, // Center Court
    {"C002", 1000}, // Championship Court
    {"C003", 750} // Progression Court
};

// Function to get the current capacity of a court
int getCourtCapacity (const string& courtID) {
    // Loop through the courts array list
    for (int i = 0; i < 3; i++) {
        if (courts[i].courtID == courtID) { // If the courtID matches
            return courts[i].capacity;
        }
    }
    return 0; // Return 0 if the courtID is not found
    cout << "\nCourtID not found.";
}

// Function to update the court capacity 
// Decrease for entry
// Increase for exit
void updateCourtCapacity (const string& courtID, int seats, bool isEntry) {
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
            cout << "\nUpdated capacity for court " << courtID << ": " >> courts[i].capacity << "\n";
            break;
        }
    }
}

// Function to validate if a date is in April 2025
bool isDateInApril2025 (const string& dateTime) {
    // Extract the date format DD-MM-YYYY HH:MM:SS
    string datePart = dateTime.substr(0,10); // DatePart E.g.: 25-04-2025
    // Extract the month and year
    string month = datePart.substr(3,2); // Start from index 3, extract 2 characters
    string year = datePart.substr(6,4); // Start from index 6, extract 4 characters
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
Match* readMatchesFromFile (int& matchCount) {
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
    while (getline (inFile, line)) {
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
        matchID.erase(0, matchID.find_first_not_of(" \t") + 1);

        stageID.erase(0, stageID.find_first_not_of(" \t"));
        stageID.erase(0, stageID.find_first_not_of(" \t") + 1);

        roundID.erase(0, roundID.find_first_not_of(" \t"));
        roundID.erase(0, roundID.find_first_not_of(" \t") + 1);

        p1ID.erase(0, p1ID.find_first_not_of(" \t"));
        p1ID.erase(0, p1ID.find_first_not_of(" \t") + 1);

        p2ID.erase(0, p2ID.find_first_not_of(" \t"));
        p2ID.erase(0, p2ID.find_first_not_of(" \t") + 1);

        dateTime.erase(0, dateTime.find_first_not_of(" \t"));
        dateTime.erase(0, dateTime.find_first_not_of(" \t") + 1);

        matchStatus.erase(0, matchStatus.find_first_not_of(" \t"));
        matchStatus.erase(0, matchStatus.find_first_not_of(" \t") + 1);

        courtID.erase(0, courtID.find_first_not_of(" \t"));
        courtID.erase(0, courtID.find_first_not_of(" \t") + 1);

        // Check the matches date
        if (!isDateInApril2025(dateTime)) {
            cout << "The matched " << matchID << " is not in April 2025.\n";
            continue;
        }

        // Only process include mathces with status "waiting"
        if (matchStatus != "waiting") {
            cout << "The matched " << matchID << " status is " << matchStatus << ". Only waiting matches are available for ticket purchase.\n"
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
void addtoSalesRecord (Spectator* spectator, const string& status) {
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
    SalesRecord* current = salesRecordList; // Record List head
    // Traverse the record list
    while (current != nullptr) {
        outFile << current->salesID << "," << current->spectatorName << "," << current->ticketsQuantity << "," << current->ticketType << "," << current->purchasedDateTime << "," << current->status << "\n";
        current = current->next; // Move to add the next record
    }
    outFile.close(); // Close the file
}

// Funtion to view the sales records from text file
void viewSalesRecord () {
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

// Funtion to set the priority based on the tickt type
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
void purchaseTickets (int& ticketCounter) {
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
    cin.ignore ();
    getline(cin, name);

    // Get ticket type with validation
    cout << "Enter ticket type (VIP, Early-bird, General): ";
    getline(cin, ticketType);
    while (ticketType != "VIP" && ticketType != "Early-bird" && ticketType != "General") {
        cout << "\nInvalid ticket type! Please enter again (VIP, Early-bird, General): ";
        getlien(cin, ticketType);
    }


    // Display and select a match to purchase tickets
    cout << "===================================Select match===================================\n";
    Match* current = matches;
    int index = 1;
    while (current != nullptr) {
        cout << index << ". MatchID: " << current->matchID << ", StageID:" << current->stage << ", Round: " << current->round << ", Scheduled: " << current->dateTime << ", Status: " << current->matchStatus << ", Court: " << current->courtID << "\n";
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
    Spectator* spectator = new Spectator{name, ticketType, 0, ticketID, courtID, seatsQuantity, nullptr, matchID, dateTime};
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
            addtoSalesRecord(s, "Purchased");
        }
        // Prompt if the capacity is exceeded
        else {
            cout << "Court capacity exceeded. Cannot sell ticket to " << s->name << " on court " << s->courtID << "\n";
            // Record when rejected to sale the ticket
            addtoSalesRecord(s, "Rejected");
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

// Function to handle court entry through different gates
void courtEntryGate (GateStack* gateStacks, char* gateNames, int& entryGateIndex) {
    string ticketID;
    cout << "Please insert yout ticketID to enter the court: ";
    cin.ignore();
    getline(cin, ticketID);

    // Search for the spectator by ticketID
    Spectator* spectator = searchByTicketID(ticketID);
    // If the ticketID is not found
    if (spectator == nullptr) {
        cout << "ticketID " << ticketID << " not found!\n";
        return;
    }

    // Handle the spectator's quantity and splitting to different gates if necessary
    // Total seats to process
    int remainingCapacity = spectator->seatsQuantity;
    // Maximum seats per gate
    int capacityPerGate = MAX_GATE_CAPACITY;
    // Start with the current gate index
    int currentGateIndex = entryGateIndex;
    // Continue until reached the capacity of per gate
    while (remainingCapacity > 0) {
        // Select the next gate in cycle process
        char gate = gateNames[currentGateIndex];
        // Determine capacity for the gate
        int capacityToAssign = min(remainingCapacity, capacityPerGate);
        // Check if the gate can accmmodate spectators
        if (gateStacks[currentGateIndex.size + capacityToAssign <= MAX_GATE_CAPACITY]) {
            // Push spectator into the gate stack
            gateStacks[currentGateIndex].push(spectator); 
            cout << "Ticket buyer " << spectator.name << " enter through gate " << gate << " with " << capacityToAssign << " seats.\n";
            // Reduce the remaining capacity
            remainingCapacity -= capacityToAssign;
            
        }
    }
    
    
}


