#ifndef KeyEvaluationStats_h
#define KeyEvaluationStats_h

//============================================================================
/**
	Class representing .

	@author		Johan Pauwels
	@date		20101008
*/
//============================================================================
#include <Eigen/Core>

class KeyEvaluationStats
{
public:

	/** Default constructor. */
	KeyEvaluationStats(const Eigen::ArrayXXd& inConfusionMatrix, const size_t inNumOfChromas = 12);

	/** Destructor. */
	virtual ~KeyEvaluationStats();

	const double getCorrectKeys() const;
	const double getCorrectNoKeys() const;
	const double getKeyDeletions() const;
	const double getKeyInsertions() const;

	const double getAdjacentKeys() const;
	const double getRelativeKeys() const;
	const double getParallelKeys() const;

	const size_t getNumOfUniquesInRef() const;
	const size_t getNumOfUniquesInTest() const;

protected:

	Eigen::ArrayXXd m_ConfusionMatrix;
	Eigen::ArrayXXd::Index m_NumOfChromas;
	Eigen::ArrayXXd::Index m_NumOfModes;
	Eigen::ArrayXXd::Index m_NumOfKeys;
	Eigen::Block<Eigen::ArrayXXd> m_KeysMatrix;

private:


};

#endif	// #ifndef KeyEvaluationStats_h
