//
// Virtual Memory Simulator Homework
// Two-level page table system
// Inverted page table with a hashing system
// Student Name: 김연서
// Student Number: B411037
//
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define PAGESIZEBITS 12			// page size = 4Kbytes
#define VIRTUALADDRBITS 32		// virtual address space size = 4Gbytes

struct pageTableEntry {
	int level;				// page table level (1 or 2)
	char valid;
	struct pageTableEntry *secondLevelPageTable;	// valid if this entry is for the first level page table (level = 1)
	int frameNumber;								// valid if this entry is for the second level page table (level = 2)
};

struct framePage {
	int number;			// frame number
	int pid;			// Process id that owns the frame
	int virtualPageNumber;			// virtual page number using the frame
	struct framePage *lruLeft;	// for LRU circular doubly linked list
	struct framePage *lruRight; // for LRU circular doubly linked list
};

struct invertedPageTableEntry {
	int pid;					// process id
	int virtualPageNumber;		// virtual page number
	int frameNumber;			// frame number allocated
	struct invertedPageTableEntry *next;
};

struct procEntry {
	char *traceName;			// the memory trace name
	int pid;					// process (trace) id
	int ntraces;				// the number of memory traces
	int num2ndLevelPageTable;	// The 2nd level page created(allocated);
	int numIHTConflictAccess; 	// The number of Inverted Hash Table Conflict Accesses
	int numIHTNULLAccess;		// The number of Empty Inverted Hash Table Accesses
	int numIHTNonNULLAccess;		// The number of Non Empty Inverted Hash Table Accesses
	int numPageFault;			// The number of page faults
	int numPageHit;				// The number of page hits
	struct pageTableEntry *firstLevelPageTable;
	FILE *tracefp;
};

struct framePage *oldestFrame; // the oldest frame pointer

int firstLevelBits, secondLevelBits, phyMemSizeBits, numProcess;

void initPhyMem(struct framePage *phyMem, int nFrame) {
	int i;
	for(i = 0; i < nFrame; i++) {
		phyMem[i].number = i;
		phyMem[i].pid = -1;
		phyMem[i].virtualPageNumber = -1;
		phyMem[i].lruLeft = &phyMem[(i-1+nFrame) % nFrame];
		phyMem[i].lruRight = &phyMem[(i+1+nFrame) % nFrame];
	}
	oldestFrame = &phyMem[0];
}

void newMapping(struct invertedPageTableEntry *head){
	struct invertedPageTableEntry *newPage = malloc(sizeof(struct invertedPageTableEntry));
	newPage->next = head->next;
	head->next = newPage;
}

void secondLevelVMSim(struct procEntry *procTable, struct framePage *phyMemFrames, int nFrame) {
	int i, j;
	unsigned addr; char rw;
	unsigned virtualPageNumber, masterPageNumber, secondaryPageNumber, offset;
	int masterPTSize, secondaryPTSize, physicalAddress;
	int currentFrameNumber;
	unsigned secondBit = (1 << secondLevelBits) - 1;

	masterPTSize = 1 << firstLevelBits;
	secondaryPTSize = 1 << (VIRTUALADDRBITS - PAGESIZEBITS - firstLevelBits);

	struct pageTableEntry masterPageTable[masterPTSize];	// Initialize master page table
	for(i = 0; i < masterPTSize; i++){
		masterPageTable[i].level = 1;
		masterPageTable[i].valid = 0;
		masterPageTable[i].secondLevelPageTable = NULL;
	}
	while(1){
		for(i = 0; i < numProcess; ++i){	// process1, process2, process3, ..., process1, process2, process3, ...
			if(fscanf(procTable[i].tracefp, "%x %c", &addr, &rw) == EOF) goto out;	// Read virtual address and read/write bit. If reach EOF, escape loop

			virtualPageNumber = addr >> PAGESIZEBITS;
			masterPageNumber = virtualPageNumber >> secondLevelBits;
			secondaryPageNumber = (addr << firstLevelBits) >> firstLevelBits >> PAGESIZEBITS;
			offset = (addr << (VIRTUALADDRBITS-PAGESIZEBITS)) >> (VIRTUALADDRBITS-PAGESIZEBITS);

			if(masterPageTable[masterPageNumber].valid == 1){	// second level page is already generated
				if(masterPageTable[masterPageNumber].secondLevelPageTable[secondaryPageNumber].valid == 1){	// page hit
					procTable[i].ntraces++;
					procTable[i].numPageHit++;

					if(oldestFrame->virtualPageNumber == virtualPageNumber && oldestFrame->pid == i){	// hit page == oldest frame, mvoe oldest frame pointer
						oldestFrame = oldestFrame->lruRight;
					}
					else{
                        currentFrameNumber = masterPageTable[masterPageNumber].secondLevelPageTable[secondaryPageNumber].frameNumber;
//
                        phyMemFrames[currentFrameNumber].lruLeft->lruRight = phyMemFrames[currentFrameNumber].lruRight;
						phyMemFrames[currentFrameNumber].lruRight->lruLeft = phyMemFrames[currentFrameNumber].lruLeft;

						oldestFrame->lruLeft->lruRight = &phyMemFrames[currentFrameNumber];
						phyMemFrames[currentFrameNumber].lruLeft = oldestFrame->lruLeft;
						oldestFrame->lruLeft = &phyMemFrames[currentFrameNumber];
						phyMemFrames[currentFrameNumber].lruRight = oldestFrame;
					}
				}
				else{	// valid bit is 0. the second level page is existed, but page fault
					procTable[i].ntraces++;
					procTable[i].numPageFault++;
					masterPageTable[masterPageNumber].secondLevelPageTable[secondaryPageNumber].valid = 1;
					masterPageTable[masterPageNumber].secondLevelPageTable[secondaryPageNumber].frameNumber = oldestFrame->number;

					if(oldestFrame->pid != -1){	// if the oldest frame is full, switch page table's valid bit
						//printf("%x replaced page's address %x %x\n", oldestFrame->virtualPageNumber, oldestFrame->virtualPageNumber >> secondLevelBits, oldestFrame->virtualPageNumber & secondBit);
						masterPageTable[oldestFrame->virtualPageNumber >> secondLevelBits].secondLevelPageTable[oldestFrame->virtualPageNumber & secondBit].valid = 0;
					}
					oldestFrame->pid = i;
					oldestFrame->virtualPageNumber = virtualPageNumber;
					oldestFrame = oldestFrame->lruRight;
				}
			}
			else{	// first time access to master page. second level page table is not existed
				if(masterPageTable[masterPageNumber].valid == 0){
					procTable[i].ntraces++;
					procTable[i].num2ndLevelPageTable++;
					procTable[i].numPageFault++;
					masterPageTable[masterPageNumber].valid = 1;
					masterPageTable[masterPageNumber].secondLevelPageTable = (struct pageTableEntry*)malloc(sizeof(struct pageTableEntry)*secondaryPTSize);
					for(j = 0; j < secondaryPTSize; ++j){	// initialize second level page table
						masterPageTable[masterPageNumber].secondLevelPageTable[j].level = 2;
						masterPageTable[masterPageNumber].secondLevelPageTable[j].valid = 0;
						masterPageTable[masterPageNumber].secondLevelPageTable[j].frameNumber = -1;
					}
					masterPageTable[masterPageNumber].secondLevelPageTable[secondaryPageNumber].valid = 1;
					masterPageTable[masterPageNumber].secondLevelPageTable[secondaryPageNumber].frameNumber = oldestFrame->number;

					if(oldestFrame->pid != -1){
						masterPageTable[oldestFrame->virtualPageNumber >> secondLevelBits].secondLevelPageTable[oldestFrame->virtualPageNumber & secondBit].valid = 0;
					}
					oldestFrame->pid = i;
					oldestFrame->virtualPageNumber = virtualPageNumber;
					oldestFrame = oldestFrame->lruRight;
				}
			}
			physicalAddress = masterPageTable[masterPageNumber].secondLevelPageTable[secondaryPageNumber].frameNumber;
			printf("2Level procID %d traceNumber %d virtual addr %x pysical addr %x\n", i, procTable[i].ntraces, addr, physicalAddress<<PAGESIZEBITS | offset);
		}
	}
out:	for(i=0; i < numProcess; i++) {	// loop escape
		printf("**** %s *****\n",procTable[i].traceName);
		printf("Proc %d Num of traces %d\n",i,procTable[i].ntraces);
		printf("Proc %d Num of second level page tables allocated %d\n",i,procTable[i].num2ndLevelPageTable);
		printf("Proc %d Num of Page Faults %d\n",i,procTable[i].numPageFault);
		printf("Proc %d Num of Page Hit %d\n",i,procTable[i].numPageHit);
		assert(procTable[i].numPageHit + procTable[i].numPageFault == procTable[i].ntraces);
	}
}

void invertedPageVMSim(struct procEntry *procTable, struct framePage *phyMemFrames, int nFrame) {
	int i, j;
	unsigned virtualAddress; char rw;
	int virtualPageNumber, physicalAddress;

	struct invertedPageTableEntry *IPHT = (struct invertedPageTableEntry*)malloc(sizeof(struct invertedPageTableEntry)*nFrame);
	struct invertedPageTableEntry *currentPage, *previousPage;

	for(i = 0; i < nFrame; ++i){
		IPHT[i].pid = -1;
		IPHT[i].virtualPageNumber = -1;
		IPHT[i].frameNumber = -1;
		IPHT[i].next = NULL;
	}

	while(1){
		for(i = 0; i < numProcess; ++i){
			if(fscanf(procTable[i].tracefp, "%x %c", &virtualAddress, &rw) == EOF) goto out2;	// fscanf virtual address and valid bit. if reach EOF, escape loop.
			procTable[i].ntraces++;

			virtualPageNumber = virtualAddress >> PAGESIZEBITS;
			currentPage = &IPHT[(virtualPageNumber + i) % nFrame];	// indexed page

			if(currentPage->next == NULL){	// no mapping info. empty entry access and page fault
				procTable[i].numIHTNULLAccess++;
				procTable[i].numPageFault++;
				//procTable[i].numIHTConflictAccess++;

				newMapping(currentPage);	// create new mapping information
				currentPage = currentPage->next;
				currentPage->pid = i;
				currentPage->virtualPageNumber = virtualPageNumber;
				currentPage->frameNumber = oldestFrame->number;

				oldestFrame->pid = i;
				oldestFrame->virtualPageNumber = virtualPageNumber;
				oldestFrame = oldestFrame->lruRight;
			}
			else{
				procTable[i].numIHTNonNULLAccess++;	// non null access

				while(1){
                    currentPage = currentPage->next;
       				procTable[i].numIHTConflictAccess++;

					if(currentPage->pid == i && currentPage->virtualPageNumber == virtualPageNumber){
                        if(currentPage->frameNumber != -1){
                            procTable[i].numPageHit++;  // else page hit
                            //procTable[i].numIHTConflictAccess++;
                            if(oldestFrame->virtualPageNumber == virtualPageNumber && oldestFrame->pid == i){	// if hit page's frame is the oldest frame
                                oldestFrame = oldestFrame->lruRight;
                            }
                            else{   // else if hit page frame is not oldest frame
                                phyMemFrames[currentPage->frameNumber].lruLeft->lruRight = phyMemFrames[currentPage->frameNumber].lruRight;
                                phyMemFrames[currentPage->frameNumber].lruRight->lruLeft = phyMemFrames[currentPage->frameNumber].lruLeft;

                                oldestFrame->lruLeft->lruRight = &phyMemFrames[currentPage->frameNumber];
                                phyMemFrames[currentPage->frameNumber].lruLeft = oldestFrame->lruLeft;
                                oldestFrame->lruLeft = &phyMemFrames[currentPage->frameNumber];
                                phyMemFrames[currentPage->frameNumber].lruRight = oldestFrame;
                            }
                            break;
						}
/*
                        else if(currentPage->frameNumber == -1){  // page fault
                            procTable[i].numPageFault++;
                            //procTable[i].numIHTConflictAccess++;
							currentPage->frameNumber = oldestFrame->number;

                            if(oldestFrame->virtualPageNumber == -1){   // frame is empty
                                oldestFrame->pid = i;
                                oldestFrame->virtualPageNumber = virtualPageNumber;
                                oldestFrame = oldestFrame->lruRight;
                                break;
                            }
                            else if(oldestFrame->virtualPageNumber != -1){ // 기존에 할당 되어 있던 frame을 다른 page 에 할당 할 경우 기존의 매핑 정보를 없애야합니다.
                                currentPage = &IPHT[(oldestFrame->virtualPageNumber + oldestFrame->pid) % nFrame];
                                previousPage = currentPage;
                                currentPage = currentPage->next;
                                while(1){
                                    if(currentPage->frameNumber == oldestFrame->number){
                                        previousPage->next = currentPage->next;
                                        //currentPage->frameNumber = -1;
                                        break;
                                    }
                                    else{
                                        previousPage = currentPage;
                                        currentPage = currentPage->next;
                                    }
                                }
                                oldestFrame->pid = i;
                                oldestFrame->virtualPageNumber = virtualPageNumber;
                                oldestFrame = oldestFrame->lruRight;
                                break;
                            }
                        }
*/
                    }
					//else{	// page fault
						if(currentPage->next == NULL){	// if current page's next entry is null
                            procTable[i].numPageFault++;
							//procTable[i].numIHTConflictAccess++;
							currentPage = &IPHT[(virtualPageNumber + i) % nFrame];
							newMapping(currentPage);

							currentPage = currentPage->next;
							currentPage->pid = i;
							currentPage->virtualPageNumber = virtualPageNumber;
							currentPage->frameNumber = oldestFrame->number;

							//currentPage->next = NULL;
							if(oldestFrame->pid != -1){ // 기존에 할당 되어 있던 frame을 다른 page 에 할당 할 경우 기존의 매핑 정보를 없애야합니다.
                                currentPage = &IPHT[(oldestFrame->virtualPageNumber + oldestFrame->pid) % nFrame];
                                previousPage = currentPage;
                                currentPage = currentPage->next;
                                while(1){
                                    if(currentPage->frameNumber == oldestFrame->number){
                                        previousPage->next = currentPage->next;
                                        //currentPage->frameNumber = -1;
                                        break;
                                    }
                                    previousPage = currentPage;
                                    currentPage = currentPage->next;
                                }
                            }
							oldestFrame->pid = i;
							oldestFrame->virtualPageNumber = virtualPageNumber;
							oldestFrame = oldestFrame->lruRight;
							break;
						//}
						/*else{	// current page's next entry is not null, explore next page
							procTable[i].numIHTConflictAccess++;
							currentPage = currentPage->next;	// Non null access
						}*/
                    }
				}
			}
			physicalAddress = ((oldestFrame->lruLeft->number) << PAGESIZEBITS) | (virtualAddress << (VIRTUALADDRBITS-PAGESIZEBITS)) >> (VIRTUALADDRBITS-PAGESIZEBITS);
			printf("IHT procID %d traceNumber %d virtual addr %x pysical addr %x\n", i, procTable[i].ntraces, virtualAddress, physicalAddress);
		}
	}

out2:	for(i=0; i < numProcess; i++) {
		printf("**** %s *****\n",procTable[i].traceName);
		printf("Proc %d Num of traces %d\n",i,procTable[i].ntraces);
		printf("Proc %d Num of Inverted Hash Table Access Conflicts %d\n",i,procTable[i].numIHTConflictAccess);
		printf("Proc %d Num of Empty Inverted Hash Table Access %d\n",i,procTable[i].numIHTNULLAccess);
		printf("Proc %d Num of Non-Empty Inverted Hash Table Access %d\n",i,procTable[i].numIHTNonNULLAccess);
		printf("Proc %d Num of Page Faults %d\n",i,procTable[i].numPageFault);
		printf("Proc %d Num of Page Hit %d\n",i,procTable[i].numPageHit);
		assert(procTable[i].numPageHit + procTable[i].numPageFault == procTable[i].ntraces);
		assert(procTable[i].numIHTNULLAccess + procTable[i].numIHTNonNULLAccess == procTable[i].ntraces);
	}
}

int main(int argc, char *argv[]) {
	int i;
	unsigned int addr;
	char rw;

	firstLevelBits = atoi(argv[1]);
	phyMemSizeBits = atoi(argv[2]);
	numProcess = argc-3;
	secondLevelBits = VIRTUALADDRBITS - PAGESIZEBITS - firstLevelBits;

	if (argc < 4) {
	     printf("Usage : %s firstLevelBits PhysicalMemorySizeBits TraceFileNames\n",argv[0]); exit(1);
	}

	if (phyMemSizeBits < PAGESIZEBITS) {
		printf("PhysicalMemorySizeBits %d should be larger than PageSizeBits %d\n",phyMemSizeBits,PAGESIZEBITS); exit(1);
	}
	if (VIRTUALADDRBITS - PAGESIZEBITS - firstLevelBits <= 0 ) {
		printf("firstLevelBits %d is too Big\n",firstLevelBits); exit(1);
	}

	// initialize procTable for two-level page table
	struct procEntry procTable[numProcess];
	for(i = 0; i < numProcess; i++) {
		// opening a tracefile for the process
		printf("process %d opening %s\n",i,argv[i+3]);
		procTable[i].tracefp = fopen(argv[i+3], "r");
		procTable[i].traceName = argv[i+3];
		procTable[i].ntraces = 0;
		procTable[i].num2ndLevelPageTable = 0;
		procTable[i].numPageFault = 0;
		procTable[i].numPageHit = 0;
	}

	int nFrame = (1<<(phyMemSizeBits-PAGESIZEBITS)); assert(nFrame>0);
	struct framePage* phyMem = (struct framePage*)malloc(sizeof(struct framePage)*nFrame);

	printf("\nNum of Frames %d Physical Memory Size %ld bytes\n",nFrame, (1L<<phyMemSizeBits));

	printf("=============================================================\n");
	printf("The 2nd Level Page Table Memory Simulation Starts .....\n");
	printf("=============================================================\n");

	initPhyMem(phyMem, nFrame);	// initialize physical memory
	secondLevelVMSim(procTable, phyMem, nFrame);

	// initialize procTable for the inverted Page Table
	for(i = 0; i < numProcess; i++) {
		// rewind tracefiles
		rewind(procTable[i].tracefp);
		procTable[i].ntraces = 0;
		procTable[i].numIHTConflictAccess = 0;
		procTable[i].numIHTNULLAccess = 0;
		procTable[i].numIHTNonNULLAccess = 0;
		procTable[i].numPageFault = 0;
		procTable[i].numPageHit = 0;
	}

	printf("=============================================================\n");
	printf("The Inverted Page Table Memory Simulation Starts .....\n");
	printf("=============================================================\n");

	initPhyMem(phyMem, nFrame);	// initialize physical memory
	invertedPageVMSim(procTable, phyMem, nFrame);

	return(0);
}
