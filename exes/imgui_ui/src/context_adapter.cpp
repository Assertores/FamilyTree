#include "context_adapter.hpp"

#include <family_tree/api.h>

#include <any>

#define FILL_API_FIELD(aName) \
	result.aName = aElement.aName.has_value() ? aElement.aName.value().c_str() : nullptr

#define RETREAVE_API_FIELD(aName) \
	if (IsDefaultString(aContext, aElement.aName) == 0) result.aName = aElement.aName

namespace ui {
namespace {
template <class T>
class Managed {
public:
	using DataModel = std::shared_ptr<std::vector<std::any>>;

	template <class... Args>
	Managed(T aElement, Args&&... aData); // NOLINT

	operator T(); // NOLINT

	DataModel TransfareData();

private:
	T myElement;
	DataModel myData;
};

template <class T>
template <class... Args>
Managed<T>::Managed(T aElement, Args&&... aData)
	: myElement(std::move(aElement))
	, myData(new std::vector<std::any>{std::any(std::forward<Args>(aData))...}) {}

template <class T>
Managed<T>::operator T() {
	return myElement;
}

template <class T>
typename Managed<T>::DataModel
Managed<T>::TransfareData() {
	auto result = std::move(myData);
	myData.reset();
	return result;
}

// NOTE: livetime of original needs to superseed api version
Managed<::Residence>
ToApi(const ui::Residence& aElement) {
	::Residence result{};

	FILL_API_FIELD(name);
	FILL_API_FIELD(startDate);
	FILL_API_FIELD(endDate);

	return {result};
}

// NOTE: livetime of original needs to superseed api version
::Person
// TODO: fix Memory model
ToApi(
	const ui::Person& aElement,
	std::vector<const char*>& aData1,
	std::vector<const char*>& aData2,
	std::vector<const char*>& aData3,
	std::vector<::Residence>& aData4) {
	::Person result{};

	result.id = aElement.id;
	FILL_API_FIELD(title);
	aData1.reserve(aElement.firstNames.size());
	for (const auto& value : aElement.firstNames) {
		if (value.empty()) {
			continue;
		}
		aData1.emplace_back(value.c_str());
	}
	result.firstNameCount = aData1.size();
	result.firstNames = aData1.data();
	FILL_API_FIELD(titleOfNobility);
	aData2.reserve(aElement.lastNames.size());
	for (const auto& value : aElement.lastNames) {
		if (value.empty()) {
			continue;
		}
		aData2.emplace_back(value.c_str());
	}
	result.lastNameCount = aData2.size();
	result.lastNames = aData2.data();
	FILL_API_FIELD(gender);
	FILL_API_FIELD(dateOfBirth);
	FILL_API_FIELD(placeOfBirth);
	FILL_API_FIELD(dateOfDeath);
	FILL_API_FIELD(placeOfDeath);
	aData3.reserve(aElement.professions.size());
	for (const auto& value : aElement.professions) {
		if (value.empty()) {
			continue;
		}
		aData3.emplace_back(value.c_str());
	}
	result.professionCount = aData3.size();
	result.professions = aData3.data();

	aData4.reserve(aElement.placeOfResidences.size());
	for (const auto& value : aElement.placeOfResidences) {
		auto element = ToApi(value);
		aData4.emplace_back(element);
	}
	result.placeOfResidenceCount = aData4.size();
	result.placeOfResidences = aData4.data();
	FILL_API_FIELD(remark);

	return result;
}

ui::Residence
FromApi(::Residence aElement, ::Context* aContext) {
	ui::Residence result{};

	RETREAVE_API_FIELD(name);
	RETREAVE_API_FIELD(startDate);
	RETREAVE_API_FIELD(endDate);

	return result;
}

ui::Person
FromApi(::Person aElement, ::Context* aContext) {
	ui::Person result{};

	result.id = aElement.id;
	RETREAVE_API_FIELD(title);
	for (int i = 0; i < aElement.firstNameCount; i++) {
		result.firstNames.emplace_back(aElement.firstNames[i]);
	}
	RETREAVE_API_FIELD(titleOfNobility);
	for (int i = 0; i < aElement.lastNameCount; i++) {
		result.lastNames.emplace_back(aElement.lastNames[i]);
	}
	RETREAVE_API_FIELD(gender);
	RETREAVE_API_FIELD(dateOfBirth);
	RETREAVE_API_FIELD(placeOfBirth);
	RETREAVE_API_FIELD(dateOfDeath);
	RETREAVE_API_FIELD(placeOfDeath);
	for (int i = 0; i < aElement.professionCount; i++) {
		result.professions.emplace_back(aElement.professions[i]);
	}
	for (int i = 0; i < aElement.placeOfResidenceCount; i++) {
		result.placeOfResidences.emplace_back(FromApi(aElement.placeOfResidences[i], aContext));
	}
	RETREAVE_API_FIELD(remark);

	return result;
}

class FullContextAdapter : public ContextAdapter {
public:
	explicit FullContextAdapter(std::filesystem::path aPath);

	std::vector<Person> GetPersonsMatchingPattern(Person aPrototype, size_t aMinMatches) override;
	void ShowImagesOfPerson(PersonId aId) override;
	void PlayPerson(PersonId aId) override;

private:
	Context* myContext{};
};

FullContextAdapter::FullContextAdapter(std::filesystem::path aPath) {
	myContext = CreateWithCSVAndJSON(aPath.u8string().c_str(), nullptr);
}

std::vector<Person>
FullContextAdapter::GetPersonsMatchingPattern(Person aPrototype, size_t aMinMatches) {
	std::vector<const char*> data1;
	std::vector<const char*> data2;
	std::vector<const char*> data3;
	std::vector<::Residence> data4;
	auto prototype = ToApi(aPrototype, data1, data2, data3, data4);
	size_t count = 0;

	auto* cArray = ::GetPersonsMatchingPattern(myContext, prototype, aMinMatches, &count, nullptr);

	std::vector<Person> result;
	result.reserve(count);
	for (size_t i = 0; i < count; i++) {
		result.emplace_back(FromApi(cArray[i], myContext));
	}

	return result;
}

void
FullContextAdapter::ShowImagesOfPerson(PersonId aId) {
	::ShowImagesOfPerson(myContext, aId, nullptr);
}

void
FullContextAdapter::PlayPerson(PersonId aId) {
	::PlayPerson(myContext, aId, nullptr);
}
} // namespace

std::shared_ptr<ContextAdapter>
ContextAdapter::Create(std::filesystem::path aPath) {
	return std::make_shared<FullContextAdapter>(std::move(aPath));
}
} // namespace ui
