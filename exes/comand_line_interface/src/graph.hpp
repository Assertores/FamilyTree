#pragma once

#include <family_tree/types.h>

#include <limits>
#include <map>
#include <memory>
#include <vector>
#include <functional>

namespace graph {
class IPort {
public:
	virtual ~IPort() = default;

	[[nodiscard]] virtual size_t GetPortPosition() = 0;
};

class IGraphElement {
public:
	virtual ~IGraphElement() = default;

	[[nodiscard]] virtual bool PrintNextLine(std::vector<char>& aCanvas) = 0;
};

class IPerson : public IGraphElement {
public:
	static std::shared_ptr<IPerson> CreatePerson(Person aPerson, std::function<bool(const char*)> aIsDefaultString);

	[[nodiscard]] virtual IPort* CreatePort(bool aOnTop) = 0;
	[[nodiscard]] virtual size_t GetEndOfElement() = 0;
	virtual void SetPreviousPort(IPerson* aPort);
	virtual void SetNextElement(IPerson* aElement);

protected:
	IPerson* myPreviousPort = nullptr; // NOLINT
	IPerson* myNextElement = nullptr;  // NOLINT
};

class IFamilie : public IGraphElement {
public:
	static std::shared_ptr<IFamilie> CreateProxy();

	virtual void SetNextFamilie(std::shared_ptr<IFamilie> aFamilie) = 0;
	virtual void InsertPorts(std::vector<char>& aCanvas) = 0;
};

class FamilieBuilder {
public:
	void AddParent(IPerson* aParent, int aGeneration);
	void AddChild(IPerson* aChild, int aGeneration);

	[[nodiscard]] bool operator<(const FamilieBuilder& aOther) const;

	[[nodiscard]] std::shared_ptr<IPerson> BuildGeneration(
		int aGeneration,
		IFamilie& aFamilieUnderneathGeneration) const;

private:
	mutable std::shared_ptr<IPerson> myProxy = nullptr;
	int myMin = std::numeric_limits<int>::max();
	int myMax = std::numeric_limits<int>::min();

	std::map<int, std::vector<IPerson*>> myParents;
	std::map<int, std::vector<IPerson*>> myChildrens;
};
} // namespace graph