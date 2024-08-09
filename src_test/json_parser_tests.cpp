#include "json_parser_tests.hpp"

#include "check.hpp"

#include <json_parser.h>

#include <string>
#include <vector>

static bool theWasCalled = false;
JsonParsingDispatchTable theDispatch{};
std::vector<std::string> theKeys{};
std::vector<std::string> theStringValues{};
std::vector<int> theIntValues{};

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
		theWasCalled = false;
		theDispatch = {};
		theKeys = {};

		theDispatch.parseInt = [](auto) { theWasCalled = true; };
		theDispatch.parseString = [](auto) { theWasCalled = true; };
		theDispatch.getKeyHandler = [](const auto* aKey) {
			theKeys.emplace_back(aKey);
			return theDispatch;
		};

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
	theWasCalled = false;
	theDispatch = {};
	theKeys = {};

	theDispatch.parseInt = [](auto) { theWasCalled = true; };
	theDispatch.parseString = [](auto) { theWasCalled = true; };
	theDispatch.getKeyHandler = [](const auto* aKey) {
		theKeys.emplace_back(aKey);
		return theDispatch;
	};

	std::string json = R"json({"key":"value"})json";
	ParseJson(json.data(), theDispatch);

	CHECK(theKeys.size(), 1);
	CHECK(theKeys[0], "key");
	return true;
}

bool
MultipleKeysAreDetected() {
	theWasCalled = false;
	theDispatch = {};
	theKeys = {};

	theDispatch.parseInt = [](auto) { theWasCalled = true; };
	theDispatch.parseString = [](auto) { theWasCalled = true; };
	theDispatch.getKeyHandler = [](const auto* aKey) {
		theKeys.emplace_back(aKey);
		return theDispatch;
	};

	std::string json = R"json({"key":"value","key2":234})json";
	ParseJson(json.data(), theDispatch);

	CHECK(theKeys.size(), 2);
	CHECK(theKeys[0], "key");
	CHECK(theKeys[1], "key2");
	return true;
}

bool
StringValueParserIsCalled() {
	theWasCalled = false;
	theDispatch = {};
	theKeys = {};
	theStringValues = {};

	theDispatch.parseInt = [](auto) { theWasCalled = true; };
	theDispatch.parseString = [](const auto* aValue) {
		theStringValues.emplace_back(aValue);
		theWasCalled = true;
	};
	theDispatch.getKeyHandler = [](const auto* aKey) {
		theKeys.emplace_back(aKey);
		return theDispatch;
	};

	std::string json = R"json({"key":"value"})json";
	ParseJson(json.data(), theDispatch);

	CHECK(theKeys.size(), 1);
	CHECK(theKeys[0], "key");
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
	theWasCalled = false;
	theDispatch = {};
	theIntValues = {};

	theDispatch.parseInt = [](int aValue) { theIntValues.emplace_back(aValue); };
	theDispatch.parseString = [](const auto* aValue) {};
	theDispatch.getKeyHandler = [](const auto* aKey) { return theDispatch; };

	std::string json = R"json({"key":1234})json";
	ParseJson(json.data(), theDispatch);

	CHECK(theIntValues.size(), 1);
	CHECK(theIntValues[0], 1234);
	return true;
}

bool
IgnoresWhitespaceCharacters() {
	theDispatch = {};
	theKeys = {};
	theStringValues = {};
	theIntValues = {};

	theDispatch.parseInt = [](int aValue) { theIntValues.emplace_back(aValue); };
	theDispatch.parseString = [](const auto* aValue) { theStringValues.emplace_back(aValue); };
	theDispatch.getKeyHandler = [](const auto* aKey) {
		theKeys.emplace_back(aKey);
		return theDispatch;
	};

	std::string json = R"json({   "key": "value" 	,
	"key2"            	   :  1234
})json";
	ParseJson(json.data(), theDispatch);

	CHECK(theKeys.size(), 2);
	CHECK(theKeys[0], "key");
	CHECK(theKeys[1], "key2");
	CHECK(theStringValues.size(), 1);
	CHECK(theStringValues[0], "value");
	CHECK(theIntValues.size(), 1);
	CHECK(theIntValues[0], 1234);
	return true;
}

bool
CanDealWithArrayOfInts() {
	theWasCalled = false;
	theDispatch = {};
	theIntValues = {};

	theDispatch.parseInt = [](int aValue) { theIntValues.emplace_back(aValue); };
	theDispatch.parseString = [](const auto* aValue) {};
	theDispatch.getKeyHandler = [](const auto* aKey) { return theDispatch; };

	std::string json = R"json({"key":[1234,22,333]})json";
	ParseJson(json.data(), theDispatch);

	CHECK(theIntValues.size(), 3);
	CHECK(theIntValues[0], 1234);
	CHECK(theIntValues[1], 22);
	CHECK(theIntValues[2], 333);
	return true;
}

bool
CallsKeyHandlerOnlyOnceForArray() {
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

	CHECK(theKeys.size(), 1);
	CHECK(theKeys[0], "key");
	return true;
}

bool
ArrayCanCountainStrings() {
	theWasCalled = false;
	theDispatch = {};
	theIntValues = {};
	theStringValues = {};

	theDispatch.parseInt = [](int aValue) { theIntValues.emplace_back(aValue); };
	theDispatch.parseString = [](const auto* aValue) { theStringValues.emplace_back(aValue); };
	theDispatch.getKeyHandler = [](const auto* aKey) { return theDispatch; };

	std::string json = R"json({"key":[1234,"abcd",333]})json";
	ParseJson(json.data(), theDispatch);

	CHECK(theIntValues.size(), 2);
	CHECK(theIntValues[0], 1234);
	CHECK(theIntValues[1], 333);
	CHECK(theStringValues.size(), 1);
	CHECK(theStringValues[0], "abcd");
	return true;
}

bool
ArrayCanCountainMultipleStrings() {
	theWasCalled = false;
	theDispatch = {};
	theIntValues = {};
	theStringValues = {};

	theDispatch.parseInt = [](int aValue) { theIntValues.emplace_back(aValue); };
	theDispatch.parseString = [](const auto* aValue) { theStringValues.emplace_back(aValue); };
	theDispatch.getKeyHandler = [](const auto* aKey) { return theDispatch; };

	std::string json = R"json({"key":["hello","abcd",333]})json";
	ParseJson(json.data(), theDispatch);

	CHECK(theIntValues.size(), 1);
	CHECK(theIntValues[0], 333);
	CHECK(theStringValues.size(), 2);
	CHECK(theStringValues[0], "hello");
	CHECK(theStringValues[1], "abcd");
	return true;
}

bool
CanDealWithObjectInObject() {
	theWasCalled = false;
	theDispatch = {};
	theKeys = {};
	theIntValues = {};
	theStringValues = {};

	theDispatch.parseInt = [](int aValue) { theIntValues.emplace_back(aValue); };
	theDispatch.parseString = [](const auto* aValue) { theStringValues.emplace_back(aValue); };
	theDispatch.getKeyHandler = [](const auto* aKey) {
		theKeys.emplace_back(aKey);
		return theDispatch;
	};

	std::string json = R"json({"key":{"innerKey":1234}})json";
	ParseJson(json.data(), theDispatch);

	CHECK(theKeys.size(), 2);
	CHECK(theKeys[0], "key");
	CHECK(theKeys[1], "innerKey");
	CHECK(theIntValues.size(), 1);
	CHECK(theIntValues[0], 1234);
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
		theDispatch.parseInt = [](int aValue) { theIntValues.emplace_back(aValue); };
		theDispatch.parseString = [](const auto* aValue) { theStringValues.emplace_back(aValue); };
		theDispatch.getKeyHandler = [](const auto* aKey) { return theDispatch; };
		return theDispatch;
	};

	std::string json = R"json({"key":{"innerKey":1234}})json";
	ParseJson(json.data(), theDispatch);

	CHECK(theWasCalled, false);
	CHECK(theIntValues.size(), 1);
	return true;
}