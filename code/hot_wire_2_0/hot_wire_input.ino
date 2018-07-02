// Game selection
#define SELECT_1_PIND 4
#define SELECT_2_PIND 5
#define SELECT_4_PIND 6


// start button detection
#define bnStart_pinD 9
bool bnStart_alreadyPressed=false;
bool bnStart_hasPressEvent=false;
unsigned long bnStart_debounceComplete=0;

// wiresensor detection
#define wire_pinA 0
bool wire_hasContact = false;  //Debounced flag about the wire conact
unsigned long wire_debounceComplete = 0;        

// area sensor detection
#define areaA_pinA 1
bool areaA_hasContact = false;
unsigned long areaA_debounceComplete = 0;        

// wiresensor detection
#define areaB_pinA 2
bool areaB_hasContact = false;
unsigned long areaB_debounceComplete = 0;        

