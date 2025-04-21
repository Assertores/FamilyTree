#pragma once

#include "view.hpp"

#include <familytree_adapter/context_adapter.hpp>

namespace ui {
class DetaildView : public View {
public:
	DetaildView(std::shared_ptr<ContextAdapter> aContext, Person aPerson);
	std::shared_ptr<View> Print(WindowFactory aWindowFactory) override;

private:
	std::shared_ptr<ContextAdapter> myContext;
	Person myPerson;

	void PrivShowName();
	void PrivShowDates();
	void PrivShowProfessions();
	void PrivShowResidence();
	void PrivShowRemarks();
};
} // namespace ui
