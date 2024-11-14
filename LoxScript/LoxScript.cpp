#include "LoxScript.h"
#include "VM.h"

//static void DebugLog(REALstring str)
//{
//	//typedef void (*FuncTy)(REALstring);
//	//FuncTy fp = (FuncTy)REALLoadFrameworkMethod("System.DebugLog(msg As String)");
//	//if (fp) fp(str);
//	char* buffer = (char*)REALCString(str);
//#if WIN32
//	OutputDebugStringA(buffer);
//#else
//	fprintf(stderr, buffer);
//#endif
//
//}

static void DebugLog(const char* buffer, ...)
{
	va_list args;
	va_start(args, buffer);

#if WIN32
#define MAX_BUFFER 1024
	int sz= vsnprintf(NULL, 0, buffer, args);
	char buf[MAX_BUFFER];
	vsnprintf(buf, MAX_BUFFER, buffer, args);
	OutputDebugStringA(buf);
#else
	vfprintf(stderr, buffer, args);
#endif

	va_end(args);
}

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
	me->mContext = nil;
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

const char* getNameProp(const char* prop) {
	size_t sz = strlen(prop);
	char* ret = (char*)malloc(sz);
	size_t i = 0;
	for (; i < sz; i++) {
		if (isspace(prop[i])) {
			break;
		}
		ret[i] = prop[i];
	}
	ret[i] = '\0';

	return ret;
}

const char* getTypeProp(const char* prop) {
	size_t sz = strlen(prop);
	char* ret = (char*)malloc(sz);
	size_t i = sz - 1;
	for (; i >= 0; i--) {
		if (isspace(prop[i])) {
			break;
		}
	}
	i++;
	size_t j = 0;
	for (; (j+ i)<= sz; j++) {
		ret[j] = prop[j+ i];
	}
	ret[j] = '\0';

	return ret;
}

void addPropsToVm(REALobjectStruct* context, VM* vm) {
	if (!context) return;

	RBInteger numProps = REALCountClassProperties(context);

	for (RBInteger i = 0; i < numProps; i++) {
		void* getter = nil;
		void* setter = nil;
		long param = 0;
		REALstring declaration = nil;
		RBBoolean r = REALGetClassProperty(context, i, &getter, &setter, &param, &declaration);

		if (r) {
			const char* prop = REALCString(declaration);
			const char* name = getNameProp(prop);
			const char* ntyp = getTypeProp(prop);
			//DebugLog("%s = %s", name, ntyp);

			if (strcmp(ntyp, "String") == 0 && getter) {
				REALstring value;
				bool r = REALGetPropValueString(context, name, &value);
				if (r) {
					DebugLog("%s = \"%s\"", name, REALCString(value));
					// add global var
					//const char* cvalue = REALCString(value);
					//ObjString* str = copyString(&vm->gc, &vm->strings, cvalue, strlen(cvalue));
					//pushTemp(&vm->gc, OBJ_VAL(str));
					//addConstant(&vm->gc, &fun->chunk, OBJ_VAL(str));
				}
			}
			
		}
	}
}

static void LoxScriptClassRun(REALobject instance) {
	// Get the LoxScriptClassData from our object
	ClassData(LoxScriptClassDefinition, instance, LoxScriptClassData, me);

	char* source = (char*)REALCString(me->mSource);

	VM vm;

	initVM(&vm, stdout, stderr);

	vm.writeFn = &writeFn;
	vm.userData = instance;

	// add context props as global variables:
	addPropsToVm(me->mContext, &vm);

	InterpretResult result = interpret(&vm, source);

	freeVM(&vm);

	me->mState = (int)result;

}

void PluginEntry(void)
{
	// Since we want our class to be console safe,
	SetClassConsoleSafe(&LoxScriptClassDefinition);

	// Register our class
	REALRegisterClass(&LoxScriptClassDefinition);
}