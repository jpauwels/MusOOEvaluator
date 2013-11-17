#ifndef SimilarityScoreNote_h
#define SimilarityScoreNote_h

//============================================================================
/**
	Class representing .

	@author		Johan Pauwels
	@date		20101206
*/
//============================================================================
#include <Eigen/Core>
#include "SimilarityScore.h"
#include "MusOO/NoteMidi.h"

class SimilarityScoreNote : public SimilarityScore<MusOO::Note>
{
public:

	/** Default constructor. */
	SimilarityScoreNote(const std::string& inScoreSelect);

	/** Destructor. */
	virtual ~SimilarityScoreNote();

	virtual const double score(const MusOO::Note& inRef, const MusOO::Note& inTest);

protected:


private:

	MusOO::NoteMidi m_LowestNote;


};

#endif	// #ifndef SimilarityScoreNote_h
