#include <stdio.h>
#include <stdlib.h>
#include <math.h>


typedef struct Node {
    double x;
    double derivative;
    struct Node* next;
} Node;


double f(double x) {
    return x*x + 2*x + 1;
}


double derivative(double x, double h) {
    return (f(x + h) - f(x)) / h;
}


Node* addNode(Node* head, double x, double deriv) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->x = x;
    newNode->derivative = deriv;
    newNode->next = NULL;

    if(head == NULL) {
        return newNode;
    } else {
        Node* temp = head;
        while(temp->next != NULL) temp = temp->next;
        temp->next = newNode;
        return head;
    }
}


void printList(Node* head) {
    Node* temp = head;
    while(temp != NULL) {
        printf("x = %.2lf, f'(x) = %.2lf\n", temp->x, temp->derivative);
        temp = temp->next;
    }
}


void freeList(Node* head) {
    Node* temp;
    while(head != NULL) {
        temp = head;
        head = head->next;
        free(temp);
    }
}

int main() {
    Node* head = NULL;
    double h = 0.0001;

    
    for(double x = 0; x <= 5; x += 1.0) {
        double deriv = derivative(x, h);
        head = addNode(head, x, deriv);
    }

    printList(head);
    freeList(head);

    return 0;
}
