#pragma once
#include <vector>
using namespace std;

typedef struct PageReference {

	int page_num;
} PageReference;


typedef struct Frame {
	int index;
	int page; // page frame pointer
	int R; // R bit
} Frame;

vector<Frame> victims;

vector<Frame> table;
vector<PageReference> pages;

void genRefs();
PageReference genPage();
Frame createEmptyFrame(int index);
void get(int PageReference);
int getRef();
void toSwap(Frame *frame);

void showMem();
void showSummary();

void Algo_clock();
#pragma once
