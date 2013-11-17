//============================================================================
/**
	Implementation file for SimilarityScoreChord.h
	
	@author		Johan Pauwels
	@date		20100915
*/
//============================================================================

// Includes
#include <vector>
	using std::vector;
#include <algorithm>
#include <stdexcept>
	using std::runtime_error;
	using std::invalid_argument;
#include "SimilarityScoreChord.h"
#include "MusOO/ChordQM.h"

using std::string;
using std::set;

using namespace MusOO;

SimilarityScoreChord::SimilarityScoreChord(const std::string& inPreset)
{
	if (!inPreset.compare("Mirex2009"))
	{
		initialize("mirex09", set<ChordType>(), set<ChordType>(), "exact");
	}
	else if (!inPreset.compare("4TriadsInput"))
	{
		set<ChordType> theInputLimitingSet;
		insertInversions(theInputLimitingSet, ChordType::major());
        insertInversions(theInputLimitingSet, ChordType::minor());
        insertInversions(theInputLimitingSet, ChordType::diminished());
        insertInversions(theInputLimitingSet, ChordType::augmented());
		initialize("triads", theInputLimitingSet, set<ChordType>(), "exact");
	}
	else if (!inPreset.compare("4TriadsOutput"))
	{
		set<ChordType> theOutputLimitingSet;
		theOutputLimitingSet.insert(ChordType::major());
		theOutputLimitingSet.insert(ChordType::minor());
		theOutputLimitingSet.insert(ChordType::diminished());
		theOutputLimitingSet.insert(ChordType::augmented());
        theOutputLimitingSet.insert(ChordType::none());
		initialize("triads", set<ChordType>(), theOutputLimitingSet, "exact");
	}
	else if (!inPreset.compare("6TriadsInput"))
	{
		set<ChordType> theInputLimitingSet;
        insertInversions(theInputLimitingSet, ChordType::major());
        insertInversions(theInputLimitingSet, ChordType::minor());
        insertInversions(theInputLimitingSet, ChordType::diminished());
        insertInversions(theInputLimitingSet, ChordType::augmented());
        insertInversions(theInputLimitingSet, ChordType::suspendedSecond());
        insertInversions(theInputLimitingSet, ChordType::suspendedFourth());
		initialize("triads", theInputLimitingSet, set<ChordType>(), "exact");
	}
	else if (!inPreset.compare("6TriadsOutput"))
	{
		set<ChordType> theOutputLimitingSet;
		theOutputLimitingSet.insert(ChordType::major());
		theOutputLimitingSet.insert(ChordType::minor());
		theOutputLimitingSet.insert(ChordType::diminished());
		theOutputLimitingSet.insert(ChordType::augmented());
		theOutputLimitingSet.insert(ChordType::suspendedSecond());
		theOutputLimitingSet.insert(ChordType::suspendedFourth());
        theOutputLimitingSet.insert(ChordType::none());
		initialize("triads", set<ChordType>(), theOutputLimitingSet, "exact");
	}
    else if (!inPreset.compare("Triads"))
    {
        initialize("triads", set<ChordType>(), set<ChordType>(), "exact");
    }
    else if (!inPreset.compare("Tetrads"))
    {
        initialize("tetrads", set<ChordType>(), set<ChordType>(), "exact");
    }
    else if (!inPreset.compare("Mirex2010"))
    {
        initialize("none", set<ChordType>(), set<ChordType>(), "mirex2010");
    }
    else if (!inPreset.compare("ChromaRecall"))
    {
        initialize("none", set<ChordType>(), set<ChordType>(), "chromarecall");
    }
    else if (!inPreset.compare("ChromaPrecision"))
    {
        initialize("none", set<ChordType>(), set<ChordType>(), "chromaprecision");
    }
    else if (!inPreset.compare("ChromaFmeasure"))
    {
        initialize("none", set<ChordType>(), set<ChordType>(), "chromafmeasure");
    }
    else if (!inPreset.compare("Bass"))
    {
        initialize("bass", set<ChordType>(), set<ChordType>(), "exact");
    }
    else if (!inPreset.compare("Root") || !inPreset.compare("MirexRoot"))
    {
        initialize("root", set<ChordType>(), set<ChordType>(), "exact");
    }
	else if (!inPreset.compare("TetradsOnly"))
	{
		set<ChordType> theOutputLimitingSet;
        theOutputLimitingSet.insert(ChordType::augmentedSeventh());
        theOutputLimitingSet.insert(ChordType::augmentedMajorSeventh());
        theOutputLimitingSet.insert(ChordType::majorFlatFifth().addInterval(Interval::minorSeventh()));
        theOutputLimitingSet.insert(ChordType::majorFlatFifth().addInterval(Interval::majorSeventh()));
        theOutputLimitingSet.insert(ChordType::dominantSeventh());
        theOutputLimitingSet.insert(ChordType::majorSeventh());
        theOutputLimitingSet.insert(ChordType::majorSixth());
        theOutputLimitingSet.insert(ChordType::halfDiminished());
        theOutputLimitingSet.insert(ChordType::diminishedSeventh());
        theOutputLimitingSet.insert(ChordType::minorSharpFifth().addInterval(Interval::minorSeventh()));
        theOutputLimitingSet.insert(ChordType::minorSharpFifth().addInterval(Interval::majorSeventh()));
        theOutputLimitingSet.insert(ChordType::minorSeventh());
        theOutputLimitingSet.insert(ChordType::minorMajorSeventh());
        theOutputLimitingSet.insert(ChordType::minorSixth());
        theOutputLimitingSet.insert(ChordType::suspendedFourthSeventh());
        theOutputLimitingSet.insert(ChordType::suspendedFourth().addInterval(Interval::majorSeventh()));
        theOutputLimitingSet.insert(ChordType::suspendedFourth().addInterval(Interval::majorSixth()));
        theOutputLimitingSet.insert(ChordType::suspendedSecond().addInterval(Interval::minorSeventh()));
        theOutputLimitingSet.insert(ChordType::suspendedSecond().addInterval(Interval::majorSeventh()));
        theOutputLimitingSet.insert(ChordType::suspendedSecond().addInterval(Interval::majorSixth()));
		initialize("tetrads", set<ChordType>(), theOutputLimitingSet, "exact");
	}
	else if (!inPreset.compare("TriadsInput"))
	{
		set<ChordType> theInputLimitingSet;
        insertInversions(theInputLimitingSet, ChordType::major());
        insertInversions(theInputLimitingSet, ChordType::minor());
        insertInversions(theInputLimitingSet, ChordType::diminished());
        insertInversions(theInputLimitingSet, ChordType::augmented());
        insertInversions(theInputLimitingSet, ChordType::suspendedSecond());
        insertInversions(theInputLimitingSet, ChordType::suspendedFourth());
		initialize("triads", theInputLimitingSet, set<ChordType>(), "exact");
	}
    else if (!inPreset.compare("MirexMajMin"))
	{
		set<ChordType> theOutputLimitingSet;
        theOutputLimitingSet.insert(ChordType::major());
        theOutputLimitingSet.insert(ChordType::minor());
        theOutputLimitingSet.insert(ChordType::none());
		initialize("triads", set<ChordType>(), theOutputLimitingSet, "exact");
	}
    else if (!inPreset.compare("MirexMajMinBass"))
	{
		set<ChordType> theOutputLimitingSet;
        insertInversions(theOutputLimitingSet, ChordType::major());
        insertInversions(theOutputLimitingSet, ChordType::minor());
        theOutputLimitingSet.insert(ChordType::none());
		initialize("triadsWithBass", set<ChordType>(), theOutputLimitingSet, "exact");
	}
    else if (!inPreset.compare("MirexSevenths"))
	{
        set<ChordType> theOutputLimitingSet;
        theOutputLimitingSet.insert(ChordType::major());
        theOutputLimitingSet.insert(ChordType::minor());
        theOutputLimitingSet.insert(ChordType::dominantSeventh());
        theOutputLimitingSet.insert(ChordType::majorSeventh());
        theOutputLimitingSet.insert(ChordType::minorSeventh());
        theOutputLimitingSet.insert(ChordType::none());
		initialize("tetrads", set<ChordType>(), theOutputLimitingSet, "exact");
	}
    else if (!inPreset.compare("MirexSeventhsBass"))
	{
        set<ChordType> theOutputLimitingSet;
        insertInversions(theOutputLimitingSet, ChordType::major());
        insertInversions(theOutputLimitingSet, ChordType::minor());
        insertInversions(theOutputLimitingSet, ChordType::dominantSeventh());
        insertInversions(theOutputLimitingSet, ChordType::majorSeventh());
        insertInversions(theOutputLimitingSet, ChordType::minorSeventh());
        theOutputLimitingSet.insert(ChordType::none());
		initialize("tetradsWithBass", set<ChordType>(), theOutputLimitingSet, "exact");
	}
    else
	{
		throw runtime_error("Unknown score preset: " + inPreset);
	}
}

void SimilarityScoreChord::initialize(const std::string& inMapping, const std::set<ChordType> inInputLimitingSet, const std::set<ChordType> inOutputLimitingSet, const std::string& inScoring)
{
	m_Mapping = inMapping;
    m_Scoring = inScoring;
	m_InputLimitingSet = inInputLimitingSet;
	
	// Mapped types as used in confusion matrix are determined by mapping
    if (!m_Mapping.compare("mirex09"))
    {
        m_MappedTypes.insert(ChordType::major());
        m_MappedTypes.insert(ChordType::minor());
    }
    else if (!m_Mapping.compare("triads"))
    {
        m_MappedTypes.insert(ChordType::major());
        m_MappedTypes.insert(ChordType::minor());
        m_MappedTypes.insert(ChordType::diminished());
        m_MappedTypes.insert(ChordType::augmented());
        m_MappedTypes.insert(ChordType::suspendedSecond());
        m_MappedTypes.insert(ChordType::suspendedFourth());
        m_MappedTypes.insert(ChordType::majorFlatFifth());
        m_MappedTypes.insert(ChordType::minorSharpFifth());
    }
    else if (!m_Mapping.compare("triadsWithBass"))
    {
        insertInversions(m_MappedTypes, ChordType::major());
        insertInversions(m_MappedTypes, ChordType::minor());
        insertInversions(m_MappedTypes, ChordType::diminished());
        insertInversions(m_MappedTypes, ChordType::augmented());
        insertInversions(m_MappedTypes, ChordType::suspendedSecond());
        insertInversions(m_MappedTypes, ChordType::suspendedFourth());
        insertInversions(m_MappedTypes, ChordType::majorFlatFifth());
        insertInversions(m_MappedTypes, ChordType::minorSharpFifth());
    }
    else if (!m_Mapping.compare("tetrads"))
    {
        m_MappedTypes.insert(ChordType::major());
        m_MappedTypes.insert(ChordType::minor());
        m_MappedTypes.insert(ChordType::diminished());
        m_MappedTypes.insert(ChordType::augmented());
        m_MappedTypes.insert(ChordType::suspendedSecond());
        m_MappedTypes.insert(ChordType::suspendedFourth());
        m_MappedTypes.insert(ChordType::majorFlatFifth());
        m_MappedTypes.insert(ChordType::minorSharpFifth());
        m_MappedTypes.insert(ChordType::augmentedSeventh());
        m_MappedTypes.insert(ChordType::augmentedMajorSeventh());
        m_MappedTypes.insert(ChordType::majorFlatFifth().addInterval(Interval::minorSeventh()));
        m_MappedTypes.insert(ChordType::majorFlatFifth().addInterval(Interval::majorSeventh()));
        m_MappedTypes.insert(ChordType::dominantSeventh());
        m_MappedTypes.insert(ChordType::majorSeventh());
        m_MappedTypes.insert(ChordType::majorSixth());
        m_MappedTypes.insert(ChordType::halfDiminished());
        m_MappedTypes.insert(ChordType::diminishedSeventh());
        m_MappedTypes.insert(ChordType::minorSharpFifth().addInterval(Interval::minorSeventh()));
        m_MappedTypes.insert(ChordType::minorSharpFifth().addInterval(Interval::majorSeventh()));
        m_MappedTypes.insert(ChordType::minorSeventh());
        m_MappedTypes.insert(ChordType::minorMajorSeventh());
        m_MappedTypes.insert(ChordType::minorSixth());
        m_MappedTypes.insert(ChordType::suspendedFourthSeventh());
        m_MappedTypes.insert(ChordType::suspendedFourth().addInterval(Interval::majorSeventh()));
        m_MappedTypes.insert(ChordType::suspendedFourth().addInterval(Interval::majorSixth()));
        m_MappedTypes.insert(ChordType::suspendedSecond().addInterval(Interval::minorSeventh()));
        m_MappedTypes.insert(ChordType::suspendedSecond().addInterval(Interval::majorSeventh()));
        m_MappedTypes.insert(ChordType::suspendedSecond().addInterval(Interval::majorSixth()));
    }
    else if (!m_Mapping.compare("tetradsWithBass"))
    {
        insertInversions(m_MappedTypes, ChordType::major());
        insertInversions(m_MappedTypes, ChordType::minor());
        insertInversions(m_MappedTypes, ChordType::diminished());
        insertInversions(m_MappedTypes, ChordType::augmented());
        insertInversions(m_MappedTypes, ChordType::suspendedSecond());
        insertInversions(m_MappedTypes, ChordType::suspendedFourth());
        insertInversions(m_MappedTypes, ChordType::majorFlatFifth());
        insertInversions(m_MappedTypes, ChordType::minorSharpFifth());
        insertInversions(m_MappedTypes, ChordType::augmentedSeventh());
        insertInversions(m_MappedTypes, ChordType::augmentedMajorSeventh());
        insertInversions(m_MappedTypes, ChordType::majorFlatFifth().addInterval(Interval::minorSeventh()));
        insertInversions(m_MappedTypes, ChordType::majorFlatFifth().addInterval(Interval::majorSeventh()));
        insertInversions(m_MappedTypes, ChordType::dominantSeventh());
        insertInversions(m_MappedTypes, ChordType::majorSeventh());
        insertInversions(m_MappedTypes, ChordType::majorSixth());
        insertInversions(m_MappedTypes, ChordType::halfDiminished());
        insertInversions(m_MappedTypes, ChordType::diminishedSeventh());
        insertInversions(m_MappedTypes, ChordType::minorSharpFifth().addInterval(Interval::minorSeventh()));
        insertInversions(m_MappedTypes, ChordType::minorSharpFifth().addInterval(Interval::majorSeventh()));
        insertInversions(m_MappedTypes, ChordType::minorSeventh());
        insertInversions(m_MappedTypes, ChordType::minorMajorSeventh());
        insertInversions(m_MappedTypes, ChordType::minorSixth());
        insertInversions(m_MappedTypes, ChordType::suspendedFourthSeventh());
        insertInversions(m_MappedTypes, ChordType::suspendedFourth().addInterval(Interval::majorSeventh()));
        insertInversions(m_MappedTypes, ChordType::suspendedFourth().addInterval(Interval::majorSixth()));
        insertInversions(m_MappedTypes, ChordType::suspendedSecond().addInterval(Interval::minorSeventh()));
        insertInversions(m_MappedTypes, ChordType::suspendedSecond().addInterval(Interval::majorSeventh()));
        insertInversions(m_MappedTypes, ChordType::suspendedSecond().addInterval(Interval::majorSixth()));
    }
    else if (m_Mapping == "none" || m_Mapping == "bass" || m_Mapping == "root")
    {
        m_MappedTypes.insert(ChordType::rootOnly());
    }
    else
    {
        throw runtime_error("Unknown mapping: " + m_Mapping);
    }

	// If output limiting set specified, take intersection of output limiting and mapped set
	if (!inOutputLimitingSet.empty())
	{
        std::set_intersection(m_MappedTypes.begin(), m_MappedTypes.end(), inOutputLimitingSet.begin(), inOutputLimitingSet.end(),
                              std::inserter(m_OutputLimitingSet,m_OutputLimitingSet.begin()));
        if (inOutputLimitingSet.count(ChordType::none()))
        {
            m_OutputLimitingSet.insert(ChordType::none());
        }
	}
	m_NumOfMappedTypes = m_MappedTypes.size();
	this->m_NumOfCategories = 12*m_NumOfMappedTypes;
    const vector<Chroma> theChromas = Chroma::circleOfFifths(s_firstChroma);
    if (m_Mapping != "bass")
    {
        for (vector<Chroma>::const_iterator theChromaIt = theChromas.begin(); theChromaIt != theChromas.end(); ++theChromaIt)
        {
            for (set<ChordType>::const_iterator theTypeIt = m_MappedTypes.begin(); theTypeIt != m_MappedTypes.end(); ++theTypeIt)
            {
                this->m_CategoryLabels.push_back(ChordQM(Chord(*theChromaIt,*theTypeIt)).str());
            }
        }
    }
    else
    {
        for (vector<Chroma>::const_iterator theChromaIt = theChromas.begin(); theChromaIt != theChromas.end(); ++theChromaIt)
        {
            this->m_CategoryLabels.push_back(theChromaIt->str());
        }
    }
    ++this->m_NumOfCategories;
    this->m_CategoryLabels.push_back(ChordQM("N").str());
    // this->m_CategoryLabels.push_back(ChordQM(ChordQM::undefined()).str());
}

SimilarityScoreChord::~SimilarityScoreChord()
{
	// Nothing to do...
}

const double SimilarityScoreChord::score(const Chord& inRefChord, const Chord& inTestChord)
{
    this->m_RefCategory = calcChordCategory(inRefChord, this->m_MappedRefLabel);
    this->m_TestCategory = calcChordCategory(inTestChord, this->m_MappedTestLabel);
    
    // Check input limiting set
    if (m_InputLimitingSet.empty() || m_InputLimitingSet.count(inRefChord.type()) > 0)
    {
        // Check for unmappable chords in test sequence
        if (this->m_TestCategory == this->m_NumOfCategories)
        {
            throw invalid_argument("The chord " + ChordQM(inTestChord).str() + " in the test sequence cannot be handled by the current evaluating rules");
        }
        
        // Check for unmappabe chords in reference sequence
        if (this->m_RefCategory != this->m_NumOfCategories)
        {
            // Check output limiting set
            if (m_OutputLimitingSet.empty() || m_OutputLimitingSet.count(this->m_MappedRefLabel.type()) > 0)
            {
                if (m_Scoring == "exact")
                {
                    if (this->m_RefCategory == this->m_TestCategory)
                    {
                        return 1.;
                    }
                    else
                    {
                        return 0.;
                    }
                }
                else if (m_Scoring == "mirex2010")
                {
                    size_t theNumOfCommonChromas = inRefChord.commonChromas(inTestChord).size();
                    if (theNumOfCommonChromas > 2 ||
                        (!inRefChord.isTrueChord() && !inTestChord.isTrueChord()) ||
                        (theNumOfCommonChromas > 1 && (inRefChord.type().triad(false) == ChordType::diminished() || inRefChord.type().triad(false) == ChordType::augmented()))/* || inRefChord.type().cardinality() == theNumOfCommonChromas*/)
                    {
                        return 1.;
                    }
                    else
                    {
                        return 0.;
                    }
                }
                else if (m_Scoring == "chromarecall")
                {
                    if (inRefChord.isTrueChord())
                    {
                        double theNumOfCommonChromas = inRefChord.commonChromas(inTestChord).size();
                        return theNumOfCommonChromas / inRefChord.type().cardinality();
                    }
                    else
                    {
                        if (!inTestChord.isTrueChord())
                        {
                            return 1.;
                        }
                        else
                        {
                            return 0.;
                        }
                    }
                }
                else if (m_Scoring == "chromaprecision")
                {
                    if (inTestChord.isTrueChord())
                    {
                        double theNumOfCommonChromas = inRefChord.commonChromas(inTestChord).size();
                        return theNumOfCommonChromas / inTestChord.type().cardinality();        }
                    else
                    {
                        if (!inRefChord.isTrueChord())
                        {
                            return 1.;
                        }
                        else
                        {
                            return 0.;
                        }
                    }
                }
                else if (m_Scoring == "chromafmeasure")
                {
                    if (inRefChord.isTrueChord() && inTestChord.isTrueChord())
                    {
                        double theNumOfCommonChromas = inRefChord.commonChromas(inTestChord).size();
                        return 2 * theNumOfCommonChromas / (inRefChord.type().cardinality() + inTestChord.type().cardinality());        }
                    else
                    {
                        if (!inRefChord.isTrueChord() && !inTestChord.isTrueChord())
                        {
                            return 1.;
                        }
                        else
                        {
                            return 0.;
                        }
                    }
                }
                else
                {
                    throw runtime_error("Unknown scoring function: " + m_Scoring);
                }
            }
        }
    }
    return -1.;
}

const size_t SimilarityScoreChord::calcChordCategory(const MusOO::Chord& inChord, MusOO::Chord& outMappedChord) const
{
	if (inChord == Chord::silence() || inChord == Chord::none())
	{
        outMappedChord = Chord::none();
		return 12*m_NumOfMappedTypes;
	}
    else if (m_Mapping == "bass")
    {
        outMappedChord = Chord(inChord.bass(true), ChordType::rootOnly());
        return Interval(s_firstChroma, inChord.bass(true)).circleStepsCW();
    }
	else
	{
        const ChordType theMappedChordType = calcMappedChordType(inChord.type());
        outMappedChord = Chord(inChord.root(), theMappedChordType);
        set<ChordType>::const_iterator theTypeIt = m_MappedTypes.find(theMappedChordType);
		// Check whether the triad belongs to the given dictionary
		if (theTypeIt != m_MappedTypes.end())
		{
            int theRootIndex = Interval(s_firstChroma, inChord.root()).circleStepsCW();
			return theRootIndex * m_NumOfMappedTypes + distance(m_MappedTypes.begin(), theTypeIt);
		}
		else
		{
			return this->m_NumOfCategories;
		}
	}
}

const ChordType SimilarityScoreChord::calcMappedChordType(const ChordType& inChordType) const
{
    //mapping
    if (!m_Mapping.compare("mirex09"))
    {
        return mirexMapping(inChordType);
    }
    else if (!m_Mapping.compare("triads"))
    {
        return inChordType.triad(false);
    }
    else if (!m_Mapping.compare("triadsWithBass"))
    {
        return inChordType.triad(true);
    }
    else if (!m_Mapping.compare("tetrads"))
    {
        return inChordType.tetrad(false);
    }
    else if (!m_Mapping.compare("tetradsWithBass"))
    {
        return inChordType.tetrad(true);
    }
    else if (m_Mapping == "none" || m_Mapping == "root" || m_Mapping == "rootWithBass")
    {
        return ChordType::rootOnly();
    }
    else
    {
        throw runtime_error("Unknown mapping: " + m_Mapping);
    }
}

const ChordType SimilarityScoreChord::mirexMapping(const ChordType& inChordType) const
{
	ChordType theTriadType = inChordType.triad(false);
	if (theTriadType == ChordType::major() || theTriadType == ChordType::augmented() || theTriadType == ChordType::suspendedSecond() || theTriadType == ChordType::suspendedFourth() || theTriadType == ChordType::majorFlatFifth())
	{
		return ChordType::major();
	}
	else if (theTriadType == ChordType::minor() || theTriadType == ChordType::diminished() || theTriadType == ChordType::minorSharpFifth())
	{
		return ChordType::minor();
	}
	else
	{
		return ChordType::rootOnly();
	}
}

void SimilarityScoreChord::insertInversions(std::set<ChordType>& inSet, const ChordType& inChordType) const
{
    vector<ChordType> theInversions = inChordType.inversions();
    inSet.insert(theInversions.begin(), theInversions.end());
}
