#pragma once

#include "view.hpp"

namespace ui {
class SetupView final : public View {
public:
	std::shared_ptr<View> Print() override;
};
} // namespace ui
