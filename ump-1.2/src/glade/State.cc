// 	$Id: State.cc,v 1.1 2007/12/16 21:30:37 davegutz Exp davegutz $	
// State machine used to drive completion of Blink setup

// Includes section
using namespace std;
#include <common.h>
#include <string>      // string classes
#include <string.h>    // strings
#include <iostream>    // cerr etc
#include <iomanip>     // setprecision etc
#include "State.h"
extern int verbose;

/////////////////  State Class  ////////////////////////////////////////////////
State::State() : name_(""), numDepends_(0), numDependsNot_(0), depend_(NULL),
		 dependNot_(NULL), dependName_(NULL), dependNameNot_(NULL),
		 dependNameFunc_(NULL), dependNameFuncNot_(NULL),
		 callbackSignal_(NULL), callback_(NULL), callbackHandle_(NULL),
		 callbackDisplayName_(NULL), numCallbacks_(0),
		 complete_(0), completep_(0), total_(0), emitReady_(0),
		 dependFunc_(NULL), dependFuncNot_(NULL),
		 numDependsFunc_(0), numDependsFuncNot_(0)
{
  depend_               = new const int*[50];
  dependNot_            = new const int*[50];
  dependName_           = new string*[50];
  dependNameNot_        = new string*[50];
  dependNameFunc_       = new string*[50];
  dependNameFuncNot_    = new string*[50];
  callbackSignal_       = new string*[50];
  callback_             = new GtkWidget*[50];
  callbackHandle_       = new gulong[50];
  callbackDisplayName_  = new string*[50];
  dependFunc_           = new function_t[50];
  dependFuncNot_        = new function_t[50];
}
State::State(const string name)
  : name_(name), numDepends_(0), numDependsNot_(0), depend_(NULL),
    dependNot_(NULL), dependName_(NULL), dependNameNot_(NULL),
    dependNameFunc_(NULL), dependNameFuncNot_(NULL),
    callbackSignal_(NULL), callback_(NULL), callbackHandle_(NULL),
    callbackDisplayName_(NULL), numCallbacks_(0), complete_(0),
    completep_(0), total_(0), emitReady_(0), dependFunc_(NULL),
    dependFuncNot_(NULL), numDependsFunc_(0), numDependsFuncNot_(0)
{
  depend_               = new const int*[50];
  dependNot_            = new const int*[50];
  dependName_           = new string*[50];
  dependNameNot_        = new string*[50];
  dependNameFunc_       = new string*[50];
  dependNameFuncNot_    = new string*[50];
  callbackSignal_       = new string*[50];
  callback_             = new GtkWidget*[50];
  callbackHandle_       = new gulong[50];
  callbackDisplayName_  = new string*[50];
  dependFunc_           = new function_t[50];
  dependFuncNot_        = new function_t[50];
}
State::State(const State & S)
 : name_(S.name_), numDepends_(S.numDepends_),
   numDependsNot_(S.numDependsNot_), depend_(NULL),
   dependNot_(NULL), dependName_(NULL), dependNameNot_(NULL),
   dependNameFunc_(NULL), dependNameFuncNot_(NULL),
   callbackSignal_(NULL), callback_(NULL), callbackHandle_(NULL),
   callbackDisplayName_(NULL), numCallbacks_(S.numCallbacks_),
   complete_(S.complete_), completep_(S.completep_),
   total_(S.total_), emitReady_(S.emitReady_),
   dependFunc_(NULL), dependFuncNot_(NULL),
   numDependsFunc_(S.numDependsFunc_),
   numDependsFuncNot_(S.numDependsFuncNot_)
{
  depend_               = new const int*[50];
  dependNot_            = new const int*[50];
  dependName_           = new string*[50];
  dependNameNot_        = new string*[50];
  dependNameFunc_       = new string*[50];
  dependNameFuncNot_    = new string*[50];
  callbackSignal_       = new string*[50];
  callback_             = new GtkWidget*[50];
  callbackHandle_       = new gulong[50];
  callbackDisplayName_  = new string*[50];
  dependFunc_           = new function_t[50];
  dependFuncNot_        = new function_t[50];

  for ( int i=0; i<S.numDepends_; i++    )
    depend_[i] = S.depend_[i];

  for ( int i=0; i<S.numDependsNot_; i++ )
    dependNot_[i] = S.dependNot_[i];

  for ( int i=0; i<S.numDepends_;    i++ )
    dependName_[i] = S.dependName_[i];

  for ( int i=0; i<S.numDependsNot_; i++ )
    dependNameNot_[i] = S.dependNameNot_[i];

  for ( int i=0; i<S.numDependsFunc_; i++ )
    dependNameFunc_[i] = S.dependNameFunc_[i];
  
  for ( int i=0; i<S.numDependsFuncNot_; i++ )
    dependNameFuncNot_[i] = S.dependNameFuncNot_[i];
  
  for ( int i=0; i<S.numCallbacks_; i++ )
    callbackSignal_[i] = S.callbackSignal_[i];

  for ( int i=0; i<S.numCallbacks_; i++ )
    callback_[i] = S.callback_[i];

  for ( int i=0; i<S.numCallbacks_; i++ )
    callbackHandle_[i] = S.callbackHandle_[i];
  
  for ( int i=0; i<S.numCallbacks_; i++ )
    callbackDisplayName_[i] = S.callbackDisplayName_[i];

}
State & State::operator=(const State & S)
{
  if ( this == &S ) return *this;
  name_                = S.name_;
  numDepends_          = S.numDepends_;
  numDependsNot_       = S.numDependsNot_;

  for ( int i=0; i<numDepends_; i++    )
    if ( depend_[i] ) zap(depend_[i]);
  if ( depend_             ) zaparr(depend_);

  for ( int i=0; i<numDependsNot_; i++ )
    if ( dependNot_[i] ) zap(dependNot_[i]);
  if ( dependNot_          ) zaparr(dependNot_);

  for ( int i=0; i<numDepends_;    i++ )
    if ( dependName_[i] ) zap(dependName_[i]);
  if ( dependName_         ) zaparr(dependName_);

  for ( int i=0; i<numDependsNot_; i++ )
    if ( dependNameNot_[i] ) zap(dependNameNot_[i]);
  if ( dependNameNot_      ) zaparr(dependNameNot_);

  for ( int i=0; i<numDependsFunc_; i++ )
    if ( dependNameFunc_[i] ) zap(dependNameFunc_[i]);
  if ( dependNameFunc_     ) zaparr(dependNameFunc_);

  for ( int i=0; i<numDependsFuncNot_; i++ )
    if ( dependNameFuncNot_[i] ) zap(dependNameFuncNot_[i]);
  if ( dependNameFuncNot_  ) zaparr(dependNameFuncNot_); 

  for ( int i=0; i<numCallbacks_; i++ )
    if ( callbackSignal_[i] ) zap(callbackSignal_[i]);
  if ( callbackSignal_     ) zaparr(callbackSignal_); 

  for ( int i=0; i<numCallbacks_; i++ )
    if ( callback_[i] ) zap(callback_[i]);
  if ( callback_           ) zaparr(callback_);

  for ( int i=0; i<numCallbacks_; i++ )
  if ( callbackHandle_     ) zaparr(callbackHandle_);

  for ( int i=0; i<numCallbacks_; i++ )
    if ( callbackDisplayName_[i] ) zap(callbackDisplayName_[i]);
  if ( callbackDisplayName_) zaparr(callbackDisplayName_); 

  dependName_          = new string*[50];
  dependNameNot_       = new string*[50];
  dependNameFunc_      = new string*[50];
  dependNameFuncNot_   = new string*[50];
  depend_              = new const int*[50];
  dependNot_           = new const int*[50];
  callbackSignal_      = new string*[50];
  callbackHandle_      = new gulong[50];
  callback_            = new GtkWidget*[50];
  callbackDisplayName_ = new string*[50];
  dependFunc_          = new function_t[50];
  dependFuncNot_       = new function_t[50];

  for ( int i=0; i<S.numDepends_; i++    )
    depend_[i] = S.depend_[i];

  for ( int i=0; i<S.numDependsNot_; i++ )
    dependNot_[i] = S.dependNot_[i];

  for ( int i=0; i<S.numDepends_;    i++ )
    dependName_[i] = S.dependName_[i];

  for ( int i=0; i<S.numDependsNot_; i++ )
    dependNameNot_[i] = S.dependNameNot_[i];

  for ( int i=0; i<S.numDependsFunc_; i++ )
    dependNameFunc_[i] = S.dependNameFunc_[i];
  
  for ( int i=0; i<S.numDependsFuncNot_; i++ )
    dependNameFuncNot_[i] = S.dependNameFuncNot_[i];
  
  for ( int i=0; i<S.numCallbacks_; i++ )
    callbackSignal_[i] = S.callbackSignal_[i];

  for ( int i=0; i<S.numCallbacks_; i++ )
    callback_[i] = S.callback_[i];

  for ( int i=0; i<S.numCallbacks_; i++ )
    callbackHandle_[i] = S.callbackHandle_[i];
  
  for ( int i=0; i<S.numCallbacks_; i++ )
    callbackDisplayName_[i] = S.callbackDisplayName_[i];

  numCallbacks_        = S.numCallbacks_;
  complete_            = S.complete_;
  completep_           = S.completep_;
  total_               = S.total_;
  emitReady_           = S.emitReady_;
  numDependsFunc_      = S.numDependsFunc_;
  numDependsFuncNot_   = S.numDependsFuncNot_;
  return *this;   // Allow stacked assignments
}

// Add a callback, return number
int State::addCallback(GtkWidget *callback, const string callbackSignal,
		       gulong hand, const string displayName)
{
  callback_[numCallbacks_]            = callback;
  callbackSignal_[numCallbacks_]      = new string(callbackSignal);
  callbackHandle_[numCallbacks_]      = hand;
  callbackDisplayName_[numCallbacks_] = new string(displayName);
  return(++numCallbacks_);
}

// Block all callbacks
void State::blockAll()
{
  for (int i=0; i<numCallbacks_; i++)
    {
      g_signal_handler_block(callback_[i], callbackHandle_[i]);
#ifdef VERBOSE
      if ( 1 < verbose ) cout << "State : blockAll : \'"
			      << *callbackDisplayName_[i] << "\' blocked\n"
			      << flush;
#endif
    }
}

// Unblock all callbacks
void State::unBlockAll()
{
  for (int i=0; i<numCallbacks_; i++)
    {
      g_signal_handler_unblock(callback_[i], callbackHandle_[i]);
#ifdef VERBOSE
      if ( 1 < verbose ) cout << "State : unblockAll : \'"
			      << *callbackDisplayName_[i] << "\' unblocked\n"
			      << flush;
#endif
    }
}


// Total up the status
void State::totalize()
{
  complete_ = 1;
  total_    = 0;
  for (int i=0; i<numDepends_; i++)
    {
      if ( !*depend_[i] ) complete_ = 0;
      else total_++;
    }
  for (int i=0; i<numDependsNot_; i++)
    {
      if ( *dependNot_[i] ) complete_ = 0;
      else total_++;
    }
  for (int i=0; i<numDependsFunc_; i++)
    {
      if ( !dependFunc_[i]() ) complete_ = 0;
      else total_++;
    }
  for (int i=0; i<numDependsFuncNot_; i++)
    {
      if ( dependFuncNot_[i]() ) complete_ = 0;
      else total_++;
    }
  total_ += complete_;  // Do this to detect changes in complete for callbacks
}

// Update the completeness of the state and return total
void State::emitAll()
{
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "State : emitAll : \'" << name_ << "\'\n"
			  << flush;
#endif
  for ( int i=0; i<numCallbacks_; i++ )
    {
#ifdef VERBOSE
      if ( verbose ) cout << "MESSAGE(State) : emitAll : \'" << name_
			  << "\' emitting \'"
			  << *callbackDisplayName_[i] << "\'(\'"
			  << *callbackSignal_[i]
			  << "\')\n" << flush;
#endif
      g_signal_emit_by_name(callback_[i], callbackSignal_[i]->data());
    }
  emitReady_ = 0;
}

// Update the completeness of the state and return total
int State::update()
{
  completep_ = complete_;
  totalize();     // updates complete_

  if ( complete_ != completep_ )
    {
      emitReady_ = 1;
      emitAll();
    }
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "State : update : \'" << name_
			  << "\' " << complete_ << "\n" << flush;
#endif
  
  return (total_);
}

// Update the completeness of the state and return total
int State::updateWithBlocking()
{
  State::blockAll();
  completep_ = complete_;
  totalize();     // updates complete_

  if ( complete_ != completep_ )
  {
    emitReady_ = 1;
  }
  State::unBlockAll();
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "State : updateWithBlocking : \'" << name_
			  << "\' " << complete_ << "\n" << flush;
#endif
  return (total_);
}

// Add depend.  Return the new number of depends.
int State::addDepend(const string name, int *depend)
{
  dependName_[numDepends_] = new string(name);
  depend_[numDepends_++]   = depend;
  return numDepends_;
}

// Add depend of not.  Return the new number of depends.
int State::addDependNot(const string name, int *dependNot)
{
  dependNameNot_[numDependsNot_] = new string(name);
  dependNot_[numDependsNot_++]   = dependNot;
  return numDependsNot_;
}

// Add depend on function pointer
int State::addDepend(const string name, function_t func)
{
  dependNameFunc_[numDependsFunc_] = new string(name);
  dependFunc_[numDependsFunc_++] = func;
  return numDependsFunc_;
}

// Add depend not on function pointer
int State::addDependNot(const string name, function_t funcNot)
{
  dependNameFuncNot_[numDependsFuncNot_] = new string(name);
  dependFuncNot_[numDependsFuncNot_++] = funcNot;
  return numDependsFuncNot_;
}

// Display State
ostream & operator<< (ostream & out, const State &S)
{
  out << "\'" << S.name_ << "\' : total=" << S.total_ << ", complete="
      << S.complete_ << ", emitReady=" << S.emitReady_ << endl << flush;
  for ( int i=0; i<S.numDepends_; i++ )
    {
      out << "\'" << S.name_ << "\' : depends  =\'" << *S.dependName_[i]
	  << "\' : " << *S.depend_[i] << endl << flush;
    }
  for ( int i=0; i<S.numDependsNot_; i++ )
    {
      out << "\'" << S.name_ << "\' : dependsNot  =\'" << *S.dependNameNot_[i]
	  << "\' : " << *S.dependNot_[i] << endl << flush;
    }
  for ( int i=0; i<S.numDependsFunc_; i++ )
    {
      out << "\'" << S.name_ << "\' : dependsFunc  =\'" << *S.dependNameFunc_[i]
	  << "\' : " << *S.dependFunc_[i] << endl << flush;
    }
  for ( int i=0; i<S.numDependsFuncNot_; i++ )
    {
      out << "\'" << S.name_ << "\' : dependsFuncNot  =\'"
	  << *S.dependNameFuncNot_[i]
	  << "\' : " << *S.dependFuncNot_[i] << endl << flush;
    }
  for ( int i=0; i<S.numCallbacks_; i++ )
    {
      out << "\'" << S.name_ << "\' : callbacks=\'"
	  << *S.callbackDisplayName_[i] << "\' : \'"
	  << S.callbackSignal_[i]->data() << "\'\n" << flush;
    }
  return out;
}

////////////////// class StateMachine //////////////////////////////////////////
StateMachine::StateMachine()
  : totalp_(0), totalDepends_(0), totalDependsNot_(0), numStates_(0),
    name_(NULL), state_(NULL), blockUpdate_(0), complete_(0)
{
  state_ = new State*[50];
  name_  = new string*[50];
}
StateMachine::StateMachine(const StateMachine & S)
  : totalp_(S.totalp_), totalDepends_(S.totalDepends_),
    totalDependsNot_(S.totalDependsNot_), numStates_(S.numStates_),
    name_(NULL), state_(NULL), blockUpdate_(S.blockUpdate_),
    complete_(S.complete_)
{
  state_ = new State*[50];
  for ( int i=0; i<50; i++ ) state_[i] = S.state_[i];
  name_  = new string*[50];
  for ( int i=0; i<50; i++ ) name_[i]  = S.name_[i];
}
StateMachine & StateMachine::operator=(const StateMachine & S) {return *this;}

int *StateMachine::status(const string query)
{
  for (int i=0; i<numStates_; i++)
    {
      if ( query == *name_[i] )
	{
	  return state_[i]->completePtr();
	}
    }
  cerr << "ERROR(StateMachine) : status : " << __FILE__ << " : " << __LINE__
       << " state \'" << query << "\' not found" << endl << flush;
  exit(1);
}

int StateMachine::complete(const string query)
{
  for (int i=0; i<numStates_; i++)
    {
      if ( query == *name_[i] )
	{
	  return state_[i]->complete();
	}
    }
  cerr << "ERROR(StateMachine) : complete : " << __FILE__ << " : " << __LINE__
       << " state \'" << query << "\' not found" << endl << endl << flush;
  exit(1);
}

// Add callback.  Exit on failure.
void StateMachine::addCallback(const int index, GtkWidget *callback,
			       const string callbackSignal, gulong hand,
			       const string displayName)
{
  state_[index]->addCallback(callback, callbackSignal, hand, displayName);
  return;
}

// Add a state upon request.  Return new total number of states.
int StateMachine::addState(const string name)
{
  state_[numStates_] = new State(name);
  for ( int i=0; i<numStates_-1; i++)
    {
      if ( *name_[i] == name )
	{
	  cerr << "ERROR(StateMachine) : addState : " << __FILE__ << " : " 
	       << __LINE__ << "state name \'" << name 
	       << "\' already exists.  Quitting.\n" << flush;
	  exit(1);
	}
    }

  name_[numStates_++] = new string(name);

#ifdef VERBOSE
  if ( 1 < verbose ) cout << "StateMachine : addState : added \'" << name
			  << "\'\n" << flush;
#endif

  return (numStates_-1);
}

void StateMachine::addDepend(const int index, const string name, int *depend)
{
  if ( !( 0 <= index && numStates_ > index ) )
    {
      cerr << "ERROR(StateMachine) : addDepend : " << __FILE__ << " : " 
	   << __LINE__ << " index = " << index
	   << " is out of bounds for number of states = " << numStates_
	   << endl << flush;
      exit(1);
    }
  state_[index]->addDepend(name, depend);
  totalDepends_ += state_[index]->total();

#ifdef VERBOSE
  if ( 1 < verbose ) cout << "StateMachine : addDepend : added \'" << name
			  << "\'\n" << flush;
#endif
  return;
}

void StateMachine::addDependNot(const int index, const string name,
				int *dependNot)
{
  if ( !( 0 <= index && numStates_ > index ) )
    {
      cerr << "ERROR(StateMachine) : addDependNot : " << __FILE__ << " : " 
	   << __LINE__ << " index = " << index
	   << " is out of bounds for number of states = " << numStates_
	   << endl << flush;
      exit(1);
    }
  state_[index]->addDependNot(name, dependNot);
  totalDependsNot_ += state_[index]->total();

#ifdef VERBOSE
  if ( 1 < verbose ) cout << "StateMachine : addDependNot : added \'" << name
			  << "\'\n" << flush;
#endif
  return;
}

void StateMachine::addDepend(const int index, const string name,
			       function_t func)
{
  if ( !( 0 <= index && numStates_ > index ) )
    {
      cerr << "ERROR(StateMachine) : addDepend : " << __FILE__ << " : " 
	   << __LINE__ << " index = " << index
	   << " is out of bounds for number of states = " << numStates_
	   << endl << flush;
      exit(1);
    }
  state_[index]->addDepend(name, func);
  totalDepends_ += state_[index]->total();

#ifdef VERBOSE
  if ( 1 < verbose ) cout << "StateMachine : addDepend : added \'" << name
			  << "\'\n" << flush;
#endif
  return;
}

void StateMachine::addDependNot(const int index, const string name,
				function_t funcNot)
{
  if ( !( 0 <= index && numStates_ > index ) )
    {
      cerr << "ERROR(StateMachine) : addDependNot : " << __FILE__ << " : " 
	   << __LINE__ << " index = " << index
	   << " is out of bounds for number of states = " << numStates_
	   << endl << flush;
      exit(1);
    }
  state_[index]->addDependNot(name, funcNot);
  totalDependsNot_ += state_[index]->total();

#ifdef VERBOSE
  if ( 1 < verbose ) cout << "StateMachine : addDependNot : added \'"
			  << name << "\'\n" << flush;
#endif
  return;
}

// Block all callbacks
void StateMachine::blockAll()
{
  for ( int i=0; i<numStates_; i++)
    {
      state_[i]->blockAll();
#ifdef VERBOSE
      if ( 1 < verbose ) cout << "StateMachine : blockAll : \'"
			      << state_[i]->name() << "\' blocked all\n"
			      << flush;
#endif
    }
}

// Unblock all callbacks
void StateMachine::unBlockAll()
{
  for ( int i=0; i<numStates_; i++)
    {
      state_[i]->unBlockAll();
#ifdef VERBOSE
      if ( 1 < verbose ) cout << "StateMachine : blockAll : \'"
			      << state_[i]->name() << "\' unblocked all\n"
			      << flush;
#endif
    }
}

// Update the status of all the states.  Return percentage complete.
float StateMachine::update()
{
  return ( updater(0) );
}
void StateMachine::update(const string query)
{
  for (int i=0; i<numStates_; i++)
    {
      if ( query == *name_[i] )
	{
	  if ( blockUpdate_ )
	    {   
#ifdef VERBOSE
	      if ( 1 < verbose )
		cout << "MESSAGE(StateMachine) : update : \'" << *name_[i]
		     << "\' bypassed.\n" << flush;
#endif
	      return;
	    }
	  state_[i]->update();
	  return;
	}
    }
  cerr << "ERROR(StateMachine) : update(string query) : "
       << __FILE__ << " : " << __LINE__
       << " state \'" << query << "\' not found" << endl << endl << flush;
  exit(1);
}

// Update the status of all the states.  Return percentage complete.
float StateMachine::updateWithBlocking()
{
  return ( updater(1) );
}
void StateMachine::updateWithBlocking(const string query)
{
  for (int i=0; i<numStates_; i++)
    {
      if ( query == *name_[i] )
	{
	  if ( blockUpdate_ )
	    {   
#ifdef VERBOSE
	      if ( 1 < verbose )
		cout << "MESSAGE(StateMachine) : updateWithBlocking : \'"
		     << *name_[i] << "\' bypassed.\n" << flush;
#endif
	      return;
	    }
	  state_[i]->updateWithBlocking();
	  return;
	}
    }
  cerr << "ERROR(StateMachine) : updateWithBlocking(string query) : "
       << __FILE__ << " : " << __LINE__
       << " state \'" << query << "\' not found" << endl << endl << flush;
  exit(1);
}

// Block all callbacks
void StateMachine::blockUpdate()
{
  blockUpdate_ = 1;
#ifdef VERBOSE
  if ( verbose ) cout << "MESSAGE(StateMachine) : blockUpdate\n" << flush;
#endif
}

// Unblock all callbacks
void StateMachine::unBlockUpdate()
{
  blockUpdate_ = 0;
#ifdef VERBOSE
  if ( verbose ) cout << "MESSAGE(StateMachine) : unBlockUpdate\n" << flush;
#endif
}

// Update the status of all the states.  Return percentage complete.
float StateMachine::updater(const int withBlocking)
{
  int total = totalp_;

  if ( blockUpdate_ )
    {   
#ifdef VERBOSE
      if ( 1 < verbose )
	cout << "MESSAGE(StateMachine) : updater : bypassed\n" << flush;
#endif
    }
  else
    {
      // Update total
      complete_ = 1;
      const int countMax = 10;
      int count = 0;   // while loop failsafe
      do
	{
	  totalp_ = total;
	  total   = 0;
	  
	  if ( withBlocking )
	    for (int i=0; i<numStates_; i++)
	      total += state_[i]->updateWithBlocking();
	  
	  else
	    for (int i=0; i<numStates_; i++)
	      {
		total += state_[i]->update();
		if ( !state_[i]->complete() ) complete_ = 0;
	      }
	  
	} while ( total != totalp_ && countMax > ++count );
      if ( countMax == count )
	{
	  cerr << "WARNING(StateMachine) : update : " << __FILE__ << " : "
	       << __LINE__ << " iterations exeeded.  Possible non-convergence.\n"
	       << flush;
	}
    } // blockUpdate_
  
  // Recalculate number of depends in case it changed
  totalDepends_ = 0;
  for ( int i=0; i<numStates_; i++) totalDepends_ += state_[i]->numDepends();
  totalDependsNot_ = 0;
  for ( int i=0; i<numStates_; i++)
    totalDependsNot_ += state_[i]->numDependsNot();

  return ( total / (totalDepends_ + totalDependsNot_) * 100. );
}

// Display State
ostream & operator<< (ostream & out, const StateMachine &SM)
{
  for ( int i=0; i<SM.numStates_; i++ )
    {
      out << *(SM.state_[i]);
    }
  out << "MESSAGE(StateMachine) : total="
      << SM.totalDepends_+SM.totalDependsNot_<< endl << flush;
  return out;
}
