#include "LoxScript.h"
#include "VM.h"

static void writeFn(VM* vm, const char* text)
{
	//printf("%s", text);

	if (vm->userData == NULL) {
		return;
	}

	REALobject instance = (REALobject)vm->userData;

	// Try to load the function pointer for our method from
	// the object instance passed in.
	void(*fpAction)(REALobject instance, REALstring msg) =
		(void(*)(REALobject, REALstring))REALGetEventInstance((REALcontrolInstance)instance, &LoxScriptClassEvents[0]);

	if (fpAction) {
		// The user has implemented the Action event, so now
		// we can call it and return the data.
		fpAction(instance, REALBuildStringWithEncoding(text, ::strlen(text), kREALTextEncodingASCII));
	}

}

// Initializes the data for our LoxScriptClassData object
static void LoxScriptClassInitializer(REALobject instance)
{
	// Get the TestClassData from our object
	ClassData(LoxScriptClassDefinition, instance, LoxScriptClassData, me);

	me->mSource = nil;
	me->mState = 0; 
}

// Finalize the data for our LoxScriptClassData object
static void LoxScriptClassFinalizer(REALobject instance)
{
	// Get the TestClassData from our object
	ClassData(LoxScriptClassDefinition, instance, LoxScriptClassData, me);

	// Be sure to clean up any object references which
	// we may have
	REALUnlockString(me->mSource);
}

void MsgBox(REALstring msg)
{
	void (*REALMsgBox)(REALstring) = (void (*)(REALstring))REALLoadFrameworkMethod("MsgBox( s as String )");
	if (REALMsgBox) {
		REALMsgBox(msg);
	}
}

static void LoxScriptClassCompile(REALobject instance, REALstring source)
{
	// Get the TestClassData from our object
	ClassData(LoxScriptClassDefinition, instance, LoxScriptClassData, me);

	// Unlock our old data
	REALUnlockString(me->mSource);

	// Store the new data
	me->mSource = source;

	// And lock it
	REALLockString(me->mSource);
}

static void LoxScriptClassRun(REALobject instance) {
	// Get the TestClassData from our object
	ClassData(LoxScriptClassDefinition, instance, LoxScriptClassData, me);

	size_t numbytes = REALStringLength(me->mSource);
	char* buffer = (char*)REALGetStringContents(me->mSource, &numbytes);
	size_t sz = snprintf(NULL, 0, "%s", buffer);
	char* source = (char*)malloc(sz + 1);
	snprintf(source, sz + 1, "%s", buffer);

	VM vm;

	initVM(&vm, stdout, stderr);

	vm.writeFn = &writeFn;
	vm.userData = instance;

	InterpretResult result = interpret(&vm, source);

	freeVM(&vm);

	if (result == INTERPRET_COMPILE_ERROR) {
		me->mState = 1;
	}
	if (result == INTERPRET_RUNTIME_ERROR) {
		me->mState = 2;
	}

	//MsgBox(
	//	REALBuildStringWithEncoding("Run!", ::strlen("Run!"), kREALTextEncodingASCII)
	//);
}

void PluginEntry(void)
{
	// Since we want our class to be console safe,
	SetClassConsoleSafe(&LoxScriptClassDefinition);

	// Register our class
	REALRegisterClass(&LoxScriptClassDefinition);
}