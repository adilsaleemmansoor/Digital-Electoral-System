#include <Adafruit_Fingerprint.h>
#include "adalogo.h"
#include "adaqrcode.h"
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Thermal.h>
#include <SoftwareSerial.h>

// Pin definitions
const int button1Pin = 4;  // Left navigation
const int button2Pin = 5;  // Confirm vote
const int button3Pin = 6;  // Right navigation
const int printerRX = 10;
const int printerTX = 11;

int buttonState1 = 0;
int buttonState2 = 0;
int buttonState3 = 0;
int G_led = 8; // choose the pin for the Green LED
int R_led = 9; // choose the pin for the Red Led
const int buzzer = 7;

int id = 0;
int vote_taken = 0;
int totalVotes;
int PTI = 0, PPP = 0, ANP = 0;
String winner_name = "";
String winnerPercentage;

// Array to store voter IDs and a counter for the number of voters
const int MAX_VOTERS = 100; // Maximum number of voters we expect
int voter_ids[MAX_VOTERS];
int voter_count = 0;

// Variables for candidate selection
int currentCandidateIndex = 0;
String candidates[] = {"PTI", "PPP", "ANP"};
int numCandidates = sizeof(candidates) / sizeof(candidates[0]);

// Initialize SoftwareSerial for the printer
SoftwareSerial printerSerial(printerTX, printerRX);
Adafruit_Thermal printer(&printerSerial);

// Fingerprint sensor pins
SoftwareSerial fingerSerial(2, 3); // RX, TX
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fingerSerial);

// Initialize LCD
LiquidCrystal_I2C lcd(0x27, 24, 4); // Change the I2C address if needed

void setup() { 
  // Initialize Serial communication with ESP32
  Serial.begin(115200); // Assuming ESP32 is connected to the main Serial

  // Initialize software serial for the printer
  printerSerial.begin(9600);
  printer.begin();
  
  // Initialize the fingerprint sensor
  fingerSerial.begin(57600);
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }
  finger.getTemplateCount();
  Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");

  // Initialize button pins
  pinMode(buzzer, OUTPUT);
  pinMode(button1Pin, INPUT);
  pinMode(button2Pin, INPUT);
  pinMode(button3Pin, INPUT);
  
  // Initialize the LCD
  lcd.begin();
  lcd.backlight();
  displayWelcomeMessage();
  delay(2000);
  lcd.clear();

}

void loop() {
  vote_taken = 0;
  displayScanPrompt();
  displayCandidates();
  enableButtons();

  id = getFingerprintIDez();
  if (id >= 0) {
    displayVoterID(id);
  
    lcd.setCursor(0, 1);
    if (id == 6) {
      lcd.print("Admin Mode");
      delay(2000);
      lcd.clear();
      determineWinner();
      printResults();
      displayWinner(); 
      sendResultsToESP32(); // Send results to ESP32
      while (1);
    }
    if (!hasVoterVoted(id)) {
      processVote();
      thankVoter();
    } else {
      handleDuplicateVote();
    }
  }
}
void displayWelcomeMessage() {
 lcd.clear();
  lcd.print("Digital Electoral");
  lcd.setCursor(0, 1);
  lcd.print("System");
  delay(3000);
  lcd.clear();
}
void displayCandidates() {
  
  lcd.clear();
  lcd.print("`CANDIDATES");
  lcd.setCursor(0, 1);
  lcd.print("PTI:");
  lcd.print(PTI);
  lcd.setCursor(0, 2);
  lcd.print("PPP:");
  lcd.print(PPP);
  lcd.setCursor(0, 3);
  lcd.print("ANP:");
  lcd.print(ANP);
}
void displayScanPrompt() {
  lcd.clear(); 
  lcd.setCursor(0, 0);  
  lcd.print("Please place your"); 
  lcd.setCursor(0, 1);
  lcd.print("finger");
}
void displayVoterID(int id) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Voter ID: ");
  lcd.print(id);
  delay(2000);
  lcd.clear();
}
void displayVotePrompt() {
  lcd.clear();
  lcd.setCursor(0, 0);  
  lcd.print("Give Your vote"); 
  lcd.setCursor(0, 1);
  lcd.print("Press Button");
  delay(500);
}
void displayCandidateSelection() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Select Candidate");
  lcd.setCursor(0, 2);
  lcd.print(candidates[currentCandidateIndex]);
  delay(300);
}
bool confirmVote() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Confirm Vote:");
  lcd.setCursor(0, 1);
  lcd.print(candidates[currentCandidateIndex]);
  lcd.setCursor(0, 2);
  lcd.print("Press Button Again");

  // Wait for confirmation button press
  long startTime = millis();
  while (millis() - startTime < 5000) {  // 5-second window to confirm the vote
    if (digitalRead(button2Pin) == LOW) {
      delay(3000); // Debounce delay
      return true;
    }
  }
  return false;  // Time out, return false
}

void processVote() {
  currentCandidateIndex = 0; // Reset candidate selection
  bool voteRecorded = false;
  
  while (!voteRecorded) {
    displayCandidateSelection();

    voter_ids[voter_count++] = id;  //Detect Duplicate Votes...
    if (digitalRead(button1Pin) == LOW) {
      currentCandidateIndex = (currentCandidateIndex + numCandidates - 1) % numCandidates;
      delay(500); // Debounce delay
    }
    if (digitalRead(button3Pin) == LOW) {
      currentCandidateIndex = (currentCandidateIndex + 1) % numCandidates;
      delay(500); // Debounce delay
    }
    
    if (digitalRead(button2Pin) == LOW) {
      if (confirmVote()) {
        recordVote(candidates[currentCandidateIndex]);
        handleVote(currentCandidateIndex);
        voteRecorded = true;
        delay(2000);
      } else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Vote Canceled");
        delay(2000);
        // Go back to candidate selection
      }
    }
  }
}


/*void processVote() {
  bool voteRecorded = false;
  bool voteValid = false;

  while (!voteRecorded) {
    displayVotePrompt();
    voter_ids[voter_count++] = id;  //Detect Duplicate Votes...
    
    if (digitalRead(button1Pin) == LOW) {
      recordVote("PTI");
      handleVote(PTI, "PTI");
      voteValid = true;
      voteRecorded = true;
    }
    if (digitalRead(button2Pin) == LOW) {
      recordVote("PPP");
      voteRecorded = true;
      handleVote(PPP, "PPP");
      voteValid = true;
    }
    if (digitalRead(button3Pin) == LOW) {
      recordVote("ANP");
      voteRecorded = true;
      handleVote(ANP, "ANP");
      voteValid = true;
    }
    delay(100); // Debounce delay
  }
}*/

void handleVote(int candidateIndex) {
  if (candidateIndex == 0) {
    PTI++;
  } else if (candidateIndex == 1) {
    PPP++;
  } else if (candidateIndex == 2) {
    ANP++;
  }

  // Activate buzzer and LEDs to indicate successful vote
  activateBuzzerAndLED(1);

  // Display confirmation to the voter
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Vote for ");
  lcd.print(candidates[candidateIndex]);
  lcd.setCursor(0, 1);
  lcd.print("recorded.");
  delay(2000);
}
/*void handleVote(int& candidateVotes, const String& candidateName) {
  candidateVotes++; // Increment the vote count for the candidate
   
  // Activate buzzer and LEDs to indicate successful vote
  activateBuzzerAndLED(1);

  // Display confirmation to the voter
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Vote for");
  lcd.setCursor(0, 1);
  lcd.print(candidateName);
  //printer.println(candidate);
  lcd.setCursor(0, 2);
  lcd.print("recorded.");
  delay(2000);
}*/

void recordVote(String candidate) {
  // Display candidate name on LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("You voted for:");
  lcd.setCursor(0, 1);
  lcd.print(candidate);
  delay(2000);

  // Print candidate name using thermal printer
  printer.justify('R');
  printer.printBitmap(266, 282, bitmap_screenshot_gauges);
  printer.setSize('M');
  printer.println("You voted for:");
  printer.println(candidate);

  // Print QR code based on the candidate
  printer.justify('R');
  printer.printBitmap(adaqrcode_width, adaqrcode_height, adaqrcode_data);
  printer.println(F("Thanks for voting..."));

  printer.feed(2);
}
/*void recordVote(String candidate) {
  // Display candidate name on LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("You voted for:");
  lcd.setCursor(0, 1);
  lcd.print(candidate);
  delay(2000);

  // Print candidate name using thermal printer
  printer.printBitmap(266, 282, bitmap_screenshot_gauges);
  printer.setSize('M');
  printer.println("You voted for:");
  printer.println(candidate);

// Print QR code based on the candidate
  if (candidate == "PTI") {
    printer.printBitmap(adaqrcode_width, adaqrcode_height, adaqrcode_data);
    printer.println(F("Thanks for voting..."));
  } else if (candidate == "PPP") {
    printer.printBitmap(adaqrcode_width, adaqrcode_height, adaqrcode_data);
    printer.println(F("Thanks for voting..."));
  } else if (candidate == "ANP") {
    printer.printBitmap(adaqrcode_width, adaqrcode_height, adaqrcode_data);
    printer.println(F("Thanks for voting..."));
  }

  // Print barcode based on the candidate
  /*if (candidate == "PTI") {
    printer.printBarcode("PTI123", CODE39); // Adjust the barcode type and data as needed
  } else if (candidate == "PPP") {
    printer.printBarcode("PPP456", CODE39); // Adjust the barcode type and data as needed
  } else if (candidate == "ANP") {
    printer.printBarcode("ANP789", CODE39); // Adjust the barcode type and data as needed
  }
  printer.feed(2);
}*/
void thankVoter() {
  lcd.clear();
  lcd.setCursor(0, 0);  
  lcd.print("Thanks for your"); 
  lcd.setCursor(0, 1);
  lcd.print("vote");
  delay(2000);
  activateBuzzerAndLED(1);
}

void printResults() {
  // Calculate the total number of votes cast
   totalVotes = PTI + PPP + ANP;
  
  // Calculate the percentage of votes for the winner
  float winnerPercentage = 0.0;
  if (totalVotes > 0) {
    if (winner_name == "PTI") {
      winnerPercentage = (PTI / (float)totalVotes) * 100;
    } else if (winner_name == "PPP") {
      winnerPercentage = (PPP / (float)totalVotes) * 100;
    } else if (winner_name == "ANP") {
      winnerPercentage = (ANP / (float)totalVotes) * 100;
    }
  }

  lcd.clear();
  Serial.println("Election Results");
  // First Row: Display "Printing: 'Form 45'"
  lcd.setCursor(0, 0);
  lcd.print("Printing: 'Form 45'");

  // Second Row: Display Total Votes
  lcd.setCursor(0, 1);
  lcd.print("Total Votes: ");
  lcd.print(totalVotes);
  // Third Row: Display PTI on the left and PPP on the right
  lcd.setCursor(0, 2);
  lcd.print("PTI:");
  Serial.println("PTI:");
  lcd.print(PTI);
  Serial.println(PTI);
  
  lcd.setCursor(10, 2); // Adjust this value if your screen width differs
  lcd.print("PPP:");
  Serial.println("PPP:");
  lcd.print(PPP);
  Serial.println(PPP);
  
  // Fourth Row: Display ANP
  lcd.setCursor(0, 3);
  lcd.print("ANP:");
  Serial.println("ANP:");
  lcd.print(ANP);
  Serial.println(ANP);


  Serial.println("TotalVotes:");
  Serial.println(totalVotes);
  Serial.println("Winner: " + winner_name + " \n (" + (winnerPercentage) + "% of total votes)");
  printer.boldOn();
  printer.printBitmap(266, 282, bitmap_screenshot_gauges);

  printer.justify('R');
  printer.println(F("Election Results"));
  printer.boldOff();
  printer.justify('R');
  printer.println("Total Votes: " + String(totalVotes));
  printer.justify('R');
  printer.println("PTI: " + String(PTI) + " (" + String((PTI / (float)totalVotes) * 100) + "%)");
  printer.justify('R');
  printer.println("PPP: " + String(PPP) + " (" + String((PPP / (float)totalVotes) * 100) + "%)");
  printer.justify('R');
  printer.println("ANP: " + String(ANP) + " (" + String((ANP / (float)totalVotes) * 100) + "%)");
  printer.justify('R');
  printer.printBitmap(adaqrcode_width, adaqrcode_height, adaqrcode_data);
  printer.justify('R');
  printer.setSize('M');
  printer.println("Winner: " + winner_name + " \n (" + (winnerPercentage) + "% of total votes)");

  printer.feed(3);
  activateBuzzerAndLED(1);
  delay(3000);
}

void sendResultsToESP32() {
  // Format the data as a JSON string
  String data = "{\"totalVotes\":" + String(totalVotes) + 
                 ",\"PTI\":" + String(PTI) + 
                 ",\"PPP\":" + String(PPP) + 
                 ",\"ANP\":" + String(ANP) + 
                 ",\"winner\":\"" + winner_name + "\"}\n";
  
  // Print JSON data to Serial Monitor for verification
  Serial.print("Sending JSON data: ");
  Serial.println(data);

  // Send the JSON string to the ESP32
  Serial.println(data);
}

void determineWinner() {
  if (PTI > PPP && PTI > ANP) {
    winner_name = "PTI";
  } else if (PPP > PTI && PPP > ANP) {
    winner_name = "PPP";
  } else if (ANP > PTI && ANP > PPP) {
    winner_name = "ANP";
  } else {
    winner_name = "Tied";
  }
}

void displayWinner() {
  lcd.clear();
  lcd.setCursor(0, 0);
  printer.setSize('M');  
  lcd.print("Winner Party");
  //printer.println(F("Winner Party")); 
  lcd.setCursor(0, 1);
  lcd.print(winner_name);
  //printer.println(winner_name);
  //printer.feed(2);
  delay(500);
}

void enableButtons() {
  pinMode(button1Pin, INPUT);
  pinMode(button2Pin, INPUT);
  pinMode(button3Pin, INPUT);
}

void disableButtons() {
  pinMode(button1Pin, OUTPUT);
  pinMode(button2Pin, OUTPUT);
  pinMode(button3Pin, OUTPUT);
}

void handleDuplicateVote() {
  lcd.clear();
  lcd.setCursor(0, 0);  
  lcd.print("Duplicate Vote"); 
  lcd.setCursor(0, 1);
  lcd.print("Buttons disabled");
  delay(2000);
  
  // Disable voting buttons
  digitalWrite(button1Pin, LOW);
  digitalWrite(button2Pin, LOW);
  digitalWrite(button3Pin, LOW);
  
  activateBuzzerAndLED(3);
}

void activateBuzzerAndLED(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(buzzer, HIGH);
    digitalWrite(R_led, HIGH);
    digitalWrite(G_led, LOW);
    delay(1000);
    digitalWrite(buzzer, LOW);
    digitalWrite(R_led, LOW);
    digitalWrite(G_led, HIGH);
    delay(1000);
  }
}

int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK) return -1;

  Serial.print("Found ID #"); Serial.print(finger.fingerID); 
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  return finger.fingerID; 
}

bool hasVoterVoted(int voter_id) {
  for (int i = 0; i < voter_count; i++) {
    if (voter_ids[i] == voter_id) {
      return true;
    }
  }
  return false;
}
