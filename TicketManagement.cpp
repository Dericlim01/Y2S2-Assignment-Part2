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
const int MAX_GATE_CAPACITY = 20; // Maximum capacity of per gate
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

// Insert new spectator value based on priority (adding people in order)
void enqueuePriorityQueue(Spectator* spectator) {
    // Create a new node for the spectator
    Node* newNode = new Node{spectator, nullptr};
    int spectatorPriority = spectator->priority;  // Get the priority of the spectator
    // Check if the queue is empty or highest priority spectator than the front (Vip> General> Early-bird)
    if (ticketQueueFront == nullptr || spectatorPriority > (ticketQueueFront->spectator->priority)) {
        // Insert at front
        newNode->next = ticketQueueFront;
        ticketQueueFront = newNode;
    } else {
        // Start from the front of the queue
        Node* current = ticketQueueFront;
        while (current->next != nullptr && (current->next->spectator->priority) >= spectatorPriority) {
            current = current->next;
        }
        // Insert the new node in the correct position
        newNode->next = current->next;
        current->next = newNode;
    }
}

// Function to remove and return the highest priority spectator from the Priority Queue
Spectator* dequeuePriorityQueue() {
    if (ticketQueueFront == nullptr) {
        return nullptr;
    }
    Node* temp = ticketQueueFront;
    Spectator* spectator = temp->spectator;
    ticketQueueFront = ticketQueueFront->next;
    delete temp;
    return spectator;
}

// Check if the priority queue is empty
bool isPriorityQueueEmpty() {
    // Return true if the queue is empty, else return false
    return ticketQueueFront == nullptr;
}

// Spectator List for storing all Spectators
Node* spectatorList = nullptr; // Head of the spectator list

// Function to add the spectator to spectator list
void addToSpectatorList(Spectator* spectator) {
    // Create a new node for the spectator
    Node* newNode = new Node{spectator, nullptr};
    // Insert at the head of the list
    newNode->next = spectatorList;
    // Update the head to the new node
    spectatorList = newNode;
}

// Function to search for a Spectator by TicketID
Spectator* searchByTicketID(const string& ticketID) {
    // Start from the head of the spectator list
    Node* current = spectatorList;
    // Traverse the spectator list
    while (current != nullptr) {
        // Check if the ticketID match
        if (current->spectator->ticketID == ticketID) {
            // Return the spectator if found
            return current->spectator;
        }
        // Move to the next node
        current = current->next;
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

// Function to get the current capacity of a court
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

// Function to update the court capacity (during entry and exit)
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

// Function to validate if a date is in April 2025
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

// Function to read match data from Matches.txt into a linked list
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
        Match* match1 = new Match{"M001", "S001", "R001", "APUTCP001", "APUTCP002", "28-04-2025 14:00:00", "waiting", "C001", nullptr};
        Match* match2 = new Match{"M002", "S002", "R002", "APUTCP003", "APUTCP004", "28-04-2025 16:00:00", "waiting", "C002", nullptr};
        Match* match3 = new Match{"M003", "S003", "R003", "APUTCP005", "APUTCP006", "29-04-2025 10:00:00", "waiting", "C003", nullptr};
        match1->next = match2; // Link the default matches
        match2->next = match3; // Link the default matches also
        head = match1; // Set the head to the first match
        matchCount = 3; // Update the match count
        return head; // Return the linked list of default matches
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

    // Error handling if not valid matches are found
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
    return head; // Return to the macthes linked list
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

// Function to add a sales record to the list and write into Sales.txt
void addToSalesRecord(Spectator* spectator, const string& status) {
    SalesRecord* newRecord = new SalesRecord; // Create a new sales record
    static int salesCounter = 1; // Static counter for generating unique salesID

    // Generating a unique salesID e.g. TKS001 TKS002
    newRecord->salesID = "TKS" + string(3 - to_string(salesCounter).length(), '0') + to_string(salesCounter);
    newRecord->spectatorName = spectator->name; // Set the spectator name
    newRecord->ticketsQuantity = spectator->seatsQuantity; // Set the number of tickets
    newRecord->ticketType = spectator->ticketType; // Set the ticket type
    newRecord->ticketID = spectator->ticketID; // Set the ticketID
    // Get current date and time when purchasing
    time_t now = time(0);
    struct tm* timeinfo = localtime(&now);
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%d-%m-%Y %H:%M:%S", timeinfo);
    newRecord->purchasedDateTime = string(buffer); // Set the purchase date and time
    newRecord->status = status; // Set the status Purchased/Rejected
    newRecord->next = nullptr; // Move to the next pointer

    // Insert into salesRecordList in sorted order by salesID
    if (salesRecordList == nullptr || salesRecordList->salesID > newRecord->salesID) {
        newRecord->next = salesRecordList;
        salesRecordList = newRecord;
    } else {
        SalesRecord* current = salesRecordList;
        // Traverse to find the correct position for insertion
        while (current->next != nullptr && current->next->salesID < newRecord->salesID) {
            current = current->next;
        }
        newRecord->next = current->next; // Inserting new record
        current->next = newRecord;
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

// Function to add a spectator to the priority queue without assigning ticketID
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
        cout << index << ". MatchID: " << current->matchID << ", StageID: " << current->stageID << ", RoundID: " << current->roundID << ", " << current->p1ID << " vs " << current->p2ID << ", Scheduled: " << current->dateTime << ", Status: " << current->matchStatus << ", Court: " << current->courtID << "\n";
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
    matchID = current->matchID; // Set the matchID
    courtID = current->courtID; // Set the courtID
    dateTime = current->dateTime; // Set the date and time

    cout << "Enter number of tickets to purchase: ";
    cin >> seatsQuantity;
    // Validate the number of tickets
    while (seatsQuantity <= 0) {
        cout << "Invalid quantity! Enter a positive number: ";
        cin >> seatsQuantity;
    }

    // Create a new spectator with the provided details
    Spectator* spectator = new Spectator{name, ticketType, 0, "", courtID, seatsQuantity, matchID, dateTime, nullptr, {0}};
    spectator->priority = getPriority(spectator->ticketType); // Set the priority
    enqueuePriorityQueue(spectator); // Add the spectator to priority queue
    cout << "\nSpectator " << name << " (Type: " << ticketType << ") added to the queue.\n";

    // Free the memory allocated for the matches list
    while (matches != nullptr) {
        Match* temp = matches;
        matches = matches->next;
        delete temp;
    }
}

// Function to process the entire ticket queue in priority order and assign ticketID
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
        int courtCapacity = getCourtCapacity(s->courtID); // Get the current capacity of the court
        // Check if the court has enough capacity
        if (courtCapacity >= s->seatsQuantity) {
            // Generate a unique ticketID e.g. T001 T002
            s->ticketID = "T" + string(3 - to_string(ticketCounter).length(), '0') + to_string(ticketCounter);
            ticketCounter++; // Increment the ticket counter

            // Display the ticket purchase details
            cout << "Ticket purchased: TicketID: " << s->ticketID << ", Name: " << s->name << ", Type: " << s->ticketType << ", Court: " << s->courtID << ", Match: " << s->matchID << ", DateTime: " << s->dateTime << ", Seats: " << s->seatsQuantity << "\n";
            // Update the court capacity
            updateCourtCapacity(s->courtID, s->seatsQuantity, true);
            // Add the spectator to the spectator list
            addToSpectatorList(s);
            // Record the sales status as Purchased
            addToSalesRecord(s, "Purchased");
        } 
        // If the court capacity is exceeded
        else {
            cout << "Court capacity exceeded. Cannot sell ticket to " << s->name << " on court " << s->courtID << "\n";
            // ticketID for rejected record
            s->ticketID = "T" + string(3 - to_string(ticketCounter).length(), '0') + to_string(ticketCounter);
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

// Function to enqueue a gate request
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
        gateRequestRear->next = newRequest;
        gateRequestRear = newRequest;
    }
}

// Function to dequeue a gate request
GateRequest* dequeueGateRequest() {
    // Check if the queue is empty
    if (gateRequestFront == nullptr) {
        return nullptr; // Return nullptr
    }
    GateRequest* temp = gateRequestFront; // To store the front request temporarily
    gateRequestFront = gateRequestFront->next; // Move the front to the next request
    // If the queue becomes empty
    if (gateRequestFront == nullptr) {
        gateRequestRear = nullptr; // Reset the rear
    }
    temp->next = nullptr;
    return temp;
}

// Function to check if the gate request is empty
bool isGateRequestQueueEmpty() {
    return gateRequestFront == nullptr; // True: Queue is empty
}

// Stack for Entry/Exit at Each Gate
struct GateStack {
    Node* top; // Top of the stack for the gate
    int size; // Represents total seats, not number of spectators

    GateStack() : top(nullptr), size(0) {} // Constructor to initialize an empty stack

    // Push a spectator into the gate stack
    void push(Spectator* spectator, int seatsToAssign) {
        // Check if the gate capacity would be exceeded
        if (size + seatsToAssign > MAX_GATE_CAPACITY) {
            cout << "\nGate capacity reached. Please choose the next gate.\n";
            return;
        }
        Node* newNode = new Node{spectator, nullptr}; // Create a new node for the spectator
        newNode->next = top; // Push the node into the stack
        top = newNode; 
        size += seatsToAssign; // Increment the size by number of the seats
    }

    // Pop spectator from the gate stack and remove the number of seats with he or she
    void pop(int seatsToRemove) {
        // If the stack is empty then do nothing
        if (top == nullptr) {
            return;
        }
        // Store the top node temporarily
        Node* temp = top;
        top = top->next; // Pop the top node
        size -= seatsToRemove; // Decrement the size by the number of seats
        delete temp; // Free the memory of the pop node
    }

    // Check if the gate stack is empty
    bool isEmpty() {
        return top == nullptr; // True: is empty
    }
};

// Function to handle entry or exit court gates requests through different gates
void processGateRequests(GateStack* gateStacks, char* gateNames) {
    // Process each gate request in the queue
    while (!isGateRequestQueueEmpty()) {
        GateRequest* request = dequeueGateRequest(); // Dequeue the next request
        // Check if the request is empty and skip to the next
        if (request == nullptr) {
            continue;
        }

        // Find the ticketID 
        Spectator* spectator = searchByTicketID(request->ticketID);
        // Check if the spectator is not found
        if (spectator == nullptr) {
            cout << "TicketID " << request->ticketID << " is not found.\n";
            delete request; // Free the memory of the request
            continue;
        }

        // Check if the request is for entry
        if (request->isEntry) {
            int remainingSeats = spectator->seatsQuantity; // Total seats to assign
            int seatsPerGate = MAX_GATE_CAPACITY; // Maximum seats per gate
            int totalSeatsAssigned = 0; // Track teh total seats assigned
            int gateIndex = 0; // Start from the first gate

            // Initialize the gate seats array for the spectators
            for (int i = 0; i < NUM_GATES; i++) {
                spectator->gateSeats[i] = 0;
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
                    spectator->gateSeats[gateIndex] = seatsToAssign;
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
                updateCourtCapacity(spectator->courtID, totalSeatsAssigned, true, false);
            }
        } 
        // If the request is for exit
        else {
            bool hasEntered = false; // Flag to check the spectator entered
            // Check all gates
            for (int i = 0; i < NUM_GATES; i++) {
                if (spectator->gateSeats[i] > 0) { // If the spectator has seats in any gate
                    hasEntered = true;
                    break;
                }
            }

            // If the spectator has not entered
            if (!hasEntered) {
                cout << "\nNo entry record found for " << spectator->name << ". Cannot process exit request.\n";
                // Free the memory of the exit request
                delete request;
                continue;
            }

            // Process the exit for each gate 
            for (int gateIndex = 0; gateIndex < NUM_GATES; gateIndex++) {
                int seatsAssigned = spectator->gateSeats[gateIndex];
                // Check if the spectator has seats in this gate
                if (seatsAssigned > 0) {
                    char gate = gateNames[gateIndex]; // Get the gate character
                    gateStacks[gateIndex].pop(seatsAssigned); // Pop the spectator from the gate stack
                    cout << "\nTicket buyer " << spectator->name << " exits through gate " << gate << " with " << seatsAssigned << " seats.\n";
                }
            }
            // Update the court capacity
            updateCourtCapacity(spectator->courtID, spectator->seatsQuantity, false, false);
            // Reset the gate seats array for the spectator
            for (int i = 0; i < NUM_GATES; i++) {
                spectator->gateSeats[i] = 0;
            }
        }
        // Free the memory of the processed request
        delete request;
    }
}

// Function to display and handle the Ticket Sales Menu
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

// Function to display and handle the Spectator Management Menu
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

// Main function to run the program
int main() {
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
        spectatorList = spectatorList->next;
        delete temp->spectator;
        delete temp;
    }

    // Free the memory allocated for the sales record list
    while (salesRecordList != nullptr) {
        SalesRecord* temp = salesRecordList;
        salesRecordList = salesRecordList->next;
        delete temp;
    }
    // Free the memory allocated for gate request queue
    while (gateRequestFront != nullptr) {
        GateRequest* temp = gateRequestFront;
        gateRequestFront = gateRequestFront->next;
        delete temp;
    }

    return 0;
}