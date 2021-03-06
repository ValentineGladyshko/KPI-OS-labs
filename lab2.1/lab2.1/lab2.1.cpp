#include "stdafx.h"
#include <time.h>
#include <iostream>

struct Page {
	int page;
	bool exist;
	bool modified;
	bool requested;
	long long time;
	Page *next;
};

Page* getEmptyPage() {
	Page* temp = new Page();
	temp->page = -1;
	temp->exist = false;
	temp->modified = false;
	temp->requested = false;
	temp->time = clock();
	return temp;
}

Page* getEmptyClock(int count) {
	Page *head = getEmptyPage();
	Page* tail = head;
	for (int i = 0; i < count - 1; ++i) {
		Page *nextOne = getEmptyPage();
		tail->next = nextOne;
		tail = nextOne;
	}
	tail->next = head;
	return head;
}

void deleteClock(Page* clock) {
	Page *head = clock;
	head = head->next;
	clock->next = nullptr;
	while (head != nullptr) {
		Page *temp = head;
		head = head->next;
		delete temp;
	}
}

Page* insertNewPage(Page* clock1, int page) {
	Page* head = clock1;
	while (head != nullptr) {
		if (head->exist
			&& head->page == page) {
			head->time = clock();
			head->requested = true;
			return clock1;
		}
		else if (!head->exist) {
			head->exist = true;
			head->page = page;
			return clock1;
		}
		head = head->next;
		if (head == clock1) {
			break;
		}
	}
	//there is no one free cell
	while (head != nullptr) {
		long long end = clock();
		if (!head->requested && (end - head->time) > 20) {
			head->page = page;
			head->time = clock();
			return head->next;
		}
		head->requested = false;
		head = head->next;
	}
}

void printClock(Page* clock) {
	printf("Clock: \n");
	Page* head = clock;
	while (head != nullptr) {
		std::cout << "\tValue: " << head->page << ", Requested: " << head->requested << ", Time: " << head->time << std::endl;
		head = head->next;
		if (head == clock)
			return;
	}
}

int main()
{
	Page* clock = getEmptyClock(8);
	for (int i = 0; i < 8; ++i)
		clock = insertNewPage(clock, i + 1);
	printClock(clock);
	clock = insertNewPage(clock, 1);
	clock = insertNewPage(clock, 3);
	clock = insertNewPage(clock, 4);
	printClock(clock);
	clock = insertNewPage(clock, 9);
	clock = insertNewPage(clock, 10);
	printClock(clock);
	deleteClock(clock);
	system("pause");
	return 0;
}