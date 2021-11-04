// $Id: State.h,v 1.1 2007/12/16 21:12:20 davegutz Exp davegutz $
#ifndef State_h
#define State_h 1
#include <string>    // string
#include <zap.h>

#define VERBOSE      // VERBOSE option adds no time penalty to Blink

extern "C" {
 #include <gnome.h>                  // gnome
 #include <glade/glade.h>            // gnome
 #include <unistd.h>                 // gnome
 #include <libgnome/libgnome.h>      // gnome
 #include <libgnomevfs/gnome-vfs.h>  // gnome
}

/////////////////  State Class  ////////////////////////////////////////////////
typedef int (*function_t)(void); 
class State
{
 protected:
  string      name_;          // name of the state
  int         numDepends_;    // number of input depends
  int         numDependsNot_; // number of input depends to be notted
  const int **depend_;        // value of depends
  const int **dependNot_;     // value of depends to be notted
  string    **dependName_;    // name of depends
  string    **dependNameNot_;    // name of depends to be notted
  string    **dependNameFunc_;    // name of function depends
  string    **dependNameFuncNot_; // name of function depends to be notted
  string    **callbackSignal_;// callback array
  GtkWidget **callback_;      // callback array
  gulong     *callbackHandle_;// callback handle array
  string    **callbackDisplayName_;  // name of depends
  int         numCallbacks_;  // number of callbackSignal_, callback_ pairs
  int         complete_;      // status of the state
  int         completep_;     // past status of the state
  int         total_;         // numerical value of depends, for relative change
  int         emitReady_;     // ready to emit, but haven't emitted
  function_t *dependFunc_;    // function pointer depends
  function_t *dependFuncNot_; // function pointer depends to be notted
  int         numDependsFunc_;    // number input function depends
  int         numDependsFuncNot_; // number input function depends to be notted
 public:
  State();
  State(const string name);
  State(const State & S);
  State & operator=(const State & S);
  virtual ~State()
    {
      zaparr(callbackHandle_);
      for ( int i=0; i<numDepends_; i++ ) zap(dependName_[i]);
      zaparr(dependName_);
      for ( int i=0; i<numDependsNot_; i++ ) zap(dependNameNot_[i]);
      zaparr(dependNameNot_);
      zaparr(depend_);
      zaparr(dependNot_);
      for ( int i=0; i<numDependsFunc_; i++ ) zap(dependNameFunc_[i]);
      zaparr(dependNameFunc_);
      for ( int i=0; i<numDependsFuncNot_; i++ ) zap(dependNameFuncNot_[i]);
      zaparr(dependNameFuncNot_);
      zaparr(dependFunc_);
      zaparr(dependFuncNot_);
      for ( int i=0; i<numCallbacks_; i++ ) zap(callbackSignal_[i]);
      zaparr(callbackSignal_);
      zaparr(callback_);
      for ( int i=0; i<numCallbacks_; i++ ) zap(callbackDisplayName_[i]);
      zaparr(callbackDisplayName_);
    }
  State & operator= (const State S)
  {
    if ( this==&S ) return *this;  // Allows self-assignment.
    return *this;                  // Allows stacked assignments.
  }

  // Element access
  friend ostream & operator<< (ostream & out, const State & S);
  int   *completePtr()         {return &complete_;};
  int    complete()            {return complete_;};
  int    numDepends()          {return numDepends_;};
  int    numDependsNot()       {return numDependsNot_;};
  int    total()               {return total_;};
  int    emitReady()           {return emitReady_;};
  string name()                {return name_;};
  
  // Functions
  int   addCallback(GtkWidget *callback, const string callbackSignal,
		   gulong hand, const string displayName);
                              // add an initial callback, return number
  int   addDepend(const string name, int *depend);
  int   addDependNot(const string name, int *dependNot);
  int   addDepend(const string name, function_t func);
  int   addDependNot(const string name, function_t funcNot);
  void  blockAll();           // block all callbacks
  void  emitAll();            // emit all signals
  void  totalize();           // refresh totals
  void  unBlockAll();         // unblock all callbacks
  int   update();             // totalize, return total, emit signals
  int   updateWithBlocking(); // Update with blocking. Return % complete.
};


////////////////// class StateMachine ///////////////////////
class StateMachine
{
 protected:
  int        totalp_;         // past state status for depends fulfilled
  int        totalDepends_;   // total value of depends met
  int        totalDependsNot_;// total value of dependsNot met
  int        numStates_;      // number states
  string   **name_;           // array of state names.
  State    **state_;          // states array
  int        blockUpdate_;    // bypass state update when set
  int        complete_;       // if all states are complete
 public:
  StateMachine();
  StateMachine(const StateMachine & S);
  StateMachine & operator=(const StateMachine & S);
  virtual ~StateMachine()
    {
      {
	for( int i=0; i<numStates_; i++ )
	  {
	    zap(state_[i]);
	    zap(name_[i]);
	  }
	zaparr(state_);
	zaparr(name_);
      }
    }
  StateMachine & operator= (const StateMachine SM)
    {
      if ( this==&SM ) return *this;  // Allows self-assignment.
      return *this;                   // Allows stacked assignments.
    }

  // Element access
  friend ostream & operator<< (ostream & out, const StateMachine & SM);
  int    numStates() {return numStates_;};
  int   *status(const string query);
  State *state(const int stateNum)       {return state_[stateNum];};
  int    complete(const string query);
  int    complete()                      {return complete_;};

  // Functions
  void  addCallback(const int index, GtkWidget *callback,
		    const string callbackSignal, gulong hand,
		    const string displayName);
  void  addDepend(const int index, const string name, int *depend);
  void  addDependNot(const int index, const string name, int *dependNot);
  void  addDepend(const int index, const string name, function_t func);
  void  addDependNot(const int index, const string name, function_t funcNot);
  int   addState(const string name); // Add a new state.
  float update();                    // Update all states & emit.
                                     //  Return % complete.
  void  update(const string query);  // Update query state.
  float updateWithBlocking();        // Update all states. Return % complete.
  void  updateWithBlocking(const string query);  // Update query state.
  void  blockAll();                  // block all callbacks
  void  blockUpdate();               // block state updates
  void  unBlockAll();                // unblock all callbacks
  void  unBlockUpdate();             // unblock state updates
  float updater(const int withBlocking); // update utitlity driver
};

  
#endif // State_h
