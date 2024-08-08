#pragma once

bool NullptrWillNotDoAnything();
bool EmptyStringWillNotDoAnything();
bool EmptyJsonWillNotCallParsingOnTheHandler();
bool AKeyIsSuccessfulyParsed();
bool StringValueIsNotConsideredAKey();
bool MultipleKeysAreDetected();
bool StringValueParserIsCalled();
bool IntParserIsNotCalledForStringValue();
bool IntValueParserIsCalled();
bool IgnoresWhitespaceCharacters();
bool CanDealWithArrayOfInts();
bool CallsKeyHandlerOnlyOnceForArray();
bool ArrayCanCountainStrings();
bool CanDealWithObjectInObject();
bool InnerKeysAreCalledOnInnerDispatchTable();