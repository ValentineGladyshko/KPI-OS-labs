#pragma warning (disable : 4996)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <fstream>
#include <algorithm>
#include "algorithm.h"

int framesInMemory = 4;
int maxPageReference = 15;
int Calls = 20;
int last_PageReference = -1;
int hits = 0;
int misses = 0;
FILE *logfile = fopen("logfile.txt", "w");
Frame *clock_hand = NULL;

int main() {
	for (int i = 0; i < framesInMemory; ++i) {
		table.push_back(createEmptyFrame(i));
	}

	genRefs();

	for (int i = 0; i < Calls; i++) {
		get(getRef());
	}

	showSummary();


	fclose(logfile);
	system("pause");
	return 0;
}


void genRefs() {
	for (int i = 0; i < Calls; i++) {
		pages.push_back(genPage());
	}

	return;
}

PageReference genPage() {
	PageReference page;
	page.page_num = rand() % maxPageReference;
	return page;
}

Frame createEmptyFrame(int index) {
	Frame Myframe;
	Myframe.index = index;
	Myframe.page = -1;
	Myframe.R = 0;

	return Myframe;
}

int getRef() {
	if (!pages.empty()) {
		int page_num = pages.begin()->page_num;
		pages.erase(pages.begin());

		return page_num;
	}
	else {
		return rand() % maxPageReference;
	}
}

void get(int pageReference) {

	last_PageReference = pageReference;
	Algo_clock();

	showSummary();
	showMem();
}

void toSwap(Frame *frame) {

	printf("To swap: page index: %d, page: %d\n", frame->index, frame->page);
	fprintf(logfile, "To swap: page index: %d, page: %d\n", frame->index, frame->page);

	Frame victim;
	victim = *frame;
	victim.index = 1;

	reverse(victims.begin(), victims.end());
	victims.push_back(victim);
	reverse(victims.begin(), victims.end());
}

void Algo_clock() {

	Frame *Myframe = &table[0];


	int fault = 0;
	int ind = 1;

	while (Myframe != NULL && Myframe->page > -1 && Myframe->page != last_PageReference) {
		if (ind == framesInMemory)
			Myframe = NULL;
		else
			Myframe = &table[ind++];
	}

	if (Myframe != NULL) {
		if (Myframe->page == -1) {
			Myframe->page = last_PageReference;
			Myframe->R = 0;
			fault = 1;
		}

	}
	else {
		while (clock_hand == NULL || clock_hand->R == 1) {
			if (clock_hand == NULL) {
				clock_hand = &table[0];
			}
			else {
				clock_hand->R = 0;

				if (clock_hand->index == framesInMemory - 1) {
					clock_hand = NULL;
				}
				else {
					clock_hand = &table[clock_hand->index + 1];
				}
			}
		}

		toSwap(clock_hand);


		clock_hand->page = last_PageReference;
		clock_hand->R = 1;

		fault = 1;
	}

	if (fault == 1) misses++;
	else hits++;

	return;
}

void showStats() {
	showSummary();
	showMem();
}

void showSummary() {
	printf("Frames in Mem: %d, \n", framesInMemory);
	printf("Hits: %d, \n", hits);
	printf("Misses: %d, \n", misses);
	//	printf("Hit Persent: %f\n", (double)hits / (double)(hits + misses));

	fprintf(logfile, "Frames in Mem: %d, \n", framesInMemory);
	fprintf(logfile, "Hits: %d, \n", hits);
	fprintf(logfile, "Misses: %d, \n", misses);
	//fprintf(logfile, "Hit Persent: %f\n", (double)hits / (double)(hits + misses));
}

void showMem() {
	int colsize = 9;

	printf("\n%-*s: ", 13, "Pages in mem");
	fprintf(logfile, "\n%-*s: ", 13, "Pages in mem");

	for (int i(0); i < table.size(); i++) {
		if (table[i].page == -1) {
			printf("%*s", colsize, " ");
			fprintf(logfile, "%*s", colsize, " ");
		}
		else {
			fprintf(logfile, "%*d", colsize, table[i].page);
			printf("%*d", colsize, table[i].page);
		}
	}

	printf("\n%-*s: ", 13, "R");
	fprintf(logfile, "\n%-*s: ", 13, "R");

	for (int i(0); i < table.size(); i++) {
		printf("%*d", colsize, table[i].R);
		fprintf(logfile, "%*d", colsize, table[i].R);
	}

	printf("\n\n");
	fprintf(logfile, "\n\n");
}
