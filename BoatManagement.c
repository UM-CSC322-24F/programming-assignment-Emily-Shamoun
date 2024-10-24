#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX_BOAT_NUMBER 120
#define MAX_NAME_LENGTH 128

typedef enum {
    slip,
    land,
    trailor,
    storage,
    no_place
} PlaceType;

typedef union{
  int slipNumber;
  char bayLetter;
  char licenseTag[6];
  int storageNumber;
}ExtraInfo;

typedef struct{
  char name[128];
  int length;
  PlaceType placeType;
  ExtraInfo extraInfo;
  float amountOwed;
}BoatInfo;

//----------------------------------------------------------------------------------------------------------------------------------------------------------


// Convert a string to a place
PlaceType StringToPlaceType(char * PlaceString) {
  
  if (!strcasecmp(PlaceString,"slip")) {
    return(slip);
  }
  if (!strcasecmp(PlaceString,"land")) {
    return(land);
  }
  if (!strcasecmp(PlaceString,"trailor")) {
    return(trailor);
  }
  if (!strcasecmp(PlaceString,"storage")) {
    return(storage);
  }
  return(no_place);
}


// Convert a place to a string
char * PlaceToString(PlaceType Place) {
  
  switch (Place) {
    case slip:
      return("slip");
    case land:
      return("land");
    case trailor:
      return("trailor");
    case storage:
      return("storage");
    case no_place:
      return("no_place");
    default:
      printf("How the faaark did I get here?\n");
      exit(EXIT_FAILURE);
      break;
  }
}


void * Malloc(size_t Size) {
  void * Memory;
  
  if ((Memory = malloc(Size)) == NULL) {
    perror("Cannot malloc");
    exit(EXIT_FAILURE);
  }
  return(Memory);  
}


// Reads the boats from a file, stores each boat in the array boatArray, and returns the number of boats stored.
int getBoats(BoatInfo *boatArray[], char *fileName){
  FILE *file;
  int boatIndex;
  char infoLine[256];

  // opens the given file in read mode and checks to see if it exists.
  file = fopen(fileName,"r");
  if (file == NULL) {
    printf("Could not open file\n");
    return -1;
  }
  
  boatIndex = 0;

  /* While the file has another line, memory is malloced to store a new BoatInfo object (boat). Then the line is read and its information is stored in boat, and boat is stored in boatArray.
     Each time a new boat is added to the array, boatIndex gets incremented to keep record of how many boats are in the array. */
  while(fgets(infoLine, sizeof(infoLine), file)){
    BoatInfo *boat = (BoatInfo*)Malloc(sizeof(BoatInfo));
    char type[9], extra[7];
    
    sscanf(infoLine, "%[^,],%d,%[^,],%[^,],%f", boat->name, &boat->length, type, extra, &boat->amountOwed);
    boat->placeType = StringToPlaceType(type);

    // Stores the extra information in the correct category baised on the place type.
    switch (boat->placeType) {
      case slip:
        boat->extraInfo.slipNumber = atoi(extra);
	break;
      case land:
	boat->extraInfo.bayLetter = extra[0];
	break;
      case trailor:
        strcpy(boat->extraInfo.licenseTag, extra);
	break;
      case storage:
        boat->extraInfo.storageNumber = atoi(extra);
        break;
      case no_place:
        break;
    }
    boatArray[boatIndex] = boat;
    boatIndex++;     
  }
  fclose(file);
  return boatIndex;
}


// Displays the each boat with all of their information.
void printInventory(BoatInfo *boatArray[], int numberOfBoats){
  int index;

  for(index = 0; index < numberOfBoats; index++){
    printf("%-20s %3d' %8s ", boatArray[index]->name, boatArray[index]->length, PlaceToString(boatArray[index]->placeType));
    
    // The switch is used to correctly display the extra information for each boat baised on the place type.
    switch (boatArray[index]->placeType) {
      case slip:
        printf("  # %2d ", boatArray[index]->extraInfo.slipNumber);
	break;
      case land:
	printf("%6c ", boatArray[index]->extraInfo.bayLetter);
	break;
      case trailor:
	printf("%6s ", boatArray[index]->extraInfo.licenseTag);
        break;
      case storage:
        printf("  # %2d ", boatArray[index]->extraInfo.storageNumber);
        break;
      case no_place:
        break;
    }
    printf(" Owes $%4.2f\n", boatArray[index]->amountOwed);
  }
}


// The comparator function for qsort that is used to compare two BoatInfo objects by name.
int compareNames(const void *b1, const void *b2) {
  const BoatInfo *boat1 = *(const BoatInfo **)b1;
  const BoatInfo *boat2 = *(const BoatInfo **)b2;
  return strcmp(boat1->name, boat2->name);
}


// Mallocs memory for a new boat that will be added to boatArray. Returns the number of boats in the array.
int addBoat(BoatInfo *boatArray[], int numberOfBoats){
  BoatInfo *boat = (BoatInfo*)Malloc(sizeof(BoatInfo));
  char type[9], extra[7];

  // Prompts the user to enter information for the boat. 
  printf("Please enter the boat data in CSV format                 : ");
  scanf(" %[^,],%d,%[^,],%[^,],%f", boat->name, &boat->length, type, extra, &boat->amountOwed);
  boat-> placeType = StringToPlaceType(type);

  // The switch us used to put the extra information into the correct extraInfo category baised on the place type.
  switch (StringToPlaceType(type)) {
    case slip:
       boat->extraInfo.slipNumber = atoi(extra);
       break;
     case land:
       boat->extraInfo.bayLetter = extra[0];
       break;
     case trailor:
       strcpy(boat->extraInfo.licenseTag, extra);
       break;
     case storage:
       boat->extraInfo.storageNumber = atoi(extra);
       break;
     case no_place:
       break;
   }

  // The boat is added to the end of the boatArray, and numberOfBoats is incremented. The array is then sorted by qsort to put the boats in alphabetical order by their names.
  boatArray[numberOfBoats++] = boat;
  qsort(boatArray, numberOfBoats, sizeof(BoatInfo*), compareNames);

  return numberOfBoats;
}


// Converts each character of the given string to uppercase. 
char* makeUpper(char *string){
  int index;
  
  for(index = 0; string[index] != '\0'; index++){
    string[index] = toupper(string[index]);
  }
  return string;
}


// Checks to see if a given name matches a name of a boat in boatArray. If the name matches an existing boat name, then that boat is returned, else NULL is returned.
BoatInfo* validBoatName(BoatInfo *boatArray[], int numberOfBoats, char *name){
  int index;
  char temp[MAX_NAME_LENGTH];
  
  for(index = 0; index < numberOfBoats; index++){
    strcpy(temp, boatArray[index]->name);
    // If strcmp returns 0, then the names match and the boat with the matching name is returned. Calls makeUpper for the boat name and the given name to make both name's case insensitive.
    if(strcmp(makeUpper(name), makeUpper(temp)) == 0){
      return boatArray[index];
    }
  }
  return NULL ;
}

// Removes an existing boat from boatArray and returns the number of boats left in the array.
int removeBoat(BoatInfo *boatArray[], int numberOfBoats){
  BoatInfo *boat;
  char name[MAX_NAME_LENGTH];

  // Prompts the user to enter a boat name and calls validBoatName to check that the name is valid.
  printf("Please enter the boat name                               : ");
  scanf("\n%[^\n]s", name);
  boat = validBoatName(boatArray, numberOfBoats, name);

  // If the name is not valid, then the user is notified and the funtion returns the number of boats in the array.
  if(boat == NULL){
    printf("No boat with that name\n");
    return numberOfBoats;
  }
  
  // Else the boat that is to be removed is replaced by the the last boat in the array and numberOfBoats is decrimented. The array is then sorted to keep the boats in alphabetical name order.
  *boat = *boatArray[--numberOfBoats];
  qsort(boatArray, numberOfBoats, sizeof(BoatInfo*), compareNames);
  
  return numberOfBoats;
}


// Allows the user to make a payment for an existing boat.
void payment(BoatInfo *boatArray[], int numberOfBoats){
  char name[MAX_NAME_LENGTH];
  BoatInfo *boat;
  float paymentAmount;
  
  // Prompts the user to enter a name of a boat. Then vaildBoatName is called to make sure a boat with the given name exists in boatArray. 
  printf("Please enter the boat name                               : ");
  scanf("\n%[^\n]s", name);
  boat = validBoatName(boatArray, numberOfBoats, name);

  // If the name is invalid, then the user is notified and the function exists.
  if(boat == NULL){
    printf("Invalid name \n");
    return;
  }
  
  /* If the name is valid, then the user is prompted to enter an amount to be paid. If the payment amount exceeds the anount owed the user is notified and the function exist.
     Else the amount owed is subtracted by the payment amount. */
  printf("Please enter the amount to be paid                       : ");
  scanf("%f", &paymentAmount);
  if(paymentAmount > boat->amountOwed){
    printf("That is more than the amount owed, %.2f\n", boat->amountOwed);
    return;
  }
  boat->amountOwed -= paymentAmount;
}


// Calculates the price of the storage of each boat for one month baised on the boat's place type and length.
float calculatePrice(BoatInfo *boat){
  float price;
  
  switch(boat->placeType){
    case slip:
      price = 12.5 * boat->length;
      break;
    case land:
      price = 14.0 * boat->length;
      break;
    case trailor:
      price = 25.0 * boat->length;
      break;
    case storage:
      price = 11.2 * boat->length;
      break;
    case no_place:
      price = 0;
      break;
  }
  return price;
}


// Runs through each element in boatArray and adds the price of one month (by calling calculatePrice) to the existing price the boat has.
void month(BoatInfo *boatArray[], int numberOfBoats){
  int index;
  
  for(index = 0; index < numberOfBoats; index++){
    boatArray[index]->amountOwed += calculatePrice(boatArray[index]);
  }
}


// Puts the boats that are stored in boatArray into a file.
void putBoats(BoatInfo *boatArray[], int numberOfBoats,char *fileName){
  FILE *file;
  int index;

  // The given file is opened in writing mode.
  file = fopen(fileName,"w");

  // For each index in the array, the information stored in each boat is writen into the file. Once this is completed, the file is closed 
  for(index = 0; index < numberOfBoats; index++){
    fprintf(file, "%s,%d,%s,", boatArray[index]->name, boatArray[index]->length, PlaceToString(boatArray[index]->placeType));

    // The switch is used to put the correct extra information into the file baised on the place type. 
    switch (boatArray[index]->placeType) {
      case slip:
        fprintf(file,"%d,", boatArray[index]->extraInfo.slipNumber);
	break;
      case land:
	fprintf(file,"%c,", boatArray[index]->extraInfo.bayLetter);
	break;
      case trailor:
	fprintf(file,"%s,", boatArray[index]->extraInfo.licenseTag);
	break;
      case storage:
	fprintf(file,"%d,", boatArray[index]->extraInfo.storageNumber);
        break;
      case no_place:
	break;
    }
    fprintf(file,"%.2f\n", boatArray[index]->amountOwed);
  }
  fclose(file);
}


int main(int argc, char *argv[]){
  char *fileName, option;
  int numberOfBoats;
  BoatInfo *boatArray[MAX_BOAT_NUMBER];

  // If a file name is not given, then an error message is displayed and the program stops running.
  if(argc <= 1){
    printf("Error, file not entered\n");
    return EXIT_FAILURE;
  }

  printf("Welcome to the Boat Management System\n");
  printf("-------------------------------------\n\n");

  /* Gets the file name form the command line, and calls getBoats to get all of the BoatInfo objects (boats) from the file and store them in boatArray.
     qsort is then called to sort the array. */
  fileName = argv[1];
  numberOfBoats = getBoats(boatArray, fileName);
  qsort(boatArray, numberOfBoats, sizeof(BoatInfo*), compareNames);

  // This while loop is the main menu of the management system. At the start it prompts the user to enter one of the characters listed.
  while(1){
    printf("(I)nventory, (A)dd, (R)emove, (P)ayment, (M)onth, e(X)it : ");
    scanf(" %c", &option);
    
    // If 'X' or ('x') is entered, putBoats is called to put all of the boat information stored in boatArray are put into the given file. The exit message is printed and the while loop is exited.
    if(toupper(option) == 'X'){
      putBoats(boatArray, numberOfBoats, fileName);
      printf("\nExiting the Boat Management System\n");
      break;
    }
    
    // If any other character is entered, the switch is entered.
    switch (toupper(option)){
      // If 'I' or ('i') is entered, printInventory is called.
      case 'I':
	printInventory(boatArray, numberOfBoats);
	printf("\n");
	break;
      // If 'A' or ('a') is entered, addBoat is called.
      case 'A':
	numberOfBoats = addBoat(boatArray, numberOfBoats);
	printf("\n");
	break;
      // If 'R' or ('r') is entered, removeBoat is called.
      case 'R':
	numberOfBoats = removeBoat(boatArray, numberOfBoats);
	printf("\n");
        break;
      // If 'P' or ('p') is entered, payment is called.
      case 'P':
	payment(boatArray, numberOfBoats);
	printf("\n");
	break;
      // If 'M' or ('m')  is entered, month is called.
      case 'M':
	month(boatArray, numberOfBoats);
	printf("\n");
	break;
      // If a character that is not one of the listed characters is entered, then the user is notified that the option is invalid and the while loop runs again.
      default:
	printf("Invalid option %c\n", option);
	printf("\n");
	break;
    }	
  }
  
  // After all the boats are stored in the given file, all of the memory allocated to each boat is freed.
  for (int i = 0; i < numberOfBoats; i++){
    free(boatArray[i]);
  }
  return EXIT_SUCCESS;
}

