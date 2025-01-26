#pragma once

#include <functional>
#include <memory>

namespace ui {
class View {
public:
	using WindowFactory = std::function<void(std::shared_ptr<View>)>;

	virtual ~View() = default;
	virtual void Enter() {};
	virtual std::shared_ptr<View> Print(WindowFactory aWindowFactory) = 0;
	virtual void Exit() {};
};
} // namespace ui
