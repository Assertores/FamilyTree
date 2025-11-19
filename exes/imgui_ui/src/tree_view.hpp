#pragma once

#include "view.hpp"

#include <familytree_adapter/context_adapter.hpp>

#include <map>

namespace ui {
class TreeView final
	: public View
	, public IGraphBuilderStrategy {
public:
	TreeView(
		std::shared_ptr<ContextAdapter> aContext,
		PersonId aPerson,
		size_t aDistance,
		std::shared_ptr<View> aBackView);

	void Enter() override;
	std::shared_ptr<View> Print(WindowFactory aWindowFactory) override;

	void PersonInGenerationStrategy(PersonId aId, int aGeneration) override;
	void FamilieStrategy(std::vector<PersonId> aParents, std::vector<PersonId> aChilds) override;

private:
	struct LocPerson {
	public:
		float myX = 0;
		float myY = 0;
		Person myPersonData{};

		void Print() const;
	};
	struct Family {
		Family(std::vector<PersonId> aParents, std::vector<PersonId> aChildrens)
			: myParents(std::move(aParents))
			, myChildrens(std::move(aChildrens)) {}
		std::vector<PersonId> myParents;
		std::vector<PersonId> myChildrens;

		[[nodiscard]] bool operator==(const Family& aOther) const;

		float myLeft = 0;
		float myRight = 0;
		float myHorizontalLine = 0;
	};

	std::map<PersonId, LocPerson> myPersons;
	std::vector<Family> myFamilies;

	float myMinNumber = 0;
	bool myFirstFrame = true;

	std::shared_ptr<ContextAdapter> myContext;
	PersonId myPerson;
	size_t myDistance;
	std::shared_ptr<View> myBackView;

	void PrivCalculateFamiltyMetadata();
};
} // namespace ui
