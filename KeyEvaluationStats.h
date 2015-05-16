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
	KeyEvaluationStats(const Eigen::ArrayXXd& inConfusionMatrix, const Eigen::ArrayXXd::Index inNumOfChromas = 12);

	/** Destructor. */
	virtual ~KeyEvaluationStats();

	const double getCorrectKeys() const;
	const double getCorrectNoKeys() const;
	const double getKeyDeletions() const;
	const double getKeyInsertions() const;

	const double getAdjacentKeys() const;
	const double getRelativeKeys() const;
    const double getParallelKeys() const;
    const double getChromaticKeys() const;

	const Eigen::ArrayXXd::Index getNumOfUniquesInRef() const;
	const Eigen::ArrayXXd::Index getNumOfUniquesInTest() const;
    
    const Eigen::ArrayXXd getCorrectKeysPerMode() const;

protected:

	Eigen::ArrayXXd m_ConfusionMatrix;
	Eigen::ArrayXXd::Index m_NumOfChromas;
	Eigen::ArrayXXd::Index m_NumOfModes;
	Eigen::ArrayXXd::Index m_NumOfKeys;
	Eigen::Block<Eigen::ArrayXXd> m_KeysMatrix;

private:


};

#endif	// #ifndef KeyEvaluationStats_h
