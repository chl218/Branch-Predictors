//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include "predictor.h"

//
// TODO:Student Information
//
const char *studentName = "Chao Li";
const char *studentID   = "A97421703";
const char *email       = "chl218@eng.ucsd.edu";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
                           "Tournament", "Custom" };

int ghistoryBits; // Number of bits used for Global History
int lhistoryBits; // Number of bits used for Local History
int pcIndexBits;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type
int verbose;

//------------------------------------------------------------------------------
// Predictor Global Variables
//------------------------------------------------------------------------------

uint8_t  *ghr;    // global history register
uint8_t **lht;    // local history table

uint8_t *lbht;    // local branch history table
uint8_t *gbht;    // global branch history table
uint8_t *cbht;    // chooser branch history table

uint32_t lhtDepth;

uint32_t lbhtDepth;
uint32_t gbhtDepth;
uint32_t cbhtDepth;

uint8_t printDepth;

uint8_t  currPrediction;
uint8_t  currPHTChoice;
uint32_t currChoiceBHTIndex;
uint32_t currTakenBHTIndex;
uint32_t currNottakenBHTIndex;

uint8_t **cpht;
uint8_t **tpht;
uint8_t **npht;

uint32_t cphtDepth;
uint32_t tphtDepth;
uint32_t nphtDepth;

//uint8_t *cbht;
uint8_t *tbht;
uint8_t *nbht;

uint32_t cbhtDepth;
uint32_t tbhtDepth;
uint32_t nbhtDepth;

uint8_t bimode_ghr_bits = 12;
uint8_t bimode_lhr_bits = 10;

uint8_t LBHT_MAX;
uint8_t GBHT_MAX;
uint8_t CBHT_MAX;

uint8_t CTHRESHOLD;
uint8_t LTHRESHOLD;
uint8_t GTHRESHOLD;
//------------------------------------------------------------------------------
// Helper Functions
//------------------------------------------------------------------------------

uint32_t
getGHR() {
   uint32_t val = 0;
   for(int i = 0; i < ghistoryBits; i++) {
      val |= ghr[i] << i;
   }
   return val;
}

uint32_t
getPC(uint32_t pc) {
   uint32_t mask = 0;
   for(int i = 0; i < pcIndexBits; i++) {
      mask |= 1 << i;
   }
   return pc & mask;
}

uint32_t
getMaskedValue(uint32_t val, uint32_t bits) {
   uint32_t mask = 0;
   for(int i = 0; i < bits; i++) {
      mask |= 1 << i;
   }
   return val & mask;
}

uint8_t
predict(uint8_t prediction, uint8_t threshold) {
   return prediction >= threshold ? TAKEN : NOTTAKEN;
}

void
print_ghr() {
   PRETTY_PRINT1(printDepth, "ghr: ", 0);
   for(int i = ghistoryBits-1; i >= 0; i--) {
      fprintf(stderr, "%d", ghr[i]);
   }
   fprintf(stderr, "\n");
}

void
print_lht(uint32_t pc, uint8_t outcome) {
   uint32_t index = getPC(pc);
   PRETTY_PRINT1(printDepth, "hlt: ", 1);
   printDepth++;
   PRETTY_PRINT2(printDepth, "update value: ", outcome, 1);
   PRETTY_PRINT2(printDepth, "index:        ", index, 1);
   PRETTY_PRINT1(printDepth, "lht value:    ", 0);
   printDepth--;
   for(int i = lhistoryBits-1; i >= 0; i--) {
       fprintf(stderr, "%d", lht[index][i]);
   }
   fprintf(stderr, "\n");
}

void
print_bht(char *s, uint8_t **bht, uint32_t index) {
   PRETTY_PRINT1(printDepth, s, 1);
   printDepth++;
   PRETTY_PRINT2(printDepth, "index: ", index, 1);
   PRETTY_PRINT2(printDepth, "value: ", (*bht)[index], 1);
   printDepth--;
}


//------------------------------------------------------------------------------
// Initialize Predictor
//------------------------------------------------------------------------------
void
init_depths(){
   switch(bpType) {
      case STATIC:
         break;
      case GSHARE:
         gbhtDepth   = 1 << ghistoryBits;
         pcIndexBits = ghistoryBits;
         break;
      case CUSTOM:
      case TOURNAMENT:
         lhtDepth  = 1 << pcIndexBits;
         lbhtDepth = 1 << lhistoryBits;
         gbhtDepth = 1 << ghistoryBits;
         cbhtDepth = 1 << ghistoryBits;
         break; 
      // case CUSTOM:
      //    cphtDepth = 1 << ghistoryBits;
      //    tphtDepth = 1 << ghistoryBits;
      //    nphtDepth = 1 << ghistoryBits;

      //    cbhtDepth = 1 << lhistoryBits;
      //    tbhtDepth = 1 << lhistoryBits;
      //    nbhtDepth = 1 << lhistoryBits;

      //    break;
      default:
         break;
   }
}

void
init_ghr() { 
   ghr = (uint8_t *) malloc(sizeof(uint8_t) * ghistoryBits);
   if(ghr == NULL) {
      fprintf(stderr, "ERROR: ghr malloc failed\n");
      exit(1);
   }
   memset(ghr, NOTTAKEN, sizeof(uint8_t) * ghistoryBits);

}

void
init_bht(uint8_t** bht, uint32_t depth, uint8_t initVal) {
   *bht = (uint8_t *) malloc(sizeof(uint8_t) * depth);
   if(bht == NULL) {
      fprintf(stderr, "ERROR: bht malloc failed\n");
      exit(1);
   }
   memset(*bht, initVal, sizeof(uint8_t) * depth);
}


void 
init_lht() {
   
   lht = (uint8_t **) malloc(sizeof(uint8_t *) * lhtDepth);
   if(lht == NULL) {
      fprintf(stderr, "ERROR: lht malloc failed\n");
      exit(1);
   }

   for(int i = 0; i < lhtDepth; i++) {
      lht[i] = (uint8_t *) malloc(sizeof(uint8_t) * lhistoryBits);
      if(lht == NULL) {
         fprintf(stderr, "ERROR: lht malloc failed\n");
         exit(1);
      }    
   }

   for(int i = 0; i < lhtDepth; i++) {
      for(int j = 0; j < lhistoryBits; j++) {
         lht[i][j] = NOTTAKEN;
      }
   }
}


void
init_pht() {

   cpht = (uint8_t **) malloc(sizeof(uint8_t *) * cphtDepth);
   tpht = (uint8_t **) malloc(sizeof(uint8_t *) * tphtDepth);
   npht = (uint8_t **) malloc(sizeof(uint8_t *) * nphtDepth);
   if(tpht == NULL || npht == NULL) {
      fprintf(stderr, "ERROR: cpht/tpht/npht malloc failed\n");
      exit(1);
   }

   for(int i = 0; i < tphtDepth; i++) {
      cpht[i] = (uint8_t *) malloc(sizeof(uint8_t) * lhistoryBits);
      tpht[i] = (uint8_t *) malloc(sizeof(uint8_t) * lhistoryBits);
      npht[i] = (uint8_t *) malloc(sizeof(uint8_t) * lhistoryBits);
      if(tpht == NULL || npht == NULL) {
         fprintf(stderr, "ERROR: cpht/tpht/npht malloc failed\n");
         exit(1);
      }    
   }

   for(int i = 0; i < tphtDepth; i++) {
      for(int j = 0; j < lhistoryBits; j++) {
         cpht[i][j] = NOTTAKEN;
         tpht[i][j] = NOTTAKEN;
         npht[i][j] = NOTTAKEN;
      }
   }
}

void
init_predictor() {

   printDepth = 0;
   
   CBHT_MAX = ST;
   LBHT_MAX = ST;
   GBHT_MAX = ST;

   CTHRESHOLD = WT;
   LTHRESHOLD = WT;
   GTHRESHOLD = WT;

   init_depths();  
   switch(bpType) {
      case GSHARE:
         init_ghr();
         init_bht(&gbht, gbhtDepth, WN);
         break;
      case TOURNAMENT:
         init_ghr();
         init_bht(&gbht, gbhtDepth, WN);
         init_lht();
         init_bht(&lbht, lbhtDepth, WN);
         init_bht(&cbht, cbhtDepth, WN);
         break;
      // case CUSTOM:   // Bi-Mode
         // ghistoryBits = pcIndexBits = bimode_ghr_bits;
         // lhistoryBits = bimode_lhr_bits;
         // init_depths();
         // init_ghr();
         // init_pht();
         // init_bht(&cbht, cbhtDepth);
         // init_bht(&tbht, tbhtDepth);
         // init_bht(&nbht, nbhtDepth);  
      case CUSTOM:
         ghistoryBits = 13;
         lhistoryBits = 11;
         pcIndexBits  = 11;
      
         LBHT_MAX   = _ST;
         LTHRESHOLD = _WT;

         init_depths();  
         init_ghr();
         init_bht(&gbht, gbhtDepth, WN);
         init_lht();
         init_bht(&lbht, lbhtDepth, _WN);
         init_bht(&cbht, cbhtDepth, WN);
         break;
      default:
         break;
   }

   if(DEBUG) {
      fprintf(stderr, "[ ");
      fprintf(stderr, "bpType:%s|",       bpName[bpType]);
      
      fprintf(stderr, "pcIndexBits:%u|", pcIndexBits);

      fprintf(stderr, "ghistoryBits:%u|", ghistoryBits);
      fprintf(stderr, "gbhtDepth:%u|",    gbhtDepth);
      fprintf(stderr, "cbhtDepth:%u|",    cbhtDepth);

      fprintf(stderr, "lhistoryBits:%u|", lhistoryBits);
      fprintf(stderr, "lhtDepth:%u|",     lhtDepth);
      fprintf(stderr, "lbhtDepth:%u ]\n", lbhtDepth);
   }

   if(DEBUG_CUSTOM) {
      fprintf(stderr, "[ ");
      fprintf(stderr, "bpType:%s|",       bpName[bpType]);
      
      fprintf(stderr, "pcIndexBits:%u|",  pcIndexBits);
      fprintf(stderr, "ghistoryBits:%u|", ghistoryBits);
      fprintf(stderr, "lhistoryBits:%u|", lhistoryBits);

      fprintf(stderr, "cphtDepth:%u|", cphtDepth);
      fprintf(stderr, "tphtDepth:%u|", tphtDepth);
      fprintf(stderr, "nphtDepth:%u|", nphtDepth);

      fprintf(stderr, "cbhtDepth:%u|",  cbhtDepth);
      fprintf(stderr, "tbhtDepth:%u|",  tbhtDepth);
      fprintf(stderr, "nbhtDepth:%u ]\n", nbhtDepth);
   }

}


//------------------------------------------------------------------------------
// Make Prediction
//------------------------------------------------------------------------------

//------------------------------------//
//     GShare                         //
//------------------------------------//
uint32_t
gshare_xor(uint32_t pc) {
   return getGHR() ^ getPC(pc);
}

//------------------------------------//
//     Tournament                     //
//------------------------------------//
uint32_t
getLHT(uint32_t index) {
   uint32_t val = 0;
   for(int i = 0; i < lhistoryBits; i++) {

      val |= lht[index][i] << i;
   }
   return val;
}

uint8_t 
chooser(uint8_t threshold) {
   return predict(cbht[getGHR()], threshold);
}

uint8_t
global_predictor(uint8_t threshold) {
   return predict(gbht[getGHR()], threshold);
}

uint8_t
local_predictor(uint32_t pc, uint8_t threshold) {
   return predict(lbht[getLHT(getPC(pc))], threshold);
}


// //------------------------------------//
// //     Bi-Mode                        //
// //------------------------------------//
// uint8_t
// pht_choice(uint32_t pc) {
//    uint32_t cphtIndex  = getPC(pc);
   
//    uint32_t bhtIndex = 0;
//    for(int i = 0; i < lhistoryBits; i++) {
//       bhtIndex |= cpht[cphtIndex][i] << i;
//    }

//    if(DEBUG_CUSTOM) {
//       PRETTY_PRINT1(printDepth, "pht_choice", 1);
//       printDepth++;
//       PRETTY_PRINT2(printDepth, "cphtIndex: ", cphtIndex, 1);
//       PRETTY_PRINT2(printDepth, "bhtIndex: ", bhtIndex, 1);
//       PRETTY_PRINT2(printDepth, "cbht: ", cbht[bhtIndex], 1);
//       printDepth--;
//    }

//    currChoiceBHTIndex = bhtIndex;
//    return predict(cbht[bhtIndex], WT);
// }

// uint8_t
// pht_taken(uint32_t pc) {
//    printDepth++;
//    uint32_t tphtIndex  = gshare_xor(pc);
//    printDepth--;

//    uint32_t bhtIndex = 0;
//    for(int i = 0; i < lhistoryBits; i++) {
//       bhtIndex |= tpht[tphtIndex][i] << i;
//    }

//    if(DEBUG_CUSTOM) {
//       PRETTY_PRINT1(printDepth, "pht_taken", 1);
//       printDepth++;
//       PRETTY_PRINT2(printDepth, "tphtIndex: ", tphtIndex, 1);
//       PRETTY_PRINT2(printDepth, "bhtIndex: ", bhtIndex, 1);
//       PRETTY_PRINT2(printDepth, "tbht: ", tbht[bhtIndex], 1);
//       printDepth--;
//    }

//    currTakenBHTIndex = bhtIndex;
//    return predict(tbht[bhtIndex], WT);
// }

// uint8_t
// pht_nottaken(uint32_t pc) {
//    printDepth++;
//    uint32_t nphtIndex  = gshare_xor(pc);
//    printDepth--;

//    uint32_t bhtIndex = 0;
//    for(int i = 0; i < lhistoryBits; i++) {
//       bhtIndex |= npht[nphtIndex][i] << i;
//    }

//    if(DEBUG_CUSTOM) {
//       PRETTY_PRINT1(printDepth, "pht_nottaken", 1);
//       printDepth++;
//       PRETTY_PRINT2(printDepth, "tphtIndex: ", nphtIndex, 1);
//       PRETTY_PRINT2(printDepth, "bhtIndex: ", bhtIndex, 1);
//       PRETTY_PRINT2(printDepth, "tbht: ", nbht[bhtIndex], 1);
//       printDepth--;
//    }

//    currNottakenBHTIndex = bhtIndex;
//    return predict(nbht[bhtIndex], WT);
// }


// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t
make_prediction(uint32_t pc) {

   // Make a prediction based on the bpType
   switch (bpType) {
      case STATIC:     return TAKEN;
      case GSHARE:     return predict(gbht[gshare_xor(pc)], GTHRESHOLD);
      case CUSTOM:
      case TOURNAMENT: return chooser(CTHRESHOLD) ? local_predictor(pc, LTHRESHOLD)
                                                  : global_predictor(GTHRESHOLD);

      // case CUSTOM: { // Bi-Mode
      //    if(DEBUG_CUSTOM) {
      //       fprintf(stderr, "make_prediction:\n");
      //       fprintf(stderr, "|-- pc: 0x%x\n", pc);
      //       print_ghr();
      //    }
         
      //    uint8_t choose_pht   = pht_choice(pc);
      //    uint8_t taken_pht    = pht_taken(pc);
      //    uint8_t nottaken_pht = pht_nottaken(pc);

      //    currPHTChoice  = choose_pht ? TAKEN : NOTTAKEN;
      //    currPrediction = choose_pht ? taken_pht : nottaken_pht;
      //    return currPrediction;
      // } 
      default:          break;
   }

   
   // If there is not a compatable bpType then return NOTTAKEN
   return NOTTAKEN;
}



//------------------------------------------------------------------------------
// Train Predictor
//------------------------------------------------------------------------------

//------------------------------------//
//     GShare                         //
//------------------------------------//
void
update_ghr(uint8_t outcome) {

   for(int i = ghistoryBits-1; i > 0; i--) {
      ghr[i] = ghr[i-1];
   }
   ghr[0] = outcome;
}

void
update_bht(uint8_t** bht, uint32_t index, uint8_t outcome, uint8_t bht_max) {
   uint8_t predicted = (*bht)[index];
   if(outcome == NOTTAKEN) {
      (*bht)[index] = predicted == SN ? SN : --predicted;
   }
   else if(outcome == TAKEN) {
      (*bht)[index] = predicted == bht_max ? bht_max : ++predicted;
   }
}

//------------------------------------//
//     Tournament                     //
//------------------------------------//

void
update_cbht(uint32_t pc, uint8_t outcome, uint8_t BHT_MAX) {

   uint8_t lpredicted = lbht[getLHT(getPC(pc))];

   uint32_t index     = getGHR();
   uint8_t gpredicted = gbht[index];
   uint8_t cpredicted = cbht[index];

   uint8_t goutcome = predict(gpredicted, GTHRESHOLD);
   uint8_t loutcome = predict(lpredicted, LTHRESHOLD);

   // WT - 2 - global predictor     ST - 3 - global predictor
   if(loutcome == outcome && goutcome != outcome) {
      cbht[index] = cpredicted == BHT_MAX ? BHT_MAX : ++cpredicted;
   }
   // SN - 0 - local predictor      WN - 1 - local predictor
   else if(loutcome != outcome && goutcome == outcome) {
      cbht[index] = cpredicted == SN ? SN : --cpredicted; 
   }
}

void
update_lht(uint32_t pc, uint8_t outcome) {
   uint32_t index = getPC(pc);
   for(int i = lhistoryBits-1; i > 0; i--) {
      lht[index][i] = lht[index][i-1];
   }
   lht[index][0] = outcome;
}

// //------------------------------------//
// //     Bi-Mode                        //
// //------------------------------------//
// void
// update_choosen(uint32_t pc, uint8_t outcome) {

//    uint32_t phtIndex = gshare_xor(pc);

//    if(currPHTChoice) {
//       update_bht(&tbht, currTakenBHTIndex, outcome, 3);
//       for(int i = lhistoryBits-1; i > 0; i--) {
//          tpht[phtIndex][i] = tpht[phtIndex][i-1];
//       }
//       tpht[phtIndex][0] = outcome;
//    }
//    else {
//       update_bht(&nbht, currNottakenBHTIndex, outcome, 3);
//       for(int i = lhistoryBits-1; i > 0; i--) {
//          npht[phtIndex][i] = npht[phtIndex][i-1];
//       }
//       npht[phtIndex][0] = outcome;
//    }
// }

// void
// update_chooser(uint32_t pc, uint8_t outcome) {

//    if(currPrediction == outcome || currPHTChoice == outcome) {
//       uint32_t phtIndex = getPC(pc);

//       update_bht(&cbht, currChoiceBHTIndex, outcome, 3);

//       for(int i = lhistoryBits-1; i > 0; i--) {
//          cpht[phtIndex][i] = cpht[phtIndex][i-1];
//       }

//       cpht[phtIndex][0] = outcome;
//    }
// }


// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//
void
train_predictor(uint32_t pc, uint8_t outcome) {

   switch (bpType) {
      case STATIC:
         break;
      case GSHARE:
         update_bht(&gbht, gshare_xor(pc), outcome, GBHT_MAX);
         update_ghr(outcome);
         break;
      case CUSTOM:
      case TOURNAMENT:
         update_cbht(pc, outcome, CBHT_MAX);
         update_bht(&lbht, getLHT(getPC(pc)), outcome, LBHT_MAX);
         update_bht(&gbht, getGHR(), outcome, GBHT_MAX);
         update_ghr(outcome);
         update_lht(pc, outcome);
         break;

      // case CUSTOM:   // Bi-Mode
      //    update_choosen(pc, outcome);
      //    update_chooser(pc, outcome);
      //    update_ghr(outcome);
      //    if(DEBUG_CUSTOM) {
      //       fprintf(stderr, "train_predictor:\n");
      //       PRETTY_PRINT2(printDepth, "predictor outcome: ", outcome, 1);
      //       print_ghr();
      //       print_bht("chbt", &cbht, currChoiceBHTIndex);
      //       print_bht("thbt", &tbht, currTakenBHTIndex);
      //       print_bht("nhbt", &nbht, currNottakenBHTIndex);
      //    }
      //    break;

      default: 
         break;
   }
}

