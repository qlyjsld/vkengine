#include "ConsoleVariableSystem.h"

#include <string>
#include <unordered_map>

enum class ConsoleVariableType : char
{
    INT,
    FLOAT,
    STRING
};

class ConsoleVariableParameter
{
public:

    friend class ConsoleVariableSystemImplementation;

    uint32_t arrayIndex;
    ConsoleVariableType type;
    ConsoleVariableFlag flag;
    std::string name;
    std::string description;
};

template<typename T>
struct ConsoleVariableStorage
{
    T initial;
    T current;
    ConsoleVariableParameter* parameter;
};

template<typename T>
struct ConsoleVariableArray
{
    ConsoleVariableStorage<T>* variables;
    uint32_t lastVariable{ 0 };
    size_t arraySize{ 0 };

    ConsoleVariableArray(size_t size)
    : arraySize(size)
    {
        variables = new ConsoleVariableStorage<T>[size];
    }

    ~ConsoleVariableArray()
    {
        // delete variables;
    }

    T* getCurrent(uint32_t index)
    {
        return &variables[index].current;
    }

    void setCurrent(uint32_t index, T newValue)
    {
        variables[index].current = newValue;
    }

    uint32_t pushBack(T newValue, ConsoleVariableParameter* parameter)
    {
        uint32_t index = lastVariable;

        if (index < arraySize)
        {
            variables[index].initial = newValue;
            variables[index].current = newValue;
            variables[index].parameter = parameter;

            parameter->arrayIndex = index;

            lastVariable++;
        }

        return index;
    }
};

class ConsoleVariableSystemImplementation : public ConsoleVariableSystem
{
public:

    constexpr static size_t MAX_INT_VARIABLES = 16;
    ConsoleVariableArray<int> intArray{ MAX_INT_VARIABLES };

    constexpr static size_t MAX_FLOAT_VARIABLES = 16;
    ConsoleVariableArray<float> floatArray{ MAX_FLOAT_VARIABLES };

    constexpr static size_t MAX_STRING_VARIABLES = 16;
    ConsoleVariableArray<std::string> stringArray{ MAX_STRING_VARIABLES };
    
    template<typename T>
    ConsoleVariableArray<T>* getVariableArray();

    template<typename T>
	T* getVariableCurrentByHash(uint32_t namehash) {
		ConsoleVariableParameter* parameter = getVariableParameter(namehash);
		if (!parameter)
        {
			return nullptr;
		}
		else
        {
			return getVariableArray<T>()->getCurrent(parameter->arrayIndex);
		}
	}

	template<typename T>
	void setVariableCurrentByHash(uint32_t namehash, T value)
	{
		ConsoleVariableParameter* parameter = getVariableParameter(namehash);
		if (parameter)
		{
			getVariableArray<T>()->setCurrent(parameter->arrayIndex, value);
		}
	}

    ConsoleVariableParameter* getVariableParameter(StringUtils::StringHash hash) override final;
    int* getIntVariableCurrentByHash(StringUtils::StringHash hash) override final;
    float* getFloatVariableCurrentByHash(StringUtils::StringHash hash) override final;
    const char* getStringVariableCurrentByHash(StringUtils::StringHash hash) override final;
    void setIntVariableCurrentByHash(StringUtils::StringHash hash, int newValue) override final;
    void setFloatVariableCurrentByHash(StringUtils::StringHash hash, float newValue) override final;
    void setStringVariableCurrentByHash(StringUtils::StringHash hash, const char* newValue) override final;
    ConsoleVariableParameter* createIntVariable(int defaultValue, int currentValue, const char* name, const char* description) override final;
    ConsoleVariableParameter* createFloatVariable(float defaultValue, float currentValue, const char* name, const char* description) override final;
    ConsoleVariableParameter* createStringVariable(const char* defaultValue, const char* currentValue, const char* name, const char* description) override final;

private:

	ConsoleVariableParameter* initVariable(const char* name, const char* description);
	std::unordered_map<uint32_t, ConsoleVariableParameter> savedVariables;
};

ConsoleVariableSystem* ConsoleVariableSystem::get()
{
    static ConsoleVariableSystemImplementation consoleVariableSystem{};
    return &consoleVariableSystem;
}

template<>
ConsoleVariableArray<int>* ConsoleVariableSystemImplementation::getVariableArray()
{
    return &intArray;
}

template<>
ConsoleVariableArray<float>* ConsoleVariableSystemImplementation::getVariableArray()
{
    return &floatArray;
}

template<>
ConsoleVariableArray<std::string>* ConsoleVariableSystemImplementation::getVariableArray()
{
    return &stringArray;
}

ConsoleVariableParameter* ConsoleVariableSystemImplementation::initVariable(const char* name, const char* description)
{
    if (getVariableParameter(name)) return nullptr;

    uint32_t namehash = StringUtils::StringHash{ name };
	savedVariables[namehash] = ConsoleVariableParameter{};

	ConsoleVariableParameter& newParameter = savedVariables[namehash];

	newParameter.name = name;
	newParameter.description = description;

	return &newParameter;
}

ConsoleVariableParameter* ConsoleVariableSystemImplementation::getVariableParameter(StringUtils::StringHash hash)
{
    auto it = savedVariables.find(hash);

	if (it != savedVariables.end())
	{
		return &(*it).second;
	}

	return nullptr;
}

int* ConsoleVariableSystemImplementation::getIntVariableCurrentByHash(StringUtils::StringHash hash)
{
    return getVariableCurrentByHash<int>(hash);
}

float* ConsoleVariableSystemImplementation::getFloatVariableCurrentByHash(StringUtils::StringHash hash)
{
    return getVariableCurrentByHash<float>(hash);
}

const char* ConsoleVariableSystemImplementation::getStringVariableCurrentByHash(StringUtils::StringHash hash)
{
    return getVariableCurrentByHash<std::string>(hash)->c_str();
}

void ConsoleVariableSystemImplementation::setIntVariableCurrentByHash(StringUtils::StringHash hash, int newValue)
{
    setVariableCurrentByHash<int>(hash, newValue);
}

void ConsoleVariableSystemImplementation::setFloatVariableCurrentByHash(StringUtils::StringHash hash, float newValue)
{
    setVariableCurrentByHash<float>(hash, newValue);
}

void ConsoleVariableSystemImplementation::setStringVariableCurrentByHash(StringUtils::StringHash hash, const char* newValue)
{
    setVariableCurrentByHash<std::string>(hash, newValue);
}

ConsoleVariableParameter* ConsoleVariableSystemImplementation::createIntVariable(int defaultValue, int currentValue, const char* name, const char* description)
{
	ConsoleVariableParameter* newParameter = initVariable(name, description);
	if (!newParameter) return nullptr;

	newParameter->type = ConsoleVariableType::INT;
	
    getVariableArray<int>()->pushBack(currentValue, newParameter);

	return newParameter;
}

ConsoleVariableParameter* ConsoleVariableSystemImplementation::createFloatVariable(float defaultValue, float currentValue, const char* name, const char* description)
{
	ConsoleVariableParameter* newParameter = initVariable(name, description);
	if (!newParameter) return nullptr;

	newParameter->type = ConsoleVariableType::FLOAT;
	
    getVariableArray<float>()->pushBack(currentValue, newParameter);

	return newParameter;
}

ConsoleVariableParameter* ConsoleVariableSystemImplementation::createStringVariable(const char* defaultValue, const char* currentValue, const char* name, const char* description)
{
	ConsoleVariableParameter* newParameter = initVariable(name, description);
	if (!newParameter) return nullptr;

	newParameter->type = ConsoleVariableType::STRING;
	
    getVariableArray<std::string>()->pushBack(currentValue, newParameter);

	return newParameter;
}

//get the cvar data purely by type and array index
template<typename T>
T* getCurrentByIndex(int32_t index)
{
	return ((ConsoleVariableSystemImplementation*)ConsoleVariableSystem::get())->getVariableArray<T>()->getCurrent(index);
}

//set the cvar data purely by type and index
template<typename T>
void setCurrentByIndex(int32_t index, T data)
{
	((ConsoleVariableSystemImplementation*)ConsoleVariableSystemImplementation::get())->getVariableArray<T>()->setCurrent(index, data);
}

//cvar float constructor
AutoInt::AutoInt(int value, const char* name, const char* description, ConsoleVariableFlag flag)
{
	ConsoleVariableParameter* parameter = ConsoleVariableSystemImplementation::get()->createIntVariable(value, value, name, description);
	parameter->flag = flag;
	index = parameter->arrayIndex;
}

int AutoInt::get()
{
	return *getCurrentByIndex<type>(index);
}

void AutoInt::set(int f)
{
	setCurrentByIndex<type>(index, f);
}

//cvar float constructor
AutoFloat::AutoFloat(float value, const char* name, const char* description, ConsoleVariableFlag flag)
{
	ConsoleVariableParameter* parameter = ConsoleVariableSystemImplementation::get()->createFloatVariable(value, value, name, description);
	parameter->flag = flag;
	index = parameter->arrayIndex;
}

float AutoFloat::get()
{
	return *getCurrentByIndex<type>(index);
}

void AutoFloat::set(float f)
{
	setCurrentByIndex<type>(index, f);
}

//cvar float constructor
AutoString::AutoString(std::string value, const char* name, const char* description, ConsoleVariableFlag flag)
{
	ConsoleVariableParameter* parameter = ConsoleVariableSystemImplementation::get()->createStringVariable(value.c_str(), value.c_str(), name, description);
	parameter->flag = flag;
	index = parameter->arrayIndex;
}

std::string AutoString::get()
{
	return *getCurrentByIndex<type>(index);
}

void AutoString::set(std::string f)
{
	setCurrentByIndex<type>(index, f);
}
