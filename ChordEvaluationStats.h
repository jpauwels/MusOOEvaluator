#ifndef ChordEvaluationStats_h
#define ChordEvaluationStats_h

//============================================================================
/**
	Class representing .

	@author		Johan Pauwels
	@date		20100921
*/
//============================================================================
#include <vector>
#include <Eigen/Core>
#include "MusOO/Chord.h"

class ChordEvaluationStats
{
public:

	/** Default constructor. */
	ChordEvaluationStats(const Eigen::ArrayXXd& inConfusionMatrix, const std::vector<MusOO::Chord>& inChords, const Eigen::Index inNumOfChromas = 12);

	/** Destructor. */
	virtual ~ChordEvaluationStats();

	const double getCorrectChords() const;
	const double getCorrectNoChords() const;
	const double getChordDeletions() const;
	const double getChordInsertions() const;
	const double getChordSubstitutions() const;

	const double getOnlyRootCorrect() const;
	const double getOnlyTypeCorrect() const;
	const double getBothRootAndTypeWrong() const;

	const Eigen::Index getNumOfUniquesInRef() const;
	const Eigen::Index getNumOfUniquesInTest() const;

	const double getRefChordsDuration() const;
	const double getRefNoChordsDuration() const;
    
    const Eigen::ArrayXXd getCorrectChordsPerType() const;
    const double getChordsWithNWrong(const Eigen::Index inNumOfWrongChromas) const;
    const double getChordsWithSDI(const Eigen::Index inNumOfSubstitutedChromas, const Eigen::Index inNumOfDeletedChromas, const Eigen::Index inNumOfInsertedChromas) const;
    const double getChordsWithUnknownWrong() const;

protected:


private:

	const Eigen::ArrayXXd m_ConfusionMatrix;
	const Eigen::Index m_NumOfChromas;
	const Eigen::Index m_NumOfChordTypes;
	const Eigen::Index m_NumOfChords;

	Eigen::ArrayXd m_OnlyRoots;
	Eigen::ArrayXd m_OnlyTypes;
    const Eigen::ArrayXXd m_ChordsMatrix;
    
    const bool m_HasTestCatchAllChords;
    const bool m_HasRefNoChord;
    
    Eigen::Array<Eigen::Index, Eigen::Dynamic, Eigen::Dynamic> m_CardinalityDiff;
    Eigen::Array<Eigen::Index, Eigen::Dynamic, Eigen::Dynamic> m_NumOfWrongChromas;
};

#endif	// #ifndef ChordEvaluationStats_h
