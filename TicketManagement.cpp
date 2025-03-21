#include <iostream>
#include <cstring>

using namespace std;

// Constants for Priority (used to integer value comparison)
const int VIP_PRIORITY = 3; // Highest Priority
const int EARLY_BIRD_PRIORITY = 2; // Medium Priority
const int GENERAL_PRIORITY = 1; // Lowest Priority

// Constants for Gates
const int MAX_GATE_CAPACITY = 20;
// For Gates A,B,C,D,E,F,G
const int NUM_GATES = 6; 

// Structure for a Spectator
struct Spectator {
    string name;
    string ticketType;
    int priority;
    string ticketID;
    string courtID;
    int seatsQuantity;
    // Linked List Structures
    Spectator* next; // Pointer to the next node
};

// Structure Node for a Priority and Queue (making a queue line)
struct Node {
    Spectator* spectator; // Person waiting
    Node* next; // Pointer to the next node in line
};

// Priority Queue for Ticket Sales
Node* ticketQueueFront = nullptr;

// Insert new spectator value based on priority (adding people in order)
void enqueuePriorityQueue (Spectator* spectator) {
    Node* newNode = new Node {spectator, nullptr}; // Make a newline to add new person
    int spectatorPriority = spectator->priority; //Check priority of spectators
    // If the queue is empty or new spectator has higher priority
    // Line is empty 
    // OR
    // Someone have the higher priority
    if (ticketQueueFront == nullptr || spectatorPriority > (ticketQueueFront->spectator->priority)) {
        newNode->next = ticketQueueFront;
        ticketQueueFront = newNode;
    }
    else {
        Node* current = ticketQueueFront;
        while (current->next != nullptr && (current->next->spectator->priority) >= (spectatorPriority)) {
            current = current->next;
        }
        newNode->next = current->next;
        current->next = newNode;
    }
}

Spectator* dequeuePriorityQueue () {
    if (ticketQueueFront == nullptr) return nullptr; // Do nothing if there is no one in line
    Node* temp = ticketQueueFront; 
    Spectator* spectator = temp->spectator; // Store temp when the first person leave
    ticketQueueFront = ticketQueueFront->next; // Move the line to next person
    delete temp; // Delete the old first person
    return spectator; // Return to the spectators
}

