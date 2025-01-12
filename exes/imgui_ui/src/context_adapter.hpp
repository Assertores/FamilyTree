#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <vector>

namespace ui {
using String = std::optional<std::string>;

// NOTE: copy from family_tree/types.h header sothat it doesn't need to be included here.
using PersonId = size_t;

struct Residence {
	String name;
	String startDate;
	String endDate;
};

struct Person {
	PersonId id;
	String title;
	std::vector<std::string> firstNames;
	String titleOfNobility;
	std::vector<std::string> lastNames;
	String gender;
	String dateOfBirth;
	String placeOfBirth;
	String dateOfDeath;
	String placeOfDeath;
	std::vector<std::string> professions;
	std::vector<Residence> placeOfResidences;
	String remark;
};

class ContextAdapter {
public:
	static std::shared_ptr<ContextAdapter> Create(std::filesystem::path aPath);
	virtual ~ContextAdapter() = default;

	virtual std::vector<Person> GetPersonsMatchingPattern(
		Person aPrototype,
		size_t aMinMatches) = 0;
	virtual void ShowImagesOfPerson(PersonId aId) = 0;
	virtual void PlayPerson(PersonId aId) = 0;
};
} // namespace ui
