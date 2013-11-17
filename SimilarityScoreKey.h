#ifndef SimilarityScoreKey_h
#define SimilarityScoreKey_h

//============================================================================
/**
	Class representing .

	@author		Johan Pauwels
	@date		20100914
*/
//============================================================================
#include <Eigen/Core>
#include "SimilarityScore.h"
#include "MusOO/Key.h"

class SimilarityScoreKey : public SimilarityScore<MusOO::Key>
{
public:

	/** Default constructor. */
	SimilarityScoreKey(const std::string& inScoreSelect);

	/** Destructor. */
	virtual ~SimilarityScoreKey();

	virtual const double score(const MusOO::Key& inRefKey, const MusOO::Key& inTestKey);

	EIGEN_MAKE_ALIGNED_OPERATOR_NEW //see http://eigen.tuxfamily.org/dox/StructHavingEigenMembers.html

protected:


private:

	const size_t calcKeyCategory(const MusOO::Key& inKey);
	Eigen::Array<double, 2, 12> m_ScoreMatrix;

};

#endif	// #ifndef SimilarityScoreKey_h
