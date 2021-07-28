#pragma once
#include "StringUtils.h"

class ConsoleVariableParameter;

enum class ConsoleVariableFlag : uint32_t
{
    NONE = 0
};

class ConsoleVariableSystem
{
public:
    
    static ConsoleVariableSystem* get();

    virtual ConsoleVariableParameter* getVariableParameter(StringUtils::StringHash hash) = 0;
    virtual int* getIntVariableCurrentByHash(StringUtils::StringHash hash) = 0;
    virtual float* getFloatVariableCurrentByHash(StringUtils::StringHash hash) = 0;
    virtual const char* getStringVariableCurrentByHash(StringUtils::StringHash hash) = 0;
    virtual void setIntVariableCurrentByHash(StringUtils::StringHash hash, int newValue) = 0;
    virtual void setFloatVariableCurrentByHash(StringUtils::StringHash hash, float newValue) = 0;
    virtual void setStringVariableCurrentByHash(StringUtils::StringHash hash, const char* newValue) = 0;
    virtual ConsoleVariableParameter* createIntVariable(int defaultValue, int currentValue, const char* name, const char* description) = 0;
    virtual ConsoleVariableParameter* createFloatVariable(float defaultValue, float currentValue, const char* name, const char* description) = 0;
    virtual ConsoleVariableParameter* createStringVariable(const char* defaultValue, const char* currentValue, const char* name, const char* description) = 0;
};

template<typename T>
struct AutoVariable
{
protected:
	int index;
	using type = T;
};

struct AutoInt : AutoVariable<int>
{
	AutoInt(int value, const char* name, const char* description, ConsoleVariableFlag flag);

	int get();
	void set(int val);
};

struct AutoFloat : AutoVariable<float>
{
	AutoFloat(float value, const char* name, const char* description, ConsoleVariableFlag flag);

	float get();
	void set(float val);
};

struct AutoString : AutoVariable<std::string>
{
	AutoString(std::string value, const char* name, const char* description, ConsoleVariableFlag flag);

	std::string get();
	void set(std::string val);
};
