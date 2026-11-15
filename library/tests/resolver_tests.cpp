// Unit tests of the Resolver: Parents, Children, Siblings, Partners and Generations, derived from
// the Relations when a Context is created.

#include "resolver_tests.hpp"

#include "support/test_support.hpp"

#include <check.hpp>

#include <string>
#include <vector>

namespace {

using namespace test;

using Ids = std::vector<FT_PersonId>;

// A Context with Persons 1 to aCount and the given rows of the Relations File.
struct Family {
	DataFolder folder;
	ContextGuard context;
	TraceBuffer trace;
	FT_Result result = FT_ERROR_INVALID_ARGUMENT;

	Family(FT_PersonId aCount, const std::vector<std::string>& aRows) {
		for (FT_PersonId id = 1; id <= aCount; ++id) {
			folder.AddPerson(std::to_string(id), PersonFile(id));
		}
		folder.SetRelations(aRows);
		result = Create(folder, context, trace);
	}

	using View =
		FT_Result (*)(const FT_Context*, FT_PersonId, const FT_PersonId**, size_t*, FT_Trace*);

	Ids Get(View aView, FT_PersonId aPerson) const {
		const FT_PersonId* ids = nullptr;
		size_t count = 0;
		if (aView(context.Get(), aPerson, &ids, &count, nullptr) != FT_OK) {
			return {-1};
		}
		return Sorted(ids, count);
	}

	[[nodiscard]] Ids Parents(FT_PersonId aPerson) const { return Get(FT_GetParents, aPerson); }
	[[nodiscard]] Ids Children(FT_PersonId aPerson) const { return Get(FT_GetChildren, aPerson); }
	[[nodiscard]] Ids Siblings(FT_PersonId aPerson) const { return Get(FT_GetSiblings, aPerson); }
	[[nodiscard]] Ids Partners(FT_PersonId aPerson) const { return Get(FT_GetPartners, aPerson); }

	[[nodiscard]] Ids CommonParents(const Ids& aPersons) const {
		std::vector<FT_PersonId> buffer(Capacity());
		size_t count = 0;
		if (FT_GetCommonParents(
				context.Get(),
				aPersons.data(),
				aPersons.size(),
				buffer.data(),
				buffer.size(),
				&count,
				nullptr)
			!= FT_OK) {
			return {-1};
		}
		return Sorted(buffer.data(), count);
	}

	[[nodiscard]] Ids CommonChildren(const Ids& aPersons) const {
		std::vector<FT_PersonId> buffer(Capacity());
		size_t count = 0;
		if (FT_GetCommonChildren(
				context.Get(),
				aPersons.data(),
				aPersons.size(),
				buffer.data(),
				buffer.size(),
				&count,
				nullptr)
			!= FT_OK) {
			return {-1};
		}
		return Sorted(buffer.data(), count);
	}

	[[nodiscard]] int Generation(FT_PersonId aPerson, FT_PersonId aReference) const {
		int generation = -1000;
		if (FT_GetRelativeGeneration(context.Get(), aPerson, aReference, &generation, nullptr)
			!= FT_OK) {
			return -1000;
		}
		return generation;
	}

	[[nodiscard]] size_t Capacity() const {
		size_t count = 0;
		(void)FT_GetPersonCount(context.Get(), &count, nullptr);
		return count;
	}
};

// docs/requirements/req_0018.md
bool
ChildrenComeFromStrictlyLowerRelations() {
	Family family(3, {"1,2,StrictlyLower", "1,3,StrictlyLower"});
	CHECK(family.result, FT_OK);
	CHECK(family.Children(1), Ids({2, 3}));
	CHECK(family.Parents(2), Ids({1}));
	return true;
}

// docs/requirements/req_0018.md
bool
ParentsComeFromStrictlyHigherRelations() {
	Family family(2, {"2,1,StrictlyHigher"});
	CHECK(family.result, FT_OK);
	CHECK(family.Parents(2), Ids({1}));
	CHECK(family.Children(1), Ids({2}));
	return true;
}

// docs/requirements/req_0018.md
bool
UnrestrictedRelationsMakeNoParentsOrChildren() {
	Family family(2, {"1,2,Unrestricted,verheiratet"});
	CHECK(family.result, FT_OK);
	CHECK(family.Children(1), Ids());
	CHECK(family.Parents(2), Ids());
	return true;
}

// docs/requirements/req_0018.md
bool
CommonParentsOfHalfSiblings() {
	// 1 and 2 are the Parents of 4; 1 and 3 are the Parents of 5.
	Family family(
		5,
		{"1,4,StrictlyLower", "2,4,StrictlyLower", "1,5,StrictlyLower", "3,5,StrictlyLower"});
	CHECK(family.result, FT_OK);
	CHECK(family.CommonParents({4, 5}), Ids({1}));
	CHECK(family.CommonParents({4}), Ids({1, 2}));
	return true;
}

// docs/requirements/req_0018.md
bool
CommonChildrenOfTwoParents() {
	Family family(
		5,
		{"1,4,StrictlyLower", "2,4,StrictlyLower", "1,5,StrictlyLower", "3,5,StrictlyLower"});
	CHECK(family.result, FT_OK);
	CHECK(family.CommonChildren({1, 2}), Ids({4}));
	CHECK(family.CommonChildren({1}), Ids({4, 5}));
	CHECK(family.CommonChildren({2, 3}), Ids());
	return true;
}

// docs/requirements/req_0018.md
bool
AnEmptySetHasNoCommonParentsOrChildren() {
	Family family(3, {"1,2,StrictlyLower", "1,3,StrictlyLower"});
	CHECK(family.result, FT_OK);
	CHECK(family.CommonParents({}), Ids());
	CHECK(family.CommonChildren({}), Ids());
	return true;
}

// docs/requirements/req_0019.md
bool
SiblingsShareAtLeastOneParent() {
	Family family(
		5,
		{"1,4,StrictlyLower", "2,4,StrictlyLower", "1,5,StrictlyLower", "3,5,StrictlyLower"});
	CHECK(family.result, FT_OK);
	CHECK(family.Siblings(4), Ids({5}));
	CHECK(family.Siblings(5), Ids({4}));
	return true;
}

// docs/requirements/req_0019.md
bool
APersonWithoutParentsHasNoSiblings() {
	Family family(3, {"1,2,StrictlyLower"});
	CHECK(family.result, FT_OK);
	CHECK(family.Siblings(1), Ids());
	CHECK(family.Siblings(3), Ids());
	return true;
}

// docs/requirements/req_0020.md
bool
PartnersShareAtLeastOneChild() {
	Family family(
		5,
		{"1,4,StrictlyLower", "2,4,StrictlyLower", "1,5,StrictlyLower", "3,5,StrictlyLower"});
	CHECK(family.result, FT_OK);
	CHECK(family.Partners(1), Ids({2, 3}));
	CHECK(family.Partners(2), Ids({1}));
	return true;
}

// docs/requirements/req_0020.md
bool
AnUnrestrictedRelationAloneMakesNoPartners() {
	Family family(2, {"1,2,Unrestricted,bester Freund"});
	CHECK(family.result, FT_OK);
	CHECK(family.Partners(1), Ids());
	return true;
}

// docs/requirements/req_0017.md
bool
RelationsOfAPersonContainItAsFirstOrSecond() {
	Family family(3, {"1,2,StrictlyLower", "3,1,Unrestricted", "2,3,Unrestricted"});
	CHECK(family.result, FT_OK);
	const FT_Relation* relations = nullptr;
	size_t count = 0;
	CHECK(FT_GetRelations(family.context.Get(), 1, &relations, &count, nullptr), FT_OK);
	CHECK(count, 2U);
	for (size_t i = 0; i < count; ++i) {
		CHECK(relations[i].first == 1 || relations[i].second == 1, true);
	}
	return true;
}

// docs/decisions/adr_0017.md
bool
ARelationToAnUnknownPersonIsLeftOutWithAWarning() {
	Family family(2, {"1,2,StrictlyLower", "1,99,StrictlyLower"});
	CHECK(family.result, FT_OK_WITH_WARNINGS);
	CHECK(family.Children(1), Ids({2}));
	CHECK(family.trace.HasWarning("99"), true);
	return true;
}

// docs/requirements/req_0021.md, docs/requirements/req_0045.md
bool
AChildIsOneGenerationBelowItsParent() {
	Family family(3, {"1,2,StrictlyLower", "2,3,StrictlyLower"});
	CHECK(family.result, FT_OK);
	CHECK(family.Generation(2, 1), 1);
	CHECK(family.Generation(3, 1), 2);
	CHECK(family.Generation(1, 3), -2);
	CHECK(family.Generation(2, 2), 0);
	return true;
}

// docs/requirements/req_0045.md
bool
APersonWhoMarriedInIsInTheGenerationOfThePartner() {
	// 5 and 6 are the Parents of 1; 1 and 2 are the Parents of 3; 2 has no recorded Parents.
	Family family(
		6,
		{"5,1,StrictlyLower", "6,1,StrictlyLower", "1,3,StrictlyLower", "2,3,StrictlyLower"});
	CHECK(family.result, FT_OK);
	CHECK(family.Generation(2, 1), 0);
	CHECK(family.Generation(2, 5), 1);
	return true;
}

// docs/requirements/req_0045.md
bool
AllParentsOfAChildAreInTheSameGeneration() {
	// 1 is the Parent of 2, who is the Parent of 3; 4 has no recorded Parents; 3 and 4 are the
	// Parents of 5. 4 has to move down to the Generation of 3.
	Family family(
		5,
		{"1,2,StrictlyLower", "2,3,StrictlyLower", "3,5,StrictlyLower", "4,5,StrictlyLower"});
	CHECK(family.result, FT_OK);
	CHECK(family.Generation(4, 3), 0);
	CHECK(family.Generation(4, 1), 2);
	CHECK(family.Generation(5, 4), 1);
	return true;
}

// docs/requirements/req_0045.md
bool
EveryPersonIsInTheHighestGenerationTheRulesAllow() {
	// 1 is the Parent of 2 and 3; 2 is the Parent of 4. 3 has no Children and stays one
	// Generation below 1, not lower.
	Family family(4, {"1,2,StrictlyLower", "1,3,StrictlyLower", "2,4,StrictlyLower"});
	CHECK(family.result, FT_OK);
	CHECK(family.Generation(3, 1), 1);
	CHECK(family.Generation(3, 2), 0);
	return true;
}

// docs/requirements/req_0021.md, docs/requirements/req_0045.md
bool
GenerationsOfSeparateFamilyGraphsAreCountedFromTheirOwnTop() {
	// 1 is the Parent of 2; 3 is the Parent of 4, who is the Parent of 5; no Relation connects
	// the two Family Graphs.
	Family family(5, {"1,2,StrictlyLower", "3,4,StrictlyLower", "4,5,StrictlyLower"});
	CHECK(family.result, FT_OK);
	CHECK(family.Generation(3, 1), 0);
	CHECK(family.Generation(5, 2), 1);
	CHECK(family.Generation(1, 5), -2);
	return true;
}

// docs/requirements/req_0045.md
bool
AnUnrestrictedRelationDoesNotMoveGenerations() {
	Family family(3, {"1,2,StrictlyLower", "3,2,Unrestricted"});
	CHECK(family.result, FT_OK);
	CHECK(family.Generation(3, 1), 0);
	return true;
}

// docs/requirements/req_0047.md
bool
APersonWhoIsTheirOwnAncestorPreventsTheContext() {
	Family family(3, {"1,2,StrictlyLower", "2,3,StrictlyLower", "3,1,StrictlyLower"});
	CHECK(family.result, FT_ERROR_CONFLICTING_GENERATIONS);
	CHECK(static_cast<bool>(family.context), false);
	CHECK(family.trace.HasError(), true);
	return true;
}

// docs/requirements/req_0047.md
bool
AParentWhoIsAnAncestorOfTheOtherParentPreventsTheContext() {
	// 1 and 2 are the Parents of 3; 2 is the Parent of 4; 4 is the Parent of 1.
	Family family(
		4,
		{"1,3,StrictlyLower", "2,3,StrictlyLower", "2,4,StrictlyLower", "4,1,StrictlyLower"});
	CHECK(family.result, FT_ERROR_CONFLICTING_GENERATIONS);
	CHECK(static_cast<bool>(family.context), false);
	return true;
}

// docs/requirements/req_0047.md
bool
TheConflictingRelationsAreReportedToTheTrace() {
	Family family(2, {"1,2,StrictlyLower", "2,1,StrictlyLower"});
	CHECK(family.result, FT_ERROR_CONFLICTING_GENERATIONS);
	CHECK(family.trace.HasError("1"), true);
	CHECK(family.trace.HasError("2"), true);
	return true;
}

} // namespace

bool
ResolverSuite() {
	auto result = SUITE("Resolver");
	result &= RUN(ChildrenComeFromStrictlyLowerRelations);
	result &= RUN(ParentsComeFromStrictlyHigherRelations);
	result &= RUN(UnrestrictedRelationsMakeNoParentsOrChildren);
	result &= RUN(CommonParentsOfHalfSiblings);
	result &= RUN(CommonChildrenOfTwoParents);
	result &= RUN(AnEmptySetHasNoCommonParentsOrChildren);
	result &= RUN(SiblingsShareAtLeastOneParent);
	result &= RUN(APersonWithoutParentsHasNoSiblings);
	result &= RUN(PartnersShareAtLeastOneChild);
	result &= RUN(AnUnrestrictedRelationAloneMakesNoPartners);
	result &= RUN(RelationsOfAPersonContainItAsFirstOrSecond);
	result &= RUN(ARelationToAnUnknownPersonIsLeftOutWithAWarning);
	result &= RUN(AChildIsOneGenerationBelowItsParent);
	result &= RUN(APersonWhoMarriedInIsInTheGenerationOfThePartner);
	result &= RUN(AllParentsOfAChildAreInTheSameGeneration);
	result &= RUN(EveryPersonIsInTheHighestGenerationTheRulesAllow);
	result &= RUN(GenerationsOfSeparateFamilyGraphsAreCountedFromTheirOwnTop);
	result &= RUN(AnUnrestrictedRelationDoesNotMoveGenerations);
	result &= RUN(APersonWhoIsTheirOwnAncestorPreventsTheContext);
	result &= RUN(AParentWhoIsAnAncestorOfTheOtherParentPreventsTheContext);
	result &= RUN(TheConflictingRelationsAreReportedToTheTrace);
	return result;
}
