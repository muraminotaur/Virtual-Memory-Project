// addrspace.cc 
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -N -T 0 option 
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "noff.h"
#ifdef HOST_SPARC
#include <strings.h>
// Added by Jacob Slagle
#include "bitmap.h"
// End Changes by JS
#endif

//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the 
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------
// task
static void 
SwapHeader (NoffHeader *noffH)
{
	noffH->noffMagic = WordToHost(noffH->noffMagic);
	noffH->code.size = WordToHost(noffH->code.size);
	noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
	noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
	noffH->initData.size = WordToHost(noffH->initData.size);
	noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
	noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
	noffH->uninitData.size = WordToHost(noffH->uninitData.size);
	noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
	noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	First, set up the translation from program memory to physical 
//	memory.  For now, this is really simple (1:1), since we are
//	only uniprogramming, and we have a single unsegmented page table
//
//	"executable" is the file containing the object code to load into memory
//----------------------------------------------------------------------


//----------------------------------------------------------------------
// Bitmap Usage --- Added by Jacob Slagle
//  This Bitmap will be used to track which physical pages are available
//  to use within the pageTable.
//----------------------------------------------------------------------

AddrSpace::AddrSpace(OpenFile *executable){
    NoffHeader noffH;
    unsigned int i, size;

/*
checking header for Nachos Object File Format
 */
    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) && 
		(WordToHost(noffH.noffMagic) == NOFFMAGIC))
    	SwapHeader(&noffH);
    ASSERT(noffH.noffMagic == NOFFMAGIC);

// how big is address space?
    size =    noffH.code.size               // size of code
            + noffH.initData.size           // initial data (?)
            + noffH.uninitData.size         // uninit data (?)
			+ UserStackSize;	            // we need to increase the size to leave room for the stack

    numPages = divRoundUp(size, PageSize);
    size = numPages * PageSize;

/*
change this to display if there is insufficient
memory and to terminate 
this file and *only this file*
 */

//start code changes by Wyatt Woodall
    dynamicPages=0;
    sourceFile=executable;
//end code changes by Wyatt Woodall
//start code changes by Chikaodi Nnanna

/*
use a buffer to read in the contents of
executable and write them into the swap file
 */
    char swapFileName[20];
    sprintf(swapFileName, "%d.swap", currentThread->getID());
    fileSystem->Create(swapFileName, size);             //create ID.swap
    swapFile = fileSystem->Open(swapFileName);         
    char *exeContents = new char [size];                //buffer contents will be read into
    if (noffH.code.size > 0) {                          //read contents into buffer
        executable->ReadAt(&exeContents[noffH.code.virtualAddr], noffH.code.size, noffH.code.inFileAddr);
    }
    if (noffH.initData.size > 0) {
        executable->ReadAt(&exeContents[noffH.initData.virtualAddr], noffH.initData.size, noffH.initData.inFileAddr);
    }
    swapFile->WriteAt(exeContents, size, 0);            //write what's in buffer into the swap file
    delete[] exeContents;                               //delete buffer

//end code changes by Chikaodi Nnanna    
// Start changes by Jacob Slagle 

//ASSERT(numPages <= NumPhysPages);		// check we're not trying
						// to run anything too big --
						// at least until we have
						// virtual memory
    printf("\nNum pages: %d\n", numPages);
    if(numPages > validPages->NumClear()){
        printf("\nNot enough memory available.\n");
        numPages = 0;
        return;
    }

    DEBUG('a', "Initializing address space, num pages %d, size %d\n", 
					numPages, size);
// first, set up the translation 

/*
 *  building page table
 */ 

    pageTable = new TranslationEntry[numPages];
    //start code changes by Wyatt Woodall
    /*
    for (i = 0; i < numPages; i++) {
	pageTable[i].virtualPage = i;	// for now, virtual page # = phys page #

	pageTable[i].physicalPage = validPages->Find();
    //start code change by wyatt woodall
	pageTable[i].valid = FALSE;
    //end code changes by wyatt woodall
	pageTable[i].use = FALSE;
	pageTable[i].dirty = FALSE;
	pageTable[i].readOnly = FALSE;  // if the code segment was entirely on 
					// a separate page, we could set its 
					// pages to be read-only
    }
    */
   //end code changes by Wyatt Woodall
    
// zero out the entire address space, to zero the unitialized data segment 
// and the stack segment
//    bzero(machine->mainMemory, size);

// then, copy in the code and data segments into memory

// End Changes by Jacob Slagle
// where the page table -> memory happens
// start code changes by Wyatt Woodall
/* 
    if (noffH.code.size > 0) {
        DEBUG('a', "Initializing code segment, at 0x%x, size %d\n", 
			noffH.code.virtualAddr, noffH.code.size);
        executable->ReadAt(&(machine->mainMemory[noffH.code.virtualAddr]),
			noffH.code.size, noffH.code.inFileAddr);
    }
    if (noffH.initData.size > 0) {
        DEBUG('a', "Initializing data segment, at 0x%x, size %d\n", 
			noffH.initData.virtualAddr, noffH.initData.size);
        executable->ReadAt(&(machine->mainMemory[noffH.initData.virtualAddr]),
			noffH.initData.size, noffH.initData.inFileAddr);
    }
*/
//end code changes by wyatt woodall

}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
    //Start changes by Jacob Slagle
    for(int i = 0; i < numPages; i++){
        validPages->Clear(pageTable[i].physicalPage);
    }
    //End Changes by Jacob Slagle
    //start code changes Wyatt Woodall
    delete sourceFile;
    //end code changes Wyatt Woodall
    delete pageTable;
    //start code changes Chikaodi Nnanna
    delete swapFile;
    //end code changes Chikaodi Nnanna
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
	machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);	

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

   // Set the stack register to the end of the address space, where we
   // allocated the stack; but subtract off a bit, to make sure we don't
   // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    DEBUG('a', "Initializing stack register to %d\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState() 
{}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState() 
{
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
}

//Start Changes by Jacob Slagle

//----------------------------------------------------------------------
// AddrSpace::Valid
//  Returns true if allocation is successful, false if otherwise.
//----------------------------------------------------------------------

bool
AddrSpace::Valid(){
    return numPages != 0;
}

//End Changes by Jacob Slagle

//start code changes by Wyatt Woodall

//----------------------------------------------------------------------
// AddrSpace::fillPageTable
//  Fills the page table, for primary use in the PageFaultException. Returns true
//  if succesful. Returns False if page table cannot be allocated.
//----------------------------------------------------------------------

bool
AddrSpace::fillPageTable(){
    if(validPages->NumClear()==0){
        return FALSE;
    }
    pageTable[dynamicPages].virtualPage = dynamicPages;
	pageTable[dynamicPages].physicalPage = validPages->Find();
	pageTable[dynamicPages].valid = TRUE;
	pageTable[dynamicPages].use = FALSE;
	pageTable[dynamicPages].dirty = FALSE;
	pageTable[dynamicPages].readOnly = FALSE;
    dynamicPages++;
    validPages->Print();
    return TRUE;
}

//----------------------------------------------------------------------
// AddrSpace::getCurrentPhysPage()
//  Returns the index for the current physical page needed for the user program to run
//----------------------------------------------------------------------

unsigned int
AddrSpace::getCurrentPhysPage(){
    return pageTable[dynamicPages-1].physicalPage;
}

//----------------------------------------------------------------------
// AddrSpace::readFile()
//  Uses the executable and loads the page into memory.
//----------------------------------------------------------------------
void
AddrSpace::readPage(){
    int physPageNum = pageTable[dynamicPages-1].physicalPage;
    //sourceFile->ReadAt(&(machine->mainMemory[physPageNum * PageSize]),
        //PageSize,(machine->ReadRegister(BadVAddrReg) / PageSize) * PageSize * dynamicPages + sizeof(NoffHeader));
    /*printf("\nMain Mem:\n");
    for(int i=0;i<32;i++){
        printf("PageNum: %d\n",i);
        for(int j=0;j<PageSize;j++){
            printf("%02x ", machine->mainMemory[(i * PageSize)+ i]);
        }
        printf("\n\n\n");
    }
    */
    printf("\n");   
    //start code changes by Chikaodi Nnanna
    // TODO: condition for when demand paging vs swap files
    swapFile->ReadAt(&(machine->mainMemory[physPageNum * PageSize]),
       PageSize, (machine->registers[BadVAddrReg] / PageSize) * PageSize);
    //end code changes by Chikaodi Nnanna
    printf("test\n");
}
//end code changes by Wyatt Woodall