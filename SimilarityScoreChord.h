#ifndef SimilarityScoreChord_h
#define SimilarityScoreChord_h

//============================================================================
/**
	Class representing .

	@author		Johan Pauwels
	@date		20100915
*/
//============================================================================
#include "SimilarityScore.h"
#include "MusOO/Chord.h"

class SimilarityScoreChord : public SimilarityScore<MusOO::Chord>
{
public:

	/** Default constructor. */
	SimilarityScoreChord(const std::string& inScoreSelect);

	/** Destructor. */
	virtual ~SimilarityScoreChord();

	virtual const double score(const MusOO::Chord& inRefChord, const MusOO::Chord& inTestChord);

protected:


private:
	void initialize(const std::string& inMapping, const std::set<MusOO::ChordType> inInputLimitingSet, const std::set<MusOO::ChordType> inOutputLimitingSet, const std::string& inScoring);
	const size_t calcChordIndex(const MusOO::Chord& inChord, MusOO::Chord& outMappedChord) const;
    const MusOO::ChordType calcMappedChordType(const MusOO::ChordType& inChordType) const;
	const MusOO::ChordType mirexMapping(const MusOO::ChordType& inChordType) const;
    void insertInversions(std::set<MusOO::ChordType>& inSet, const MusOO::ChordType& inChordType) const;

	std::string m_Mapping;
	std::set<MusOO::ChordType> m_InputLimitingSet;
    std::set<MusOO::ChordType> m_OutputLimitingSet;
	std::set<MusOO::ChordType> m_MappedTypes;
	size_t m_NumOfMappedTypes;
    std::string m_Scoring;
};

#endif	// #ifndef SimilarityScoreChord_h
