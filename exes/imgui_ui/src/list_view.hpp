#pragma once

#include "view.hpp"

#include <context_adapter.hpp>

#include <array>
#include <string>
#include <vector>

namespace ui {
class ListView final : public View {
public:
	ListView(std::shared_ptr<ContextAdapter> aContext, std::vector<Person> aSearchResults, std::shared_ptr<View> aClearView);
	std::shared_ptr<View> Print() override;

private:
	std::shared_ptr<ContextAdapter> myContext;
	std::vector<Person> mySearchResults;
	std::shared_ptr<View> myClearView;

	void PrivShowPerson(const Person& aPerson);

	void PrivShowName(const Person& aPerson);
	void PrivShowDates(const Person& aPerson);
	void PrivShowProfessions(const Person& aPerson);
	void PrivShowResidence(const Person& aPerson);
	void PrivShowRemarks(const Person& aPerson);
};
} // namespace ui
