#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace ui {
class Entry {
public:
	virtual ~Entry() = default;

	virtual void Show() const = 0;
};

class Trace : public Entry {
public:
	explicit Trace(std::filesystem::path aPath);

	[[nodiscard]] std::string GetName() const;

	void Show() const override;

private:
	std::string myName;
	std::vector<std::shared_ptr<Entry>> myEntries;
	bool myIsValid = false;
};
} // namespace ui
