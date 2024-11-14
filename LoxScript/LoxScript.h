#pragma once
#ifndef LOXSCRIPT_H
#define LOXSCRIPT_H

#if WIN32
#include "WinHeader++.h"
#endif

#include "rb_plugin.h"

extern "C" {

#include "VM.h"

	// Generally, classes allocate storage for each instance
	// of the class.  This storage can hold all of the information
	// that an individual class instance will use. 
	struct LoxScriptClassData {
		REALstring mSource;
		int mState;
		REALobject mContext;
	};

	// This is where we put all of the forward declarations for
	// functions needed by the plugin SDK.
	static void LoxScriptClassInitializer(REALobject instance);
	static void LoxScriptClassFinalizer(REALobject instance);
	static void LoxScriptClassCompile(REALobject instance, REALstring source);
	static void LoxScriptClassRun(REALobject instance);

	static void DebugLog(const char* buffer, ...);
	static void writeFn(VM* vm, const char* text);

	// Define the properties which our class is going to expose.
	REALproperty LoxScriptClassProperties[] = {
	{ "", "Source", "String", REALconsoleSafe, REALstandardGetter, REALstandardSetter, FieldOffset(LoxScriptClassData, mSource) },
	{ "", "State", "Integer", REALconsoleSafe, REALstandardGetter, nil, FieldOffset(LoxScriptClassData, mState) },
	{ "", "Context", "Object", REALconsoleSafe, REALstandardGetter, REALstandardSetter, FieldOffset(LoxScriptClassData, mContext) },
	};

	// Now we are going to define the methods that we want our class
	REALmethodDefinition LoxScriptClassMethods[] = {
	{ (REALproc)LoxScriptClassCompile, REALnoImplementation, "Compile( source as String )", REALconsoleSafe },
	{ (REALproc)LoxScriptClassRun, REALnoImplementation, "Run()", REALconsoleSafe },
	};

	// The next code item we're going to work on is the event.  Your
	REALevent LoxScriptClassEvents[] = {
		{ "Print( msg as String )", },
	};

	// Define the actual class itself. 
	REALclassDefinition LoxScriptClassDefinition = {
		// This field specifies the current Plugin SDK version.  You 
		// should always set the value to kCurrentREALControlVersion.
		kCurrentREALControlVersion,

		// This field specifies the name of the class which will be
		// exposed to the user
		"LoxScript",

		// If your class has a Super, you can set the super here.  For
		// instance, if you want your class to inherit from the 
		// REALbasic Dictionary class, you would put "Dictionary" here.
		// A value of nil specifies that this class has no super.
		nil,

		// This field specifies the size of the class instance storage
		// we defined above.
		sizeof(LoxScriptClassData),

		// This field is reserved and should always be 0
		0,

		// If your class needs to initialize any of its instance
		// data, then you can specify an initializer method.
		//
		// TIP: You can specify the default values for your class instance
		// data by providing an Initializer method in the REALclassDefinition.
		(REALproc)LoxScriptClassInitializer,

		// If your class needs to finalize any of its instance
		// data, the you can specify a finalizer method.
		//
		// TIP: If you've got any object references (including string
		// references) stored in your class instance data, you should
		// specify a finalizer method which unlocks all of the
		// object references (so that you do not leak them).
		(REALproc)LoxScriptClassFinalizer,

		// Properties
		LoxScriptClassProperties,

		// How many properties are there?
		//
		// TIP: You can use the sizeof operator to calculate this for
		// you automatically
		_countof(LoxScriptClassProperties),

		// Methods
		LoxScriptClassMethods,
		_countof(LoxScriptClassMethods),

		// Events which the user implements
		LoxScriptClassEvents,
		_countof(LoxScriptClassEvents),

		// Event instances, which we are skipping over.  Whenever
		// you want to skip over a field within the structure, you
		// can replace it with nil values, like this:
		nil,
		0,

		// If the class implements any interfaces, then you can
		// list the interface names here (separate multiple names
		// with a comma, just like in REALbasic).
		nil,

		// The next two fields are for bindings, which are a mystery.
		nil,
		0,

		// Back to things which get used qith some frequency: Constants!
		nil,
		0,

		// The next field is for flags.  You don't have worry about
		// setting these.  There are helper methods for any flags
		// you'd like to set.
		0,

		// The next field defines shared properties.
		nil,
		0,

		// The final field defined shared methods.
		nil,
		0,
	};

}

#endif