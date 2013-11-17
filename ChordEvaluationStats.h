#ifndef ChordEvaluationStats_h
#define ChordEvaluationStats_h

//============================================================================
/**
	Class representing .

	@author		Johan Pauwels
	@date		20100921
*/
//============================================================================
#include <Eigen/Core>

class ChordEvaluationStats
{
public:

	/** Default constructor. */
	ChordEvaluationStats(const Eigen::ArrayXXd inConfusionMatrix, const size_t inNumOfChromas = 12);

	/** Destructor. */
	virtual ~ChordEvaluationStats();

	const double getCorrectChords() const;
	const double getCorrectNoChords() const;
	const double getChordDeletions() const;
	const double getChordInsertions() const;

	const double getOnlyRootCorrect() const;
	const double getOnlyTypeCorrect() const;
	const double getAllWrong() const;

	const size_t getNumOfUniquesInRef() const;
	const size_t getNumOfUniquesInTest() const;

	const double getRefChordsDuration() const;
	const double getRefNoChordsDuration() const;

protected:


private:

	Eigen::ArrayXXd m_ConfusionMatrix;
	size_t m_NumOfChromas;
	size_t m_NumOfChordTypes;
	size_t m_NumOfChords;

	Eigen::ArrayXd m_OnlyRoots;
	Eigen::ArrayXd m_OnlyTypes;
	double m_AllWrong;
	double m_CorrectChords;
	Eigen::ArrayXXd m_ChordsMatrix;

};

#endif	// #ifndef ChordEvaluationStats_h
