#include "json_parser_tests.hpp"

#include "check.hpp"

#include <json_parser.h>

#include <map>
#include <string>
#include <vector>

static bool theWasCalled = false;
JsonParsingDispatchTable theDispatch{};
std::string theCurrentKey{};
std::vector<std::string> theKeys{};
std::multimap<std::string, std::string> theStringValues{};
std::multimap<std::string, int> theIntValues{};

void
LocDefaultSetupForDispatchTable() {
	theDispatch = {};
	theCurrentKey = {};
	theKeys = {};
	theStringValues = {};
	theIntValues = {};

	theDispatch.parseInt = [](auto aValue) { theIntValues.insert({theCurrentKey, aValue}); };
	theDispatch.parseString = [](const auto* aValue) {
		theStringValues.insert({theCurrentKey, aValue});
	};
	theDispatch.getKeyHandler = [](const auto* aKey) {
		theCurrentKey += "/";
		theCurrentKey += aKey;
		theKeys.emplace_back(aKey);
		return theDispatch;
	};
}

std::vector<std::string>
LocCollapsStringValues() {
	std::vector<std::string> result;
	for (const auto& [_, value] : theStringValues) {
		result.emplace_back(value);
	}
	return result;
}

std::vector<int>
LocCollapsIntValues() {
	std::vector<int> result;
	for (const auto& [_, value] : theIntValues) {
		result.emplace_back(value);
	}
	return result;
}

bool
NullptrWillNotDoAnything() {
	theWasCalled = false;
	theDispatch = {};

	theDispatch.parseInt = [](auto) { theWasCalled = true; };
	theDispatch.parseString = [](auto) { theWasCalled = true; };
	theDispatch.getKeyHandler = [](auto) {
		theWasCalled = true;
		return theDispatch;
	};

	ParseJson(nullptr, theDispatch);

	CHECK(theWasCalled, false);
	return true;
}

bool
EmptyStringWillNotDoAnything() {
	theWasCalled = false;
	theDispatch = {};

	theDispatch.parseInt = [](auto) { theWasCalled = true; };
	theDispatch.parseString = [](auto) { theWasCalled = true; };
	theDispatch.getKeyHandler = [](auto) {
		theWasCalled = true;
		return theDispatch;
	};

	std::string json;
	ParseJson(json.data(), theDispatch);

	CHECK(theWasCalled, false);
	return true;
}

bool
EmptyJsonWillNotCallParsingOnTheHandler() {
	theWasCalled = false;
	theDispatch = {};

	theDispatch.parseInt = [](auto) { theWasCalled = true; };
	theDispatch.parseString = [](auto) { theWasCalled = true; };
	theDispatch.getKeyHandler = [](auto) { return theDispatch; };

	std::string json = "{}";
	ParseJson(json.data(), theDispatch);

	CHECK(theWasCalled, false);
	return true;
}

bool
AKeyIsSuccessfulyParsed() {
	std::vector keys = {"key", "abc", "hello", "world"};

	for (const auto& key : keys) {
		LocDefaultSetupForDispatchTable();

		std::string json = R"json({")json";
		json += key;
		json += R"json(":123})json";
		ParseJson(json.data(), theDispatch);

		CHECK(theKeys.size(), 1);
		CHECK(theKeys[0], key);
	}
	return true;
}

bool
StringValueIsNotConsideredAKey() {
	LocDefaultSetupForDispatchTable();

	std::string json = R"json({"key":"value"})json";
	ParseJson(json.data(), theDispatch);

	CHECK(theKeys.size(), 1);
	CHECK(theKeys[0], "key");
	return true;
}

bool
MultipleKeysAreDetected() {
	LocDefaultSetupForDispatchTable();

	std::string json = R"json({"key":"value","key2":234})json";
	ParseJson(json.data(), theDispatch);

	CHECK(theKeys.size(), 2);
	CHECK(theKeys[0], "key");
	CHECK(theKeys[1], "key2");
	return true;
}

bool
StringValueParserIsCalled() {
	LocDefaultSetupForDispatchTable();

	std::string json = R"json({"key":"value"})json";
	ParseJson(json.data(), theDispatch);

	auto stringValues = LocCollapsStringValues();
	CHECK(stringValues.size(), 1);
	CHECK(stringValues[0], "value");
	return true;
}

bool
IntParserIsNotCalledForStringValue() {
	theWasCalled = false;
	theDispatch = {};

	theDispatch.parseInt = [](auto) { theWasCalled = true; };
	theDispatch.parseString = [](const auto* aValue) {};
	theDispatch.getKeyHandler = [](const auto* aKey) { return theDispatch; };

	std::string json = R"json({"key":"value"})json";
	ParseJson(json.data(), theDispatch);

	CHECK(theWasCalled, false);
	return true;
}

bool
IntValueParserIsCalled() {
	LocDefaultSetupForDispatchTable();

	std::string json = R"json({"key":1234})json";
	ParseJson(json.data(), theDispatch);

	auto intValues = LocCollapsIntValues();
	CHECK(intValues.size(), 1);
	CHECK(intValues[0], 1234);
	return true;
}

bool
IgnoresWhitespaceCharacters() {
	LocDefaultSetupForDispatchTable();

	std::string json = R"json({   "key": "value" 	,
	"key2"            	   :  1234
})json";
	ParseJson(json.data(), theDispatch);

	auto stringValues = LocCollapsStringValues();
	auto intValues = LocCollapsIntValues();
	CHECK(theKeys.size(), 2);
	CHECK(theKeys[0], "key");
	CHECK(theKeys[1], "key2");
	CHECK(stringValues.size(), 1);
	CHECK(stringValues[0], "value");
	CHECK(intValues.size(), 1);
	CHECK(intValues[0], 1234);
	return true;
}

bool
CanDealWithArrayOfInts() {
	LocDefaultSetupForDispatchTable();

	std::string json = R"json({"key":[1234,22,333]})json";
	ParseJson(json.data(), theDispatch);

	auto intValues = LocCollapsIntValues();
	CHECK(intValues.size(), 3);
	CHECK(intValues[0], 1234);
	CHECK(intValues[1], 22);
	CHECK(intValues[2], 333);
	return true;
}

bool
CallsKeyHandlerOnceForEveryElementInArray() {
	theWasCalled = false;
	theDispatch = {};
	theKeys = {};

	theDispatch.parseInt = [](int aValue) {};
	theDispatch.parseString = [](const auto* aValue) {};
	theDispatch.getKeyHandler = [](const auto* aKey) {
		theKeys.emplace_back(aKey);
		return theDispatch;
	};

	std::string json = R"json({"key":[1234,22,333]})json";
	ParseJson(json.data(), theDispatch);

	CHECK(theKeys.size(), 3);
	CHECK(theKeys[0], "key");
	CHECK(theKeys[1], "key");
	CHECK(theKeys[2], "key");
	return true;
}

bool
ArrayCanCountainStrings() {
	LocDefaultSetupForDispatchTable();

	std::string json = R"json({"key":[1234,"abcd",333]})json";
	ParseJson(json.data(), theDispatch);

	auto stringValues = LocCollapsStringValues();
	auto intValues = LocCollapsIntValues();
	CHECK(intValues.size(), 2);
	CHECK(intValues[0], 1234);
	CHECK(intValues[1], 333);
	CHECK(stringValues.size(), 1);
	CHECK(stringValues[0], "abcd");
	return true;
}

bool
ArrayCanCountainMultipleStrings() {
	LocDefaultSetupForDispatchTable();

	std::string json = R"json({"key":["hello","abcd",333]})json";
	ParseJson(json.data(), theDispatch);

	auto stringValues = LocCollapsStringValues();
	auto intValues = LocCollapsIntValues();
	CHECK(intValues.size(), 1);
	CHECK(intValues[0], 333);
	CHECK(stringValues.size(), 2);
	CHECK(stringValues[0], "hello");
	CHECK(stringValues[1], "abcd");
	return true;
}

bool
CanDealWithObjectInObject() {
	LocDefaultSetupForDispatchTable();

	std::string json = R"json({"key":{"innerKey":1234}})json";
	ParseJson(json.data(), theDispatch);

	auto intValues = LocCollapsIntValues();
	CHECK(theKeys.size(), 2);
	CHECK(theKeys[0], "key");
	CHECK(theKeys[1], "innerKey");
	CHECK(intValues.size(), 1);
	CHECK(intValues[0], 1234);
	return true;
}

bool
InnerKeysAreCalledOnInnerDispatchTable() {
	theWasCalled = false;
	theDispatch = {};
	theIntValues = {};
	theStringValues = {};

	theDispatch.parseInt = [](int aValue) { theWasCalled = true; };
	theDispatch.parseString = [](const auto* aValue) { theWasCalled = true; };
	theDispatch.getKeyHandler = [](const auto* aKey) {
		theDispatch.parseInt = [](int aValue) { theIntValues.insert({theCurrentKey, aValue}); };
		theDispatch.parseString = [](const auto* aValue) {
			theStringValues.insert({theCurrentKey, aValue});
		};
		theDispatch.getKeyHandler = [](const auto* aKey) { return theDispatch; };
		return theDispatch;
	};

	std::string json = R"json({"key":{"innerKey":1234}})json";
	ParseJson(json.data(), theDispatch);

	CHECK(theWasCalled, false);
	CHECK(theIntValues.size(), 1);
	return true;
}

bool
CanHandlerArrayOfObjects() {
	LocDefaultSetupForDispatchTable();

	std::string json =
		R"json({"key":[{"innerKey": 1, "innerStuff": "someString"},{"differentInnerKey": 23423},{}]})json";
	ParseJson(json.data(), theDispatch);

	CHECK(theKeys.size(), 6);
	{
		auto iterator = theIntValues.find("/key/innerKey");
		CHECK(iterator == theIntValues.end(), false);
		CHECK(iterator->second, 1);
	}
	{
		auto iterator = theStringValues.find("/key/innerKey/innerStuff");
		CHECK(iterator == theStringValues.end(), false);
		CHECK(iterator->second, "someString");
	}
	{
		auto iterator = theIntValues.find("/key/innerKey/innerStuff/key/differentInnerKey");
		CHECK(iterator == theIntValues.end(), false);
		CHECK(iterator->second, 23423);
	}
	return true;
}