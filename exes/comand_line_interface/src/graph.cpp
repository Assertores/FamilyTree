#include "graph.hpp"

#include <algorithm>
#include <iostream>
#include <set>
#include <string>

namespace graph {
namespace {
class PersonPort : public IPort {
public:
	[[nodiscard]] size_t GetPortPosition() override { return myPos; }

	void SetPosition(size_t aNewPos) { myPos = aNewPos; }

private:
	size_t myPos = 0;
};

class Person : public IPerson {
public:
	Person(::Person aPerson, std::function<bool(const char*)> aIsDefaultString);

	[[nodiscard]] bool PrintNextLine(std::vector<char>& aCanvas) override;
	[[nodiscard]] IPort* CreatePort(bool aOnTop) override;
	[[nodiscard]] size_t GetEndOfElement() override;
	void SetPreviousPort(IPerson* aPort) override;

private:
	// TODO: change this with real data eventualy
	size_t myWidth = 0;
	size_t myHight = 0;
	std::vector<std::string> myContent;

	size_t myCurrentLine = 0;

	using Ports = std::vector<std::shared_ptr<PersonPort>>;
	Ports myTopPorts;
	Ports myBottomPorts;
};

class FamilieConnector
	: public IPerson
	, public IPort {
public:
	[[nodiscard]] IPort* CreatePort(bool aOnTop) override;
	[[nodiscard]] bool PrintNextLine(std::vector<char>& aCanvas) override;
	[[nodiscard]] size_t GetEndOfElement() override;
	[[nodiscard]] size_t GetPortPosition() override;
};

class Familie : public IFamilie {
public:
	[[nodiscard]] bool PrintNextLine(std::vector<char>& aCanvas) override;
	void SetNextFamilie(std::shared_ptr<IFamilie> aFamilie) override;
	void InsertPorts(std::vector<char>& aCanvas) override;

	void AddParent(IPort* aParent);
	void AddChild(IPort* aChild);

private:
	std::shared_ptr<IFamilie> myNextFamilie = nullptr;
	bool myHasBeenPrinted = false;

	std::vector<IPort*> myParents;
	std::vector<IPort*> myChildrens;
};

class ProxyFamilie : public IFamilie {
public:
	[[nodiscard]] bool PrintNextLine(std::vector<char>& aCanvas) override;
	void SetNextFamilie(std::shared_ptr<IFamilie> aFamilie) override;
	void InsertPorts(std::vector<char>& aCanvas) override;

private:
	std::shared_ptr<IFamilie> myRealFamilie = nullptr;
};

Person::Person(::Person aPerson, std::function<bool(const char*)> aIsDefaultString) {
	myContent.emplace_back(std::to_string(aPerson.id));

	std::string line{};
	if (!aIsDefaultString(aPerson.title)) {
		line += aPerson.title;
		line += ' ';
	}
	line += aPerson.firstNames[0];
	if (!aIsDefaultString(aPerson.titleOfNobility)) {
		line += ' ';
		line += aPerson.titleOfNobility;
	}
	line += ' ';
	line += aPerson.lastNames[0];
	if (!line.empty()) {
		myContent.emplace_back(line);
		line.clear();
	}

	if (!aIsDefaultString(aPerson.dateOfBirth)) {
		line += "* ";
		line += aPerson.dateOfBirth;
		if (!aIsDefaultString(aPerson.placeOfBirth)) {
			line += " (";
			line += aPerson.placeOfBirth;
			line += ')';
		}
	}
	if (!line.empty()) {
		myContent.emplace_back(line);
		line.clear();
	}

	if (!aIsDefaultString(aPerson.dateOfDeath)) {
		line += "* ";
		line += aPerson.dateOfDeath;
		if (!aIsDefaultString(aPerson.placeOfDeath)) {
			line += " (";
			line += aPerson.placeOfDeath;
			line += ')';
		}
	}
	if (!line.empty()) {
		myContent.emplace_back(line);
		line.clear();
	}

	myHight = myContent.size() + 4;
	for (const auto& line : myContent) {
		myWidth = std::max(myWidth, line.size());
	}
	myWidth += 2;
}

bool
Person::PrintNextLine(std::vector<char>& aCanvas) {
	if (myCurrentLine == 0 || myCurrentLine >= myHight - 1) {
		auto start = aCanvas.size();
		Ports& ports = myCurrentLine == 0 ? myTopPorts : myBottomPorts;
		for (size_t i = 0; i < myWidth; i++) {
			auto isPort = std::any_of(ports.begin(), ports.end(), [&](const auto& aElement) {
				return aElement->GetPortPosition() == start + i;
			});
			aCanvas.push_back(isPort ? '|' : ' ');
		}
	} else if (myCurrentLine == 1 || myCurrentLine == myHight - 2) {
		auto start = aCanvas.size();
		aCanvas.push_back('+');

		Ports& ports = myCurrentLine == 1 ? myTopPorts : myBottomPorts;
		for (size_t i = 1; i < myWidth - 1; i++) {
			auto isPort = std::any_of(ports.begin(), ports.end(), [&](const auto& aElement) {
				return aElement->GetPortPosition() == start + i;
			});
			aCanvas.push_back(isPort ? '+' : '-');
		}

		aCanvas.push_back('+');
	} else {
		aCanvas.push_back('|');
		aCanvas.insert(
			aCanvas.end(),
			myContent[myCurrentLine - 2].begin(),
			myContent[myCurrentLine - 2].end());
		aCanvas.insert(aCanvas.end(), myWidth - 2 - myContent[myCurrentLine - 2].length(), ' ');
		aCanvas.push_back('|');
	}
	myCurrentLine++;

	bool notDone = false;
	if (myNextElement != nullptr) {
		notDone = myNextElement->PrintNextLine(aCanvas);
	}
	return notDone || myCurrentLine < myHight;
}

IPort*
Person::CreatePort(bool aOnTop) {
	auto newPort = std::make_shared<PersonPort>();

	Ports& ports = aOnTop ? myTopPorts : myBottomPorts;
	ports.push_back(newPort);

	size_t start = 0;
	if (myPreviousPort != nullptr) {
		start = myPreviousPort->GetEndOfElement();
	}

	auto offset = start + (myWidth / 2) - ports.size();
	if (offset % 2 == (aOnTop ? 0 : 1)) {
		offset++;
	}
	for (size_t i = 0; i < ports.size(); i++) {
		ports[i]->SetPosition(offset + i);
	}

	return newPort.get();
}

size_t
Person::GetEndOfElement() {
	if (myPreviousPort == nullptr) {
		return myWidth;
	}
	return myPreviousPort->GetEndOfElement() + myWidth;
}

void
Person::SetPreviousPort(IPerson* aPort) {
	IPerson::SetPreviousPort(aPort);

	// TODO: might still be broken if person connections aren't set up in order
	auto start = myPreviousPort->GetEndOfElement();
	auto topOffset = start + (myWidth / 2) - myTopPorts.size();
	auto bottomOffset = start + (myWidth / 2) - myBottomPorts.size();
	if (topOffset % 2 == 0) {
		topOffset++;
	}
	if (bottomOffset % 2 == 1) {
		bottomOffset++;
	}
	for (size_t i = 0; i < myTopPorts.size(); i++) {
		myTopPorts[i]->SetPosition(topOffset + (i * 2));
	}
	for (size_t i = 0; i < myBottomPorts.size(); i++) {
		myBottomPorts[i]->SetPosition(bottomOffset + (i * 2));
	}
}

IPort*
FamilieConnector::CreatePort(bool aOnTop) {
	return this;
}

bool
FamilieConnector::PrintNextLine(std::vector<char>& aCanvas) {
	aCanvas.push_back('|');
	if (myNextElement == nullptr) {
		return false;
	}
	return myNextElement->PrintNextLine(aCanvas);
}

size_t
FamilieConnector::GetEndOfElement() {
	size_t pos = 0;
	if (myPreviousPort != nullptr) {
		pos = myPreviousPort->GetEndOfElement();
	}
	return pos + 1;
}

size_t
FamilieConnector::GetPortPosition() {
	return GetEndOfElement() - 1;
}

bool
Familie::PrintNextLine(std::vector<char>& aCanvas) {
	if (myHasBeenPrinted) {
		for (const auto& port : myChildrens) {
			auto pos = port->GetPortPosition();
			if (pos >= aCanvas.size()) {
				aCanvas.resize(pos + 1, ' ');
			}
			aCanvas[pos] = '|';
		}
		return myNextFamilie->PrintNextLine(aCanvas);
	}
	myHasBeenPrinted = true;

	std::set<size_t> ports{};
	auto min = std::numeric_limits<size_t>::max();
	auto max = std::numeric_limits<size_t>::min();
	auto work = [&](const auto& aPort) {
		auto pos = aPort->GetPortPosition();
		ports.insert(pos);
		if (pos > max) {
			max = pos;
		}
		if (pos < min) {
			min = pos;
		}
	};

	for (const auto& port : myParents) {
		work(port);
	}
	for (const auto& port : myChildrens) {
		work(port);
	}

	if (max >= aCanvas.size()) {
		aCanvas.resize(max + 1, ' ');
	}
	for (size_t i = min; i <= max; i++) {
		if (ports.find(i) == ports.end()) {
			if (aCanvas[i] == '|') {
				aCanvas[i] = ')';
			} else {
				aCanvas[i] = '-';
			}
		} else {
			aCanvas[i] = '+';
		}
	}

	if (myNextFamilie != nullptr) {
		myNextFamilie->InsertPorts(aCanvas);
	}
	return myNextFamilie != nullptr;
}

void
Familie::SetNextFamilie(std::shared_ptr<IFamilie> aFamilie) {
	if (myNextFamilie == nullptr) {
		myNextFamilie = std::move(aFamilie);
		return;
	}
	myNextFamilie->SetNextFamilie(std::move(aFamilie));
}

void
Familie::InsertPorts(std::vector<char>& aCanvas) {
	for (const auto& port : myParents) {
		auto pos = port->GetPortPosition();
		if (pos >= aCanvas.size()) {
			aCanvas.resize(pos + 1, ' ');
		}
		if (aCanvas[pos] == ' ') {
			aCanvas[pos] = '|';
		} else if (aCanvas[pos] == '-') {
			aCanvas[pos] = ')';
		}
	}
	if (myNextFamilie != nullptr) {
		myNextFamilie->InsertPorts(aCanvas);
	}
}

void
Familie::AddParent(IPort* aParent) {
	myParents.emplace_back(aParent);
}

void
Familie::AddChild(IPort* aChild) {
	myChildrens.emplace_back(aChild);
}

[[nodiscard]] bool
ProxyFamilie::PrintNextLine(std::vector<char>& aCanvas) {
	if (myRealFamilie != nullptr) {
		return myRealFamilie->PrintNextLine(aCanvas);
	}
	return false;
}

void
ProxyFamilie::SetNextFamilie(std::shared_ptr<IFamilie> aFamilie) {
	if (myRealFamilie == nullptr) {
		myRealFamilie = std::move(aFamilie);
		return;
	}
	myRealFamilie->SetNextFamilie(std::move(aFamilie));
}

void
ProxyFamilie::InsertPorts(std::vector<char>& aCanvas) {
	if (myRealFamilie != nullptr) {
		myRealFamilie->InsertPorts(aCanvas);
	}
}
} // namespace

std::shared_ptr<IPerson>
IPerson::CreatePerson(::Person aPerson, std::function<bool(const char*)> aIsDefaultString) {
	return std::make_shared<graph::Person>(aPerson, std::move(aIsDefaultString));
}

void
IPerson::SetPreviousPort(IPerson* aPort) {
	myPreviousPort = aPort;
}

void
IPerson::SetNextElement(IPerson* aElement) {
	myNextElement = aElement;
}

std::shared_ptr<IFamilie>
IFamilie::CreateProxy() {
	return std::make_shared<ProxyFamilie>();
}

void
FamilieBuilder::AddParent(IPerson* aParent, int aGeneration) {
	myParents[aGeneration].push_back(aParent);
	myMin = std::min(myMin, aGeneration);
	myMax = std::max(myMax, aGeneration);
}

void
FamilieBuilder::AddChild(IPerson* aChild, int aGeneration) {
	myChildrens[aGeneration].push_back(aChild);
	myMin = std::min(myMin, aGeneration);
	myMax = std::max(myMax, aGeneration);
}

bool
FamilieBuilder::operator<(const FamilieBuilder& aOther) const {
	if (myParents != aOther.myParents) {
		return myParents < aOther.myParents;
	}
	if (myChildrens != aOther.myChildrens) {
		return myChildrens < aOther.myChildrens;
	}
	return false;
}

std::shared_ptr<IPerson>
FamilieBuilder::BuildGeneration(int aGeneration, IFamilie& aFamilieUnderneathGeneration) const {
	if (aGeneration < myMin || aGeneration >= myMax) {
		return nullptr;
	}
	auto proxy = std::move(myProxy);

	bool isEmpty = true;

	auto familie = std::make_shared<Familie>();
	if (proxy != nullptr) {
		familie->AddParent(proxy->CreatePort(false));
		isEmpty = false;
	}

	auto parents = myParents.find(aGeneration);
	if (parents != myParents.end()) {
		for (const auto& parent : parents->second) {
			familie->AddParent(parent->CreatePort(false));
			isEmpty = false;
		}
	}

	auto children = myChildrens.find(aGeneration + 1);
	if (children != myChildrens.end()) {
		for (const auto& child : children->second) {
			familie->AddChild(child->CreatePort(true));
			isEmpty = false;
		}
	}

	if (aGeneration + 1 < myMax) {
		myProxy = std::make_shared<FamilieConnector>();
		familie->AddChild(myProxy->CreatePort(true));
		isEmpty = false;
	}

	if (isEmpty) {
		return nullptr;
	}

	aFamilieUnderneathGeneration.SetNextFamilie(std::move(familie));

	return proxy;
}
} // namespace graph